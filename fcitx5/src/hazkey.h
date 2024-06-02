#ifndef _FCITX5_HAZUKEY_HAZUKEY_H_
#define _FCITX5_HAZUKEY_HAZUKEY_H_

#include <fcitx/addonfactory.h>
#include <fcitx/addonmanager.h>
#include <fcitx/inputmethodengine.h>
#include <fcitx/instance.h>
#include <iconv.h>

#include "hazkey_config.h"
#include "hazkey_state.h"

namespace fcitx {

class HazkeyEngine : public InputMethodEngineV2 {
   public:
    // constructor
    HazkeyEngine(Instance *instance);
    // handle key event and pass it to HazkeyState
    void keyEvent(const InputMethodEntry &entry, KeyEvent &keyEvent) override;
    // called when input method changes to Hazkey
    void activate(const InputMethodEntry &, InputContextEvent &) override;
    // called when input method changes to another input method
    void deactivate(const InputMethodEntry &, InputContextEvent &) override;

    auto factory() const { return &factory_; }
    auto instance() const { return instance_; }

   private:
    Instance *instance_;
    std::shared_ptr<HazkeyConfig> config_;
    FactoryFor<HazkeyState> factory_;
    iconv_t conv_;
};

class HazkeyEngineFactory : public AddonFactory {
    AddonInstance *create(AddonManager *manager) override {
        FCITX_UNUSED(manager);
        return new HazkeyEngine(manager->instance());
    }
};

}  // namespace fcitx
#endif  // _FCITX5_HAZUKEY_HAZUKEY_H_
