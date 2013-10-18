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

#include "common.h"
#include "dictwidget.h"
#include "adddictdialog.h"
#include "dictmodel.h"
#include "rulemodel.h"
#include "ui_dictwidget.h"
#include <fcitx-config/xdg.h>

SkkDictWidget::SkkDictWidget(QWidget* parent): FcitxQtConfigUIWidget(parent)
    ,m_ui(new Ui::SkkDictWidget)
{
    m_ui->setupUi(this);
    m_dictModel = new DictModel(this);
    m_ruleModel = new RuleModel(this);
    m_ui->ruleLabel->setText(_("&Rule:"));

    m_ui->dictionaryView->setModel(m_dictModel);
    m_ui->ruleComboBox->setModel(m_ruleModel);

    connect(m_ui->addDictButton, SIGNAL(clicked(bool)), this, SLOT(addDictClicked()));
    connect(m_ui->defaultDictButton, SIGNAL(clicked(bool)), this,  SLOT(defaultDictClicked()));
    connect(m_ui->removeDictButton, SIGNAL(clicked(bool)), this, SLOT(removeDictClicked()));
    connect(m_ui->moveUpDictButton, SIGNAL(clicked(bool)), this, SLOT(moveUpDictClicked()));
    connect(m_ui->moveDownDictButton, SIGNAL(clicked(bool)), this, SLOT(moveDownClicked()));
    connect(m_ui->ruleComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(ruleChanged(int)));

    load();
}

SkkDictWidget::~SkkDictWidget()
{
    delete m_ui;
}

QString SkkDictWidget::addon()
{
    return "fcitx-skk";
}

QString SkkDictWidget::title()
{
    return _("Dictionary Manager");
}

QString SkkDictWidget::icon()
{
    return "fcitx-skk";
}

void SkkDictWidget::load()
{
    m_dictModel->load();

    FILE* fp = FcitxXDGGetFileWithPrefix("skk", "rule", "r", NULL);

    QString sline;
    do {
        if (!fp) {
            break;
        }

        QFile f;
        QByteArray line;
        if (f.open(fp, QIODevice::ReadOnly)) {;
            line = f.readLine();
            f.close();
        }
        fclose(fp);

        sline = QString::fromUtf8(line).trimmed();

        if (sline.isEmpty()) {
            sline = "default";
        }
    } while(0);
    m_ruleModel->load();
    int idx = m_ruleModel->findRule(sline);
    idx = idx < 0 ? 0 : idx;
    m_ui->ruleComboBox->setCurrentIndex(idx);
    Q_EMIT changed(false);
}

void SkkDictWidget::save()
{
    m_dictModel->save();
    QString name = m_ruleModel->data(m_ruleModel->index(m_ui->ruleComboBox->currentIndex(), 0), Qt::UserRole).toString();
    FILE* fp = FcitxXDGGetFileUserWithPrefix("skk", "rule", "w", NULL);
    if (!fp) {
        return;
    }

    QFile f;
    if (f.open(fp, QIODevice::WriteOnly)) {
        f.write(name.toUtf8());
        f.close();
    }

    fclose(fp);
    Q_EMIT changed(false);
}

void SkkDictWidget::addDictClicked()
{
    AddDictDialog dialog;
    int result = dialog.exec();
    if (result == QDialog::Accepted) {
        m_dictModel->add(dialog.dictionary());
        Q_EMIT changed(true);
    }
}

void SkkDictWidget::defaultDictClicked()
{
    m_dictModel->defaults();
    Q_EMIT changed(true);
}

void SkkDictWidget::removeDictClicked()
{
    if (m_ui->dictionaryView->currentIndex().isValid()) {
        m_dictModel->removeRow(m_ui->dictionaryView->currentIndex().row());
        Q_EMIT changed(true);
    }
}

void SkkDictWidget::moveUpDictClicked()
{
    int row = m_ui->dictionaryView->currentIndex().row();
    if (m_dictModel->moveUp(m_ui->dictionaryView->currentIndex())) {
        m_ui->dictionaryView->selectionModel()->setCurrentIndex(
            m_dictModel->index(row - 1), QItemSelectionModel::ClearAndSelect);
        Q_EMIT changed(true);
    }
}

void SkkDictWidget::moveDownClicked()
{
    int row = m_ui->dictionaryView->currentIndex().row();
    if (m_dictModel->moveDown(m_ui->dictionaryView->currentIndex())) {
        m_ui->dictionaryView->selectionModel()->setCurrentIndex(
            m_dictModel->index(row + 1), QItemSelectionModel::ClearAndSelect);
        Q_EMIT changed(true);
    }
}

void SkkDictWidget::ruleChanged(int )
{
    Q_EMIT changed(true);
}
