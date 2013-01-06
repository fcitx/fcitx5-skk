/***************************************************************************
 *   Copyright (C) 2012~2012 by Yichao Yu                                  *
 *   yyc1992@gmail.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.              *
 ***************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcitx/ime.h>
#include <fcitx-config/fcitx-config.h>
#include <fcitx-config/xdg.h>
#include <fcitx-config/hotkey.h>
#include <fcitx-utils/log.h>
#include <fcitx-utils/utils.h>
#include <fcitx-utils/utf8.h>
#include <fcitx/candidate.h>
#include <fcitx/context.h>
#include <fcitx/keys.h>
#include <fcitx/ui.h>

#include "skk.h"
#include "config.h"

static void *FcitxSkkCreate(FcitxInstance *instance);
static void FcitxSkkDestroy(void *arg);
static boolean FcitxSkkInit(void *arg);
static INPUT_RETURN_VALUE FcitxSkkDoInput(void *arg, FcitxKeySym sym,
                                          unsigned int state);
static INPUT_RETURN_VALUE FcitxSkkGetCandWords(void *arg);
static void FcitxSkkReset(void *arg);

FCITX_DEFINE_PLUGIN(fcitx_skk, ime, FcitxIMClass) = {
    .Create = FcitxSkkCreate,
    .Destroy = FcitxSkkDestroy
};

static const FcitxIMIFace skk_iface = {
    .Init = FcitxSkkInit,
    .ResetIM = FcitxSkkReset,
    .DoInput = FcitxSkkDoInput,
    .GetCandWords = FcitxSkkGetCandWords,
    .PhraseTips = NULL,
    .Save = NULL,
    .ReloadConfig = NULL,
    .KeyBlocker = NULL,
    .UpdateSurroundingText = NULL,
    .DoReleaseInput = NULL,
};

static void*
FcitxSkkCreate(FcitxInstance *instance)
{
    FcitxSkk *skk = fcitx_utils_new(FcitxSkk);
    bindtextdomain("fcitx-skk", LOCALEDIR);

    skk->owner = instance;

    FcitxInstanceRegisterIMv2(instance, skk, "skk", _("Skk"), "skk",
                              skk_iface, 1, "ja");
    return skk;
}

static void
FcitxSkkDestroy(void *arg)
{
    FcitxSkk *skk = (FcitxSkk*)arg;
    if (fcitx_unlikely(!arg))
        return;
    free(arg);
}

static boolean
FcitxSkkInit(void *arg)
{
    FcitxSkk *skk = (FcitxSkk*)arg;
    if (!arg)
        return false;
    FcitxInstanceSetContext(skk->owner, CONTEXT_IM_KEYBOARD_LAYOUT, "us");
    return true;
}

static INPUT_RETURN_VALUE
FcitxSkkDoInput(void *arg, FcitxKeySym sym, unsigned int state)
{
    FcitxSkk *skk = (FcitxSkk*)arg;
    return IRV_TO_PROCESS;
}

static INPUT_RETURN_VALUE
FcitxSkkGetCandWords(void *arg)
{
    FcitxSkk *skk = (FcitxSkk*)arg;
    return IRV_DISPLAY_CANDWORDS;
}

static void
FcitxSkkReset(void *arg)
{
    FcitxSkk *skk = (FcitxSkk*)arg;
}
