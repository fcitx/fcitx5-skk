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

#ifndef DICTMODEL_H
#define DICTMODEL_H
#include <QAbstractItemModel>
#include <QSet>
#include <QFile>

class DictModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit DictModel(QObject* parent = 0);
    virtual ~DictModel();
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex());

    void load();
    void load(QFile& file);
    void defaults();
    bool save();
    void add(const QMap<QString, QString>& dict);
    bool moveDown(const QModelIndex& currentIndex);
    bool moveUp(const QModelIndex& currentIndex);
private:
    QSet<QString> m_requiredKeys;
    QList< QMap< QString, QString> > m_dicts;
};

#endif // DICTMODEL_H
