/***************************************************************************
 *   Copyright (C) 2012~2012 by Yichao Yu                                  *
 *   yyc1992@gmail.com                                                     *
 *   Copyright (C) 2012~2013 by Weng Xuetian                               *
 *   wengxt@gmail.com                                                      *
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
#include <fcitx/hook.h>
#include <libskk/libskk.h>

#include "skk.h"
#include "config.h"

UT_icd dict_icd = { sizeof(void*), 0, 0, 0};

static void *FcitxSkkCreate(FcitxInstance *instance);
static void FcitxSkkDestroy(void *arg);
static void FcitxSkkReloadConfig(void *arg);
static boolean FcitxSkkLoadDictionary(FcitxSkk* skk);
static boolean FcitxSkkLoadRule(FcitxSkk* skk);
static void FcitxSkkApplyConfig(FcitxSkk* skk);

CONFIG_DEFINE_LOAD_AND_SAVE(Skk, FcitxSkkConfig, "fcitx-skk")

static boolean FcitxSkkInit(void *arg);
static INPUT_RETURN_VALUE FcitxSkkDoInputReal(void *arg, FcitxKeySym sym,
                                          unsigned int state);
static INPUT_RETURN_VALUE FcitxSkkDoInput(void *arg, FcitxKeySym sym,
                                          unsigned int state);
static INPUT_RETURN_VALUE FcitxSkkDoReleaseInput(void *arg, FcitxKeySym sym,
                                          unsigned int state);
static INPUT_RETURN_VALUE FcitxSkkDoCandidate(void *arg, FcitxKeySym sym,
                                             unsigned int state);
static INPUT_RETURN_VALUE FcitxSkkGetCandWords(void *arg);
static void FcitxSkkReset(void *arg);

FCITX_DEFINE_PLUGIN(fcitx_skk, ime2, FcitxIMClass2) = {
    FcitxSkkCreate,
    FcitxSkkDestroy,
    FcitxSkkReloadConfig,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
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

typedef struct _SkkStatus {
    const char* icon;
    const char* label;
    const char* description;
} SkkStatus;

SkkStatus input_mode_status[] = {
    {"",  "\xe3\x81\x82", N_("Hiragana") },
    {"", "\xe3\x82\xa2", N_("Katakana") },
    {"", "\xef\xbd\xb1", N_("Half width Katakana") },
    {"", "A", N_("Latin") },
    {"", "\xef\xbc\xa1", N_("Wide latin") },
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

void skk_candidate_update_preedit_cb (SkkContext* context, GParamSpec *pspec,  gpointer user_data)
{
    FcitxSkk *skk = (FcitxSkk*) user_data;
    skk->updatePreedit = true;
}

void skk_candidate_list_selected_cb (SkkCandidateList* self, SkkCandidate* c, gpointer user_data)
{
    FcitxSkk *skk = (FcitxSkk*) user_data;
    skk->selected = true;
    gchar* output = skk_context_poll_output(skk->context);

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

typedef enum _FcitxSkkDictType {
    FSDT_Invalid,
    FSDT_File,
    FSTD_Server
} FcitxSkkDictType;


void FcitxSkkResetHook(void *arg)
{
    FcitxSkk *skk = (FcitxSkk*)arg;
    FcitxIM* im = FcitxInstanceGetCurrentIM(skk->owner);
#define RESET_STATUS(STATUS_NAME) \
    if (im && strcmp(im->uniqueName, "skk") == 0) \
        FcitxUISetStatusVisable(skk->owner, STATUS_NAME, true); \
    else \
        FcitxUISetStatusVisable(skk->owner, STATUS_NAME, false);

    RESET_STATUS("skk-input-mode")
}

boolean FcitxSkkLoadDictionary(FcitxSkk* skk)
{
    FILE* fp = FcitxXDGGetFileWithPrefix("skk", "dictionary_list", "r", NULL);
    if (!fp) {
        return false;
    }

    UT_array dictionaries;
    utarray_init(&dictionaries, &dict_icd);

    char *buf = NULL;
    size_t len = 0;
    char *trimmed = NULL;

    while (getline(&buf, &len, fp) != -1) {
        if (trimmed)
            free(trimmed);
        trimmed = fcitx_utils_trim(buf);

        UT_array* list = fcitx_utils_split_string(trimmed, ',');
        do {
            if (utarray_len(list) < 3) {
                break;
            }

            FcitxSkkDictType type = FSDT_Invalid;
            int mode = 0;
            char* path = NULL;
            char* host = NULL;
            char* port = NULL;
            char* encoding = NULL;
            utarray_foreach(item, list, char*) {
                char* key = *item;
                char* value = strchr(*item, '=');
                if (!value)
                    continue;
                *value = '\0';
                value++;

                if (strcmp(key, "type") == 0) {
                    if (strcmp(value, "file") == 0) {
                        type = FSDT_File;
                    } else if (strcmp(value, "server") == 0) {
                        type = FSTD_Server;
                    }
                } else if (strcmp(key, "file") == 0) {
                    path = value;
                } else if (strcmp(key, "mode") == 0) {
                    if (strcmp(value, "readonly") == 0) {
                        mode = 1;
                    } else if (strcmp(value, "readwrite") == 0) {
                        mode = 2;
                    }
                } else if (strcmp(key, "host") == 0) {
                    host = value;
                } else if (strcmp(key, "port") == 0) {
                    port = value;
                } else if (strcmp(key, "encoding") == 0) {
                    encoding = value;
                }
            }

            encoding = encoding ? encoding : "EUC-JP";

            if (type == FSDT_Invalid) {
                break;
            } else if (type == FSDT_File) {
                if (path == NULL || mode == 0) {
                    break;
                }
                if (mode == 1) {
                    if(strlen(path) > 4 && !strcmp(path + strlen(path) - 4, ".cdb")) {
                        SkkCdbDict* dict = skk_cdb_dict_new(path, encoding, NULL);
                        if (dict) {
                            utarray_push_back(&dictionaries, &dict);
                        }
                    } else {
                        SkkFileDict* dict = skk_file_dict_new(path, encoding, NULL);
                        if (dict) {
                            utarray_push_back(&dictionaries, &dict);
                        }
                    }
                } else {
                    char* needfree = NULL;
                    char* realpath = NULL;
                    if (strncmp(path, "$FCITX_CONFIG_DIR/", strlen("$FCITX_CONFIG_DIR/")) == 0) {
                        FcitxXDGGetFileUserWithPrefix("", path + strlen("$FCITX_CONFIG_DIR/"), NULL, &needfree);
                        realpath = needfree;
                    } else {
                        realpath = path;
                    }
                    SkkUserDict* userdict = skk_user_dict_new(realpath, encoding, NULL);
                    if (needfree) {
                        free(needfree);
                    }
                    if (userdict) {
                        utarray_push_back(&dictionaries, &userdict);
                    }
                }
            } else if (type == FSTD_Server) {
                host = host ? host : "localhost";
                port = port ? port : "1178";

                errno = 0;
                int iPort = atoi(port);
                if (iPort <= 0 || iPort > UINT16_MAX) {
                    break;
                }

                SkkSkkServ* dict = skk_skk_serv_new(host, iPort, encoding, NULL);
                if (dict) {
                    utarray_push_back(&dictionaries, &dict);
                }
            }

        } while(0);
        fcitx_utils_free_string_list(list);
    }

    if (buf)
        free(buf);
    if (trimmed)
        free(trimmed);

    boolean result = false;
    if (utarray_len(&dictionaries) != 0) {
        result = true;
        skk_context_set_dictionaries(skk->context,
                                     (SkkDict**)utarray_front(&dictionaries),
                                     utarray_len(&dictionaries));
    }

    utarray_done(&dictionaries);
    return result;
}

boolean FcitxSkkLoadRule(FcitxSkk* skk)
{
    FILE* fp = FcitxXDGGetFileWithPrefix("skk", "rule", "r", NULL);
    SkkRuleMetadata* meta = NULL;

    do {
        if (!fp) {
            break;
        }

        char* line = NULL;
        size_t bufsize = 0;
        getline(&line, &bufsize, fp);
        fclose(fp);

        if (!line) {
            break;
        }

        char* trimmed = fcitx_utils_trim(line);
        meta = skk_rule_find_rule(trimmed);
        free(trimmed);
        free(line);
    } while(0);

    if (!meta) {
        meta = skk_rule_find_rule("default");
        if (!meta) {
            return false;
        }
    }

    SkkRule* rule = skk_rule_new(meta->name, NULL);
    if (!rule) {
        return false;
    }

    skk_context_set_typing_rule(skk->context, rule);
    return true;
}


const char* FcitxSkkGetInputModeIconName(void* arg)
{
    FcitxSkk *skk = (FcitxSkk*)arg;
    return input_mode_status[skk_context_get_input_mode(skk->context)].icon;
}

void FcitxSkkUpdateInputModeMenu(struct _FcitxUIMenu *menu)
{
    FcitxSkk *skk = (FcitxSkk*) menu->priv;
    menu->mark = skk_context_get_input_mode(skk->context);
}

boolean FcitxSkkInputModeMenuAction(struct _FcitxUIMenu *menu, int index)
{
    FcitxSkk *skk = (FcitxSkk*) menu->priv;
    skk_context_set_input_mode(skk->context, (SkkInputMode) index);
    return true;
}

void FcitxSkkUpdateInputMode(FcitxSkk* skk)
{
    SkkInputMode mode = skk_context_get_input_mode(skk->context);
    FcitxUISetStatusString(skk->owner,
                           "skk-input-mode",
                           _(input_mode_status[mode].label),
                           _(input_mode_status[mode].description));
}

static void  _skk_input_mode_changed_cb                (GObject    *gobject,
                                                        GParamSpec *pspec,
                                                        gpointer    user_data)
{
    FcitxSkk *skk = (FcitxSkk*) user_data;
    FcitxSkkUpdateInputMode(skk);
}

static void*
FcitxSkkCreate(FcitxInstance *instance)
{
    FcitxSkk *skk = fcitx_utils_new(FcitxSkk);
    bindtextdomain("fcitx-skk", LOCALEDIR);
    bind_textdomain_codeset("fcitx-skk", "UTF-8");
    skk->owner = instance;

    if (!SkkLoadConfig(&skk->config)) {
        free(skk);
        return NULL;
    }

#if !GLIB_CHECK_VERSION(2, 36, 0)
    g_type_init();
#endif

    skk_init();

    skk->context = skk_context_new(0, 0);

    if (!FcitxSkkLoadDictionary(skk) || !FcitxSkkLoadRule(skk)) {
        free(skk);
        return NULL;
    }
    skk_context_set_period_style(skk->context, SKK_PERIOD_STYLE_JA_JA);
    skk_context_set_input_mode(skk->context, SKK_INPUT_MODE_HIRAGANA);

    FcitxSkkApplyConfig(skk);

    FcitxInstanceRegisterIMv2(instance, skk, "skk", _("Skk"), "skk",
                              skk_iface, 1, "ja");

#define INIT_MENU(VARNAME, NAME, I18NNAME, STATUS_NAME, STATUS_ARRAY, SIZE) \
    do { \
        FcitxUIRegisterComplexStatus(instance, skk, \
            STATUS_NAME, \
            I18NNAME, \
            I18NNAME, \
            NULL, \
            FcitxSkkGet##NAME##IconName \
        ); \
        FcitxMenuInit(&VARNAME); \
        VARNAME.name = strdup(I18NNAME); \
        VARNAME.candStatusBind = strdup(STATUS_NAME); \
        VARNAME.UpdateMenu = FcitxSkkUpdate##NAME##Menu; \
        VARNAME.MenuAction = FcitxSkk##NAME##MenuAction; \
        VARNAME.priv = skk; \
        VARNAME.isSubMenu = false; \
        int i; \
        for (i = 0; i < SIZE; i ++) \
            FcitxMenuAddMenuItem(&VARNAME, _(STATUS_ARRAY[i].label), MENUTYPE_SIMPLE, NULL); \
        FcitxUIRegisterMenu(instance, &VARNAME); \
        FcitxUISetStatusVisable(instance, STATUS_NAME, false); \
    } while(0)

    INIT_MENU(skk->inputModeMenu, InputMode, _("Input Mode"), "skk-input-mode", input_mode_status, SKK_INPUT_MODE_LAST);

    skk->handler = g_signal_connect(skk->context, "notify::input-mode", G_CALLBACK(_skk_input_mode_changed_cb), skk);
    FcitxSkkUpdateInputMode(skk);
    skk->candidate_selected_handler = g_signal_connect(skk_context_get_candidates(skk->context), "selected", G_CALLBACK(skk_candidate_list_selected_cb), skk);
    skk->candidate_populated_handler = g_signal_connect(skk_context_get_candidates(skk->context), "populated", G_CALLBACK(skk_candidate_list_popuplated_cb), skk);
    skk->notify_preedit_handler = g_signal_connect(skk->context, "notify::preedit", G_CALLBACK(skk_candidate_update_preedit_cb), skk);
    skk->retrieve_surrounding_text_handler = g_signal_connect(skk->context, "retrieve_surrounding_text", G_CALLBACK(skk_context_retrieve_surrounding_text_cb), skk);
    skk->delete_surrounding_text_handler = g_signal_connect(skk->context, "delete_surrounding_text", G_CALLBACK(skk_context_delete_surrounding_text_cb), skk);


    gchar* AUTO_START_HENKAN_KEYWORDS[] = {
        "を", "、", "。", "．", "，", "？", "」",
        "！", "；", "：", ")", ";", ":", "）",
        "”", "】", "』", "》", "〉", "｝", "］",
        "〕", "}", "]", "?", ".", ",", "!"
    };
    skk_context_set_auto_start_henkan_keywords(skk->context, AUTO_START_HENKAN_KEYWORDS, G_N_ELEMENTS(AUTO_START_HENKAN_KEYWORDS));

    SkkRule* rule = skk_rule_new("default", NULL);
    if (rule) {
        skk_context_set_typing_rule(skk->context, rule);
    }

    FcitxIMEventHook hk;
    hk.arg = skk;
    hk.func = FcitxSkkResetHook;
    FcitxInstanceRegisterResetInputHook(instance, hk);

    return skk;
}



static void
FcitxSkkDestroy(void *arg)
{
    FcitxSkk *skk = (FcitxSkk*)arg;
    if (fcitx_unlikely(!arg))
        return;
    g_signal_handler_disconnect(skk_context_get_candidates(skk->context), skk->candidate_selected_handler);
    g_signal_handler_disconnect(skk_context_get_candidates(skk->context), skk->candidate_populated_handler);
    g_signal_handler_disconnect(skk->context, skk->handler);
    g_signal_handler_disconnect(skk->context, skk->notify_preedit_handler);
    g_signal_handler_disconnect(skk->context, skk->retrieve_surrounding_text_handler);
    g_signal_handler_disconnect(skk->context, skk->delete_surrounding_text_handler);
    g_object_unref(skk->context);
    free(arg);
}

static boolean
FcitxSkkInit(void *arg)
{
    FcitxSkk *skk = (FcitxSkk*)arg;
    if (!arg)
        return false;
    FcitxInstanceSetContext(skk->owner, CONTEXT_IM_KEYBOARD_LAYOUT, "ja");
    boolean flag = true;
    FcitxInstanceSetContext(skk->owner, CONTEXT_IM_KEYBOARD_LAYOUT, "jp");
    FcitxInstanceSetContext(skk->owner, CONTEXT_DISABLE_AUTOENG, &flag);
    FcitxInstanceSetContext(skk->owner, CONTEXT_DISABLE_QUICKPHRASE, &flag);
    FcitxInstanceSetContext(skk->owner, CONTEXT_DISABLE_FULLWIDTH, &flag);
    FcitxInstanceSetContext(skk->owner, CONTEXT_DISABLE_AUTO_FIRST_CANDIDATE_HIGHTLIGHT, &flag);
    return true;
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
    // Filter out unnecessary modifier bits
    // FIXME: should resolve virtual modifiers

    if (skk_candidate_list_get_page_visible(skk_context_get_candidates(skk->context))) {
        INPUT_RETURN_VALUE result = FcitxSkkDoCandidate (skk, sym, state);
        if (result == IRV_TO_PROCESS)
            return result;
    }

    SkkModifierType modifiers = (SkkModifierType) state & (FcitxKeyState_SimpleMask | (1 << 30));
    SkkKeyEvent* key = skk_key_event_new_from_x_keysym(sym, modifiers, NULL);
    if (!key)
        return IRV_TO_PROCESS;

    gboolean retval = skk_context_process_key_event(skk->context, key);
    gchar* output = skk_context_poll_output(skk->context);

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
    SkkCandidateList* skkCandList = skk_context_get_candidates(skk->context);
    int idx = *(int*) cand->priv;
    gboolean retval = skk_candidate_list_select_at(skkCandList, idx % skk_candidate_list_get_page_size(skkCandList));
    if (retval) {
        return IRV_DISPLAY_CANDWORDS;
    }

    return IRV_TO_PROCESS;
}

boolean FcitxSkkPaging(void* arg, boolean prev) {
    FcitxSkk *skk = (FcitxSkk*)arg;
    SkkCandidateList* skkCandList = skk_context_get_candidates(skk->context);
    boolean result;
    if (prev)
        result = skk_candidate_list_page_up(skkCandList);
    else
        result = skk_candidate_list_page_down(skkCandList);
    FcitxSkkGetCandWords(skk);
    return result;
}

static INPUT_RETURN_VALUE
FcitxSkkGetCandWords(void *arg)
{
    FcitxSkk *skk = (FcitxSkk*)arg;
    FcitxInstanceCleanInputWindow(skk->owner);
    FcitxInputState* input = FcitxInstanceGetInputState(skk->owner);
    FcitxCandidateWordList* candList = FcitxInputStateGetCandidateList(input);
    SkkCandidateList* skkCandList = skk_context_get_candidates(skk->context);
    FcitxCandidateWordSetChoose(candList, DIGIT_STR_CHOOSE);
    FcitxCandidateWordSetPageSize(candList, skk->config.pageSize);
    FcitxCandidateWordSetLayoutHint(candList, skk->config.candidateLayout);

    if (skk_candidate_list_get_page_visible(skkCandList)) {
        int i = 0;
        int j = 0;
        guint size = skk_candidate_list_get_size(skkCandList);
        gint cursor_pos = skk_candidate_list_get_cursor_pos(skkCandList);
        guint page_start = skk_candidate_list_get_page_start(skkCandList);
        guint page_size = skk_candidate_list_get_page_size(skkCandList);
        for (i = skk_candidate_list_get_page_start(skkCandList), j = 0; i < size; i ++, j++) {
            SkkCandidate* skkCandidate = skk_candidate_list_get(skkCandList, i);
            FcitxCandidateWord word;
            word.callback = FcitxSkkGetCandWord;
            word.extraType = MSG_OTHER;
            word.owner = skk;
            int* id = fcitx_utils_new(int);
            *id = j;
            word.priv = id;
            word.strExtra = NULL;
            if (skk->config.showAnnotation && skk_candidate_get_annotation(skkCandidate)) {
                fcitx_utils_alloc_cat_str(word.strExtra, " [", skk_candidate_get_annotation(skkCandidate), "]");
            }
            word.strWord = strdup(skk_candidate_get_text(skkCandidate));
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

    FcitxMessages* preedit = (FcitxInstanceICSupportPreedit(skk->owner, FcitxInstanceGetCurrentIC(skk->owner)))
                                ? FcitxInputStateGetClientPreedit(input)
                                : FcitxInputStateGetPreedit(input);


    const gchar* preeditString = skk_context_get_preedit(skk->context);
    size_t len = strlen(preeditString);
    if (len > 0) {
        guint offset, nchars;
        skk_context_get_preedit_underline(skk->context, &offset, &nchars);

        if (nchars > 0) {
            const gchar* preeditString = skk_context_get_preedit(skk->context);
            char* off = fcitx_utf8_get_nth_char(preeditString, offset);
            if (offset > 0) {
                char* left = strndup(preeditString, off - preeditString);
                FcitxMessagesAddMessageAtLast(preedit, MSG_OTHER, "%s", left);
                fcitx_utils_free(left);
            }
            char* right = fcitx_utf8_get_nth_char(off, nchars);
            char* middle = strndup(off, right - off);
            FcitxMessagesAddMessageAtLast(preedit, MSG_HIGHLIGHT, "%s", middle);
            fcitx_utils_free(middle);

            if (*right != 0) {
                FcitxMessagesAddMessageAtLast(preedit, MSG_OTHER, "%s", right);
            }
        }
        else {
            FcitxMessagesAddMessageAtLast(preedit, MSG_OTHER, "%s", preeditString);
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
    skk_context_reset(skk->context);
}

static void FcitxSkkReloadConfig(void* arg)
{
    FcitxSkk *skk = (FcitxSkk*)arg;
    SkkLoadConfig(&skk->config);
    FcitxSkkApplyConfig(skk);
    FcitxSkkLoadRule(skk);
    FcitxSkkLoadDictionary(skk);
}

void FcitxSkkApplyConfig(FcitxSkk* skk)
{
    SkkCandidateList* skkCandidates = skk_context_get_candidates(skk->context);
    skk_candidate_list_set_page_start(skkCandidates, skk->config.nTriggersToShowCandWin);
    skk_candidate_list_set_page_size(skkCandidates, skk->config.pageSize);
    skk_context_set_period_style(skk->context, skk->config.punctuationStyle);
    skk_context_set_egg_like_newline(skk->context, skk->config.eggLikeNewLine);
}
