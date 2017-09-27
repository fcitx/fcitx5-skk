#include "skk.hpp"
namespace fcitx {
    SkkState::SkkState(SkkEngine *engine):
        context_(skk_context_new(0, 0), &g_object_unref)
    {
        skk_context_set_period_style(context_.get(), SKK_PERIOD_STYLE_JA_JA);
        skk_context_set_input_mode(context_.get(), SKK_INPUT_MODE_HIRAGANA);

    
        gchar* AUTO_START_HENKAN_KEYWORDS[] = {
            "を", "、", "。", "．", "，", "？", "」",
            "！", "；", "：", ")", ";", ":", "）",
            "”", "】", "』", "》", "〉", "｝", "］",
            "〕", "}", "]", "?", ".", ",", "!"
        };
        
        skk_context_set_auto_start_henkan_keywords(context_.get(), 
                                                   AUTO_START_HENKAN_KEYWORDS,        
                                                   G_N_ELEMENTS(AUTO_START_HENKAN_KEYWORDS));
    }
    
    SkkEngine::SkkEngine(Instance *instance):
        instance_{instance},
        factory_([this](InputContext &) { return new SkkState(this); }) 
    {
        skk_init();
       
        instance_ -> inputContextManager().registerProperty("skkState", &factory_);

    

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


FCITX_ADDON_FACTORY(fcitx::SkkAddonFactory)
