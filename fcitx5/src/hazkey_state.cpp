#include "hazkey_state.h"

#include <cstring>

#include "../../azookey-kkc/libhazkey.h"
#include "hazkey_candidate.h"
#include "hazkey_engine.h"

namespace fcitx {

HazkeyState::HazkeyState(HazkeyEngine *engine, InputContext *ic)
    : engine_(engine), ic_(ic), preedit_(HazkeyPreedit(ic)) {
    composingText_ = nullptr;
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

    if (candidateList != nullptr && candidateList->focused() &&
        !event.isRelease()) {
        candidateKeyEvent(event, candidateList);
    } else if (composingText_ != nullptr && !event.isRelease()) {
        preeditKeyEvent(event, candidateList);
    } else if (!event.isRelease()) {
        noPreeditKeyEvent(event);
    } else if (composingText_ != nullptr && candidateList != nullptr &&
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
    } else if (composingText_ != nullptr) {
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
                composingText_ = kkc_get_composing_text_instance();
                kkc_input_text(composingText_, engine_->getKkcConfig(),
                               Key::keySymToUTF8(keysym).c_str(),
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
            kkc_delete_backward(composingText_);
            showPreeditCandidateList();
            break;
        case FcitxKey_Delete:
            kkc_delete_forward(composingText_);
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
            kkc_move_cursor(composingText_, -1);
            break;
        case FcitxKey_Right:
            if (isCursorMoving_) {
                bool moved = kkc_move_cursor(composingText_, 1);
                if (!moved) {
                    isCursorMoving_ = false;
                }
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
                } else {
                    kkc_input_text(composingText_, engine_->getKkcConfig(),
                                   Key::keySymToUTF8(keysym).c_str(),
                                   isDirectInputMode_);
                    showPreeditCandidateList();
                }
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
                composingText_ = kkc_get_composing_text_instance();
                kkc_input_text(composingText_, engine_->getKkcConfig(),
                               Key::keySymToUTF8(keysym).c_str(),
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
        auto correspondingCount =
            candidateList->getCandidate(candidateList->cursorIndex())
                .correspondingCount();
        kkc_complete_prefix(composingText_, correspondingCount);
        showNonPredictCandidateList();
    } else {
        reset();
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
    char *converted = nullptr;
    switch (mode) {
        case ConversionMode::Hiragana:
            converted = kkc_get_composing_hiragana(composingText_);
            break;
        case ConversionMode::KatakanaFullwidth:
            converted = kkc_get_composing_katakana_fullwidth(composingText_);
            break;
        case ConversionMode::KatakanaHalfwidth:
            converted = kkc_get_composing_katakana_halfwidth(composingText_);
            break;
        case ConversionMode::RawFullwidth:
            converted = kkc_get_composing_alphabet_fullwidth(
                composingText_, preedit_.text().c_str());
            break;
        case ConversionMode::RawHalfwidth:
            converted = kkc_get_composing_alphabet_halfwidth(
                composingText_, preedit_.text().c_str());
            break;
        default:
            return;
    }
    preedit_.setSimplePreedit(converted);
    kkc_free_text(converted);
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

    auto preeditSegmentsPtr = std::make_shared<std::vector<std::string>>();

    auto candidates = getCandidates(enabledPredictMode, nBest);

    auto candidateList = std::make_unique<HazkeyCandidateList>(
        std::move(candidates), preeditSegmentsPtr);

    candidateList->setSelectionKey(defaultSelectionKeys);

    ic_->inputPanel().reset();

    if (!preeditSegmentsPtr->empty()) {
        // preedit conversion is enabled and conversion result is found
        // show preedit conversion result
        preedit_.setMultiSegmentPreedit(*preeditSegmentsPtr, 0);
    } else {
        // preedit conversion is disabled or conversion result is not
        // available show hiragana preedit
        auto hiragana = kkc_get_composing_hiragana(composingText_);
        preedit_.setSimplePreedit(hiragana);
        kkc_free_text(hiragana);
    }

    ic_->inputPanel().setCandidateList(std::move(candidateList));
}

std::vector<std::vector<std::string>> HazkeyState::getCandidates(
    bool enabledPreeditConversion, int nBest) {
    auto ***candidates =
        kkc_get_candidates(composingText_, engine_->getKkcConfig(),
                           enabledPreeditConversion, nBest);
    std::vector<std::vector<std::string>> candidateList;
    for (int i = 0; candidates[i] != nullptr; i++) {
        std::vector<std::string> candidate;
        for (int j = 0; candidates[i][j] != nullptr; j++) {
            candidate.push_back(candidates[i][j]);
        }
        candidateList.push_back(candidate);
    }
    kkc_free_candidates(candidates);
    return candidateList;
}

void HazkeyState::showNonPredictCandidateList() {
    if (composingText_ == nullptr) {
        return;
    }

    showCandidateList(showCandidateMode::NonPredictWithFirstPreedit,
                      NormalCandidateListSize);

    auto newCandidateList = std::dynamic_pointer_cast<HazkeyCandidateList>(
        ic_->inputPanel().candidateList());
    newCandidateList->focus();

    setCandidateCursorAUX(
        std::static_pointer_cast<HazkeyCandidateList>(newCandidateList));
}

void HazkeyState::showPreeditCandidateList() {
    if (composingText_ == nullptr) {
        return;
    }
    auto hiragana = kkc_get_composing_hiragana(composingText_);
    if (hiragana == nullptr || strlen(hiragana) == 0) {
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
    auto hiragana = kkc_get_composing_hiragana_with_cursor(composingText_);
    auto newAuxText = Text(hiragana);
    // newAuxText.setCursor(1); // not working
    ic_->inputPanel().setAuxUp(newAuxText);
    kkc_free_text(hiragana);
}

/// Reset

void HazkeyState::reset() {
    FCITX_DEBUG() << "HazkeyState reset";
    if (composingText_ != nullptr) {
        kkc_free_composing_text_instance(composingText_);
    }
    isDirectConversionMode_ = false;
    // do not reset isShiftPressedAlone_ because shift may still be pressed
    isDirectInputMode_ = false;
    isCursorMoving_ = false;
    cursorIndex_ = 0;
    ic_->inputPanel().reset();
    composingText_ = nullptr;
}

}  // namespace fcitx
