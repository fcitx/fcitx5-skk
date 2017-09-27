#pragma once

#include <memory>

#include <fcitx/inputmethodengine.h>
#include <fcitx/addonfactory.h>
#include <fcitx/instance.h>
#include <fcitx/addonmanager.h>
#include <fcitx/inputcontextproperty.h>
#include <fcitx/inputcontextmanager.h>

#include <libskk/libskk.h>

namespace fcitx  {

class SkkState;
    
class SkkEngine final: public InputMethodEngine {
public:
    SkkEngine(Instance *instance);
    ~SkkEngine();
    
    void activate(const InputMethodEntry &entry, InputContextEvent &event) override;
    void keyEvent(const InputMethodEntry &entry, KeyEvent &keyEvent) override;
    void reloadConfig() override;
    void reset(const InputMethodEntry &entry, InputContextEvent &event) override;
    void save() override;
    
    auto &factory() { return factory_; }
    SkkContext* context() { return context_.get(); }
    
private:
    Instance *instance_;
    FactoryFor<SkkState> factory_;
    std::unique_ptr<SkkContext> context_;
};

class SkkAddonFactory final: public AddonFactory {
public:
    virtual AddonInstance * create(AddonManager *manager) override {
        return new SkkEngine(manager->instance());
    }
};


class SkkState final: public InputContextProperty {
public: 
    SkkState(SkkEngine* engine);
private:
    SkkContext *context_;
};


}
