/*
 * SPDX-FileCopyrightText: 2013~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef FCITX_SKK_GUI_DICTWIDGET_H
#define FCITX_SKK_GUI_DICTWIDGET_H

#include <memory>
#include <fcitxqtconfiguiwidget.h>
#include "ui_dictwidget.h"

namespace fcitx {

class SkkDictModel;

class SkkDictWidget : public FcitxQtConfigUIWidget {
    Q_OBJECT
public:
    explicit SkkDictWidget(QWidget *parent = 0);

    void load() override;
    void save() override;
    QString title() override;
    QString icon() override;

private Q_SLOTS:
    void addDictClicked();
    void defaultDictClicked();
    void removeDictClicked();
    void moveUpDictClicked();
    void moveDownClicked();

private:
    std::unique_ptr<Ui::SkkDictWidget> m_ui;
    SkkDictModel *m_dictModel;
};

} // namespace fcitx

#endif // FCITX_SKK_GUI_DICTWIDGET_H
