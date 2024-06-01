#ifndef _FCITX5_AZOOKEY_AZOOKEY_H_
#define _FCITX5_AZOOKEY_AZOOKEY_H_

#include <fcitx/addonfactory.h>
#include <fcitx/addonmanager.h>
#include <fcitx/inputmethodengine.h>
#include <fcitx/instance.h>
#include <iconv.h>

#include "azookey_config.h"
#include "azookey_state.h"

namespace fcitx {

class azooKeyEngine : public InputMethodEngineV2 {
   public:
    // constructor
    azooKeyEngine(Instance *instance);
    // handle key event and pass it to azooKeyState
    void keyEvent(const InputMethodEntry &entry, KeyEvent &keyEvent) override;
    // called when input method changes to azooKey
    void activate(const InputMethodEntry &, InputContextEvent &) override;
    // called when input method changes to another input method
    void deactivate(const InputMethodEntry &, InputContextEvent &) override;

    auto factory() const { return &factory_; }
    auto instance() const { return instance_; }

   private:
    Instance *instance_;
    std::shared_ptr<azooKeyConfig> config_;
    FactoryFor<azooKeyState> factory_;
    iconv_t conv_;
};

class azooKeyEngineFactory : public AddonFactory {
    AddonInstance *create(AddonManager *manager) override {
        FCITX_UNUSED(manager);
        return new azooKeyEngine(manager->instance());
    }
};

}  // namespace fcitx
#endif  // _FCITX5_AZOOKEY_AZOOKEY_H_
