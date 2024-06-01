#ifndef _FCITX5_AZOOKEY_AZOOKEY_PREEDIT_H_
#define _FCITX5_AZOOKEY_AZOOKEY_PREEDIT_H_

#include <azookey/libazookey_kkc.h>
#include <fcitx/inputcontext.h>
#include <fcitx/inputpanel.h>

namespace fcitx {
class azooKeyPreedit {
   public:
    azooKeyPreedit(InputContext *ic) : ic_(ic) {}

    // set the preedit text; prediction mode
    void setSimplePreedit(const std::string &text);
    // set the preedit text; multi-segment mode
    void setMultiSegmentPreedit(std::vector<std::string> &texts, int cursor);
    // set the preedit text
    void setPreedit(Text text);

   private:
    // fcitx input context pointer
    InputContext *ic_;
};

}  // namespace fcitx

#endif  // _FCITX5_AZOOKEY_AZOOKEY_PREEDIT_H_