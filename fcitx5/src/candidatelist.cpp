#include "candidatelist.h"

namespace fcitx {
void azooKeyCandidateWord::select(InputContext *ic) const {
  FCITX_INFO() << "Selected: " << text().toString();
  ic->commitString(text().toString());
  ic->reset();
}
}  // namespace fcitx