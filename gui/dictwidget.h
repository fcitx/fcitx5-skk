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

#ifndef FCITX_SKK_GUI_DICTWIDGET_H
#define FCITX_SKK_GUI_DICTWIDGET_H

#include <fcitx-qt/fcitxqtconfiguiwidget.h>

class RuleModel;
class DictModel;
namespace Ui {
class SkkDictWidget;
}

class SkkDictWidget : public FcitxQtConfigUIWidget
{
    Q_OBJECT
public:
    explicit SkkDictWidget(QWidget* parent = 0);
    virtual ~SkkDictWidget();

    virtual void load();
    virtual void save();
    virtual QString title();
    virtual QString addon();
    virtual QString icon();

private Q_SLOTS:
    void addDictClicked();
    void defaultDictClicked();
    void removeDictClicked();
    void moveUpDictClicked();
    void moveDownClicked();
    void ruleChanged(int);
private:
    Ui::SkkDictWidget* m_ui;
    DictModel* m_dictModel;
    RuleModel* m_ruleModel;
};


#endif // FCITX_SKK_GUI_DICTWIDGET_H
