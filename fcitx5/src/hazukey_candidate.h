#ifndef FCITX5_HAZUKEY_HAZUKEY_CANDIDATE_H_
#define FCITX5_HAZUKEY_HAZUKEY_CANDIDATE_H_

#include <fcitx/candidatelist.h>
#include <fcitx/inputcontext.h>
#include <hazukey/libhazukey.h>

namespace fcitx {

class HazukeyCandidateWord : public CandidateWord {
   public:
    HazukeyCandidateWord(const int index, const std::string& text,
                         const std::string& hiragana,
                         const int correspondingCount,
                         const std::vector<std::string> parts,
                         const std::vector<int> partLens)
        : CandidateWord(Text(text)),
          index_(index),
          candidate_(std::move(text)),
          hiragana_(std::move(hiragana)),
          corresponding_count_(correspondingCount),
          parts_(std::move(parts)),
          part_lens_(std::move(partLens)) {
        setText(Text(text));
    }

    // called when the candidate is selected (by pointing device?)
    void select(InputContext* ic) const override;

    std::vector<std::string> getPreedit() const;

    int correspondingCount() const { return corresponding_count_; }

   private:
    const int index_;
    const std::string candidate_;
    const std::string hiragana_;
    const int corresponding_count_;
    const std::vector<std::string> parts_;
    const std::vector<int> part_lens_;
};

class HazukeyCandidateList : public CommonCandidateList {
   public:
    // always vertical
    CandidateLayoutHint layoutHint() const override;

    const HazukeyCandidateWord& getCandidate(int localIndex) const;

    void focus(KeyList selectionKeys);

    bool focused() const { return focused_; }

   private:
    bool focused_ = false;
};

}  // namespace fcitx

#endif  // FCITX5_HAZUKEY_HAZUKEY_CANDIDATE_H_
