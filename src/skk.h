/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */
#ifndef _FCITX_SKK_SKK_H_
#define _FCITX_SKK_SKK_H_

#include <memory>

#include <fcitx-config/configuration.h>
#include <fcitx-config/iniparser.h>
#include <fcitx-utils/capabilityflags.h>
#include <fcitx-utils/i18n.h>
#include <fcitx/action.h>
#include <fcitx/addonfactory.h>
#include <fcitx/addonmanager.h>
#include <fcitx/candidatelist.h>
#include <fcitx/inputcontextmanager.h>
#include <fcitx/inputcontextproperty.h>
#include <fcitx/inputmethodengine.h>
#include <fcitx/instance.h>
#include <fcitx/menu.h>
#include <libskk/libskk.h>

namespace fcitx {

enum class CandidateChooseKey { Digit, ABC, Qwerty };

FCITX_CONFIG_ENUM_NAME_WITH_I18N(CandidateChooseKey, N_("Digit (0,1,2,...)"),
                                 N_("ABC (a,b,c,...)"),
                                 N_("Qwerty Center Row (a,s,d,...)"));

FCITX_CONFIG_ENUM_NAME_WITH_I18N(CandidateLayoutHint, N_("Not set"),
                                 N_("Vertical"), N_("Horizontal"));

FCITX_CONFIG_ENUM_NAME_WITH_I18N(SkkPeriodStyle, N_("Japanese"), N_("Latin"),
                                 N_("Wide latin"), N_("Wide latin Japanese"));

FCITX_CONFIG_ENUM_NAME_WITH_I18N(SkkInputMode, N_("Hiragana"), N_("Katakana"),
                                 N_("Half width Katakana"), N_("Latin"),
                                 N_("Wide latin"));

static_assert(SkkInputModeI18NAnnotation::enumLength <= SKK_INPUT_MODE_LAST);

struct NotEmpty {
    bool check(const std::string &value) const { return !value.empty(); }
    void dumpDescription(RawConfig &) const {}
};

struct RuleAnnotation : public EnumAnnotation {
    void dumpDescription(RawConfig &config) const {
        EnumAnnotation::dumpDescription(config);
        int length;
        auto rules = skk_rule_list(&length);
        for (int i = 0; i < length; i++) {
            config.setValueByPath("Enum/" + std::to_string(i), rules[i].name);
            config.setValueByPath("EnumI18n/" + std::to_string(i),
                                  rules[i].label);
            skk_rule_metadata_destroy(&rules[i]);
        }
        g_free(rules);
    }
};

FCITX_CONFIGURATION(
    SkkConfig, Option<std::string, NotEmpty, DefaultMarshaller<std::string>,
                      RuleAnnotation>
                   rule{this, "Rule", _("Rule"), "default"};
    OptionWithAnnotation<SkkPeriodStyle, SkkPeriodStyleI18NAnnotation>
        punctuationStyle{this, "PunctuationStyle", _("Punctuation Style"),
                         SKK_PERIOD_STYLE_JA_JA};
    OptionWithAnnotation<SkkInputMode, SkkInputModeI18NAnnotation> inputMode{
        this, "InitialInputMode", _("Initial Input Mode"),
        SKK_INPUT_MODE_HIRAGANA};
    Option<int, IntConstrain> pageSize{this, "PageSize", _("Page size"), 7,
                                       IntConstrain(1, 10)};
    OptionWithAnnotation<CandidateLayoutHint, CandidateLayoutHintI18NAnnotation>
        candidateLayout{this, "Candidate Layout", _("Candidate Layout"),
                        CandidateLayoutHint::Vertical};
    Option<bool> eggLikeNewLine{
        this, "EggLikeNewLine",
        _("Return-key does not insert new line on commit"), false};
    Option<bool> showAnnotation{this, "ShowAnnotation", _("Show Annotation"),
                                true};
    OptionWithAnnotation<CandidateChooseKey, CandidateChooseKeyI18NAnnotation>
        candidateChooseKey{this, "CandidateChooseKey", _("Candidate Key"),
                           CandidateChooseKey::Digit};
    KeyListOption prevPageKey{
        this,
        "CandidatesPageUpKey",
        _("Candidates Page Up"),
        {Key(FcitxKey_Page_Up)},
        KeyListConstrain({KeyConstrainFlag::AllowModifierLess})};
    KeyListOption nextPageKey{
        this,
        "CandidatesPageDownKey",
        _("Candidates Page Down"),
        {Key(FcitxKey_Page_Down)},
        KeyListConstrain({KeyConstrainFlag::AllowModifierLess})};
    KeyListOption cursorUpKey{
        this,
        "CursorUp",
        _("Cursor Up"),
        {Key(FcitxKey_Up)},
        KeyListConstrain({KeyConstrainFlag::AllowModifierLess})};
    KeyListOption cursorDownKey{
        this,
        "CursorDown",
        _("Cursor Down"),
        {Key(FcitxKey_Down)},
        KeyListConstrain({KeyConstrainFlag::AllowModifierLess})};
    Option<int, IntConstrain> nTriggersToShowCandWin{
        this, "NTriggersToShowCandWin",
        _("Number candidate of Triggers To Show Candidate Window"), 4,
        IntConstrain(0, 7)};
    ExternalOption dictionary{this, "Dict", _("Dictionary"),
                              "fcitx://config/addon/skk/dictionary_list"};);

template <typename T>
using GObjectUniquePtr = UniqueCPtr<T, g_object_unref>;

class SkkState;

class SkkEngine final : public InputMethodEngine {
public:
    SkkEngine(Instance *instance);
    ~SkkEngine();

    const Configuration *getConfig() const override { return &config_; }
    void activate(const InputMethodEntry &entry,
                  InputContextEvent &event) override;
    void deactivate(const fcitx::InputMethodEntry &entry,
                    fcitx::InputContextEvent &event) override;
    void keyEvent(const InputMethodEntry &entry, KeyEvent &keyEvent) override;
    void reloadConfig() override;
    void reset(const InputMethodEntry &entry,
               InputContextEvent &event) override;
    void save() override;
    std::string subMode(const InputMethodEntry &, InputContext &) override;

    auto &factory() { return factory_; }
    auto &config() { return config_; }
    auto instance() { return instance_; }
    void setConfig(const RawConfig &config) override {
        config_.load(config, true);
        safeSaveAsIni(config_, "conf/skk.conf");
        reloadConfig();
    }
    void setSubConfig(const std::string &path,
                      const fcitx::RawConfig &) override {
        if (path == "dictionary_list") {
            reloadConfig();
        }
    }

    SkkState *state(InputContext *ic) { return ic->propertyFor(&factory_); }

    const auto &dictionaries() { return dictionaries_; }
    auto modeAction() { return modeAction_.get(); }
    auto userRule() { return userRule_.get(); }

private:
    void loadRule();
    void loadDictionary();

    Instance *instance_;
    FactoryFor<SkkState> factory_;
    SkkConfig config_;
    std::vector<GObjectUniquePtr<SkkDict>> dictionaries_;
    std::vector<GObjectUniquePtr<SkkDict>> dummyEmptyDictionaries_;
    GObjectUniquePtr<SkkRule> userRule_;

    std::unique_ptr<Action> modeAction_;
    std::unique_ptr<Menu> menu_;
    std::vector<std::unique_ptr<Action>> subModeActions_;
};

class SkkAddonFactory final : public AddonFactory {
public:
    AddonInstance *create(AddonManager *manager) override {
        registerDomain("fcitx5-skk", FCITX_INSTALL_LOCALEDIR);
        return new SkkEngine(manager->instance());
    }
};

class SkkState final : public InputContextProperty {
public:
    SkkState(SkkEngine *engine, InputContext *ic);
    ~SkkState();

    void keyEvent(KeyEvent &keyEvent);
    void updateUI();
    SkkContext *context() { return context_.get(); }
    void applyConfig();
    bool needCopy() const override { return true; }
    void copyTo(InputContextProperty *state) override;

private:
    bool handleCandidate(KeyEvent &keyEvent);
    void updateInputMode();

    // callbacks and their handlers
    static void input_mode_changed_cb(GObject *gobject, GParamSpec *pspec,
                                      SkkState *skk);
    static gboolean retrieve_surrounding_text_cb(GObject *, gchar **text,
                                                 guint *cursor_pos,
                                                 SkkState *skk);
    static gboolean delete_surrounding_text_cb(GObject *, gint offset,
                                               guint nchars, SkkState *skk);

    SkkEngine *engine_;
    InputContext *ic_;
    GObjectUniquePtr<SkkContext> context_;
    bool modeChanged_ = false;
    SkkInputMode lastMode_ = SKK_INPUT_MODE_DEFAULT;
    bool lastIsEmpty_ = true;
};

} // namespace fcitx

#endif // _FCITX_SKK_SKK_H_
