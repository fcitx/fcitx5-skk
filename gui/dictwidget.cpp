/*
 * SPDX-FileCopyrightText: 2013~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */
#include "dictwidget.h"
#include <fcntl.h>
#include <memory>
#include <QDialog>
#include <QItemSelectionModel>
#include <QPushButton>
#include <QString>
#include <QWidget>
#include <fcitx-utils/fs.h>
#include <fcitx-utils/i18n.h>
#include <fcitx-utils/standardpaths.h>
#include <fcitx-utils/stringutils.h>
#include <fcitxqtconfiguiwidget.h>
#include <fcitxqti18nhelper.h>
#include "adddictdialog.h"
#include "dictmodel.h"
#include "ui_dictwidget.h"

namespace fcitx {

SkkDictWidget::SkkDictWidget(QWidget *parent)
    : FcitxQtConfigUIWidget(parent),
      m_ui(std::make_unique<Ui::SkkDictWidget>()) {
    m_ui->setupUi(this);
    m_dictModel = new SkkDictModel(this);
    auto fcitxBasePath =
        StandardPaths::global().userDirectory(StandardPathsType::PkgData) /
        "skk";
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
    Q_EMIT changed(false);
}

void SkkDictWidget::save() {
    m_dictModel->save();
    Q_EMIT changed(false);
}

void SkkDictWidget::addDictClicked() {
    AddDictDialog *dialog = new AddDictDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    connect(dialog, &QDialog::accepted, this, [this, dialog]() {
        m_dictModel->add(dialog->dictionary());
        Q_EMIT changed(true);
    });

    dialog->open();
}

void SkkDictWidget::defaultDictClicked() {
    m_dictModel->defaults();
    Q_EMIT changed(true);
}

void SkkDictWidget::removeDictClicked() {
    if (m_ui->dictionaryView->currentIndex().isValid()) {
        m_dictModel->removeRow(m_ui->dictionaryView->currentIndex().row());
        Q_EMIT changed(true);
    }
}

void SkkDictWidget::moveUpDictClicked() {
    int row = m_ui->dictionaryView->currentIndex().row();
    if (m_dictModel->moveUp(m_ui->dictionaryView->currentIndex())) {
        m_ui->dictionaryView->selectionModel()->setCurrentIndex(
            m_dictModel->index(row - 1), QItemSelectionModel::ClearAndSelect);
        Q_EMIT changed(true);
    }
}

void SkkDictWidget::moveDownClicked() {
    int row = m_ui->dictionaryView->currentIndex().row();
    if (m_dictModel->moveDown(m_ui->dictionaryView->currentIndex())) {
        m_ui->dictionaryView->selectionModel()->setCurrentIndex(
            m_dictModel->index(row + 1), QItemSelectionModel::ClearAndSelect);
        Q_EMIT changed(true);
    }
}

} // namespace fcitx
