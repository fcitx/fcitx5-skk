/*
 * SPDX-FileCopyrightText: 2013~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "dictmodel.h"
#include <QAbstractListModel>
#include <QByteArray>
#include <QDebug>
#include <QFile>
#include <QObject>
#include <QSet>
#include <QStringList>
#include <QTemporaryFile>
#include <Qt>
#include <QtGlobal>
#include <fcitx-utils/standardpaths.h>

namespace fcitx {

SkkDictModel::SkkDictModel(QObject *parent) : QAbstractListModel(parent) {
    m_knownKeys << "file"
                << "host"
                << "port"
                << "type"
                << "mode"
                << "encoding";
}

void SkkDictModel::defaults() {
    auto path = StandardPaths::fcitxPath("pkgdatadir", "skk/dictionary_list");
    QFile f(path);
    if (f.open(QIODevice::ReadOnly)) {
        load(f);
    }
}

void SkkDictModel::load() {
    auto file = StandardPaths::global().open(StandardPathsType::PkgData,
                                             "skk/dictionary_list");
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

void SkkDictModel::load(QFile &file) {
    beginResetModel();
    m_dicts.clear();

    QByteArray bytes;
    while (!(bytes = file.readLine()).isEmpty()) {
        QString line = QString::fromUtf8(bytes).trimmed();
        QStringList items = line.split(",");
        // No matter which type, it should has at least 3 keys.
        if (items.size() < 3) {
            continue;
        }

        bool failed = false;
        QMap<QString, QString> dict;
        for (const QString &item : items) {
            if (!item.contains('=')) {
                failed = true;
                break;
            }
            QString key = item.section('=', 0, 0);
            QString value = item.section('=', 1, -1);

            if (!m_knownKeys.contains(key)) {
                continue;
            }

            dict[key] = value;
        }

        if (!failed && 3 <= dict.size()) {
            m_dicts << dict;
        }
    }
    endResetModel();
}

bool SkkDictModel::save() {
    return StandardPaths::global().safeSave(
        StandardPathsType::PkgData, "skk/dictionary_list", [this](int fd) {
            QFile tempFile;
            if (!tempFile.open(fd, QIODevice::WriteOnly)) {
                return false;
            }

            using DictType = QMap<QString, QString>;

            for (const DictType &dict : m_dicts) {
                bool first = true;
                for (const QString &key : dict.keys()) {
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

int SkkDictModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return m_dicts.size();
}

bool SkkDictModel::removeRows(int row, int count, const QModelIndex &parent) {
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

QVariant SkkDictModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return {};
    }

    if (index.row() >= m_dicts.size() || index.column() != 0) {
        return {};
    }

    switch (role) {
    case Qt::DisplayRole:
        if (m_dicts[index.row()]["type"] == "file") {
            return m_dicts[index.row()]["file"];
        } else {
            return QString("%1:%2").arg(m_dicts[index.row()]["host"],
                                        m_dicts[index.row()]["port"]);
        }
    default:
        break;
    }
    return {};
}

bool SkkDictModel::moveUp(const QModelIndex &currentIndex) {
    if (currentIndex.row() > 0 && currentIndex.row() < m_dicts.size()) {
        beginResetModel();
        m_dicts.swapItemsAt(currentIndex.row() - 1, currentIndex.row());
        endResetModel();
        return true;
    }
    return false;
}

bool SkkDictModel::moveDown(const QModelIndex &currentIndex) {
    if (currentIndex.row() >= 0 && currentIndex.row() + 1 < m_dicts.size()) {
        beginResetModel();
        m_dicts.swapItemsAt(currentIndex.row() + 1, currentIndex.row());
        endResetModel();
        return true;
    }

    return false;
}

void SkkDictModel::add(const QMap<QString, QString> &dict) {
    beginInsertRows(QModelIndex(), m_dicts.size(), m_dicts.size());
    m_dicts << dict;
    endInsertRows();
}

} // namespace fcitx
