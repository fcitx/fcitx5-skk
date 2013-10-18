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

#include <QApplication>
#include <qplugin.h>
#include <libintl.h>
#include <fcitx-utils/utils.h>
#include <glib-object.h>
#include <libskk/libskk.h>
#include "main.h"
#include "dictwidget.h"

SkkConfigPlugin::SkkConfigPlugin(QObject* parent): FcitxQtConfigUIPlugin(parent)
{
#if !GLIB_CHECK_VERSION(2, 36, 0)
    g_type_init();
#endif
    skk_init();
}

FcitxQtConfigUIWidget* SkkConfigPlugin::create(const QString& key)
{
    if (key == "skk/dictionary_list") {
        return new SkkDictWidget;
    }
    return NULL;
}

QStringList SkkConfigPlugin::files()
{
    QStringList fileList;
    fileList << "skk/dictionary_list";
    return fileList;
}

QString SkkConfigPlugin::name()
{
    return "skk-config";
}

QString SkkConfigPlugin::domain()
{
    return "fcitx-skk";
}


Q_EXPORT_PLUGIN2(fcitx_skk_config, SkkConfigPlugin)
