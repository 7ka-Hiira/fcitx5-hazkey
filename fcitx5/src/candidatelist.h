#ifndef FCITX5_AZOOKEY_CANDIDATELIST_H
#define FCITX5_AZOOKEY_CANDIDATELIST_H

#include <fcitx/candidatelist.h>
#include <fcitx/inputcontext.h>
#include <iconv.h>

namespace fcitx {
class azooKeyEngine;
class azooKeyCandidateWord;
class azooKeyCandidateList;
}  // namespace fcitx

#include "azookey.h"

namespace fcitx {
class azooKeyCandidateWord : public CandidateWord {
 public:
  azooKeyCandidateWord(azooKeyEngine *engine, std::string text)
      : engine_(engine) {
    setText(Text(std::move(text)));
  }

  void select(InputContext *ic) const override;

 private:
  azooKeyEngine *engine_;
};

class azooKeyCandidateList : public CommonCandidateList {
 public:
  // azooKeyCandidateList() {}

  void updateAUX(InputContext *ic);
  void setDefaultStyle(KeyList selectionKeys);

 private:
};

}  // namespace fcitx

#endif  // FCITX5_AZOOKEY_CANDIDATELIST_H
