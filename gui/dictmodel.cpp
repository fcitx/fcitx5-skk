/***************************************************************************
 *   Copyright (C) 2013~2013 by CSSlayer                                   *
 *   wengxt@gmail.com                                                      *
 *                                                                         *
 *  This program is free software: you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, either version 3 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/

#include <QFile>
#include <QStringList>
#include <QSet>
#include <QDebug>
#include <QTemporaryFile>
#include <fcitx-config/xdg.h>

#include "dictmodel.h"

DictModel::DictModel(QObject* parent) : QAbstractListModel(parent)
{
    m_requiredKeys << "file" << "type" << "mode";
}

DictModel::~DictModel()
{

}

void DictModel::defaults()
{
    char* path = fcitx_utils_get_fcitx_path_with_filename("pkgdatadir", "skk/dictionary_list");
    QFile f(path);
    if (f.open(QIODevice::ReadOnly)) {
        load(f);
    }
}

void DictModel::load()
{
    FILE* fp = FcitxXDGGetFileWithPrefix("skk", "dictionary_list", "r", NULL);
    if (!fp) {
        return;
    }
    QFile f;
    if (!f.open(fp, QIODevice::ReadOnly)) {
        fclose(fp);
        return;
    }

    load(f);
    f.close();
    fclose(fp);
}

void DictModel::load(QFile& file)
{
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
        Q_FOREACH(const QString& item, items) {
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

bool DictModel::save()
{
    char* name = NULL;
    FcitxXDGMakeDirUser("skk");
    FcitxXDGGetFileUserWithPrefix("skk", "dictionary_list", NULL, &name);
    QString fileName = QString::fromLocal8Bit(name);
    QTemporaryFile tempFile(fileName);
    free(name);
    if (!tempFile.open()) {
        return false;
    }

    typedef QMap<QString, QString> DictType;

    Q_FOREACH(const DictType& dict, m_dicts) {
        boolean first = true;
        Q_FOREACH(const QString& key, dict.keys()) {
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

    tempFile.setAutoRemove(false);
    QFile::remove(fileName);
    if (!tempFile.rename(fileName)) {
        tempFile.remove();
        return false;
    }

    return true;
}

int DictModel::rowCount(const QModelIndex& parent) const
{
    return m_dicts.size();
}

bool DictModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (parent.isValid()) {
        return false;
    }

    if (count == 0
        || row >= m_dicts.size()
        || row + count > m_dicts.size()) {
        return false;
    }

    beginRemoveRows(parent, row, row + count - 1);
    m_dicts.erase(m_dicts.begin() + row, m_dicts.begin() + row + count);
    endRemoveRows();

    return true;
}


QVariant DictModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= m_dicts.size() || index.column() != 0) {
        return QVariant();
    }

    switch(role) {
        case Qt::DisplayRole:
            if (m_dicts[index.row()]["type"] == "file") {
                return m_dicts[index.row()]["file"];
            } else {
                return QString("%1:%2").arg(m_dicts[index.row()]["host"], m_dicts[index.row()]["port"]);
            }
    }
    return QVariant();
}

bool DictModel::moveUp(const QModelIndex& currentIndex)
{
    if (currentIndex.row() > 0
        && currentIndex.row() < m_dicts.size()) {
        beginResetModel();
        m_dicts.swap(currentIndex.row() - 1, currentIndex.row());
        endResetModel();
        return true;
    }
    return false;
}

bool DictModel::moveDown(const QModelIndex& currentIndex)
{
    if (currentIndex.row() >= 0
        && currentIndex.row() + 1 < m_dicts.size()) {
        beginResetModel();
        m_dicts.swap(currentIndex.row() + 1, currentIndex.row());
        endResetModel();
        return true;
    }

    return false;
}

void DictModel::add(const QMap< QString, QString >& dict)
{
    beginInsertRows(QModelIndex(), m_dicts.size(), m_dicts.size());
    m_dicts << dict;
    endInsertRows();
}
