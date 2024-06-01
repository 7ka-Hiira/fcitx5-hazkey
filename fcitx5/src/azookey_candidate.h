#ifndef FCITX5_AZOOKEY_AZOOKEY_CANDIDATE_H_
#define FCITX5_AZOOKEY_AZOOKEY_CANDIDATE_H_

#include <azookey/libazookey_kkc.h>
#include <fcitx/candidatelist.h>
#include <fcitx/inputcontext.h>

namespace fcitx {

class azooKeyCandidateWord : public CandidateWord {
   public:
    // azooKeyCandidateWord constructor
    azooKeyCandidateWord(const char* text, const char* hiragana,
                         const int correspondingCount,
                         const std::vector<std::string> parts,
                         const std::vector<int> partLens)
        : CandidateWord(Text(text)),
          candidate_(std::move(text)),
          hiragana_(std::move(hiragana)),
          corresponding_count_(correspondingCount),
          parts_(std::move(parts)),
          part_lens_(std::move(partLens)) {
        setText(Text(text));
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
    const std::string candidate_;
    const std::string hiragana_;
    const int corresponding_count_;
    const std::vector<std::string> parts_;
    const std::vector<int> part_lens_;
};

class azooKeyCandidateList : public CommonCandidateList {
   public:
    const azooKeyCandidateWord& azooKeyCandidate(int localIndex) const {
        return static_cast<const azooKeyCandidateWord&>(candidate(localIndex));
    }
    // set fcitx5-mozc-like default style for the candidate list
    void setDefaultStyle(KeyList selectionKeys);
};

}  // namespace fcitx

#endif  // FCITX5_AZOOKEY_AZOOKEY_CANDIDATE_H_
