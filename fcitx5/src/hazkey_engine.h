#ifndef _FCITX5_HAZKEY_HAZKEY_ENGINE_H_
#define _FCITX5_HAZKEY_HAZKEY_ENGINE_H_

#include <fcitx-config/configuration.h>
#include <fcitx-config/iniparser.h>
#include <fcitx-utils/handlertable_details.h>
#include <fcitx-utils/i18n.h>
#include <fcitx-utils/library.h>
#include <fcitx/addonfactory.h>
#include <fcitx/addonmanager.h>
#include <fcitx/inputmethodengine.h>
#include <fcitx/instance.h>
#include <fcitx/menu.h>
#include <iconv.h>

#include "../../azookey-kkc/libhazkey.h"
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

    const KkcConfig *getKkcConfig() const { return kkcConfig_; }

    auto factory() const { return &factory_; }
    auto instance() const { return instance_; }

    const Configuration *getConfig() const override { return &config_; }
    void setConfig(const RawConfig &config) override;
    void reloadConfig() override;

    const HazkeyEngineConfig &config() const { return config_; }

   private:
    HazkeyEngineConfig config_;
    const KkcConfig *kkcConfig_;
    Instance *instance_;
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
#endif  // _FCITX5_HAZKEY_HAZKEY_ENGINE_H_
