/*
 * SPDX-FileCopyrightText: 2013~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */
#include "dictwidget.h"
#include <fcntl.h>
#include <fcitx-utils/fs.h>
#include <fcitx-utils/standardpath.h>
#include <fcitx-utils/stringutils.h>
#include <fcitxqti18nhelper.h>
#include "adddictdialog.h"
#include "dictmodel.h"

namespace fcitx {

SkkDictWidget::SkkDictWidget(QWidget *parent)
    : FcitxQtConfigUIWidget(parent),
      m_ui(std::make_unique<Ui::SkkDictWidget>()) {
    m_ui->setupUi(this);
    m_dictModel = new SkkDictModel(this);
    auto fcitxBasePath = stringutils::joinPath(
        StandardPath::global().userDirectory(StandardPath::Type::PkgData),
        "skk");
    fs::makePath(fcitxBasePath);

    m_ui->dictionaryView->setModel(m_dictModel);

    connect(m_ui->addDictButton, &QPushButton::clicked, this,
            &SkkDictWidget::addDictClicked);
    connect(m_ui->defaultDictButton, &QPushButton::clicked, this,
            &SkkDictWidget::defaultDictClicked);
    connect(m_ui->removeDictButton, &QPushButton::clicked, this,
            &SkkDictWidget::removeDictClicked);
    connect(m_ui->moveUpDictButton, &QPushButton::clicked, this,
            &SkkDictWidget::moveUpDictClicked);
    connect(m_ui->moveDownDictButton, &QPushButton::clicked, this,
            &SkkDictWidget::moveDownClicked);

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
