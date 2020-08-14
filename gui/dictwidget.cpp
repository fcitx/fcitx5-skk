/*
 * SPDX-FileCopyrightText: 2013~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */
#include "dictwidget.h"
#include <fcntl.h>
#include <fcitx-utils/standardpath.h>
#include <fcitxqti18nhelper.h>
#include "adddictdialog.h"
#include "dictmodel.h"

namespace fcitx {

SkkDictWidget::SkkDictWidget(QWidget *parent)
    : FcitxQtConfigUIWidget(parent),
      m_ui(std::make_unique<Ui::SkkDictWidget>()) {
    m_ui->setupUi(this);
    m_dictModel = new DictModel(this);

    m_ui->dictionaryView->setModel(m_dictModel);

    connect(m_ui->addDictButton, SIGNAL(clicked(bool)), this,
            SLOT(addDictClicked()));
    connect(m_ui->defaultDictButton, SIGNAL(clicked(bool)), this,
            SLOT(defaultDictClicked()));
    connect(m_ui->removeDictButton, SIGNAL(clicked(bool)), this,
            SLOT(removeDictClicked()));
    connect(m_ui->moveUpDictButton, SIGNAL(clicked(bool)), this,
            SLOT(moveUpDictClicked()));
    connect(m_ui->moveDownDictButton, SIGNAL(clicked(bool)), this,
            SLOT(moveDownClicked()));

    load();
}

QString SkkDictWidget::title() { return _("Dictionary Manager"); }

QString SkkDictWidget::icon() { return "fcitx-skk"; }

void SkkDictWidget::load() {
    m_dictModel->load();
    emit changed(false);
}

void SkkDictWidget::save() {
    m_dictModel->save();
    emit changed(false);
}

void SkkDictWidget::addDictClicked() {
    AddDictDialog dialog;
    int result = dialog.exec();
    if (result == QDialog::Accepted) {
        m_dictModel->add(dialog.dictionary());
        emit changed(true);
    }
}

void SkkDictWidget::defaultDictClicked() {
    m_dictModel->defaults();
    emit changed(true);
}

void SkkDictWidget::removeDictClicked() {
    if (m_ui->dictionaryView->currentIndex().isValid()) {
        m_dictModel->removeRow(m_ui->dictionaryView->currentIndex().row());
        emit changed(true);
    }
}

void SkkDictWidget::moveUpDictClicked() {
    int row = m_ui->dictionaryView->currentIndex().row();
    if (m_dictModel->moveUp(m_ui->dictionaryView->currentIndex())) {
        m_ui->dictionaryView->selectionModel()->setCurrentIndex(
            m_dictModel->index(row - 1), QItemSelectionModel::ClearAndSelect);
        emit changed(true);
    }
}

void SkkDictWidget::moveDownClicked() {
    int row = m_ui->dictionaryView->currentIndex().row();
    if (m_dictModel->moveDown(m_ui->dictionaryView->currentIndex())) {
        m_ui->dictionaryView->selectionModel()->setCurrentIndex(
            m_dictModel->index(row + 1), QItemSelectionModel::ClearAndSelect);
        emit changed(true);
    }
}

} // namespace fcitx
