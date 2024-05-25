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

  void select(InputContext *ic) const override;

 private:
  azooKeyEngine *engine_;
};
}  // namespace fcitx

#endif  // FCITX5_AZOOKEY_CANDIDATELIST_H
