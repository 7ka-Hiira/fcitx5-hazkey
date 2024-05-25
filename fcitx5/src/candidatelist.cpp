#include "candidatelist.h"

namespace fcitx {
void azooKeyCandidateWord::select(InputContext *ic) const {
  FCITX_INFO() << "Selected: " << text().toString();
  ic->commitString(text().toString());
  ic->reset();
}

void azooKeyCandidateList::updateAUX(InputContext *ic) {
  ic->inputPanel().setAuxUp(Text("[" + std::to_string(cursorIndex() + 1) + "/" +
                                 std::to_string(totalSize()) + "]"));
}

void azooKeyCandidateList::setDefaultStyle(KeyList selectionKeys) {
  FCITX_INFO() << "setDefaultStyle";
  setLayoutHint(CandidateLayoutHint::Vertical);
  setPageSize(9);
  setSelectionKey(selectionKeys);
  setCursorIndex(0);
}
}  // namespace fcitx