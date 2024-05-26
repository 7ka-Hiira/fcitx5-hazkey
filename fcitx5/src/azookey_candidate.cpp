#include "azookey_candidate.h"

namespace fcitx {
void azooKeyCandidateWord::select(InputContext *ic) const {
  ic->commitString(text().toString());
  ic->reset();
}

void azooKeyCandidateList::setDefaultStyle(KeyList selectionKeys) {
  setLayoutHint(CandidateLayoutHint::Vertical);
  setPageSize(9);
  setSelectionKey(selectionKeys);
  setCursorIndex(0);
}
}  // namespace fcitx