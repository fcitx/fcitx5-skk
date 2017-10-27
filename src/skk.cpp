#include "skk.hpp"
namespace fcitx {

    /////////////////////////////////////////////////////////////////////////////////////
    /// SkkEngine
    
    SkkEngine::SkkEngine(Instance *instance):
        instance_{instance},
        factory_([this](InputContext & ic) { return new SkkState(this, &ic); }) 
    {
        skk_init();
       
        instance_ -> inputContextManager().registerProperty("skkState", &factory_);

    

    }
    
    void SkkEngine::activate(const InputMethodEntry &entry, InputContextEvent &event){
        (void) entry;
        (void) event;      
        // FIXME 
    }
    
    void SkkEngine::keyEvent(const InputMethodEntry &entry, KeyEvent &keyEvent) {
        (void) entry;
        (void) keyEvent;
        // FIXME
        
    }
    
    void SkkEngine::reloadConfig() {
        // FIXME
    }
    void SkkEngine::reset(const InputMethodEntry &entry, InputContextEvent &event) {
        (void) entry;
        (void) event;
        // FIXME
    }
    void SkkEngine::save() {
        // FIXME
    }

    SkkEngine::~SkkEngine() {}
    
    
    /////////////////////////////////////////////////////////////////////////////////////
    /// SkkState
    
    SkkState::SkkState(SkkEngine *engine, InputContext * ic):
        engine_(engine),
        ic_(ic),
        context_(skk_context_new(0, 0), &g_object_unref)
    {
        SkkContext * context = context_.get();
        skk_context_set_period_style(context, SKK_PERIOD_STYLE_JA_JA);
        skk_context_set_input_mode(context, SKK_INPUT_MODE_HIRAGANA);

        input_mode_changed_handler = g_signal_connect(context, "notify::input-mode", 
                G_CALLBACK(SkkState::input_mode_changed_cb), this);
        
        UpdateInputMode();
        candidate_selected_handler = g_signal_connect(skk_context_get_candidates(context), "selected", G_CALLBACK(candidate_list_selected_cb), this);
        //auto candidate_populated_handler = g_signal_connect(skk_context_get_candidates(context), "populated", G_CALLBACK(skk_candidate_list_popuplated_cb), this);
        //auto notify_preedit_handler = g_signal_connect(context, "notify::preedit", G_CALLBACK(skk_candidate_update_preedit_cb), this);
        //auto retrieve_surrounding_text_handler = g_signal_connect(context, "retrieve_surrounding_text", G_CALLBACK(skk_context_retrieve_surrounding_text_cb), this);
        //auto delete_surrounding_text_handler = g_signal_connect(context, "delete_surrounding_text", G_CALLBACK(skk_context_delete_surrounding_text_cb), this);
    
        //gchar* AUTO_START_HENKAN_KEYWORDS[] = {
        //    "を", "、", "。", "．", "，", "？", "」",
        //    "！", "；", "：", ")", ";", ":", "）",
        //    "”", "】", "』", "》", "〉", "｝", "］",
        //    "〕", "}", "]", "?", ".", ",", "!"
        //};
        //
        //skk_context_set_auto_start_henkan_keywords(context, 
        //                                           AUTO_START_HENKAN_KEYWORDS,        
        //                                           G_N_ELEMENTS(AUTO_START_HENKAN_KEYWORDS));
    }
    
    
    void SkkState::UpdateInputMode(){
        // FIXME
    }
    
    void SkkState::input_mode_changed_cb(GObject *, GParamSpec *, SkkState *skk){
        skk -> UpdateInputMode();
    }
    void SkkState::candidate_list_selected_cb(SkkCandidateList *, GParamSpec *, SkkState *skk){
        skk->selected = true;
        SkkContext * context = skk->context_.get();
        gchar* output = skk_context_poll_output(context);

        if (output && strlen(output) > 0) {
            skk -> ic_ -> commitString(output);
            //FcitxInstanceCommitString(skk->owner, FcitxInstanceGetCurrentIC(skk->owner), output);
        }

        g_free(output);
    }
}


FCITX_ADDON_FACTORY(fcitx::SkkAddonFactory)
