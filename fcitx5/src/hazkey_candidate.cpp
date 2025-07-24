#include "hazkey_candidate.h"

namespace fcitx {

/// CandidateWord

std::vector<std::string> HazkeyCandidateWord::getPreedit() const {
    if (hiragana_.empty()) return {candidate_};
    return {candidate_, hiragana_};
}

void HazkeyCandidateWord::select(InputContext* ic) const {
    KeyEvent keyEvent(ic, Key(FcitxKey_Return));
    keyEvent = KeyEvent(
        ic, Key(KeySym(FcitxKey_1 + (index_ % NormalCandidateListSize)),
                KeyState::Alt));
    ic->keyEvent(keyEvent);
}

/// CandidateList

HazkeyCandidateList::HazkeyCandidateList(
    std::vector<std::string> candidates
    // std::shared_ptr<std::vector<std::string>> preeditSegments
)
    : CommonCandidateList() {
    for (size_t i = 0; i < candidates.size(); i++) {
        auto candidate = candidates[i];
        // std::vector<std::string> parts;
        // std::vector<int> partLens;

        append(std::make_unique<HazkeyCandidateWord>(
            i, candidate, "あ"
          ));

        // save preedit which found first
        // if (preeditSegments->empty() && std::stoi(candidate[4]) == 1) {
        //     *preeditSegments = parts;
        // }
    }
}

CandidateLayoutHint HazkeyCandidateList::layoutHint() const {
    return CandidateLayoutHint::Vertical;
}

void HazkeyCandidateList::focus() {
    setPageSize(9);
    setGlobalCursorIndex(0);
}

const HazkeyCandidateWord& HazkeyCandidateList::getCandidate(
    int localIndex) const {
    return static_cast<const HazkeyCandidateWord&>(candidate(localIndex));
}

void HazkeyCandidateList::setCursorIndex(int localIndex) {
    if (localIndex < 0 || localIndex >= size()) {
        return;
    }
    int globalIndex = pageSize() * currentPage() + localIndex;
    setGlobalCursorIndex(globalIndex);
}

void HazkeyCandidateList::nextPage() {
    next();
    setCursorIndex(0);
}

void HazkeyCandidateList::prevPage() {
    prev();
    setCursorIndex(0);
}

bool HazkeyCandidateList::focused() const { return (globalCursorIndex() >= 0); }

}  // namespace fcitx
