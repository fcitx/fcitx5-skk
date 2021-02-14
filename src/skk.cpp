/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */
#include "skk.h"
#include <fcntl.h>
#include <stddef.h>
#include <fcitx-config/iniparser.h>
#include <fcitx-utils/log.h>
#include <fcitx-utils/utf8.h>
#include <fcitx/inputpanel.h>
#include <fcitx/userinterfacemanager.h>

FCITX_DEFINE_LOG_CATEGORY(skk_logcategory, "skk");

#define SKK_DEBUG() FCITX_LOGC(skk_logcategory, Debug)

namespace fcitx {

namespace {

Text skkContextGetPreedit(SkkContext *context) {
    Text preedit;

    const gchar *preeditString = skk_context_get_preedit(context);
    size_t len = strlen(preeditString);
    if (len > 0) {
        guint offset, nchars;
        skk_context_get_preedit_underline(context, &offset, &nchars);

        if (nchars > 0) {
            const gchar *preeditString = skk_context_get_preedit(context);
            const char *off = utf8::nextNChar(preeditString, offset);
            if (offset > 0) {
                preedit.append(std::string(preeditString, off),
                               TextFormatFlag::Underline);
            }
            const char *right = utf8::nextNChar(off, nchars);
            preedit.append(
                std::string(off, right),
                {TextFormatFlag::HighLight, TextFormatFlag::Underline});

            if (*right != 0) {
                preedit.append(right, TextFormatFlag::Underline);
            }
        } else {
            preedit.append(preeditString, TextFormatFlag::Underline);
        }
    }

    preedit.setCursor(len);
    return preedit;
}

} // namespace

struct {
    const char *icon;
    const char *label;
    const char *description;
} input_mode_status[] = {
    {"", "\xe3\x81\x82", N_("Hiragana")},
    {"", "\xe3\x82\xa2", N_("Katakana")},
    {"", "\xef\xbd\xb1", N_("Half width Katakana")},
    {"", "A_", N_("Latin")},
    {"", "\xef\xbc\xa1", N_("Wide latin")},
    {"", "A", N_("Direct input")},
};

auto inputModeStatus(SkkEngine *engine, InputContext *ic) {
    auto state = engine->state(ic);
    auto mode = skk_context_get_input_mode(state->context());
    return (mode >= 0 && mode < FCITX_ARRAY_SIZE(input_mode_status))
               ? &input_mode_status[mode]
               : nullptr;
}

class SkkModeAction : public Action {
public:
    SkkModeAction(SkkEngine *engine) : engine_(engine) {}

    std::string shortText(InputContext *ic) const override {
        if (auto status = inputModeStatus(engine_, ic)) {
            return stringutils::concat(status->label, " - ",
                                       _(status->description));
        }
        return "";
    }
    std::string longText(InputContext *ic) const override {
        if (auto status = inputModeStatus(engine_, ic)) {
            return _(status->description);
        }
        return "";
    }
    std::string icon(InputContext *ic) const override {
        if (auto status = inputModeStatus(engine_, ic)) {
            return status->icon;
        }
        return "";
    }

private:
    SkkEngine *engine_;
};

class SkkModeSubAction : public SimpleAction {
public:
    SkkModeSubAction(SkkEngine *engine, SkkInputMode mode)
        : engine_(engine), mode_(mode) {
        setShortText(
            stringutils::concat(input_mode_status[mode].label, " - ",
                                _(input_mode_status[mode].description)));
        setLongText(_(input_mode_status[mode].description));
        setIcon(input_mode_status[mode].icon);
        setCheckable(true);
    }
    bool isChecked(InputContext *ic) const override {
        auto state = engine_->state(ic);
        return mode_ == skk_context_get_input_mode(state->context());
    }
    void activate(InputContext *ic) override {
        auto state = engine_->state(ic);
        skk_context_set_input_mode(state->context(), mode_);
    }

private:
    SkkEngine *engine_;
    SkkInputMode mode_;
};

class SkkCandidateWord : public CandidateWord {
public:
    SkkCandidateWord(SkkEngine *engine, Text text, int idx)
        : CandidateWord(), engine_(engine), idx_(idx) {
        setText(std::move(text));
    }

    void select(InputContext *inputContext) const override {
        auto state = engine_->state(inputContext);
        auto context = state->context();
        SkkCandidateList *skkCandidates = skk_context_get_candidates(context);
        if (skk_candidate_list_select_at(
                skkCandidates,
                idx_ % skk_candidate_list_get_page_size(skkCandidates))) {
            state->updateUI();
        }
    }

private:
    SkkEngine *engine_;
    int idx_;
};

class SkkFcitxCandidateList : public CandidateList,
                              public PageableCandidateList,
                              public CursorMovableCandidateList {
public:
    SkkFcitxCandidateList(SkkEngine *engine, InputContext *ic)
        : engine_(engine), ic_(ic) {
        setPageable(this);
        setCursorMovable(this);
        auto skkstate = engine_->state(ic_);
        auto context = skkstate->context();
        SkkCandidateList *skkCandidates = skk_context_get_candidates(context);
        gint size = skk_candidate_list_get_size(skkCandidates);
        gint cursor_pos = skk_candidate_list_get_cursor_pos(skkCandidates);
        guint page_start = skk_candidate_list_get_page_start(skkCandidates);
        guint page_size = skk_candidate_list_get_page_size(skkCandidates);

        // Assume size = 27, cursor = 14, page_start = 4, page_size = 10
        // 0~3 not in page.
        // 4~13 1st page
        // 14~23 2nd page
        // 24~26 3nd page
        int currentPage = (cursor_pos - page_start) / page_size;
        int totalPage = (size - page_start + page_size - 1) / page_size;
        int pageFirst = currentPage * page_size + page_start;
        int pageLast = std::min(size, static_cast<int>(pageFirst + page_size));

        for (int i = pageFirst; i < pageLast; i++) {
            GObjectUniquePtr<SkkCandidate> skkCandidate{
                skk_candidate_list_get(skkCandidates, i)};
            Text text;
            text.append(skk_candidate_get_text(skkCandidate.get()));
            if (*engine->config().showAnnotation) {
                auto annotation =
                    skk_candidate_get_annotation(skkCandidate.get());
                // Make sure annotation is not null, empty, or equal to "?".
                // ? seems to be a special debug purpose value.
                if (annotation && annotation[0] &&
                    g_strcmp0(annotation, "?") != 0) {
                    text.append(stringutils::concat(
                        " [", skk_candidate_get_annotation(skkCandidate.get()),
                        "]"));
                }
            }
            if (i == cursor_pos) {
                cursorIndex_ = i - pageFirst;
            }

            constexpr char labels[3][11] = {
                "1234567890",
                "abcdefghij",
                "asdfghjkl;",
            };

            char label[2] = {labels[static_cast<int>(
                                 engine_->config().candidateChooseKey.value())]
                                   [(i - pageFirst) % 10],
                             '\0'};

            labels_.emplace_back(stringutils::concat(label, ". "));
            words_.emplace_back(std::make_unique<SkkCandidateWord>(
                engine, text, i - page_start));
        }

        hasPrev_ = currentPage != 0;
        hasNext_ = currentPage + 1 < totalPage;
    }

    bool hasPrev() const override { return hasPrev_; }

    bool hasNext() const override { return hasNext_; }

    void prev() override { return paging(true); }

    void next() override { return paging(false); }

    bool usedNextBefore() const override { return true; }

    void prevCandidate() override { return moveCursor(true); }

    void nextCandidate() override { return moveCursor(false); }

    const Text &label(int idx) const override { return labels_[idx]; }

    const CandidateWord &candidate(int idx) const override {
        return *words_[idx];
    }

    int size() const override { return words_.size(); }

    int cursorIndex() const override { return cursorIndex_; }

    CandidateLayoutHint layoutHint() const override {
        return *engine_->config().candidateLayout;
    }

private:
    void paging(bool prev) {
        auto skkstate = engine_->state(ic_);
        auto context = skkstate->context();
        SkkCandidateList *skkCandidates = skk_context_get_candidates(context);
        if (skk_candidate_list_get_page_visible(skkCandidates)) {
            if (prev) {
                skk_candidate_list_page_up(skkCandidates);
            } else {
                skk_candidate_list_page_down(skkCandidates);
            }
            skkstate->updateUI();
        }
    }
    void moveCursor(bool prev) {
        auto skkstate = engine_->state(ic_);
        auto context = skkstate->context();
        SkkCandidateList *skkCandidates = skk_context_get_candidates(context);
        if (skk_candidate_list_get_page_visible(skkCandidates)) {
            if (prev) {
                skk_candidate_list_cursor_up(skkCandidates);
            } else {
                skk_candidate_list_cursor_down(skkCandidates);
            }
            skkstate->updateUI();
        }
    }

    SkkEngine *engine_;
    InputContext *ic_;
    std::vector<Text> labels_;
    std::vector<std::unique_ptr<SkkCandidateWord>> words_;
    int cursorIndex_ = -1;
    bool hasPrev_ = false;
    bool hasNext_ = false;
};

/////////////////////////////////////////////////////////////////////////////////////
/// SkkEngine

SkkEngine::SkkEngine(Instance *instance)
    : instance_{instance}, factory_([this](InputContext &ic) {
          auto newState = new SkkState(this, &ic);
          newState->applyConfig();
          return newState;
      }) {
    skk_init();

    modeAction_ = std::make_unique<SkkModeAction>(this);
    menu_ = std::make_unique<Menu>();
    modeAction_->setMenu(menu_.get());

    instance_->userInterfaceManager().registerAction("skk-input-mode",
                                                     modeAction_.get());

#define _ADD_ACTION(MODE, NAME)                                                \
    subModeActions_.emplace_back(                                              \
        std::make_unique<SkkModeSubAction>(this, MODE));                       \
    instance_->userInterfaceManager().registerAction(                          \
        NAME, subModeActions_.back().get());

    _ADD_ACTION(SKK_INPUT_MODE_HIRAGANA, "skk-input-mode-hiragana");
    _ADD_ACTION(SKK_INPUT_MODE_KATAKANA, "skk-input-mode-katakana");
    _ADD_ACTION(SKK_INPUT_MODE_HANKAKU_KATAKANA,
                "skk-input-mode-hankaku-katakana");
    _ADD_ACTION(SKK_INPUT_MODE_LATIN, "skk-input-mode-latin");
    _ADD_ACTION(SKK_INPUT_MODE_WIDE_LATIN, "skk-input-mode-wide-latin");
    for (auto &subModeAction : subModeActions_) {
        menu_->addAction(subModeAction.get());
    }

    reloadConfig();

    if (!userRule_) {
        throw std::runtime_error("Failed to load any skk rule.");
    }

    instance_->inputContextManager().registerProperty("skkState", &factory_);
    instance_->inputContextManager().foreach([this](InputContext *ic) {
        auto state = this->state(ic);
        skk_context_set_input_mode(state->context(), *config_.inputMode);
        ic->updateProperty(&factory_);
        return true;
    });
}

void SkkEngine::activate(const InputMethodEntry &entry,
                         InputContextEvent &event) {
    FCITX_UNUSED(entry);

    auto &statusArea = event.inputContext()->statusArea();
    statusArea.addAction(StatusGroup::InputMethod, modeAction_.get());
}

void SkkEngine::deactivate(const InputMethodEntry &entry,
                           InputContextEvent &event) {
    if (event.type() == EventType::InputContextSwitchInputMethod) {
        auto skkstate = this->state(event.inputContext());
        auto context = skkstate->context();
        auto text = skkContextGetPreedit(context);
        auto str = text.toString();
        if (!str.empty()) {
            event.inputContext()->commitString(str);
        }
    }
    reset(entry, event);
}

void SkkEngine::keyEvent(const InputMethodEntry &entry, KeyEvent &keyEvent) {
    FCITX_UNUSED(entry);

    auto ic = keyEvent.inputContext();
    auto state = ic->propertyFor(&factory_);
    state->keyEvent(keyEvent);
}

void SkkEngine::reloadConfig() {
    readAsIni(config_, "conf/skk.conf");

    loadDictionary();
    loadRule();

    if (factory_.registered()) {
        instance_->inputContextManager().foreach([this](InputContext *ic) {
            auto state = this->state(ic);
            state->applyConfig();
            return true;
        });
    }
}
void SkkEngine::reset(const InputMethodEntry &entry, InputContextEvent &event) {
    FCITX_UNUSED(entry);
    auto state = this->state(event.inputContext());
    auto context = state->context();
    skk_context_reset(context);
    state->updateUI();
}
void SkkEngine::save() {}

std::string SkkEngine::subMode(const InputMethodEntry &, InputContext &ic) {
    if (auto status = inputModeStatus(this, &ic)) {
        return _(status->description);
    }
    return "";
}

void SkkEngine::loadRule() {
    UniqueCPtr<SkkRuleMetadata, skk_rule_metadata_free> meta{
        skk_rule_find_rule(config_.rule->data())};

    GObjectUniquePtr<SkkRule> rule;

    if (meta) {
        rule.reset(skk_rule_new(meta->name, nullptr));
    }
    if (!rule || !meta) {
        FCITX_LOGC(skk_logcategory, Error)
            << "Failed to load rule: " << config_.rule->data();
        meta.reset(skk_rule_find_rule("default"));
        if (meta) {
            rule.reset(skk_rule_new(meta->name, nullptr));
        }
    }

    if (!rule) {
        return;
    }
    userRule_ = std::move(rule);
}

typedef enum _FcitxSkkDictType {
    FSDT_Invalid,
    FSDT_File,
    FSTD_Server
} FcitxSkkDictType;

void SkkEngine::loadDictionary() {
    dictionaries_.clear();
    auto file = StandardPath::global().open(StandardPath::Type::PkgData,
                                            "skk/dictionary_list", O_RDONLY);

    UniqueFilePtr fp(fdopen(file.fd(), "rb"));
    if (!fp) {
        return;
    }

    UniqueCPtr<char> buf;
    size_t len = 0;

    while (getline(buf, &len, fp.get()) != -1) {
        const auto trimmed = stringutils::trim(buf.get());
        const auto tokens = stringutils::split(trimmed, ",");

        if (tokens.size() < 3) {
            continue;
        }

        SKK_DEBUG() << "Load dictionary: " << trimmed;

        FcitxSkkDictType type = FSDT_Invalid;
        int mode = 0;
        std::string path;
        std::string host;
        std::string port;
        std::string encoding;
        for (auto &token : tokens) {
            auto equal = token.find('=');
            if (equal == std::string::npos) {
                continue;
            }

            auto key = token.substr(0, equal);
            auto value = token.substr(equal + 1);

            if (key == "type") {
                if (value == "file") {
                    type = FSDT_File;
                } else if (value == "server") {
                    type = FSTD_Server;
                }
            } else if (key == "file") {
                path = value;
            } else if (key == "mode") {
                if (value == "readonly") {
                    mode = 1;
                } else if (value == "readwrite") {
                    mode = 2;
                }
            } else if (key == "host") {
                host = value;
            } else if (key == "port") {
                port = value;
            } else if (key == "encoding") {
                encoding = value;
            }
        }

        encoding = !encoding.empty() ? encoding : "EUC-JP";

        if (type == FSDT_Invalid) {
            continue;
        } else if (type == FSDT_File) {
            if (path.empty() || mode == 0) {
                continue;
            }
            if (mode == 1) {
                if (stringutils::endsWith(path, ".cdb")) {
                    SkkCdbDict *dict =
                        skk_cdb_dict_new(path.data(), encoding.data(), nullptr);
                    if (dict) {
                        SKK_DEBUG() << "Adding cdb dict: " << path;
                        dictionaries_.emplace_back(SKK_DICT(dict));
                    }
                } else {
                    SkkFileDict *dict = skk_file_dict_new(
                        path.data(), encoding.data(), nullptr);
                    if (dict) {
                        SKK_DEBUG() << "Adding file dict: " << path;
                        dictionaries_.emplace_back(SKK_DICT(dict));
                    }
                }
            } else {
                constexpr char configDir[] = "$FCITX_CONFIG_DIR/";
                constexpr auto len = sizeof(configDir) - 1;
                std::string realpath = path;
                if (stringutils::startsWith(path, configDir)) {
                    realpath = stringutils::joinPath(
                        StandardPath::global().userDirectory(
                            StandardPath::Type::PkgData),
                        path.substr(len));
                }
                SkkUserDict *userdict = skk_user_dict_new(
                    realpath.data(), encoding.data(), nullptr);
                if (userdict) {
                    SKK_DEBUG() << "Adding user dict: " << realpath;
                    dictionaries_.emplace_back(SKK_DICT(userdict));
                }
            }
        } else if (type == FSTD_Server) {
            host = !host.empty() ? host : "localhost";
            port = !port.empty() ? port : "1178";

            int iPort = 0;
            try {
                iPort = std::stoi(port);
                if (iPort <= 0 || iPort > UINT16_MAX) {
                    continue;
                }
            } catch (...) {
                continue;
            }

            SkkSkkServ *dict =
                skk_skk_serv_new(host.data(), iPort, encoding.data(), nullptr);
            if (dict) {
                SKK_DEBUG() << "Adding server: " << host << ":" << iPort << " "
                            << encoding;
                dictionaries_.emplace_back(SKK_DICT(dict));
            }
        }
    }
}

SkkEngine::~SkkEngine() {}

/////////////////////////////////////////////////////////////////////////////////////
/// SkkState

SkkState::SkkState(SkkEngine *engine, InputContext *ic)
    : engine_(engine), ic_(ic), context_(skk_context_new(nullptr, 0)) {
    SkkContext *context = context_.get();
    skk_context_set_period_style(context, *engine_->config().punctuationStyle);
    skk_context_set_input_mode(context, *engine_->config().inputMode);

    lastMode_ = skk_context_get_input_mode(context);
    g_signal_connect(context, "notify::input-mode",
                     G_CALLBACK(SkkState::input_mode_changed_cb), this);
    g_signal_connect(context, "retrieve_surrounding_text",
                     G_CALLBACK(retrieve_surrounding_text_cb), this);
    g_signal_connect(context, "delete_surrounding_text",
                     G_CALLBACK(delete_surrounding_text_cb), this);
    updateInputMode();

    const char *AUTO_START_HENKAN_KEYWORDS[] = {
        "を", "、", "。", "．", "，", "？", "」", "！", "；", "：",
        ")",  ";",  ":",  "）", "”",  "】", "』", "》", "〉", "｝",
        "］", "〕", "}",  "]",  "?",  ".",  ",",  "!"};

    skk_context_set_auto_start_henkan_keywords(
        context, const_cast<gchar **>(AUTO_START_HENKAN_KEYWORDS),
        G_N_ELEMENTS(AUTO_START_HENKAN_KEYWORDS));
}

SkkState::~SkkState() {
    g_signal_handlers_disconnect_by_data(context_.get(), this);
}

void SkkState::keyEvent(KeyEvent &keyEvent) {
    if (handleCandidate(keyEvent)) {
        return;
    }

    uint32_t modifiers = static_cast<uint32_t>(keyEvent.rawKey().states() &
                                               KeyState::SimpleMask);

    if (keyEvent.isRelease()) {
        modifiers |= SKK_MODIFIER_TYPE_RELEASE_MASK;
    }

    GObjectUniquePtr<SkkKeyEvent> key{skk_key_event_new_from_x_keysym(
        keyEvent.rawKey().sym(), static_cast<SkkModifierType>(modifiers),
        nullptr)};
    if (!key) {
        return;
    }

    modeChanged_ = false;
    if (skk_context_process_key_event(context_.get(), key.get())) {
        keyEvent.filterAndAccept();
    }

    updateUI();
    if (modeChanged_) {
        ic_->updateProperty(&engine_->factory());
    }
}

bool SkkState::handleCandidate(KeyEvent &keyEvent) {
    auto &config = engine_->config();
    auto context = context_.get();
    SkkCandidateList *skkCandidates = skk_context_get_candidates(context);
    if (!skk_candidate_list_get_page_visible(skkCandidates) ||
        keyEvent.isRelease()) {
        return false;
    }
    if (keyEvent.key().checkKeyList(*config.cursorUpKey)) {
        skk_candidate_list_cursor_up(skkCandidates);
        keyEvent.filterAndAccept();
    } else if (keyEvent.key().checkKeyList(*config.cursorDownKey)) {
        skk_candidate_list_cursor_down(skkCandidates);
        keyEvent.filterAndAccept();
    } else if (keyEvent.key().checkKeyList(*config.prevPageKey)) {
        skk_candidate_list_page_up(skkCandidates);
        keyEvent.filterAndAccept();
    } else if (keyEvent.key().checkKeyList(*config.nextPageKey)) {
        skk_candidate_list_page_down(skkCandidates);
        keyEvent.filterAndAccept();
    } else {
        KeyList selectionKeys;

        std::array<KeySym, 10> syms = {
            FcitxKey_1, FcitxKey_2, FcitxKey_3, FcitxKey_4, FcitxKey_5,
            FcitxKey_6, FcitxKey_7, FcitxKey_8, FcitxKey_9, FcitxKey_0,
        };
        if (*config.candidateChooseKey == CandidateChooseKey::ABC) {
            syms = {
                FcitxKey_a, FcitxKey_b, FcitxKey_c, FcitxKey_d, FcitxKey_e,
                FcitxKey_f, FcitxKey_g, FcitxKey_h, FcitxKey_i, FcitxKey_j,
            };
        } else if (*config.candidateChooseKey == CandidateChooseKey::Qwerty) {
            syms = {
                FcitxKey_a, FcitxKey_s,         FcitxKey_d, FcitxKey_f,
                FcitxKey_g, FcitxKey_h,         FcitxKey_j, FcitxKey_k,
                FcitxKey_l, FcitxKey_semicolon,
            };
        }

        KeyStates states;
        for (auto sym : syms) {
            selectionKeys.emplace_back(sym, states);
        }
        if (auto idx = keyEvent.key().keyListIndex(selectionKeys); idx >= 0) {
            skk_candidate_list_select_at(
                skkCandidates,
                idx % skk_candidate_list_get_page_size(skkCandidates));
            keyEvent.filterAndAccept();
        }
    }

    if (keyEvent.filtered()) {
        updateUI();
    }
    return keyEvent.filtered();
}

void SkkState::updateUI() {
    auto &inputPanel = ic_->inputPanel();
    auto context = context_.get();

    SkkCandidateList *skkCandidates = skk_context_get_candidates(context);

    std::unique_ptr<SkkFcitxCandidateList> candidateList;
    if (skk_candidate_list_get_page_visible(skkCandidates)) {
        candidateList = std::make_unique<SkkFcitxCandidateList>(engine_, ic_);
    }

    if (auto str = UniqueCPtr<char, g_free>{skk_context_poll_output(context)}) {
        if (str && str.get()[0]) {
            ic_->commitString(str.get());
        }
    }
    Text preedit = skkContextGetPreedit(context);

    // Skk almost filter every key, which makes it calls updateUI on release.
    // We add an additional check here for checking if the UI is empty or not.
    // If previous state is empty and the current state is also empty, we'll
    // ignore the UI update. This makes the input method info not disappear
    // immediately up key release.
    bool lastIsEmpty = lastIsEmpty_;
    bool newIsEmpty = preedit.empty() && !candidateList;
    lastIsEmpty_ = newIsEmpty;

    // Ensure we are not composing any text.
    if (modeChanged_ && newIsEmpty) {
        inputPanel.reset();
        engine_->instance()->showInputMethodInformation(ic_);
        return;
    }

    if (lastIsEmpty && newIsEmpty) {
        return;
    }

    inputPanel.reset();
    if (candidateList) {
        inputPanel.setCandidateList(std::move(candidateList));
    }

    if (ic_->capabilityFlags().test(CapabilityFlag::Preedit)) {
        inputPanel.setClientPreedit(preedit);
        ic_->updatePreedit();
    } else {
        inputPanel.setPreedit(preedit);
    }

    ic_->updateUserInterface(UserInterfaceComponent::InputPanel);
}

void SkkState::applyConfig() {
    auto &config = engine_->config();
    SkkCandidateList *skkCandidates = skk_context_get_candidates(context());
    skk_candidate_list_set_page_start(skkCandidates,
                                      *config.nTriggersToShowCandWin);
    skk_candidate_list_set_page_size(skkCandidates, *config.pageSize);
    skk_context_set_period_style(context(), *config.punctuationStyle);
    skk_context_set_egg_like_newline(context(), *config.eggLikeNewLine);
    skk_context_set_typing_rule(context(), engine_->userRule());

    std::vector<SkkDict *> dicts;
    dicts.reserve(engine_->dictionaries().size());
    for (auto &dict : engine_->dictionaries()) {
        dicts.push_back(dict.get());
    }
    skk_context_set_dictionaries(context(), dicts.data(), dicts.size());
}
void SkkState::copyTo(InputContextProperty *property) {
    auto otherState = static_cast<SkkState *>(property);
    skk_context_set_input_mode(otherState->context(),
                               skk_context_get_input_mode(context()));
}

void SkkState::updateInputMode() {
    engine_->modeAction()->update(ic_);
    auto newMode = skk_context_get_input_mode(context());
    if (lastMode_ != newMode) {
        lastMode_ = newMode;
        modeChanged_ = true;
    }
}

void SkkState::input_mode_changed_cb(GObject *, GParamSpec *, SkkState *skk) {
    skk->updateInputMode();
}

gboolean SkkState::retrieve_surrounding_text_cb(GObject *, gchar **text,
                                                guint *cursor_pos,
                                                SkkState *skk) {
    InputContext *ic = skk->ic_;
    if (!(ic->capabilityFlags().test(CapabilityFlag::SurroundingText)) ||
        !ic->surroundingText().isValid())
        return false;

    *text = g_strdup(ic->surroundingText().selectedText().c_str());
    *cursor_pos = ic->surroundingText().cursor();

    return true;
}

gboolean SkkState::delete_surrounding_text_cb(GObject *, gint offset,
                                              guint nchars, SkkState *skk) {
    InputContext *ic = skk->ic_;
    if (!(ic->capabilityFlags().test(CapabilityFlag::SurroundingText)))
        return false;
    ic->deleteSurroundingText(offset, nchars);
    return true;
}
} // namespace fcitx

FCITX_ADDON_FACTORY(fcitx::SkkAddonFactory)
