#ifndef _FCITX5_HAZUKEY_HAZUKEY_CONFIG_H_
#define _FCITX5_HAZUKEY_HAZUKEY_CONFIG_H_

#include <fcitx-utils/key.h>
#include <fcitx-utils/log.h>
#include <hazukey/libhazukey_kkc.h>

namespace fcitx {

class HazukeyConfig {
   public:
    // HazukeyConfig constructor
    HazukeyConfig() {
        FCITX_DEBUG() << "HazukeyConfig constructor";
        kkc_config_ = kkc_get_config();
    }

    // return keylist used for candidate selection
    const KeyList getSelectionKeys();

    // return the kkc config used for conversion
    const KkcConfig *getKkcConfig() { return kkc_config_; }

   private:
    // kkc config
    const KkcConfig *kkc_config_;
};
}  // namespace fcitx

#endif  // _FCITX5_HAZUKEY_HAZUKEY_CONFIG_H_