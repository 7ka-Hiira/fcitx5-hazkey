#include "azookey_config.h"

namespace fcitx {
const KeyList azooKeyConfig::getSelectionKeys() {
    return {
        Key{FcitxKey_1}, Key{FcitxKey_2}, Key{FcitxKey_3}, Key{FcitxKey_4},
        Key{FcitxKey_5}, Key{FcitxKey_6}, Key{FcitxKey_7}, Key{FcitxKey_8},
        Key{FcitxKey_9}, Key{FcitxKey_0},
    };
}
}  // namespace fcitx