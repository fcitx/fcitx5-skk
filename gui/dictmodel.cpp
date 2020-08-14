/*
 * SPDX-FileCopyrightText: 2013~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <QDebug>
#include <QFile>
#include <QSet>
#include <QStringList>
#include <QTemporaryFile>
#include <fcitx-utils/standardpath.h>

#include <fcntl.h>
#include "dictmodel.h"

namespace fcitx {

DictModel::DictModel(QObject *parent) : QAbstractListModel(parent) {
    m_requiredKeys << "file"
                   << "type"
                   << "mode";
}

void DictModel::defaults() {
    auto path =
        StandardPath::global().fcitxPath("pkgdatadir", "skk/dictionary_list");
    QFile f(path.data());
    if (f.open(QIODevice::ReadOnly)) {
        load(f);
    }
}

void DictModel::load() {
    auto file = StandardPath::global().open(StandardPath::Type::PkgData,
                                            "skk/dictionary_list", O_RDONLY);
    if (file.fd() < 0) {
        return;
    }
    QFile f;
    if (!f.open(file.fd(), QIODevice::ReadOnly)) {
        return;
    }

    load(f);
    f.close();
}

void DictModel::load(QFile &file) {
    beginResetModel();
    m_dicts.clear();

    QByteArray bytes;
    while (!(bytes = file.readLine()).isEmpty()) {
        QString line = QString::fromUtf8(bytes).trimmed();
        QStringList items = line.split(",");
        if (items.size() < m_requiredKeys.size()) {
            continue;
        }

        bool failed = false;
        QMap<QString, QString> dict;
        Q_FOREACH (const QString &item, items) {
            if (!item.contains('=')) {
                failed = true;
                break;
            }
            QString key = item.section('=', 0, 0);
            QString value = item.section('=', 1, -1);

            if (!m_requiredKeys.contains(key)) {
                continue;
            }

            dict[key] = value;
        }

        if (!failed && m_requiredKeys.size() == dict.size()) {
            m_dicts << dict;
        }
    }
    endResetModel();
}

bool DictModel::save() {
    return StandardPath::global().safeSave(
        StandardPath::Type::PkgData, "skk/dictionary_list", [this](int fd) {
            QFile tempFile;
            if (!tempFile.open(fd, QIODevice::WriteOnly)) {
                return false;
            }

            typedef QMap<QString, QString> DictType;

            Q_FOREACH (const DictType &dict, m_dicts) {
                bool first = true;
                Q_FOREACH (const QString &key, dict.keys()) {
                    if (first) {
                        first = false;
                    } else {
                        tempFile.write(",");
                    }
                    tempFile.write(key.toUtf8());
                    tempFile.write("=");
                    tempFile.write(dict[key].toUtf8());
                }
                tempFile.write("\n");
            }
            return true;
        });
}

int DictModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return m_dicts.size();
}

bool DictModel::removeRows(int row, int count, const QModelIndex &parent) {
    if (parent.isValid()) {
        return false;
    }

    if (count == 0 || row >= m_dicts.size() || row + count > m_dicts.size()) {
        return false;
    }

    beginRemoveRows(parent, row, row + count - 1);
    m_dicts.erase(m_dicts.begin() + row, m_dicts.begin() + row + count);
    endRemoveRows();

    return true;
}

QVariant DictModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= m_dicts.size() || index.column() != 0) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole:
        if (m_dicts[index.row()]["type"] == "file") {
            return m_dicts[index.row()]["file"];
        } else {
            return QString("%1:%2").arg(m_dicts[index.row()]["host"],
                                        m_dicts[index.row()]["port"]);
        }
    }
    return QVariant();
}

bool DictModel::moveUp(const QModelIndex &currentIndex) {
    if (currentIndex.row() > 0 && currentIndex.row() < m_dicts.size()) {
        beginResetModel();
        m_dicts.swapItemsAt(currentIndex.row() - 1, currentIndex.row());
        endResetModel();
        return true;
    }
    return false;
}

bool DictModel::moveDown(const QModelIndex &currentIndex) {
    if (currentIndex.row() >= 0 && currentIndex.row() + 1 < m_dicts.size()) {
        beginResetModel();
        m_dicts.swapItemsAt(currentIndex.row() + 1, currentIndex.row());
        endResetModel();
        return true;
    }

    return false;
}

void DictModel::add(const QMap<QString, QString> &dict) {
    beginInsertRows(QModelIndex(), m_dicts.size(), m_dicts.size());
    m_dicts << dict;
    endInsertRows();
}

} // namespace fcitx
