#include "skk.hpp"
namespace fcitx {
    SkkState::SkkState(SkkEngine *engine):
        engine_(engine),
        context_(skk_context_new(0, 0), &g_object_unref)
    {
        SkkContext * context = context_.get();
        skk_context_set_period_style(context, SKK_PERIOD_STYLE_JA_JA);
        skk_context_set_input_mode(context, SKK_INPUT_MODE_HIRAGANA);

        auto handler = g_signal_connect(context, "notify::input-mode", 
                G_CALLBACK(SkkState::input_mode_changed_cb), this);
        
        //FcitxSkkUpdateInputMode(this);
        //auto candidate_selected_handler = g_signal_connect(skk_context_get_candidates(context), "selected", G_CALLBACK(skk_candidate_list_selected_cb), this);
        //auto candidate_populated_handler = g_signal_connect(skk_context_get_candidates(context), "populated", G_CALLBACK(skk_candidate_list_popuplated_cb), this);
        //auto notify_preedit_handler = g_signal_connect(context, "notify::preedit", G_CALLBACK(skk_candidate_update_preedit_cb), this);
        //auto retrieve_surrounding_text_handler = g_signal_connect(context, "retrieve_surrounding_text", G_CALLBACK(skk_context_retrieve_surrounding_text_cb), this);
        //auto delete_surrounding_text_handler = g_signal_connect(context, "delete_surrounding_text", G_CALLBACK(skk_context_delete_surrounding_text_cb), this);
    
        gchar* AUTO_START_HENKAN_KEYWORDS[] = {
            "を", "、", "。", "．", "，", "？", "」",
            "！", "；", "：", ")", ";", ":", "）",
            "”", "】", "』", "》", "〉", "｝", "］",
            "〕", "}", "]", "?", ".", ",", "!"
        };
        
        skk_context_set_auto_start_henkan_keywords(context, 
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
