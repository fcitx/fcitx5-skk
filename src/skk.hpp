#pragma once

#include <fcitx/inputmethodengine.h>
#include <fcitx/addonfactory.h>
#include <fcitx/instance.h>
#include <fcitx/addonmanager.h>

namespace fcitx  {
    
class SkkEngine final: public InputMethodEngine {
public:
    SkkEngine(Instance *instance);
    ~SkkEngine();
    
    void activate(const InputMethodEntry &entry, InputContextEvent &event) override;
    void keyEvent(const InputMethodEntry &entry, KeyEvent &keyEvent) override;
    void reloadConfig() override;
    void reset(const InputMethodEntry &entry, InputContextEvent &event) override;
    void save() override;

private:
    Instance *instance_;
};

class SkkAddonFactory final: public AddonFactory {
public:
    virtual AddonInstance * create(AddonManager *manager) override {
        return new SkkEngine(manager->instance());
    }
};

FCITX_ADDON_FACTORY(SkkAddonFactory)

}
