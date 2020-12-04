/*
 * SPDX-FileCopyrightText: 2013~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "adddictdialog.h"
#include <QDebug>
#include <QFileDialog>
#include <fcitx-utils/standardpath.h>
#include <fcitxqti18nhelper.h>
#include "config.h"

#define FCITX_CONFIG_DIR "$FCITX_CONFIG_DIR"

namespace fcitx {

enum DictType { DictType_System, DictType_User, DictType_Server };

AddDictDialog::AddDictDialog(QWidget *parent)
    : QDialog(parent), m_ui(std::make_unique<Ui::AddDictDialog>()) {
    m_ui->setupUi(this);
    m_ui->typeComboBox->addItem(_("System"));
    m_ui->typeComboBox->addItem(_("User"));
    m_ui->typeComboBox->addItem(_("Server"));

    indexChanged(0);

    connect(m_ui->browseButton, &QPushButton::clicked, this,
            &AddDictDialog::browseClicked);
    connect(m_ui->typeComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &AddDictDialog::indexChanged);
    connect(m_ui->urlLineEdit, &QLineEdit::textChanged, this,
            &AddDictDialog::validate);
    connect(m_ui->hostLineEdit, &QLineEdit::textChanged, this,
            &AddDictDialog::validate);
}

QMap<QString, QString> AddDictDialog::dictionary() {
    int idx = m_ui->typeComboBox->currentIndex();
    idx = idx < 0 ? 0 : idx;
    idx = idx > 2 ? 0 : idx;

    QMap<QString, QString> dict;
    if (idx == DictType_Server) {
        dict["type"] = "server";
        dict["host"] = m_ui->hostLineEdit->text();
        dict["port"] = QString("%1").arg(m_ui->portSpinBox->value());
    } else {
        const char *type[] = {"readonly", "readwrite"};

        dict["type"] = "file";
        dict["file"] = m_ui->urlLineEdit->text();
        dict["mode"] = type[idx];
    }
    if (!m_ui->encodingEdit->text().isEmpty()) {
        dict["encoding"] = m_ui->encodingEdit->text();
    }

    return dict;
}

void AddDictDialog::indexChanged(int idx) {
    bool isServer = idx == DictType_Server;
    m_ui->pathLabel->setVisible(!isServer);
    m_ui->urlLineEdit->setVisible(!isServer);
    m_ui->browseButton->setVisible(!isServer);
    m_ui->hostLabel->setVisible(isServer);
    m_ui->hostLineEdit->setVisible(isServer);
    m_ui->portLabel->setVisible(isServer);
    m_ui->portSpinBox->setVisible(isServer);
    validate();
}

void AddDictDialog::validate() {
    const auto index = m_ui->typeComboBox->currentIndex();
    bool valid = true;
    switch (index) {
    case DictType_System:
    case DictType_User:
        if (m_ui->urlLineEdit->text().isEmpty()) {
            valid = false;
        }
        break;
    case DictType_Server:
        if (m_ui->hostLineEdit->text().isEmpty()) {
            valid = false;
        }
        break;
    }
    m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(valid);
}

void AddDictDialog::browseClicked() {
    QString path = m_ui->urlLineEdit->text();
    if (m_ui->typeComboBox->currentIndex() == DictType_System) {
        QString dir;
        if (path.isEmpty()) {
            path = SKK_DEFAULT_PATH;
        }
        QFileInfo info(path);
        path = QFileDialog::getOpenFileName(this, _("Select Dictionary File"),
                                            info.path());
    } else {
        auto fcitxBasePath = stringutils::joinPath(
            StandardPath::global().userDirectory(StandardPath::Type::PkgData),
            "skk");
        fs::makePath(fcitxBasePath);
        QString basePath =
            QDir::cleanPath(QString::fromStdString(fcitxBasePath));
        if (path.isEmpty()) {
            path = basePath;
        } else if (path.startsWith(FCITX_CONFIG_DIR "/")) {
            QDir dir(basePath);
            path = dir.filePath(path.mid(strlen(FCITX_CONFIG_DIR) + 1));
        }
        path = QFileDialog::getOpenFileName(this, _("Select Dictionary File"),
                                            path);
        if (path.startsWith(basePath + "/")) {
            path = FCITX_CONFIG_DIR + path.mid(basePath.length(), -1);
        }
    }

    if (!path.isEmpty()) {
        m_ui->urlLineEdit->setText(path);
    }
}

} // namespace fcitx
