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
#include <fcitx/candidate.h>
#include <fcitx/context.h>
#include <fcitx/keys.h>
#include <fcitx/ui.h>
#include <libskk/libskk.h>

#include "skk.h"
#include "config.h"

static void *FcitxSkkCreate(FcitxInstance *instance);
static void FcitxSkkDestroy(void *arg);
static boolean FcitxSkkInit(void *arg);
static INPUT_RETURN_VALUE FcitxSkkDoInputReal(void *arg, FcitxKeySym sym,
                                          unsigned int state);
static INPUT_RETURN_VALUE FcitxSkkDoInput(void *arg, FcitxKeySym sym,
                                          unsigned int state);
static INPUT_RETURN_VALUE FcitxSkkDoReleaseInput(void *arg, FcitxKeySym sym,
                                          unsigned int state);
static INPUT_RETURN_VALUE FcitxSkkDoCandidate(void *arg, FcitxKeySym sym,
                                             unsigned int state);
static INPUT_RETURN_VALUE FcitxSkkKeyBlocker(void *arg, FcitxKeySym sym,
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
    .KeyBlocker = 0,
    .UpdateSurroundingText = NULL,
    .DoReleaseInput = FcitxSkkDoReleaseInput,
};

static void
ut_gobject_cpy(void *dst, const void *src)
{
    GObject *const *_src = (GObject *const*)src;
    GObject **_dst = (GObject**)dst;
    *_dst = g_object_ref(*_src);
}

static void
ut_gobject_dtor(void *self)
{
    GObject **_self = (GObject**)self;
    g_object_unref(*_self);
}

static const UT_icd gobject_icd = {
    .sz = sizeof(GObject*),
    .dtor = ut_gobject_dtor,
    .copy = ut_gobject_cpy,
};

void skk_candidate_update_preedit_cb (SkkContext* ctx, GParamSpec *pspec,  gpointer user_data)
{
    FcitxSkk *skk = (FcitxSkk*) user_data;
    skk->updatePreedit = true;
}

void skk_candidate_list_selected_cb (SkkCandidateList* self, SkkCandidate* c, gpointer user_data)
{
    FcitxSkk *skk = (FcitxSkk*) user_data;
    skk->selected = true;
    gchar* output = skk_context_poll_output(skk->ctx);

    if (output && strlen(output) > 0) {
        FcitxInstanceCommitString(skk->owner, FcitxInstanceGetCurrentIC(skk->owner), output);
    }

    g_free(output);
}

void skk_candidate_list_popuplated_cb (SkkCandidateList* self, gpointer user_data)
{
    FcitxSkk *skk = (FcitxSkk*) user_data;
    skk->update_candidate = true;
}

static gboolean skk_context_retrieve_surrounding_text_cb (SkkContext* self, gchar** text, guint* cursor_pos, gpointer user_data) {
    FcitxSkk *skk = (FcitxSkk*) user_data;
    FcitxInputContext* ic = FcitxInstanceGetCurrentIC(skk->owner);
    if (!ic || !(ic->contextCaps & CAPACITY_SURROUNDING_TEXT))
        return false;

    char* _text = NULL;
    unsigned int _cursor_pos;
    if (!FcitxInstanceGetSurroundingText(skk->owner, ic, &_text, &_cursor_pos, NULL))
        return false;

    *text = g_strdup(_text);
    *cursor_pos = _cursor_pos;

    fcitx_utils_free(_text);

    return true;
}

static gboolean skk_context_delete_surrounding_text_cb (SkkContext* self, gint offset, guint nchars, gpointer user_data) {
    FcitxSkk *skk = (FcitxSkk*) user_data;
    FcitxInputContext* ic = FcitxInstanceGetCurrentIC(skk->owner);
    if (!ic || !(ic->contextCaps & CAPACITY_SURROUNDING_TEXT))
        return false;
    FcitxInstanceDeleteSurroundingText(skk->owner, ic, offset, nchars);
    return true;
}

static void*
FcitxSkkCreate(FcitxInstance *instance)
{
    FcitxSkk *skk = fcitx_utils_new(FcitxSkk);
    bindtextdomain("fcitx-skk", LOCALEDIR);

    g_type_init();

    skk_init();

    skk->owner = instance;
    utarray_init(&skk->dicts, &gobject_icd);
    SkkFileDict* dict = skk_file_dict_new("/usr/share/skk/SKK-JISYO.L", "EUC-JP", NULL);
    utarray_push_back(&skk->dicts, &dict);
    skk->ctx = skk_context_new((SkkDict**)utarray_front(&skk->dicts),
                               utarray_len(&skk->dicts));

    FcitxInstanceRegisterIMv2(instance, skk, "skk", _("Skk"), "skk",
                              skk_iface, 1, "ja");

    g_signal_connect(skk_context_get_candidates(skk->ctx), "selected", G_CALLBACK(skk_candidate_list_selected_cb), skk);
    g_signal_connect(skk_context_get_candidates(skk->ctx), "populated", G_CALLBACK(skk_candidate_list_popuplated_cb), skk);
    g_signal_connect(skk->ctx, "notify::preedit", G_CALLBACK(skk_candidate_update_preedit_cb), skk);
    g_signal_connect(skk->ctx, "retrieve_surrounding_text", G_CALLBACK(skk_context_retrieve_surrounding_text_cb), skk);
    g_signal_connect(skk->ctx, "delete_surrounding_text", G_CALLBACK(skk_context_delete_surrounding_text_cb), skk);


    const char* AUTO_START_HENKAN_KEYWORDS[] = {
        "を", "、", "。", "．", "，", "？", "」",
        "！", "；", "：", ")", ";", ":", "）",
        "”", "】", "』", "》", "〉", "｝", "］",
        "〕", "}", "]", "?", ".", ",", "!"
    };
    skk_context_set_auto_start_henkan_keywords(skk->ctx, AUTO_START_HENKAN_KEYWORDS, G_N_ELEMENTS(AUTO_START_HENKAN_KEYWORDS));
    skk_context_set_period_style(skk->ctx, SKK_PERIOD_STYLE_JA_JA);
    skk_context_set_input_mode(skk->ctx, SKK_INPUT_MODE_HIRAGANA);
    skk_context_set_egg_like_newline(skk->ctx, FALSE);

    skk_candidate_list_set_page_size(skk_context_get_candidates(skk->ctx), 7);
    skk_candidate_list_set_page_start(skk_context_get_candidates(skk->ctx), 4);

    SkkRule* rule = skk_rule_new("default", NULL);
    if (rule) {
        skk_context_set_typing_rule(skk->ctx, rule);
    }

    return skk;
}



static void
FcitxSkkDestroy(void *arg)
{
    FcitxSkk *skk = (FcitxSkk*)arg;
    if (fcitx_unlikely(!arg))
        return;
    g_object_unref(skk->ctx);
    utarray_done(&skk->dicts);
    free(arg);
}

static boolean
FcitxSkkInit(void *arg)
{
    FcitxSkk *skk = (FcitxSkk*)arg;
    if (!arg)
        return false;
    FcitxInstanceSetContext(skk->owner, CONTEXT_IM_KEYBOARD_LAYOUT, "ja");
    return true;
}

INPUT_RETURN_VALUE FcitxSkkKeyBlocker(void* arg, FcitxKeySym sym, unsigned int state)
{
    if (sym == FcitxKey_j && state == FcitxKeyState_Ctrl)
        return IRV_DO_NOTHING;
    return IRV_TO_PROCESS;
}

static INPUT_RETURN_VALUE
FcitxSkkDoInput(void *arg, FcitxKeySym sym, unsigned int state)
{
    FcitxSkk *skk = (FcitxSkk*)arg;
    FcitxInputState* input = FcitxInstanceGetInputState(skk->owner);
    sym = FcitxInputStateGetKeySym(input);
    state = FcitxInputStateGetKeyState(input);

    return FcitxSkkDoInputReal(skk, sym, state);
}

static INPUT_RETURN_VALUE
FcitxSkkDoInputReal(void *arg, FcitxKeySym sym, unsigned int state)
{
    FcitxSkk *skk = (FcitxSkk*)arg;
    FcitxInputState* input = FcitxInstanceGetInputState(skk->owner);
    FcitxCandidateWordList* candList = FcitxInputStateGetCandidateList(input);
    // Filter out unnecessary modifier bits
    // FIXME: should resolve virtual modifiers

    if (skk_candidate_list_get_page_visible(skk_context_get_candidates(skk->ctx))) {
        INPUT_RETURN_VALUE result = FcitxSkkDoCandidate (skk, sym, state);
        if (result == IRV_TO_PROCESS)
            return result;
    }

    SkkModifierType modifiers = (SkkModifierType) state & (FcitxKeyState_SimpleMask | (1 << 30));
    SkkKeyEvent* key = skk_key_event_new_from_x_keysym(sym, state, NULL);
    if (!key)
        return IRV_TO_PROCESS;

    gboolean retval = skk_context_process_key_event(skk->ctx, key);
    gchar* output = skk_context_poll_output(skk->ctx);

    g_object_unref(key);

    if (output && strlen(output) > 0) {
        FcitxInstanceCommitString(skk->owner, FcitxInstanceGetCurrentIC(skk->owner), output);
    }

    g_free(output);
    return retval ? (skk->updatePreedit || skk->update_candidate ?  IRV_DISPLAY_CANDWORDS : IRV_DO_NOTHING) : IRV_TO_PROCESS;
}

static INPUT_RETURN_VALUE
FcitxSkkDoReleaseInput(void *arg, FcitxKeySym sym, unsigned int state)
{
    FcitxSkk *skk = (FcitxSkk*)arg;
    FcitxInputState* input = FcitxInstanceGetInputState(skk->owner);
    sym = FcitxInputStateGetKeySym(input);
    state = FcitxInputStateGetKeyState(input);

    return FcitxSkkDoInputReal(skk, sym, state | (1 << 30));
}

INPUT_RETURN_VALUE FcitxSkkDoCandidate(void* arg, FcitxKeySym sym, unsigned int state)
{
    FcitxSkk *skk = (FcitxSkk*)arg;
    FcitxInputState* input = FcitxInstanceGetInputState(skk->owner);
    FcitxGlobalConfig *fc = FcitxInstanceGetGlobalConfig(skk->owner);
    FcitxCandidateWordList* candList = FcitxInputStateGetCandidateList(input);

    if (FcitxHotkeyIsHotKey(sym, state,
                            FcitxConfigPrevPageKey(skk->owner, fc))) {
        return IRV_TO_PROCESS;
    } else if (FcitxHotkeyIsHotKey(sym, state,
                                    FcitxConfigNextPageKey(skk->owner, fc))) {
        return IRV_TO_PROCESS;
    } else if (FcitxCandidateWordCheckChooseKey(candList, sym, state) >= 0) {
        return IRV_TO_PROCESS;
    }
    return IRV_DO_NOTHING;
}

static INPUT_RETURN_VALUE
FcitxSkkGetCandWord(void* arg, FcitxCandidateWord* cand)
{
    FcitxSkk *skk = (FcitxSkk*)arg;
    SkkCandidateList* skkCandList = skk_context_get_candidates(skk->ctx);
    guint pageSize = skk_candidate_list_get_page_size(skkCandList);
    int idx = *(int*) cand->priv;

    skk->selected = false;
    skk_candidate_list_select_at(skkCandList, idx);

    return IRV_DISPLAY_CANDWORDS;
}

boolean FcitxSkkPaging(void* arg, boolean prev) {
    FcitxSkk *skk = (FcitxSkk*)arg;
    SkkCandidateList* skkCandList = skk_context_get_candidates(skk->ctx);
    boolean result;
    if (prev)
        skk_candidate_list_page_up(skkCandList);
    else
        skk_candidate_list_page_down(skkCandList);
    FcitxSkkGetCandWords(skk);
    return true;
}

static INPUT_RETURN_VALUE
FcitxSkkGetCandWords(void *arg)
{
    FcitxSkk *skk = (FcitxSkk*)arg;
    FcitxInstanceCleanInputWindow(skk->owner);
    FcitxInputState* input = FcitxInstanceGetInputState(skk->owner);
    FcitxCandidateWordList* candList = FcitxInputStateGetCandidateList(input);
    SkkCandidateList* skkCandList = skk_context_get_candidates(skk->ctx);
    FcitxCandidateWordSetPageSize(candList, 7);

    if (skk_candidate_list_get_page_visible(skkCandList)) {
        int i = 0;
        int j = 0;
        guint size = skk_candidate_list_get_size(skkCandList);
        gint cursor_pos = skk_candidate_list_get_cursor_pos(skkCandList);
        guint page_start = skk_candidate_list_get_page_start(skkCandList);
        guint page_size = skk_candidate_list_get_page_size(skkCandList);
        for (i = skk_candidate_list_get_page_start(skkCandList), j = 0; i < size; i ++, j++) {
            FcitxCandidateWord word;
            word.callback = FcitxSkkGetCandWord;
            word.extraType = MSG_OTHER;
            word.owner = skk;
            int* id = fcitx_utils_new(int);
            *id = j;
            word.priv = id;
            word.strExtra = NULL;
            word.strWord = strdup(skk_candidate_get_text(skk_candidate_list_get(skkCandList, i)));
            if (i == cursor_pos) {
                word.wordType = MSG_CANDIATE_CURSOR;
            } else {
                word.wordType = MSG_OTHER;
            }
            FcitxCandidateWordAppend(candList, &word);
        }
        FcitxCandidateWordSetFocus(candList, cursor_pos - page_start);
        FcitxCandidateWordSetOverridePaging(candList, (cursor_pos - page_start) >= page_size, (size - cursor_pos) >= page_size, FcitxSkkPaging, skk, NULL);
    }
    skk->update_candidate = false;

    FcitxMessages* clientPreedit = FcitxInputStateGetClientPreedit(input);
    FcitxMessages* preedit = FcitxInputStateGetClientPreedit(input);

    const gchar* preeditString = skk_context_get_preedit(skk->ctx);
    size_t len = strlen(preeditString);
    if (len > 0) {
        guint offset, nchars;
        skk_context_get_preedit_underline(skk->ctx, &offset, &nchars);

        if (nchars > 0) {
            const gchar* preeditString = skk_context_get_preedit(skk->ctx);
            char* off = fcitx_utf8_get_nth_char(preeditString, offset);
            if (offset > 0) {
                char* left = strndup(preeditString, off - preeditString);
                FcitxMessagesAddMessageAtLast(clientPreedit, MSG_OTHER, "%s", left);
                fcitx_utils_free(left);
            }
            char* right = fcitx_utf8_get_nth_char(off, nchars);
            char* middle = strndup(off, right - off);
            FcitxMessagesAddMessageAtLast(clientPreedit, MSG_HIGHLIGHT, "%s", middle);
            fcitx_utils_free(middle);

            if (*right != 0) {
                FcitxMessagesAddMessageAtLast(clientPreedit, MSG_OTHER, "%s", right);
            }
        }
        else {
            FcitxMessagesAddMessageAtLast(clientPreedit, MSG_OTHER, "%s", preeditString);
        }
    }

    FcitxInputStateSetClientCursorPos(input, len);
    skk->updatePreedit = false;

    return IRV_DISPLAY_CANDWORDS;
}

static void
FcitxSkkReset(void *arg)
{
    FcitxSkk *skk = (FcitxSkk*)arg;
    skk_context_reset(skk->ctx);
}
