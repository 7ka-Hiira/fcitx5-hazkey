#ifndef _FCITX5_AZOOKEY_AZOOKEY_STATE_H_
#define _FCITX5_AZOOKEY_AZOOKEY_STATE_H_

#include <azookey/libazookey_kkc.h>
#include <fcitx/inputcontext.h>
#include <fcitx/inputpanel.h>

#include "azookey_candidate.h"
#include "azookey_config.h"
#include "azookey_preedit.h"

namespace fcitx {
class azooKeyState : public InputContextProperty {
   public:
    azooKeyState(InputContext *ic) : ic_(ic), preedit_(azooKeyPreedit(ic)) {
        composingText_ = nullptr;
    }

    // handle key event. call candidateKeyEvent or preeditKeyEvent
    // depends on the current mode
    void keyEvent(KeyEvent &keyEvent);
    // update the UI: candidate list and preedit text
    // other funcions don't update the UI so you need to call this
    void updateUI();
    void loadConfig(std::shared_ptr<azooKeyConfig> &config);
    // reset to the initial state
    void reset();

   private:
    enum class ConversionMode {
        None,
        Hiragana,
        KatakanaFullwidth,
        KatakanaHalfwidth,
        RawFullwidthUpper,
        RawFullwidthLower,
        RawFullwidthCapitalized,
        RawHalfwidthUpper,
        RawHalfwidthLower,
        RawHalfwidthCapitalized
    };

    void directCharactorConversion(ConversionMode mode);
    // handle key event in candidate mode
    void candidateKeyEvent(KeyEvent &keyEvent,
                           std::shared_ptr<azooKeyCandidateList> candidateList);
    // handle key event in preedit mode
    void preeditKeyEvent(
        KeyEvent &keyEvent,
        std::shared_ptr<azooKeyCandidateList> PreeditCandidateList);

    // base function to prepare candidate list
    void prepareCandidateList(bool isPredictMode, bool updatePreedit,
                              int nBest);
    // prepare candidate list for normal conversion
    void prepareNormalCandidateList();
    // prepare candidate list for prediction. shorter than normal
    void preparePredictCandidateList();

    // advance the cursor in the candidate list, update aux, set preedit text
    void advanceCandidateCursor(
        std::shared_ptr<azooKeyCandidateList> candidateList);
    // back the cursor in the candidate list, update aux, set preedit text
    void backCandidateCursor(
        std::shared_ptr<azooKeyCandidateList> candidateList);
    // update aux; label on the candidate list like "[1/100]"
    void setCandidateCursorAUX(
        std::shared_ptr<azooKeyCandidateList> candidateList);

    // prepare preedit text from the composing text
    void updateOnPreeditMode();

    // fcitx input context pointer
    InputContext *ic_;
    // shared pointer to the configuration
    std::shared_ptr<azooKeyConfig> config_;
    // preedit class
    azooKeyPreedit preedit_;
    // composing text information pointer used by libazookey-kkc
    ComposingText *composingText_;
    // true if the current mode is candidate mode
    bool isCandidateMode_ = false;
    // function key conversion mode
    ConversionMode FunctionConversionMode_ = ConversionMode::None;
};

}  // namespace fcitx

#endif  // _FCITX5_AZOOKEY_AZOOKEY_STATE_H_