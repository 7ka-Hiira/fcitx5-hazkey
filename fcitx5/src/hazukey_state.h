#ifndef _FCITX5_HAZUKEY_HAZUKEY_STATE_H_
#define _FCITX5_HAZUKEY_HAZUKEY_STATE_H_

#include <fcitx/inputcontext.h>
#include <fcitx/inputpanel.h>
#include <hazukey/libhazukey.h>

#include "hazukey_candidate.h"
#include "hazukey_config.h"
#include "hazukey_preedit.h"

namespace fcitx {
class HazukeyState : public InputContextProperty {
   public:
    HazukeyState(InputContext *ic) : ic_(ic), preedit_(HazukeyPreedit(ic)) {
        composingText_ = nullptr;
    }

    // complete the prefix and remove from composingText_
    void completePrefix(int correspondingCount);
    // handle key event. call candidateKeyEvent or preeditKeyEvent
    // depends on the current mode
    void keyEvent(KeyEvent &keyEvent);
    void loadConfig(std::shared_ptr<HazukeyConfig> &config);
    // reset to the initial state
    void reset();

   private:
    enum class ConversionMode {
        None,
        Hiragana,
        KatakanaFullwidth,
        KatakanaHalfwidth,
        RawFullwidth,
        RawHalfwidth,
    };

    enum class showCandidateMode {
        PredictWithLivePreedit,
        PredictWithPreedit,
        NonPredictWithFirstPreedit,
    };

    void directCharactorConversion(ConversionMode mode);
    // handle key event in candidate mode
    void candidateKeyEvent(KeyEvent &keyEvent,
                           std::shared_ptr<HazukeyCandidateList> candidateList);
    // handle key event in preedit mode
    void preeditKeyEvent(
        KeyEvent &keyEvent,
        std::shared_ptr<HazukeyCandidateList> PreeditCandidateList);

    // base function to prepare candidate list
    // make sure composingText_ is not nullptr
    void showCandidateList(showCandidateMode mode, int nBest);
    std::vector<std::vector<std::string>> getCandidates(bool isPredictMode,
                                                        int nBest);
    std::unique_ptr<HazukeyCandidateList> createCandidateList(
        std::vector<std::vector<std::string>> candidates,
        std::shared_ptr<std::vector<std::string>> preeditSegments);

    // prepare candidate list for normal conversion
    void showNonPredictCandidateList();
    // prepare candidate
    // list for prediction.
    // shorter than normal
    void showPredictCandidateList();

    // update the candidate cursor
    void updateCandidateCursor(
        std::shared_ptr<HazukeyCandidateList> candidateList);
    // advance the cursor in
    // the candidate list,
    // update aux, set
    // preedit text
    void advanceCandidateCursor(
        std::shared_ptr<HazukeyCandidateList> candidateList);
    // back the cursor in
    // the candidate list,
    // update aux, set
    // preedit text
    void backCandidateCursor(
        std::shared_ptr<HazukeyCandidateList> candidateList);
    // update aux; label on
    // the candidate list
    // like "[1/100]"
    void setCandidateCursorAUX(
        std::shared_ptr<HazukeyCandidateList> candidateList);

    // check if the key
    // event is inputable
    // (simple key / kana
    // key) or not
    bool isInputableEvent(const KeyEvent &keyEvent);
    // fcitx input context
    // pointer
    InputContext *ic_;
    // shared pointer to the
    // configuration
    std::shared_ptr<HazukeyConfig> config_;
    // preedit class
    HazukeyPreedit preedit_;
    // composing text
    // information pointer
    // used by
    // libhazukey-kkc
    ComposingText *composingText_;
    // true if the current
    // mode is candidate
    // mode
    bool is_candidate_mode_ = false;
};

}  // namespace fcitx

#endif  // _FCITX5_HAZUKEY_HAZUKEY_STATE_H_