#include "azookey_preedit.h"

#include "azookey_candidate.h"
#include "azookey_config.h"
#include "azookey_state.h"

namespace fcitx {

constexpr int NormalCandidateListNBest = 18;
constexpr int PredictCandidateListNBest = 4;

void azooKeyPreedit::setPreedit(Text text) {
    if (ic_->capabilityFlags().test(CapabilityFlag::Preedit)) {
        ic_->inputPanel().setClientPreedit(text);
    } else {
        ic_->inputPanel().setPreedit(text);
    }
}

void azooKeyPreedit::setSimplePreedit(const std::string &text) {
    std::vector<std::string> texts = {text};
    setMultiSegmentPreedit(texts, -1);
}

void azooKeyPreedit::setMultiSegmentPreedit(std::vector<std::string> &texts,
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