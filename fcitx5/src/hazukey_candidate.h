#ifndef FCITX5_HAZUKEY_HAZUKEY_CANDIDATE_H_
#define FCITX5_HAZUKEY_HAZUKEY_CANDIDATE_H_

#include <fcitx/candidatelist.h>
#include <fcitx/inputcontext.h>
#include <hazukey/libhazukey_kkc.h>

namespace fcitx {

class HazukeyCandidateWord : public CandidateWord {
   public:
    // HazukeyCandidateWord constructor
    HazukeyCandidateWord(const char* text, const char* hiragana,
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

class HazukeyCandidateList : public CommonCandidateList {
   public:
    const HazukeyCandidateWord& HazukeyCandidate(int localIndex) const {
        return static_cast<const HazukeyCandidateWord&>(candidate(localIndex));
    }
    // set fcitx5-mozc-like default style for the candidate list
    void setDefaultStyle(KeyList selectionKeys);
};

}  // namespace fcitx

#endif  // FCITX5_HAZUKEY_HAZUKEY_CANDIDATE_H_
