/*
 * SPDX-FileCopyrightText: 2013~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef ADDDICTDIALOG_H
#define ADDDICTDIALOG_H

#include <memory>
#include <QDialog>
#include <QMap>
#include "ui_adddictdialog.h"

namespace fcitx {

class AddDictDialog : public QDialog {
    Q_OBJECT
public:
    explicit AddDictDialog(QWidget *parent = 0);
    QMap<QString, QString> dictionary();

public Q_SLOTS:
    void browseClicked();
    void indexChanged(int);
    void validate();

private:
    std::unique_ptr<Ui::AddDictDialog> m_ui;
};

} // namespace fcitx

#endif // ADDDICTDIALOG_H
