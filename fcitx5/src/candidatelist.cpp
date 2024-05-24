#include "candidatelist.h"

namespace fcitx {
azooKeyCandidateList::azooKeyCandidateList(
    azooKeyEngine *engine, InputContext *ic,
    std::vector<std::string> *candidatesStr)
    : engine_(engine), ic_(ic), candidates_{} {
  setPageable(this);
  setCursorMovable(this);

  totalSize_ = candidatesStr->size();
  size_ = std::min(totalSize_, 10);
  totalPage_ = size_ / 10;
  for (int i = 0; i < size_; i++) {
    Text label;
    label.append(std::to_string((i + 1) % 10));
    label.append(". ");
    candidates_.emplace_back(
        std::make_unique<azooKeyCandidateWord>(engine_, candidatesStr->at(i)));
    labels_.push_back(label);
  }
}
}  // namespace fcitx