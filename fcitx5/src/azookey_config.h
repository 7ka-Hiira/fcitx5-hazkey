#ifndef _FCITX5_AZOOKEY_AZOOKEY_CONFIG_H_
#define _FCITX5_AZOOKEY_AZOOKEY_CONFIG_H_

#include <azookey/libazookey_kkc.h>
#include <fcitx-utils/key.h>
#include <fcitx-utils/log.h>

namespace fcitx {

class azooKeyConfig {
 public:
  // azooKeyConfig constructor
  azooKeyConfig() {
    FCITX_DEBUG() << "azooKeyConfig constructor";
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

#endif  // _FCITX5_AZOOKEY_AZOOKEY_CONFIG_H_