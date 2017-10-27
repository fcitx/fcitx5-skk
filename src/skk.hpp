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
    
private:
    Instance *instance_;
    FactoryFor<SkkState> factory_;
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
    SkkEngine *engine_;
    std::unique_ptr<SkkContext, decltype(&g_object_unref)> context_;

    
    bool selected;
    
    void UpdateInputMode();
    
    // callbacks and their handlers
    gulong input_mode_changed_handler;
    static void input_mode_changed_cb(GObject * gobject, GParamSpec *pspec, gpointer user_data);
    gulong candidate_selected_handler;
    static void candidate_list_selected_cb(SkkCandidateList * gobject, GParamSpec *pspec, gpointer user_data);
    
};


}
