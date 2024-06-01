#include "hazukey_candidate.h"
#include "hazukey_config.h"
#include "hazukey_preedit.h"
#include "hazukey_state.h"

namespace fcitx {

constexpr int NormalCandidateListNBest = 18;
constexpr int PredictCandidateListNBest = 4;

void HazukeyPreedit::setPreedit(Text text) {
    if (ic_->capabilityFlags().test(CapabilityFlag::Preedit)) {
        ic_->inputPanel().setClientPreedit(text);
    } else {
        ic_->inputPanel().setPreedit(text);
    }
}

void HazukeyPreedit::setSimplePreedit(const std::string &text) {
    std::vector<std::string> texts = {text};
    setMultiSegmentPreedit(texts, -1);
}

void HazukeyPreedit::setMultiSegmentPreedit(std::vector<std::string> &texts,
                                            int cursorSegment = 0) {
    auto preedit = Text();
    for (int i = 0; i < int(texts.size()); i++) {
        if (i < cursorSegment) {
            preedit.append(texts[i], TextFormatFlag::NoFlag);
        } else if (i == cursorSegment) {
            preedit.setCursor(preedit.textLength());
            preedit.append(texts[i], TextFormatFlag::HighLight);
            continue;
        } else {
            preedit.append(texts[i], TextFormatFlag::Underline);
        }
    }
    setPreedit(preedit);
}

}  // namespace fcitx