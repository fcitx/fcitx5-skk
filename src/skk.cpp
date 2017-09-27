#include "skk.hpp"
namespace fcitx {
    SkkEngine::SkkEngine(Instance *instance):
        instance_{instance},
        factory_([this](InputContext &) { return new SkkState(this); }) 
    {
        skk_context_set_period_style(context_, SKK_PERIOD_STYLE_JA_JA);
        skk_context_set_input_mode(context_, SKK_INPUT_MODE_HIRAGANA);

        instance_ -> inputContextManager().registerProperty("skkState", &factory_);
    
        wchar_t* AUTO_START_HENKAN_KEYWORDS[] = {
            "を", "、", "。", "．", "，", "？", "」",
            "！", "；", "：", ")", ";", ":", "）",
            "”", "】", "』", "》", "〉", "｝", "］",
            "〕", "}", "]", "?", ".", ",", "!"
        };
        
        skk_context_set_auto_start_henkan_keywords(context_, 
                                                   AUTO_START_HENKAN_KEYWORDS,        
                                                   G_N_ELEMENTS(AUTO_START_HENKAN_KEYWORDS));

    

    }
    
    void SkkEngine::activate(const InputMethodEntry &entry, InputContextEvent &event){
        // FIXME 
    }
    
    void SkkEngine::keyEvent(const InputMethodEntry &entry, KeyEvent &keyEvent) {
        // FIXME
        
    }
    
    void SkkEngine::reloadConfig() {
        // FIXME
    }
    void SkkEngine::reset(const InputMethodEntry &entry, InputContextEvent &event) {
        // FIXME
    }
    void SkkEngine::save() {
        // FIXME
    }

    SkkEngine::~SkkEngine() {}
}


FCITX_ADDON_FACTORY(SkkAddonFactory)
