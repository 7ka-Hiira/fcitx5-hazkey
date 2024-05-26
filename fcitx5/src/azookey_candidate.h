#ifndef FCITX5_AZOOKEY_AZOOKEY_CANDIDATE_H_
#define FCITX5_AZOOKEY_AZOOKEY_CANDIDATE_H_

#include <fcitx/candidatelist.h>
#include <fcitx/inputcontext.h>
#include <iconv.h>

namespace fcitx {

class azooKeyCandidateWord : public CandidateWord {
 public:
  // azooKeyCandidateWord constructor
  azooKeyCandidateWord(std::string text) { setText(Text(std::move(text))); }
  // select the candidate word
  void select(InputContext *ic) const override;
};

class azooKeyCandidateList : public CommonCandidateList {
 public:
  // set fcitx5-mozc-like default style for the candidate list
  void setDefaultStyle(KeyList selectionKeys);
};

}  // namespace fcitx

#endif  // FCITX5_AZOOKEY_AZOOKEY_CANDIDATE_H_
