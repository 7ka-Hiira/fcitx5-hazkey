#ifndef FCITX5_AZOOKEY_AZOOKEY_CANDIDATE_H_
#define FCITX5_AZOOKEY_AZOOKEY_CANDIDATE_H_

#include <azookey/libazookey_kkc.h>
#include <fcitx/candidatelist.h>
#include <fcitx/inputcontext.h>
#include <iconv.h>

namespace fcitx {

class azooKeyCandidateWord : public CandidateWord {
 public:
  // azooKeyCandidateWord constructor
  azooKeyCandidateWord(char* text, char* hiragana, char* correspondingCount)
      : CandidateWord(Text(text)) {
    setText(Text(text));
    candidate_ = std::move(text);
    hiragana_ = std::move(hiragana);
    corresponding_count_ = std::atoi(correspondingCount);
  }

  // candidate select event: commit and reset the input context
  void select(InputContext* ic) const override;
  // return the preedit string
  std::vector<std::string> getPreedit() const {
    if (hiragana_.empty()) return {candidate_};
    return {candidate_, hiragana_};
  }
  // return the corresponding count
  int correspondingCount() const { return corresponding_count_; }

 private:
  std::string candidate_;
  std::string hiragana_;
  int corresponding_count_;
};

class azooKeyCandidateList : public CommonCandidateList {
 public:
  const azooKeyCandidateWord& azooKeyCandidate(int idx) const {
    return static_cast<const azooKeyCandidateWord&>(candidate(idx));
  }
  // set fcitx5-mozc-like default style for the candidate list
  void setDefaultStyle(KeyList selectionKeys);
};

}  // namespace fcitx

#endif  // FCITX5_AZOOKEY_AZOOKEY_CANDIDATE_H_
