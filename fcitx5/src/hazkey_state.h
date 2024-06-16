#ifndef _FCITX5_HAZUKEY_HAZUKEY_STATE_H_
#define _FCITX5_HAZUKEY_HAZUKEY_STATE_H_

#include <fcitx/inputcontext.h>
#include <fcitx/inputpanel.h>
#include <hazkey/libhazkey.h>

#include "hazkey_candidate.h"
#include "hazkey_preedit.h"

namespace fcitx {

class HazkeyEngine;

class HazkeyState : public InputContextProperty {
   public:
    HazkeyState(HazkeyEngine *engine, InputContext *ic)
        : engine_(engine), ic_(ic), preedit_(HazkeyPreedit(ic)) {
        composingText_ = nullptr;
    }

    // complete the prefix and remove from composingText_
    void completePrefix(int correspondingCount);
    // handle key event. call candidateKeyEvent or preeditKeyEvent
    // depends on the current mode
    void keyEvent(KeyEvent &keyEvent);
    // void loadConfig(std::shared_ptr<HazkeyConfig> &config);
    //  reset to the initial state
    void reset();

   private:
    const KeyList defaultSelectionKeys = {
        Key{FcitxKey_1}, Key{FcitxKey_2}, Key{FcitxKey_3}, Key{FcitxKey_4},
        Key{FcitxKey_5}, Key{FcitxKey_6}, Key{FcitxKey_7}, Key{FcitxKey_8},
        Key{FcitxKey_9}, Key{FcitxKey_0},
    };
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
                           std::shared_ptr<HazkeyCandidateList> candidateList);
    // handle key event in preedit mode with prediction candidate list
    void preeditPredictingKeyEvent(
        KeyEvent &keyEvent,
        std::shared_ptr<HazkeyCandidateList> PreeditCandidateList);
    // handle key event in preedit mode
    void preeditKeyEvent(KeyEvent &keyEvent);

    // base function to prepare candidate list
    // make sure composingText_ is not nullptr
    void showCandidateList(showCandidateMode mode, int nBest);
    std::vector<std::vector<std::string>> getCandidates(bool isPredictMode,
                                                        int nBest);
    std::unique_ptr<HazkeyCandidateList> createCandidateList(
        std::vector<std::vector<std::string>> candidates,
        std::shared_ptr<std::vector<std::string>> preeditSegments);

    // prepare candidate list for normal conversion
    void showNonPredictCandidateList();
    // prepare candidate
    // list for prediction.
    // shorter than normal
    void showPreeditCandidateList();

    // update the candidate cursor
    void updateCandidateCursor(
        std::shared_ptr<HazkeyCandidateList> candidateList);
    // advance the cursor in
    // the candidate list,
    // update aux, set
    // preedit text
    void advanceCandidateCursor(
        std::shared_ptr<HazkeyCandidateList> candidateList);
    // back the cursor in
    // the candidate list,
    // update aux, set
    // preedit text
    void backCandidateCursor(
        std::shared_ptr<HazkeyCandidateList> candidateList);
    // update aux; label on
    // the candidate list
    // like "[1/100]"
    void setCandidateCursorAUX(
        std::shared_ptr<HazkeyCandidateList> candidateList);
    // UpAUX that shows unconverted text
    void setHiraganaAUX();
    // check if the key
    // event is inputable
    // (simple key / kana
    // key) or not
    bool isInputableEvent(const KeyEvent &keyEvent);
    // engine
    HazkeyEngine *engine_;
    // fcitx input context
    // pointer
    InputContext *ic_;
    // preedit class
    HazkeyPreedit preedit_;
    // composing text
    // information pointer
    // used by
    // libhazkey-kkc
    ComposingText *composingText_;
};

}  // namespace fcitx

#endif  // _FCITX5_HAZUKEY_HAZUKEY_STATE_H_
