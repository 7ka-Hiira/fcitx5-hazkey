#ifndef FCITX5_AZOOKEY_CANDIDATELIST_H
#define FCITX5_AZOOKEY_CANDIDATELIST_H

#include <fcitx/candidatelist.h>
#include <fcitx/inputcontext.h>
#include <iconv.h>

#include "azookey.h"

namespace fcitx {
class azooKeyCandidateWord : public CandidateWord {
 public:
  azooKeyCandidateWord(azooKeyEngine *engine, std::string text)
      : engine_(engine) {
    setText(Text(std::move(text)));
  }

  void select(InputContext *ic) const override {
    auto *state = ic->propertyFor(engine_->factory());
    state->reset();
    state->updatePreedit();
  }

 private:
  azooKeyEngine *engine_;
  std::vector<std::string> candidates_;
};

class azooKeyCandidateList : public fcitx::CandidateList,
                             public fcitx::PageableCandidateList,
                             public fcitx::CursorMovableCandidateList {
 public:
  azooKeyCandidateList(azooKeyEngine *engine, InputContext *ic,
                       std::vector<std::string> *candidatesStr);

  const Text &label(int idx) const override { return labels_[idx]; }

  const CandidateWord &candidate(int idx) const override {
    return *candidates_[idx];
  }

  int size() const override { return size_; }
  CandidateLayoutHint layoutHint() const override {
    return fcitx::CandidateLayoutHint::NotSet;
  }

  // Todo
  bool hasNext() const override { return currentPage_ < totalPage_; }
  bool hasPrev() const override { return currentPage_ > 0; }
  void prev() override { return; }
  void next() override { return; }
  bool usedNextBefore() const override { return false; }
  void prevCandidate() override { return; }
  void nextCandidate() override { return; }
  int cursorIndex() const override { return cursor_; }

 private:
  void generate(std::vector<std::string> *candidatesStr);

  azooKeyEngine *engine_;
  fcitx::InputContext *ic_;
  std::vector<Text> labels_;
  std::vector<std::unique_ptr<azooKeyCandidateWord>> candidates_;
  int totalSize_ = 0;
  int size_ = 0;
  int currentPage_ = 0;
  int totalPage_ = 0;
  int cursor_ = 0;
};
}  // namespace fcitx

#endif  // FCITX5_AZOOKEY_CANDIDATELIST_H
