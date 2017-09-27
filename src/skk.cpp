#include "skk.hpp"

namespace fcitx {
    SkkEngine::SkkEngine(Instance *instance):instance_{instance} {
        // FIXME
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
