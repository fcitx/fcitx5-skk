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

#include <QFileDialog>
#include <QDebug>
#include <fcitx-config/xdg.h>

#include "adddictdialog.h"
#include "ui_adddictdialog.h"
#include "common.h"
#include "config.h"

#define FCITX_CONFIG_DIR "$FCITX_CONFIG_DIR"

AddDictDialog::AddDictDialog(QWidget* parent): QDialog(parent)
    ,m_ui(new Ui::AddDictDialog)
{
    m_ui->setupUi(this);
    m_ui->typeLabel->setText(_("&Type:"));
    m_ui->pathLabel->setText(_("&Path:"));
    m_ui->hostLabel->setText(_("&Host:"));
    m_ui->portLabel->setText(_("&Port:"));
    m_ui->typeComboBox->addItem(_("System"));
    m_ui->typeComboBox->addItem(_("User"));
    m_ui->typeComboBox->addItem(_("Server"));

    indexChanged(0);

    connect(m_ui->browseButton, SIGNAL(clicked(bool)), this, SLOT(browseClicked()));
    connect(m_ui->typeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(indexChanged(int)));
}

AddDictDialog::~AddDictDialog()
{
    delete m_ui;
}

QMap<QString, QString> AddDictDialog::dictionary()
{
    int idx = m_ui->typeComboBox->currentIndex();
    idx = idx < 0 ? 0 : idx;
    idx = idx > 2 ? 0 : idx;

    QMap<QString, QString> dict;
    if (idx == 2) {
        dict["type"] = "server";
        dict["host"] = m_ui->hostLineEdit->text();
        dict["port"] = QString("%1").arg(m_ui->portSpinBox->value());
    } else {
        const char* type[] = {
            "readonly",
            "readwrite"
        };


        dict["type"] = "file";
        dict["file"] = m_ui->urlLineEdit->text();
        dict["mode"] = type[idx];
    }

    return dict;
}

void AddDictDialog::indexChanged(int idx)
{
    bool isServer = idx == 2;
    m_ui->pathLabel->setVisible(!isServer);
    m_ui->urlLineEdit->setVisible(!isServer);
    m_ui->browseButton->setVisible(!isServer);
    m_ui->hostLabel->setVisible(isServer);
    m_ui->hostLineEdit->setVisible(isServer);
    m_ui->portLabel->setVisible(isServer);
    m_ui->portSpinBox->setVisible(isServer);
}

void AddDictDialog::browseClicked()
{
    QString path = m_ui->urlLineEdit->text();
    if (m_ui->typeComboBox->currentIndex() == 0) {
        QString dir;
        if (path.isEmpty()) {
            path = SKK_DEFAULT_PATH;
        }
        QFileInfo info(path);
        path = QFileDialog::getOpenFileName(this, _("Select Dictionary File"), info.path());
    } else {
        char* fcitxBasePath = NULL;
        FcitxXDGGetFileUserWithPrefix("", "", NULL, &fcitxBasePath);
        QString basePath = QDir::cleanPath(QString::fromLocal8Bit(fcitxBasePath));
        free(fcitxBasePath);
        if (path.isEmpty()) {
            path = basePath;
        } else if (path.startsWith(FCITX_CONFIG_DIR "/")) {
            QDir dir(basePath);
            path = dir.filePath(path.mid(strlen(FCITX_CONFIG_DIR) + 1));
        }
        path = QFileDialog::getOpenFileName(this, _("Select Dictionary File"), path);
        if (path.startsWith(basePath + "/" )) {
            path = FCITX_CONFIG_DIR + path.mid(basePath.length(), -1);
        }
    }

    if (!path.isEmpty()) {
        m_ui->urlLineEdit->setText(path);
    }
}
