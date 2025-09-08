#ifndef _FCITX5_HAZKEY_HAZKEY_CONFIG_H_
#define _FCITX5_HAZKEY_HAZKEY_CONFIG_H_

#include <fcitx-config/configuration.h>
#include <fcitx-config/enum.h>
#include <fcitx-utils/i18n.h>
#include <fcitx-utils/library.h>
#include <fcitx/menu.h>

namespace fcitx {

/// Config

FCITX_CONFIGURATION(HazkeyEngineConfig,
                    Option<bool> showTabToSelect{
                        this, "showTabToSelect",
                        _("Show [Press Tab to Select] indicator"), true};
                    ExternalOption openHazkeySettings{
                        this, "openHazkeySettings", _("Open Hazkey Settings"),
                        stringutils::concat("hazkey-settings")};);
}  // namespace fcitx
#endif  // _FCITX5_HAZKEY_HAZKEY_CONFIG_H_
