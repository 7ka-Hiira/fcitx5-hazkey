#include "hazukey_candidate.h"

namespace fcitx {
void HazukeyCandidateWord::select(InputContext *ic) const {
    ic->commitString(candidate_);
}

void HazukeyCandidateList::setDefaultStyle(KeyList selectionKeys) {
    setLayoutHint(CandidateLayoutHint::Vertical);
    setPageSize(9);
    setSelectionKey(selectionKeys);
    setCursorIndex(0);
}
}  // namespace fcitx