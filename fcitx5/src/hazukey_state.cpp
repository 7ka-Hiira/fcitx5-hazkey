#include "hazukey_state.h"

#include <algorithm>
#include <numeric>

#include "hazukey_candidate.h"
#include "hazukey_config.h"

namespace fcitx {

constexpr int NormalCandidateListNBest = 9;
constexpr int PredictCandidateListNBest = 4;

void HazukeyState::keyEvent(KeyEvent &event) {
    FCITX_DEBUG() << "HazukeyState keyEvent";

    auto candidateList = std::dynamic_pointer_cast<HazukeyCandidateList>(
        event.inputContext()->inputPanel().candidateList());

    if (isCandidateMode_) {
        candidateKeyEvent(event, candidateList);
    } else if (composingText_ != nullptr) {
        preeditKeyEvent(event, candidateList);
    } else {
        auto key = event.key();
        if (key.check(FcitxKey_space)) {
            ic_->commitString("　");
        } else if (key.isSimple() ||
                   (key.sym() >= 0x04a1 && key.sym() <= 0x04df)) {
            // 0x04a1 - 0x04dd is the range of kana keys
            composingText_ = kkc_get_composing_text_instance();
            kkc_input_text(composingText_,
                           Key::keySymToUTF8(key.sym()).c_str());
            updateOnPreeditMode();
        } else {
            return event.filter();
        }
        updateUI();
        return event.filterAndAccept();
    }
}

void HazukeyState::preeditKeyEvent(
    KeyEvent &event,
    std::shared_ptr<HazukeyCandidateList> PreeditCandidateList) {
    FCITX_DEBUG() << "HazukeyState preeditKeyEvent";

    auto key = event.key();
    auto keysym = key.sym();
    switch (keysym) {
        case FcitxKey_Return:
            ic_->commitString(event.inputContext()
                                  ->inputPanel()
                                  .clientPreedit()
                                  .toStringForCommit());
            reset();
            break;
        case FcitxKey_BackSpace:
            kkc_delete_backward(composingText_);
            updateOnPreeditMode();
            break;
        case FcitxKey_Up:
        case FcitxKey_Tab:
            PreeditCandidateList->setDefaultStyle(config_->getSelectionKeys());
            setCandidateCursorAUX(PreeditCandidateList);
            isCandidateMode_ = true;
            break;
        case FcitxKey_Down:
            PreeditCandidateList->setDefaultStyle(config_->getSelectionKeys());
            advanceCandidateCursor(PreeditCandidateList);
            isCandidateMode_ = true;
            break;
        case FcitxKey_space:
            prepareNormalCandidateList();
            advanceCandidateCursor(
                std::dynamic_pointer_cast<HazukeyCandidateList>(
                    ic_->inputPanel().candidateList()));
            break;
        case FcitxKey_F6:
            directCharactorConversion(ConversionMode::Hiragana);
            FunctionConversionMode_ = ConversionMode::Hiragana;
            break;
        case FcitxKey_F7:
            directCharactorConversion(ConversionMode::KatakanaFullwidth);
            FunctionConversionMode_ = ConversionMode::KatakanaFullwidth;
            break;
        case FcitxKey_F8:
            directCharactorConversion(ConversionMode::KatakanaHalfwidth);
            FunctionConversionMode_ = ConversionMode::KatakanaHalfwidth;
            break;
        case FcitxKey_F9:
            ConversionMode mode;
            switch (FunctionConversionMode_) {
                case ConversionMode::RawFullwidthLower:
                    mode = ConversionMode::RawFullwidthUpper;
                    break;
                case ConversionMode::RawFullwidthUpper:
                    mode = ConversionMode::RawFullwidthCapitalized;
                    break;
                default:
                    mode = ConversionMode::RawFullwidthLower;
                    break;
            }
            directCharactorConversion(mode);
            FunctionConversionMode_ = mode;
            ic_->inputPanel().candidateList().reset();
            break;
        case FcitxKey_F10:
            ConversionMode mode2;
            switch (FunctionConversionMode_) {
                case ConversionMode::RawHalfwidthLower:
                    mode2 = ConversionMode::RawHalfwidthUpper;
                    break;
                case ConversionMode::RawHalfwidthUpper:
                    mode2 = ConversionMode::RawHalfwidthCapitalized;
                    break;
                default:
                    mode2 = ConversionMode::RawHalfwidthLower;
                    break;
            }
            directCharactorConversion(mode2);
            FunctionConversionMode_ = mode2;
            ic_->inputPanel().candidateList().reset();
            break;
        case FcitxKey_Escape:
            reset();
            break;
        case FcitxKey_kana_fullstop:
        case FcitxKey_period:
            ic_->commitString(event.inputContext()
                                  ->inputPanel()
                                  .clientPreedit()
                                  .toStringForCommit());
            ic_->commitString("。");
            reset();
            break;
        case FcitxKey_kana_conjunctive:
        case FcitxKey_comma:
            ic_->commitString(event.inputContext()
                                  ->inputPanel()
                                  .clientPreedit()
                                  .toStringForCommit());
            ic_->commitString("、");
            reset();
            break;
        default:
            if (key.isSimple() ||
                (key.sym() >= 0x04a1 && key.sym() <= 0x04df)) {
                kkc_input_text(composingText_,
                               Key::keySymToUTF8(keysym).c_str());
                updateOnPreeditMode();
            }
            break;
    }
    updateUI();
    return event.filterAndAccept();
}

void HazukeyState::candidateKeyEvent(
    KeyEvent &event, std::shared_ptr<HazukeyCandidateList> candidateList) {
    FCITX_DEBUG() << "HazukeyState candidateKeyEvent";

    auto key = event.key();
    auto keysym = key.sym();
    std::vector<std::string> preedit;
    switch (keysym) {
        case FcitxKey_Right:
        case FcitxKey_Return:
            if (FunctionConversionMode_ != ConversionMode::None) {
                ic_->commitString(
                    ic_->inputPanel().preedit().toStringForCommit());
                reset();
            } else {
                preedit = candidateList
                              ->HazukeyCandidate(candidateList->cursorIndex())
                              .getPreedit();
                ic_->commitString(preedit[0]);
                if (preedit.size() > 1) {
                    auto correspondingCount =
                        candidateList
                            ->HazukeyCandidate(candidateList->cursorIndex())
                            .correspondingCount();
                    kkc_complete_prefix(composingText_, correspondingCount);
                    prepareNormalCandidateList();
                } else {
                    reset();
                }
            }
            break;
        case FcitxKey_BackSpace:
            updateOnPreeditMode();
            break;
        case FcitxKey_space:
        case FcitxKey_Tab:
        case FcitxKey_Down:
            advanceCandidateCursor(candidateList);
            break;
        case FcitxKey_Up:
            backCandidateCursor(candidateList);
            break;
        case FcitxKey_F6:
            directCharactorConversion(ConversionMode::Hiragana);
            FunctionConversionMode_ = ConversionMode::Hiragana;
            break;
        case FcitxKey_F7:
            directCharactorConversion(ConversionMode::KatakanaFullwidth);
            FunctionConversionMode_ = ConversionMode::KatakanaFullwidth;
            break;
        case FcitxKey_F8:
            directCharactorConversion(ConversionMode::KatakanaHalfwidth);
            FunctionConversionMode_ = ConversionMode::KatakanaHalfwidth;
            break;
        case FcitxKey_F9:
            ConversionMode mode;
            switch (FunctionConversionMode_) {
                case ConversionMode::RawFullwidthLower:
                    mode = ConversionMode::RawFullwidthUpper;
                    break;
                case ConversionMode::RawFullwidthUpper:
                    mode = ConversionMode::RawFullwidthCapitalized;
                    break;
                default:
                    mode = ConversionMode::RawFullwidthLower;
                    break;
            }
            directCharactorConversion(mode);
            FunctionConversionMode_ = mode;
            break;
        case FcitxKey_F10:
            ConversionMode mode2;
            switch (FunctionConversionMode_) {
                case ConversionMode::RawHalfwidthLower:
                    mode2 = ConversionMode::RawHalfwidthUpper;
                    break;
                case ConversionMode::RawHalfwidthUpper:
                    mode2 = ConversionMode::RawHalfwidthCapitalized;
                    break;
                default:
                    mode2 = ConversionMode::RawHalfwidthLower;
                    break;
            }
            isCandidateMode_ = false;
            directCharactorConversion(mode2);
            FunctionConversionMode_ = mode2;
            ic_->inputPanel().candidateList().reset();
            break;
        case FcitxKey_Escape:
            reset();
            break;
        default:
            if (key.checkKeyList(config_->getSelectionKeys())) {
                auto localIndex = key.keyListIndex(config_->getSelectionKeys());
                preedit =
                    candidateList->HazukeyCandidate(localIndex).getPreedit();
                ic_->commitString(preedit[0]);
                if (preedit.size() > 1) {
                    auto correspondingCount =
                        candidateList->HazukeyCandidate(localIndex)
                            .correspondingCount();
                    kkc_complete_prefix(composingText_, correspondingCount);
                    prepareNormalCandidateList();
                } else {
                    reset();
                }
            } else if (key.isSimple() ||
                       (key.sym() >= 0x04a1 && key.sym() <= 0x04df)) {
                ic_->commitString(
                    ic_->inputPanel().preedit().toStringForCommit());
                reset();
                composingText_ = kkc_get_composing_text_instance();
                kkc_input_text(composingText_,
                               Key::keySymToUTF8(keysym).c_str());
            } else {
                return event.filter();
            }
            break;
    }
    updateUI();
    return event.filterAndAccept();
}

void HazukeyState::directCharactorConversion(ConversionMode mode) {
    char *converted = nullptr;
    switch (mode) {
        case ConversionMode::None:
            return;
        case ConversionMode::Hiragana:
            converted = kkc_get_composing_hiragana(composingText_);
            break;
        case ConversionMode::KatakanaFullwidth:
            converted = kkc_get_composing_katakana_fullwidth(composingText_);
            break;
        case ConversionMode::KatakanaHalfwidth:
            converted = kkc_get_composing_katakana_halfwidth(composingText_);
            break;
        case ConversionMode::RawFullwidthUpper:
            converted = kkc_get_composing_raw_fullwidth(composingText_, 1);
            break;
        case ConversionMode::RawFullwidthLower:
            converted = kkc_get_composing_raw_fullwidth(composingText_, 0);
            break;
        case ConversionMode::RawFullwidthCapitalized:
            converted = kkc_get_composing_raw_fullwidth(composingText_, 2);
            // converted[0] = ::toupper(converted[0]);
            break;
        case ConversionMode::RawHalfwidthUpper:
            converted = kkc_get_composing_raw_halfwidth(composingText_, 1);
            break;
        case ConversionMode::RawHalfwidthLower:
            converted = kkc_get_composing_raw_halfwidth(composingText_, 0);
            break;
        case ConversionMode::RawHalfwidthCapitalized:
            converted = kkc_get_composing_raw_halfwidth(composingText_, 2);
            // converted[0] = ::toupper(converted[0]);
            break;
    }
    preedit_.setSimplePreedit(converted);
    kkc_free_text(converted);
}

void HazukeyState::prepareCandidateList(bool isPredictMode, bool updatePreedit,
                                        int nBest) {
    FCITX_DEBUG() << "HazukeyState prepareCandidateList";
    if (composingText_ == nullptr) {
        return;
    }

    auto ***candidates = kkc_get_candidates(
        composingText_, config_->getKkcConfig(), isPredictMode, nBest);

    std::vector<std::string> preeditSegments = {};
    bool preeditFound = false;

    auto candidateList = std::make_unique<HazukeyCandidateList>();
    for (int i = 0; candidates[i] != nullptr; i++) {
        std::vector<std::string> parts;
        std::vector<int> partLens;

        for (int j = 5;
             (candidates[i][j] != nullptr && candidates[i][j + 1] != nullptr);
             j += 2) {
            parts.push_back(candidates[i][j]);
            partLens.push_back(atoi(candidates[i][j + 1]));
        }

        // append words to the candidate list
        candidateList->append(std::make_unique<HazukeyCandidateWord>(
            candidates[i][0], candidates[i][2], atoi(candidates[i][3]), parts,
            partLens));

        // get first non-predicting candidate for preedit
        if (updatePreedit && !preeditFound &&
            strcmp(candidates[i][4], "1") == 0) {
            preeditSegments = parts;
            preeditFound = true;
        }
    }
    candidateList->setDefaultStyle(config_->getSelectionKeys());
    ic_->inputPanel().reset();
    if (isPredictMode) {
        candidateList->setPageSize(4);
    } else {
        candidateList->setDefaultStyle(config_->getSelectionKeys());
        preedit_.setPreedit(candidateList->candidate(0).text());
    }

    if (updatePreedit && !preeditSegments.empty()) {
        preedit_.setMultiSegmentPreedit(preeditSegments, 0);
    } else if (updatePreedit) {
        auto hiragana = kkc_get_composing_hiragana(composingText_);
        preedit_.setSimplePreedit(hiragana);
        kkc_free_text(hiragana);
    }

    ic_->inputPanel().setCandidateList(std::move(candidateList));
    auto newCandidateList = ic_->inputPanel().candidateList();
    setCandidateCursorAUX(
        std::static_pointer_cast<HazukeyCandidateList>(newCandidateList));
    if (isPredictMode) {
        ic_->inputPanel().setAuxUp(Text("[Tabキーで選択]"));
    }
    isCandidateMode_ = !isPredictMode;
    kkc_free_candidates(candidates);
}

void HazukeyState::prepareNormalCandidateList() {
    prepareCandidateList(false, true, NormalCandidateListNBest);
}

void HazukeyState::preparePredictCandidateList() {
    prepareCandidateList(true, true, PredictCandidateListNBest);
}

void HazukeyState::advanceCandidateCursor(
    std::shared_ptr<HazukeyCandidateList> candidateList) {
    candidateList->nextCandidate();
    setCandidateCursorAUX(candidateList);
    auto text = candidateList->HazukeyCandidate(candidateList->cursorIndex())
                    .getPreedit();

    preedit_.setMultiSegmentPreedit(text, 0);
}

void HazukeyState::backCandidateCursor(
    std::shared_ptr<HazukeyCandidateList> candidateList) {
    candidateList->prevCandidate();
    setCandidateCursorAUX(candidateList);
    auto text = candidateList->HazukeyCandidate(candidateList->cursorIndex())
                    .getPreedit();
    preedit_.setMultiSegmentPreedit(text, 0);
}

void HazukeyState::setCandidateCursorAUX(
    std::shared_ptr<HazukeyCandidateList> candidateList) {
    auto label = "[" + std::to_string(candidateList->globalCursorIndex() + 1) +
                 "/" + std::to_string(candidateList->totalSize()) + "]";
    ic_->inputPanel().setAuxUp(Text(label));
}

void HazukeyState::updateOnPreeditMode() {
    FCITX_DEBUG() << "HazukeyState updateOnPreeditMode";

    if (composingText_ == nullptr) {
        reset();
        return;
    }

    auto hiragana = kkc_get_composing_hiragana(composingText_);

    if (hiragana == nullptr || strlen(hiragana) == 0) {
        reset();
        return;
    }

    preparePredictCandidateList();
}

void HazukeyState::updateUI() {
    ic_->updateUserInterface(UserInterfaceComponent::InputPanel);
    ic_->updatePreedit();
}

void HazukeyState::loadConfig(std::shared_ptr<HazukeyConfig> &config) {
    if (config_ == nullptr) {
        config_ = config;
    }
}

void HazukeyState::reset() {
    FCITX_DEBUG() << "HazukeyState reset";
    if (composingText_ != nullptr) {
        kkc_free_composing_text_instance(composingText_);
    }
    ic_->inputPanel().reset();
    composingText_ = nullptr;
    isCandidateMode_ = false;
    FunctionConversionMode_ = ConversionMode::None;
}

}  // namespace fcitx