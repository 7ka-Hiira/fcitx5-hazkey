#ifndef _FCITX5_HAZUKEY_HAZUKEY_PREEDIT_H_
#define _FCITX5_HAZUKEY_HAZUKEY_PREEDIT_H_

#include <fcitx/inputcontext.h>
#include <fcitx/inputpanel.h>
#include <hazukey/libhazukey_kkc.h>

namespace fcitx {
class HazukeyPreedit {
   public:
    HazukeyPreedit(InputContext *ic) : ic_(ic) {}

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

#endif  // _FCITX5_HAZUKEY_HAZUKEY_PREEDIT_H_