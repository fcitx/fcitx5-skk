/*
 * SPDX-FileCopyrightText: 2013~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "main.h"
#include <QApplication>
#include <fcitx-utils/i18n.h>
#include <glib-object.h>
#include <libskk/libskk.h>
#include <qplugin.h>
#include "dictwidget.h"

namespace fcitx {

SkkConfigPlugin::SkkConfigPlugin(QObject *parent)
    : FcitxQtConfigUIPlugin(parent) {
#if !GLIB_CHECK_VERSION(2, 36, 0)
    g_type_init();
#endif
    skk_init();
    registerDomain("fcitx5-skk", FCITX_INSTALL_LOCALEDIR);
}

FcitxQtConfigUIWidget *SkkConfigPlugin::create(const QString &key) {
    if (key == "dictionary_list") {
        return new SkkDictWidget;
    }
    return nullptr;
}

} // namespace fcitx
