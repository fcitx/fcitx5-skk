/*
 * SPDX-FileCopyrightText: 2013~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef DICTMODEL_H
#define DICTMODEL_H
#include <QAbstractItemModel>
#include <QFile>
#include <QSet>

namespace fcitx {

class SkkDictModel : public QAbstractListModel {
    Q_OBJECT
public:
    explicit SkkDictModel(QObject *parent = 0);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;
    bool removeRows(int row, int count,
                    const QModelIndex &parent = QModelIndex()) override;

    void load();
    void load(QFile &file);
    void defaults();
    bool save();
    void add(const QMap<QString, QString> &dict);
    bool moveDown(const QModelIndex &currentIndex);
    bool moveUp(const QModelIndex &currentIndex);

private:
    QSet<QString> m_knownKeys;
    QList<QMap<QString, QString>> m_dicts;
};

} // namespace fcitx

#endif // DICTMODEL_H
