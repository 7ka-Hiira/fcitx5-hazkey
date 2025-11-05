#ifndef _FCITX5_HAZKEY_HAZKEY_STATE_H_
#define _FCITX5_HAZKEY_HAZKEY_STATE_H_

#include <fcitx/inputcontext.h>
#include <fcitx/inputpanel.h>
#include <fcitx/surroundingtext.h>

#include "hazkey_candidate.h"
#include "hazkey_preedit.h"

namespace fcitx {

class HazkeyEngine;

class HazkeyState : public InputContextProperty {
   public:
    HazkeyState(HazkeyEngine* engine, InputContext* ic);

    // complete the prefix and remove from composingText_
    void candidateCompleteHandler(
        std::shared_ptr<HazkeyCandidateList> candidateList);
    void commitPreedit();
    // handle key event. call candidateKeyEvent or preeditNoPredictKeyEvent
    // depends on the current mode
    void keyEvent(KeyEvent& keyEvent);
    // void loadConfig(std::shared_ptr<HazkeyConfig> &config);
    //  reset to the initial state
    void reset();

   private:
    enum class ConversionMode {
        Hiragana,
        KatakanaFullwidth,
        KatakanaHalfwidth,
        RawFullwidth,
        RawHalfwidth,
    };

    enum class showCandidateMode {
        PredictWithLivePreedit,
        NonPredictWithFirstPreedit,
    };

    // update surrounding text
    void updateSurroundingText(std::string appendText = "");

    bool ctrlShortcutHandler(KeyEvent& keyEvent);
    // f6-f10 key handler
    void functionKeyHandler(KeyEvent& keyEvent);
    // convert to hiragana/katakana/alphanumeric directly
    void directCharactorConversion(ConversionMode mode);
    // handle key event in normal mode (no preedit)
    void noPreeditKeyEvent(KeyEvent& keyEvent);
    // handle key event in candidate mode
    void candidateKeyEvent(KeyEvent& keyEvent,
                           std::shared_ptr<HazkeyCandidateList> candidateList);
    // handle key event in preedit mode
    void preeditKeyEvent(
        KeyEvent& keyEvent,
        std::shared_ptr<HazkeyCandidateList> PreeditCandidateList);
    // base function to prepare candidate list
    // make sure composingText_ is not nullptr
    bool showCandidateList(bool isSuggest);
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
    // set AuxDown
    // like "[Tabキーで選択]" or "[直接入力]"
    void setAuxDownText(std::optional<std::string>);
    // UpAUX that shows unconverted text
    void setHiraganaAUX();
    // check if the key
    // event is inputable
    // (simple key / kana
    // key) or not
    bool isInputableEvent(const KeyEvent& keyEvent);

    bool isAltDigitKeyEvent(const KeyEvent& keyEvent);

    bool isCursorMoving_ = false;

    bool isDirectConversionMode_ = false;
    int livePreeditIndex_ = -1;
    // engine
    HazkeyEngine* engine_;
    // fcitx input context
    // pointer
    InputContext* ic_;
    // preedit class
    HazkeyPreedit preedit_;
};

}  // namespace fcitx

#endif  // _FCITX5_HAZKEY_HAZKEY_STATE_H_
