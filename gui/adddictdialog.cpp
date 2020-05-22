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

AddDictDialog::AddDictDialog(QWidget *parent)
    : QDialog(parent), m_ui(new Ui::AddDictDialog) {
    m_ui->setupUi(this);
    m_ui->typeComboBox->addItem(_("System"));
    m_ui->typeComboBox->addItem(_("User"));
    m_ui->typeComboBox->addItem(_("Server"));

    indexChanged(0);

    connect(m_ui->browseButton, SIGNAL(clicked(bool)), this,
            SLOT(browseClicked()));
    connect(m_ui->typeComboBox, SIGNAL(currentIndexChanged(int)), this,
            SLOT(indexChanged(int)));
}

QMap<QString, QString> AddDictDialog::dictionary() {
    int idx = m_ui->typeComboBox->currentIndex();
    idx = idx < 0 ? 0 : idx;
    idx = idx > 2 ? 0 : idx;

    QMap<QString, QString> dict;
    if (idx == 2) {
        dict["type"] = "server";
        dict["host"] = m_ui->hostLineEdit->text();
        dict["port"] = QString("%1").arg(m_ui->portSpinBox->value());
    } else {
        const char *type[] = {"readonly", "readwrite"};

        dict["type"] = "file";
        dict["file"] = m_ui->urlLineEdit->text();
        dict["mode"] = type[idx];
    }

    return dict;
}

void AddDictDialog::indexChanged(int idx) {
    bool isServer = idx == 2;
    m_ui->pathLabel->setVisible(!isServer);
    m_ui->urlLineEdit->setVisible(!isServer);
    m_ui->browseButton->setVisible(!isServer);
    m_ui->hostLabel->setVisible(isServer);
    m_ui->hostLineEdit->setVisible(isServer);
    m_ui->portLabel->setVisible(isServer);
    m_ui->portSpinBox->setVisible(isServer);
}

void AddDictDialog::browseClicked() {
    QString path = m_ui->urlLineEdit->text();
    if (m_ui->typeComboBox->currentIndex() == 0) {
        QString dir;
        if (path.isEmpty()) {
            path = SKK_DEFAULT_PATH;
        }
        QFileInfo info(path);
        path = QFileDialog::getOpenFileName(this, _("Select Dictionary File"),
                                            info.path());
    } else {
        auto fcitxBasePath =
            StandardPath::global().userDirectory(StandardPath::Type::PkgData);
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
