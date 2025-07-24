#include "hazkey_state.h"

#include <string>
#include <vector>

#include "hazkey_candidate.h"
#include "hazkey_engine.h"

namespace fcitx {

HazkeyState::HazkeyState(HazkeyEngine *engine, InputContext *ic)
    : engine_(engine), ic_(ic), preedit_(HazkeyPreedit(ic)) {
      createComposingTextInstance(engine_->socket());
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

    std::string composingText = getComposingText(engine_->socket(), "hiragana");

    if (candidateList != nullptr && candidateList->focused() &&
        !event.isRelease()) {
        candidateKeyEvent(event, candidateList);
    } else if (composingText != "" && !event.isRelease()) {
        preeditKeyEvent(event, candidateList);
    } else if (!event.isRelease()) {
        noPreeditKeyEvent(event);
    } else if (composingText != "" && candidateList != nullptr &&
               !candidateList->focused()) {
        setAuxDownText(std::string("[Alt+数字で選択]"));
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
                newComposingText();
                // kkc_input_text(,
                //                Key::keySymToUTF8(keysym),
                //                isDirectInputMode_);
                addToComposingText(engine_->socket(), Key::keySymToUTF8(keysym),
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
            deleteLeft(engine_->socket());
            showPreeditCandidateList();
            break;
        case FcitxKey_Delete:
            deleteRight(engine_->socket());
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
            moveCursor(engine_->socket(), -1);
            break;
        case FcitxKey_Right:
            if (isCursorMoving_) {
                // let moved =
                moveCursor(engine_->socket(), 1);
                // if (!moved) {
                //     isCursorMoving_ = false;
                // }
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
                    newComposingText();
                }
                addToComposingText(engine_->socket(), Key::keySymToUTF8(keysym),
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
                newComposingText();
                addToComposingText(engine_->socket(), Key::keySymToUTF8(keysym),
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
        completePrefix(engine_->socket());
        showNonPredictCandidateList();
    } else {
        reset();
    }
}

void HazkeyState::newComposingText() {
    FCITX_DEBUG() << "DISABLED: newComposingText()";
    // if (composingText != "") {
    //     kkc_free_composing_text_instance(composingText_);
    // }
    // composingText_ = kkc_get_composing_text_instance();
    // updateSurroundingText();
}

void HazkeyState::updateSurroundingText() {
    if (engine_->config().zenzaiSurroundingTextEnabled.value()) {
        if (ic_->capabilityFlags().test(CapabilityFlag::SurroundingText) &&
            ic_->surroundingText().isValid()) {
            auto &surroundingText = ic_->surroundingText();
            setLeftContext(engine_->socket(), surroundingText.text(),
                           surroundingText.anchor());
        }
    } else {
        setLeftContext(engine_->socket(), "", 0);
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
        case FcitxKey_Muhenkan:
            directCharactorConversion(ConversionMode::RawHalfwidth);
            break;
        default:
            break;
    }
    isDirectConversionMode_ = true;
}

void HazkeyState::directCharactorConversion(ConversionMode mode) {
    std::string converted = nullptr;
    int sock = engine_->socket();
    // TODO: cleanup
    switch (mode) {
        case ConversionMode::Hiragana:
            converted = getComposingText(sock, "hiragana");
            break;
        case ConversionMode::KatakanaFullwidth:
            converted = getComposingText(sock, "katakana_fullwidth");
            break;
        case ConversionMode::KatakanaHalfwidth:
            converted = getComposingText(sock, "katakana_halfwidth");
            break;
        case ConversionMode::RawFullwidth:
            converted = getComposingText(sock, "alphabet_fullwidth");
            break;
        case ConversionMode::RawHalfwidth:
            converted = getComposingText(sock, "alphabet_halfwidth");
            break;
        default:
            return;
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

    // auto preeditSegmentsPtr = std::make_shared<std::vector<std::string>>();

    auto candidates = getCandidates(enabledPredictMode, nBest);

    auto candidateList = std::make_unique<HazkeyCandidateList>(
        std::move(candidates));

    candidateList->setSelectionKey(defaultSelectionKeys);

    ic_->inputPanel().reset();

    // if (!preeditSegmentsPtr->empty()) {
    //     // preedit conversion is enabled and conversion result is found
    //     // show preedit conversion result
    //     preedit_.setMultiSegmentPreedit(*preeditSegmentsPtr, -1);
    // } else {
        // preedit conversion is disabled or conversion result is not
        // available show hiragana preedit
        auto hiragana = getComposingText(engine_->socket(), "hiragana");
        preedit_.setSimplePreedit(hiragana);
    // }

    ic_->inputPanel().setCandidateList(std::move(candidateList));
}

std::vector<std::string> HazkeyState::getCandidates(
    bool enabledPreeditConversion, int nBest) {
    std::vector<std::string> candidates = getServerCandidates(engine_->socket());
    return candidates;
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
    auto hiragana = getComposingText(engine_->socket(), "hiragana");
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
    setAuxDownText(std::string("[Alt+数字で選択]"));
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
    auto hiragana = getComposingHiraganaWithCursor(engine_->socket());
    auto newAuxText = Text(hiragana);
    // newAuxText.setCursor(1); // not working
    ic_->inputPanel().setAuxUp(newAuxText);
}

/// Reset

void HazkeyState::reset() {
    FCITX_DEBUG() << "HazkeyState reset";
    // isDirectConversionMode_ = false;
    // // do not reset isShiftPressedAlone_ because shift may still be pressed
    // isDirectInputMode_ = false;
    // isCursorMoving_ = false;
    // cursorIndex_ = 0;
    // ic_->inputPanel().reset();
    // composingText_ = nullptr;
}

}  // namespace fcitx
