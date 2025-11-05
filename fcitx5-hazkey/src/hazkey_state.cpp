#include "hazkey_state.h"

#include <fcitx-utils/key.h>
#include <fcitx-utils/log.h>
#include <fcitx/candidatelist.h>

#include <algorithm>
#include <optional>
#include <string>
#include <vector>

#include "commands.pb.h"
#include "fcitx-utils/keysym.h"
#include "hazkey_candidate.h"
#include "hazkey_engine.h"
#include "hazkey_server_connector.h"

namespace fcitx {

HazkeyState::HazkeyState(HazkeyEngine* engine, InputContext* ic)
    : engine_(engine), ic_(ic), preedit_(HazkeyPreedit(ic)) {
    engine_->server().newComposingText();
}

bool HazkeyState::isInputableEvent(const KeyEvent& event) {
    auto key = event.key();
    if (key.check(FcitxKey_space) || key.isSimple() ||
        Key::keySymToUTF8(key.sym()).size() > 1 ||
        (key.sym() >= 0x04a1 && key.sym() <= 0x04df)) {
        // 0x04a1 - 0x04dd is the range of kana keys
        return true;
    }
    return false;
}

void HazkeyState::commitPreedit() { preedit_.commitPreedit(); }

void HazkeyState::keyEvent(KeyEvent& event) {
    FCITX_DEBUG() << "HazkeyState keyEvent";

    std::string composingText = engine_->server().getComposingText(
        hazkey::commands::GetComposingString_CharType_HIRAGANA,
        preedit_.text());

    if (event.key().sym() == FcitxKey_Shift_L ||
        event.key().sym() == FcitxKey_Shift_R) {
        engine_->server().shiftKeyEvent(event.isRelease());
        if (composingText == "") {
            setAuxDownText(std::nullopt);
            return;
        }
    }

    auto candidateList = std::dynamic_pointer_cast<HazkeyCandidateList>(
        event.inputContext()->inputPanel().candidateList());

    if (candidateList != nullptr && candidateList->focused() &&
        !event.isRelease()) {
        candidateKeyEvent(event, candidateList);
    } else if (composingText != "" && !event.isRelease()) {
        preeditKeyEvent(event, candidateList);
    } else if (!event.isRelease()) {
        noPreeditKeyEvent(event);
    } else if (composingText != "" && candidateList != nullptr &&
               !candidateList->focused() &&
               engine_->config().showTabToSelect.value()) {
        setAuxDownText(std::string(_("[Press Tab to Select]")));
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

void HazkeyState::noPreeditKeyEvent(KeyEvent& event) {
    FCITX_DEBUG() << "HazkeyState noPredictKeyEvent";

    auto key = event.key();
    auto keysym = key.sym();

    switch (keysym) {
        case FcitxKey_Escape:
            reset();
            break;
        case FcitxKey_space:
            if (key.states() == KeyState::Shift) {
                ic_->commitString(" ");
                reset();
            } else {
                engine_->server().inputChar(" ");
                ic_->commitString(engine_->server().getComposingText(
                    hazkey::commands::GetComposingString_CharType::
                        GetComposingString_CharType_HIRAGANA,
                    ""));
                reset();
            }
            break;
        default:
            if (isInputableEvent(event)) {
                updateSurroundingText();
                engine_->server().inputChar(Key::keySymToUTF8(keysym));
                showPreeditCandidateList();
                setHiraganaAUX();
            } else {
                reset();
                return event.filter();
            }
            break;
    }

    return event.filterAndAccept();
}

void HazkeyState::preeditKeyEvent(
    KeyEvent& event,
    std::shared_ptr<HazkeyCandidateList> PredictCandidateList) {
    FCITX_DEBUG() << "HazkeyState preeditKeyEvent";

    auto key = event.key();
    auto keysym = key.sym();

    switch (keysym) {
        case FcitxKey_Return:
            preedit_.commitPreedit();
            if (livePreeditIndex_ >= 0) {
                engine_->server().completePrefix(livePreeditIndex_);
            }
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
            if (!isDirectConversionMode_ &&
                event.key().states() == KeyState::Shift) {
                engine_->server().inputChar(" ");
                showPreeditCandidateList();
            } else {
                showNonPredictCandidateList();
            }
            break;
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
            if (event.key().states() == KeyState::Ctrl) {
                ctrlShortcutHandler(event);
            } else if (isAltDigitKeyEvent(event)) {
                if (PredictCandidateList != nullptr) {
                    auto localIndex = keysym - FcitxKey_1;
                    if (localIndex < PredictCandidateList->pageSize()) {
                        PredictCandidateList->setCursorIndex(localIndex);
                        candidateCompleteHandler(PredictCandidateList);
                    }
                }
            } else if (isInputableEvent(event)) {
                if (isDirectConversionMode_) {
                    preedit_.commitPreedit();
                    reset();
                }
                engine_->server().inputChar(Key::keySymToUTF8(keysym));
                showPreeditCandidateList();
            }
            break;
    }
    return event.filterAndAccept();
}

bool HazkeyState::isAltDigitKeyEvent(const KeyEvent& event) {
    auto key = event.key();
    if (key.states() == KeyState::Alt && key.sym() >= FcitxKey_1 &&
        key.sym() <= FcitxKey_9) {
        return true;
    }
    return false;
}

void HazkeyState::candidateKeyEvent(
    KeyEvent& event, std::shared_ptr<HazkeyCandidateList> candidateList) {
    FCITX_DEBUG() << "HazkeyState candidateKeyEvent";

    auto key = event.key();
    auto keysym = key.sym();

    std::vector<std::string> preedit;
    switch (keysym) {
        case FcitxKey_Right:
            // if (event.key().states() == KeyState::Alt) {
            candidateList->nextPage();
            // }
            break;
        case FcitxKey_Left:
            // if (event.key().states() == KeyState::Alt) {
            candidateList->prevPage();
            // }
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
        case FcitxKey_Shift_L:
        case FcitxKey_Shift_R:

        default:
            if (event.key().states() == KeyState::Ctrl) {
                if (!ctrlShortcutHandler(event)) {
                    return event.filter();
                }
            } else if (isAltDigitKeyEvent(event) ||
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
                engine_->server().inputChar(Key::keySymToUTF8(keysym));
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
    // hazkey cannot get surroundingText correctly immediately after
    // committing so call it with appendText before committing.
    updateSurroundingText(preedit[0]);
    engine_->server().completePrefix(candidateList->globalCursorIndex());
    ic_->commitString(preedit[0]);
    if (preedit.size() > 1) {
        showNonPredictCandidateList();
    } else {
        reset();
    }
}

void HazkeyState::updateSurroundingText(std::string appendText) {
    if (ic_->capabilityFlags().test(CapabilityFlag::SurroundingText) &&
        ic_->surroundingText().isValid()) {
        auto& surroundingText = ic_->surroundingText();
        engine_->server().setContext(
            surroundingText.text() + appendText,
            surroundingText.anchor() + appendText.length());
    } else {
        engine_->server().setContext("", 0);
    }
}

bool HazkeyState::ctrlShortcutHandler(KeyEvent& event) {
    auto keysym = event.key().sym();
    switch (keysym) {
        case FcitxKey_u:
        case FcitxKey_U:
            directCharactorConversion(ConversionMode::Hiragana);
            isDirectConversionMode_ = true;
            break;
        case FcitxKey_i:
        case FcitxKey_I:
            directCharactorConversion(ConversionMode::KatakanaFullwidth);
            isDirectConversionMode_ = true;
            break;
        case FcitxKey_o:
        case FcitxKey_O:
            directCharactorConversion(ConversionMode::KatakanaHalfwidth);
            isDirectConversionMode_ = true;
            break;
        case FcitxKey_p:
        case FcitxKey_P:
            directCharactorConversion(ConversionMode::RawFullwidth);
            isDirectConversionMode_ = true;
            break;
        case FcitxKey_t:
        case FcitxKey_T:
            directCharactorConversion(ConversionMode::RawHalfwidth);
            isDirectConversionMode_ = true;
            break;
        default:
            FCITX_INFO() << "keysym" << keysym;
            return false;
    }
    return true;
}

void HazkeyState::functionKeyHandler(KeyEvent& event) {
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
    // TODO: use protobuf type for all program
    switch (mode) {
        case ConversionMode::Hiragana:
            converted = engine_->server().getComposingText(
                hazkey::commands::GetComposingString_CharType_HIRAGANA,
                preedit_.text());
            break;
        case ConversionMode::KatakanaFullwidth:
            converted = engine_->server().getComposingText(
                hazkey::commands::GetComposingString_CharType_KATAKANA_FULL,
                preedit_.text());
            break;
        case ConversionMode::KatakanaHalfwidth:
            converted = engine_->server().getComposingText(
                hazkey::commands::GetComposingString_CharType_KATAKANA_HALF,
                preedit_.text());
            break;
        case ConversionMode::RawFullwidth:
            converted = engine_->server().getComposingText(
                hazkey::commands::GetComposingString_CharType_ALPHABET_FULL,
                preedit_.text());
            break;
        case ConversionMode::RawHalfwidth:
            converted = engine_->server().getComposingText(
                hazkey::commands::GetComposingString_CharType_ALPHABET_HALF,
                preedit_.text());
            break;
    }
    preedit_.setSimplePreeditHighlighted(converted);
    livePreeditIndex_ = -1;
    auto candidateList = ic_->inputPanel().candidateList();
    if (candidateList) {
        ic_->inputPanel().setCandidateList(nullptr);
        setAuxDownText(std::nullopt);
    }
}

/// Show Candidate List

bool HazkeyState::showCandidateList(bool isSuggest) {
    FCITX_DEBUG() << "HazkeyState showCandidateList";

    auto response = engine_->server().getCandidates(isSuggest);

    auto candidateResult =
        std::make_unique<HazkeyCandidateList>(std::move(response.candidates()));

    candidateResult->setSelectionKey(defaultSelectionKeys);

    ic_->inputPanel().reset();

    // TODO: check live preedit config
    if (!response.live_text().empty()) {
        // preedit conversion is enabled and conversion result is found
        // show preedit conversion result
        preedit_.setSimplePreedit(response.live_text());
    } else {
        // preedit conversion is disabled or conversion result is not
        // available show hiragana preedit
        auto hiragana = engine_->server().getComposingText(
            hazkey::commands::GetComposingString_CharType_HIRAGANA,
            preedit_.text());
        preedit_.setSimplePreedit(hiragana);
    }

    livePreeditIndex_ = response.live_text_index();

    if (response.page_size() > 0) {
        ic_->inputPanel().setCandidateList(std::move(candidateResult));
        auto newFcitxCandidateList =
            std::dynamic_pointer_cast<HazkeyCandidateList>(
                ic_->inputPanel().candidateList());
        int pageSize = std::min(static_cast<size_t>(response.page_size()),
                                defaultSelectionKeys.size());
        newFcitxCandidateList->setPageSize(pageSize);
    }

    // true if the list is displayed
    return response.page_size() > 0;
}

void HazkeyState::showNonPredictCandidateList() {
    showCandidateList(false);

    livePreeditIndex_ = -1;

    // highlight all preedit text
    // because the first candidate is the result of all preedit text.
    auto currentPreedit = preedit_.text();
    preedit_.setSimplePreeditHighlighted(currentPreedit);

    auto newCandidateList = std::dynamic_pointer_cast<HazkeyCandidateList>(
        ic_->inputPanel().candidateList());
    newCandidateList->focus();
    updateCandidateCursor(newCandidateList);
    setCandidateCursorAUX(
        std::static_pointer_cast<HazkeyCandidateList>(newCandidateList));
}

void HazkeyState::showPreeditCandidateList() {
    if (engine_->server()
            .getComposingText(
                hazkey::commands::GetComposingString_CharType_HIRAGANA,
                preedit_.text())
            .size() <= 0) {
        reset();
        return;
    }
    if (showCandidateList(true) && engine_->config().showTabToSelect.value()) {
        setAuxDownText(std::string(_("[Press Tab to Select]")));
    } else {
        setAuxDownText(std::nullopt);
    }
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
    if (engine_->server().currentInputModeIsDirect()) {
        // appending fcitx::Text is supported only >= 5.1.9
        aux.append(std::string(_("[Direct Input]")));
    } else if (optText != std::nullopt) {
        aux.append(optText.value());
    }
    ic_->inputPanel().setAuxDown(aux);
}

void HazkeyState::setHiraganaAUX() {
    ic_->inputPanel().setAuxUp(
        engine_->server().getComposingHiraganaWithCursor());
}

/// Reset

void HazkeyState::reset() {
    FCITX_DEBUG() << "HazkeyState reset";
    isDirectConversionMode_ = false;
    livePreeditIndex_ = -1;
    isCursorMoving_ = false;
    engine_->server().newComposingText();
    ic_->inputPanel().reset();
}

}  // namespace fcitx
