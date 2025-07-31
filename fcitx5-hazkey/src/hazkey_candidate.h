#ifndef FCITX5_HAZKEY_HAZKEY_CANDIDATE_H_
#define FCITX5_HAZKEY_HAZKEY_CANDIDATE_H_

#include <fcitx/candidatelist.h>
#include <fcitx/inputcontext.h>
#include <fcitx/text.h>

#include <string>
// #include "hazkey_state.h"
#include "hazkey_server_connector.h"

namespace fcitx {

class HazkeyState;

constexpr int NormalCandidateListSize = 9;
constexpr int PredictCandidateListSize = 4;

const KeyList defaultSelectionKeys = {
    Key{FcitxKey_1}, Key{FcitxKey_2}, Key{FcitxKey_3}, Key{FcitxKey_4},
    Key{FcitxKey_5}, Key{FcitxKey_6}, Key{FcitxKey_7}, Key{FcitxKey_8},
    Key{FcitxKey_9}, Key{FcitxKey_0},
};

class HazkeyCandidateWord : public CandidateWord {
   public:
    HazkeyCandidateWord(const int index,
                        const HazkeyServerConnector::CandidateData data)
        : CandidateWord(Text(data.candidateText)),
          index_(index),
          candidate_(std::move(data.candidateText)),
          hiragana_(std::move(data.subHiragana)) {
        setText(Text(data.candidateText));
    }

    // called when the candidate is selected (by pointing device?)
    // calculate the index of the candidate on current page
    // and send key to select the candidate
    void select(InputContext* ic) const override;

    std::vector<std::string> getPreedit() const;

    // int correspondingCount() const { return corresponding_count_; }

   private:
    const int index_;
    const std::string candidate_;
    const std::string hiragana_;
    // const int corresponding_count_;
    // const std::vector<std::string> parts_;
    // const std::vector<int> part_lens_;
};

class HazkeyCandidateList : public CommonCandidateList {
   public:
    HazkeyCandidateList(
        std::vector<HazkeyServerConnector::CandidateData> candidates);

    // return the direction of the candidate list
    // currently always vertical
    CandidateLayoutHint layoutHint() const override;

    const HazkeyCandidateWord& getCandidate(int localIndex) const;

    // defined to support fcitx < 5.1.9
    // recent versions of fcitx provide this function as a default
    void setCursorIndex(int localIndex);

    // set the cursor top of the next/prev page
    void nextPage();
    void prevPage();

    // show cursor on the candidate list
    void focus();

    // whether the candidate list is focused
    bool focused() const;
};

}  // namespace fcitx

#endif  // FCITX5_HAZKEY_HAZKEY_CANDIDATE_H_
