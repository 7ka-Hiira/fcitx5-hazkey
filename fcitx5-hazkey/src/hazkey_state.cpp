#include "hazkey_state.h"

#include <fcitx-utils/log.h>

#include <optional>
#include <string>
#include <vector>

#include "hazkey_candidate.h"
#include "hazkey_engine.h"
#include "hazkey_server_connector.h"
#include "protocol/hazkey_server.pb.h"

namespace fcitx {

HazkeyState::HazkeyState(HazkeyEngine *engine, InputContext *ic)
    : engine_(engine), ic_(ic), preedit_(HazkeyPreedit(ic)) {
    engine_->server().createComposingTextInstance();
}

bool HazkeyState::isInputableEvent(const KeyEvent &event) {
    auto key = event.key();
    if (key.check(FcitxKey_space) || key.isSimple() ||
        Key::keySymToUTF8(key.sym()).size() > 1 ||
        (key.sym() >= 0x04a1 && key.sym() <= 0x04df)) {
        // 0x04a1 - 0x04dd is the range of kana keys
        return true;
    }
    return false;
}

void HazkeyState::keyEvent(KeyEvent &event) {
    FCITX_DEBUG() << "HazkeyState keyEvent";

    // Alphabet + Shift to enter direct input mode
    // Pressing only Shift key to toggle direct input mode
    if (!event.isRelease() &&
        // FcitxKey_A-Z means shifted LAZ keys, enter direct mode even if shift
        // is not pressed alone
        // Ctrl & Alt also make alphabets Upper case so we need to check
        // Shift state can't be detected
        (event.key().sym() >= FcitxKey_A && event.key().sym() <= FcitxKey_Z &&
         event.key().states() == KeyState::NoState)) {
        isDirectInputMode_ = true;
        isShiftPressedAlone_ = false;
    } else if (event.isRelease() &&
               (event.key().sym() == FcitxKey_Shift_L ||
                event.key().sym() == FcitxKey_Shift_R) &&
               isShiftPressedAlone_) {
        isDirectInputMode_ = !isDirectInputMode_;
        isShiftPressedAlone_ = false;
    } else if (!event.isRelease() && isShiftPressedAlone_) {
        isShiftPressedAlone_ = false;
    } else if (!event.isRelease() && (event.key().sym() == FcitxKey_Shift_L ||
                                      event.key().sym() == FcitxKey_Shift_R)) {
        isShiftPressedAlone_ = true;
    }

    auto candidateList = std::dynamic_pointer_cast<HazkeyCandidateList>(
        event.inputContext()->inputPanel().candidateList());

    std::string composingText = engine_->server().getComposingText(
        hazkey::commands::QueryData_GetComposingStringProps_CharType_HIRAGANA,
        preedit_.text());

    if (candidateList != nullptr && candidateList->focused() &&
        !event.isRelease()) {
        candidateKeyEvent(event, candidateList);
    } else if (composingText != "" && !event.isRelease()) {
        preeditKeyEvent(event, candidateList);
    } else if (!event.isRelease()) {
        noPreeditKeyEvent(event);
    } else if (composingText != "" && candidateList != nullptr &&
               !candidateList->focused()) {
        setAuxDownText(std::string(_("[Alt+Number to Select]")));
    } else {
        setAuxDownText(std::nullopt);
    }

    if (event.isRelease()) {
        return;
    }

    auto newCandidateList = std::dynamic_pointer_cast<HazkeyCandidateList>(
        ic_->inputPanel().candidateList());
    if (newCandidateList != nullptr && newCandidateList->focused()) {
        setCandidateCursorAUX(newCandidateList);
    } else if (composingText != "") {
        setHiraganaAUX();
    }
}

void HazkeyState::noPreeditKeyEvent(KeyEvent &event) {
    FCITX_DEBUG() << "HazkeyState noPredictKeyEvent";

    auto key = event.key();
    auto keysym = key.sym();

    switch (keysym) {
        case FcitxKey_space:
            if (*engine_->config().spaceStyle == SpaceStyle::Fullwidth &&
                !isDirectInputMode_ && key.states() != KeyState::Shift) {
                ic_->commitString("　");
            } else {
                ic_->commitString(" ");
            }
            break;
        default:
            if (isInputableEvent(event)) {
                updateSurroundingText();
                engine_->server().addToComposingText(Key::keySymToUTF8(keysym),
                                                     isDirectInputMode_);
                showPreeditCandidateList();
                setHiraganaAUX();
            } else {
                isDirectInputMode_ = false;
                return event.filter();
            }
            break;
    }

    return event.filterAndAccept();
}

void HazkeyState::preeditKeyEvent(
    KeyEvent &event,
    std::shared_ptr<HazkeyCandidateList> PredictCandidateList) {
    FCITX_DEBUG() << "HazkeyState preeditKeyEvent";

    auto key = event.key();
    auto keysym = key.sym();

    switch (keysym) {
        case FcitxKey_Return:
            preedit_.commitPreedit();
            reset();
            break;
        case FcitxKey_BackSpace:
            engine_->server().deleteLeft();
            showPreeditCandidateList();
            break;
        case FcitxKey_Delete:
            engine_->server().deleteRight();
            showPreeditCandidateList();
            break;
        case FcitxKey_F6:
        case FcitxKey_F7:
        case FcitxKey_F8:
        case FcitxKey_F9:
        case FcitxKey_F10:
        case FcitxKey_Muhenkan:
            functionKeyHandler(event);
            break;
        case FcitxKey_Escape:
            reset();
            break;
        case FcitxKey_space:
        case FcitxKey_Henkan:
            showNonPredictCandidateList();
            break;
        case FcitxKey_Up:
        case FcitxKey_Down:
        case FcitxKey_Tab:
            if (PredictCandidateList == nullptr) {
                showNonPredictCandidateList();
            } else {
                PredictCandidateList->focus();
                updateCandidateCursor(PredictCandidateList);
            }
            break;
        case FcitxKey_Left:
            isCursorMoving_ = true;
            engine_->server().moveCursor(-1);
            break;
        case FcitxKey_Right:
            if (isCursorMoving_) {
                engine_->server().moveCursor(1);
            }
            break;
        default:
            if (isAltDigitKeyEvent(event)) {
                if (PredictCandidateList != nullptr) {
                    auto localIndex = keysym - FcitxKey_1;
                    if (localIndex < PredictCandidateListSize) {
                        PredictCandidateList->setCursorIndex(localIndex);
                        candidateCompleteHandler(PredictCandidateList);
                    }
                }
            } else if (isInputableEvent(event)) {
                if (isDirectConversionMode_) {
                    preedit_.commitPreedit();
                    reset();
                }
                engine_->server().addToComposingText(Key::keySymToUTF8(keysym),
                                                     isDirectInputMode_);
                showPreeditCandidateList();
            }
            break;
    }
    return event.filterAndAccept();
}

bool HazkeyState::isAltDigitKeyEvent(const KeyEvent &event) {
    auto key = event.key();
    if (key.states() == KeyState::Alt && key.sym() >= FcitxKey_1 &&
        key.sym() <= FcitxKey_9) {
        return true;
    }
    return false;
}

void HazkeyState::candidateKeyEvent(
    KeyEvent &event, std::shared_ptr<HazkeyCandidateList> candidateList) {
    FCITX_DEBUG() << "HazkeyState candidateKeyEvent";

    auto key = event.key();
    auto keysym = key.sym();

    std::vector<std::string> preedit;
    switch (keysym) {
        case FcitxKey_Right:
            candidateList->nextPage();
            break;
        case FcitxKey_Left:
            candidateList->prevPage();
            break;
        case FcitxKey_Return:
            candidateCompleteHandler(candidateList);
            break;
        case FcitxKey_Escape:
        case FcitxKey_BackSpace:
            showPreeditCandidateList();
            break;
        case FcitxKey_space:
        case FcitxKey_Tab:
            if (key.states() == KeyState::Shift) {
                backCandidateCursor(candidateList);
            } else if (key.states() == KeyState::Alt_Shift) {
                // do nothing
            } else {
                advanceCandidateCursor(candidateList);
            }
            break;
        case FcitxKey_Down:
            advanceCandidateCursor(candidateList);
            break;
        case FcitxKey_Up:
            backCandidateCursor(candidateList);
            break;
        case FcitxKey_F6:
        case FcitxKey_F7:
        case FcitxKey_F8:
        case FcitxKey_F9:
        case FcitxKey_F10:
            functionKeyHandler(event);
            break;
        default:
            if (isAltDigitKeyEvent(event) ||
                key.checkKeyList(defaultSelectionKeys)) {
                auto localIndex = isAltDigitKeyEvent(event)
                                      ? keysym - FcitxKey_1
                                      : key.keyListIndex(defaultSelectionKeys);
                if (localIndex < candidateList->size()) {
                    candidateList->setCursorIndex(localIndex);
                    candidateCompleteHandler(candidateList);
                }
            } else if (isInputableEvent(event)) {
                preedit_.commitPreedit();
                reset();
                engine_->server().addToComposingText(Key::keySymToUTF8(keysym),
                                                     isDirectInputMode_);
                showPreeditCandidateList();
            } else {
                return event.filter();
            }
            break;
    }
    return event.filterAndAccept();
}

void HazkeyState::candidateCompleteHandler(
    std::shared_ptr<HazkeyCandidateList> candidateList) {
    auto preedit =
        candidateList->getCandidate(candidateList->cursorIndex()).getPreedit();
    ic_->commitString(preedit[0]);
    if (preedit.size() > 1) {
        // auto correspondingCount =
        //     candidateList->getCandidate(candidateList->cursorIndex())
        //         .correspondingCount();
        engine_->server().completePrefix(candidateList->globalCursorIndex());
        showNonPredictCandidateList();
    } else {
        reset();
    }
}

void HazkeyState::updateSurroundingText() {
    if (engine_->config().zenzaiSurroundingTextEnabled.value() &&
        ic_->capabilityFlags().test(CapabilityFlag::SurroundingText) &&
        ic_->surroundingText().isValid()) {
        auto &surroundingText = ic_->surroundingText();
        engine_->server().setLeftContext(surroundingText.text(),
                                         surroundingText.anchor());
    } else {
        engine_->server().setLeftContext("", 0);
    }
}

void HazkeyState::functionKeyHandler(KeyEvent &event) {
    auto keysym = event.key().sym();
    switch (keysym) {
        case FcitxKey_F6:
            directCharactorConversion(ConversionMode::Hiragana);
            break;
        case FcitxKey_F7:
            directCharactorConversion(ConversionMode::KatakanaFullwidth);
            break;
        case FcitxKey_F8:
            directCharactorConversion(ConversionMode::KatakanaHalfwidth);
            break;
        case FcitxKey_F9:
            directCharactorConversion(ConversionMode::RawFullwidth);
            break;
        case FcitxKey_F10:
            directCharactorConversion(ConversionMode::RawHalfwidth);
            break;
        default:
            FCITX_ERROR() << "functionKeyHandler: unhandled key code: "
                          << keysym;
            return;
    }
    isDirectConversionMode_ = true;
}

void HazkeyState::directCharactorConversion(ConversionMode mode) {
    std::string converted;
    // TODO: cleanup
    switch (mode) {
        case ConversionMode::Hiragana:
            converted = engine_->server().getComposingText(
                hazkey::commands::
                    QueryData_GetComposingStringProps_CharType_HIRAGANA,
                preedit_.text());
            break;
        case ConversionMode::KatakanaFullwidth:
            converted = engine_->server().getComposingText(
                hazkey::commands::
                    QueryData_GetComposingStringProps_CharType_KATAKANA_FULL,
                preedit_.text());
            break;
        case ConversionMode::KatakanaHalfwidth:
            converted = engine_->server().getComposingText(
                hazkey::commands::
                    QueryData_GetComposingStringProps_CharType_KATAKANA_HALF,
                preedit_.text());
            break;
        case ConversionMode::RawFullwidth:
            converted = engine_->server().getComposingText(
                hazkey::commands::
                    QueryData_GetComposingStringProps_CharType_ALPHABET_FULL,
                preedit_.text());
            break;
        case ConversionMode::RawHalfwidth:
            converted = engine_->server().getComposingText(
                hazkey::commands::
                    QueryData_GetComposingStringProps_CharType_ALPHABET_HALF,
                preedit_.text());
            break;
    }
    preedit_.setSimplePreeditHighlighted(converted);
    auto candidateList = ic_->inputPanel().candidateList();
    if (candidateList) {
        ic_->inputPanel().setCandidateList(nullptr);
        setAuxDownText(std::nullopt);
    }
}

/// Show Candidate List

void HazkeyState::showCandidateList(showCandidateMode mode, int nBest) {
    FCITX_DEBUG() << "HazkeyState showCandidateList";

    bool enabledPredictMode = mode == showCandidateMode::PredictWithLivePreedit;

    std::optional<std::string> livePreedit = std::nullopt;

    auto candidates =
        engine_->server().getCandidates(enabledPredictMode, nBest);

    // search live text compatible candidate
    for (auto candidate : candidates) {
        if (candidate.liveCompat) {
            livePreedit = candidate.candidateText;
            break;
        }
    }

    auto candidateList =
        std::make_unique<HazkeyCandidateList>(std::move(candidates));

    candidateList->setSelectionKey(defaultSelectionKeys);

    ic_->inputPanel().reset();

    if (livePreedit != std::nullopt) {
        // preedit conversion is enabled and conversion result is found
        // show preedit conversion result
        preedit_.setSimplePreedit(livePreedit.value());
    } else {
        // preedit conversion is disabled or conversion result is not
        // available show hiragana preedit
        auto hiragana = engine_->server().getComposingText(
            hazkey::commands::
                QueryData_GetComposingStringProps_CharType_HIRAGANA,
            preedit_.text());
        preedit_.setSimplePreedit(hiragana);
    }

    ic_->inputPanel().setCandidateList(std::move(candidateList));
}

void HazkeyState::showNonPredictCandidateList() {
    showCandidateList(showCandidateMode::NonPredictWithFirstPreedit,
                      NormalCandidateListSize);

    // highlight all preedit text
    // because the first candidate is the result of all preedit text.
    auto currentPreedit = preedit_.text();
    preedit_.setSimplePreeditHighlighted(currentPreedit);

    auto newCandidateList = std::dynamic_pointer_cast<HazkeyCandidateList>(
        ic_->inputPanel().candidateList());
    newCandidateList->focus();

    setCandidateCursorAUX(
        std::static_pointer_cast<HazkeyCandidateList>(newCandidateList));
}

void HazkeyState::showPreeditCandidateList() {
    auto hiragana = engine_->server().getComposingText(
        hazkey::commands::QueryData_GetComposingStringProps_CharType_HIRAGANA,
        preedit_.text());
    if (hiragana == "" || hiragana.length() == 0) {
        reset();
        return;
    }

    auto mode = *engine_->config().enablePrediction
                    ? showCandidateMode::PredictWithLivePreedit
                    : showCandidateMode::NonPredictWithFirstPreedit;

    showCandidateList(mode, PredictCandidateListSize);

    auto newCandidateList = std::dynamic_pointer_cast<HazkeyCandidateList>(
        ic_->inputPanel().candidateList());
    newCandidateList->setPageSize(PredictCandidateListSize);
    setAuxDownText(std::string(_("[Alt+Number to Select]")));
}

/// Candidate Cursor

void HazkeyState::updateCandidateCursor(
    std::shared_ptr<HazkeyCandidateList> candidateList) {
    setCandidateCursorAUX(candidateList);
    auto text =
        candidateList->getCandidate(candidateList->cursorIndex()).getPreedit();
    preedit_.setMultiSegmentPreedit(text, 0);
}

void HazkeyState::advanceCandidateCursor(
    std::shared_ptr<HazkeyCandidateList> candidateList) {
    candidateList->nextCandidate();
    updateCandidateCursor(candidateList);
}

void HazkeyState::backCandidateCursor(
    std::shared_ptr<HazkeyCandidateList> candidateList) {
    candidateList->prevCandidate();
    updateCandidateCursor(candidateList);
}

/// AUX

void HazkeyState::setCandidateCursorAUX(
    std::shared_ptr<HazkeyCandidateList> candidateList) {
    auto label = "[" + std::to_string(candidateList->globalCursorIndex() + 1) +
                 "/" + std::to_string(candidateList->totalSize()) + "]";
    ic_->inputPanel().setAuxUp(Text(label));
    setAuxDownText(std::nullopt);
}

void HazkeyState::setAuxDownText(std::optional<std::string> optText) {
    auto aux = Text();
    if (isDirectInputMode_) {
        // appending fcitx::Text is supported only >= 5.1.9
        aux.append(std::string("[直接入力]"));
    } else if (optText != std::nullopt) {
        aux.append(optText.value());
    }
    ic_->inputPanel().setAuxDown(aux);
}

void HazkeyState::setHiraganaAUX() {
    auto hiragana = engine_->server().getComposingHiraganaWithCursor();
    auto newAuxText = Text(hiragana);
    // newAuxText.setCursor(1); // not working
    ic_->inputPanel().setAuxUp(newAuxText);
}

/// Reset

void HazkeyState::reset() {
    FCITX_DEBUG() << "HazkeyState reset";
    isDirectConversionMode_ = false;
    // do not reset isShiftPressedAlone_ because shift may still be pressed
    isDirectInputMode_ = false;
    isCursorMoving_ = false;
    engine_->server().createComposingTextInstance();
    ic_->inputPanel().reset();
}

}  // namespace fcitx
