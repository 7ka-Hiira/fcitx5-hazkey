#include "azookey_state.h"

#include "azookey_candidate.h"
#include "azookey_config.h"

namespace fcitx {

constexpr int kNormalCandidateListSize = 18;
constexpr int kPredictCandidateListSize = 4;

void azooKeyState::keyEvent(KeyEvent &event) {
  FCITX_DEBUG() << "azooKeyState keyEvent";

  auto candidateList = std::dynamic_pointer_cast<azooKeyCandidateList>(
      event.inputContext()->inputPanel().candidateList());
  if (isCandidateMode_) {
    candidateKeyEvent(event, candidateList);
  } else if (composingText_ != nullptr) {
    preeditKeyEvent(event, candidateList);
  } else {
    auto key = event.key();
    if (!key.check(FcitxKey_Return) && !key.check(FcitxKey_space) &&
        key.isSimple()) {
      composingText_ = kkc_get_composing_text_instance();
      kkc_input_text(composingText_, Key::keySymToUTF8(key.sym()).c_str());
      preparePreeditOnKeyEvent();
    } else {
      return event.filter();
    }
    updateUI();
    return event.filterAndAccept();
  }
}

void azooKeyState::preeditKeyEvent(
    KeyEvent &event,
    std::shared_ptr<azooKeyCandidateList> PreeditCandidateList) {
  FCITX_DEBUG() << "azooKeyState preeditKeyEvent";

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
      preparePreeditOnKeyEvent();
      break;
    case FcitxKey_Tab:
      PreeditCandidateList->setDefaultStyle(config_->getSelectionKeys());
      setCandidateCursorAUX(PreeditCandidateList.get());
      isCandidateMode_ = true;
      break;
    case FcitxKey_space:
      prepareNormalCandidateList();
      convertToFirstCandidate();
      break;
    case FcitxKey_Escape:
      reset();
      break;
    default:
      if (key.isSimple()) {
        kkc_input_text(composingText_, Key::keySymToUTF8(keysym).c_str());
        preparePreeditOnKeyEvent();
      }
      break;
  }
  updateUI();
  return event.filterAndAccept();
}

void azooKeyState::candidateKeyEvent(
    KeyEvent &event, std::shared_ptr<azooKeyCandidateList> candidateList) {
  FCITX_DEBUG() << "azooKeyState candidateKeyEvent";

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
      preparePreeditOnKeyEvent();
      break;
    case FcitxKey_space:
    case FcitxKey_Tab:
    case FcitxKey_Down:
      advanceCandidateCursor(candidateList.get());
      break;
    case FcitxKey_Up:
      backCandidateCursor(candidateList.get());
      break;
    case FcitxKey_Escape:
      reset();
      break;
    default:
      if (key.checkKeyList(config_->getSelectionKeys())) {
        auto index = key.keyListIndex(config_->getSelectionKeys());
        candidateList->candidate(index).select(ic_);
        reset();
      } else if (key.isSimple()) {
        kkc_input_text(composingText_, Key::keySymToUTF8(keysym).c_str());
      } else {
        return event.filter();
      }
      break;
  }
  updateUI();
  return event.filterAndAccept();
}

void azooKeyState::prepareCandidateList(bool isPredictMode, int nBest) {
  FCITX_DEBUG() << "azooKeyState prepareCandidateList";
  if (composingText_ == nullptr) {
    return;
  }

  auto **candidatesStructure = kkc_get_candidates(
      composingText_, config_->getKkcConfig(), isPredictMode, nBest);

  auto candidateList = std::make_unique<azooKeyCandidateList>();
  for (int i = 0; (candidatesStructure[i] != nullptr &&
                   candidatesStructure[i + 1] != nullptr &&
                   candidatesStructure[i + 2] != nullptr);
       i += 3) {
    candidateList->append(std::make_unique<azooKeyCandidateWord>(
        candidatesStructure[i], candidatesStructure[i + 1],
        candidatesStructure[i + 2]));
  }
  candidateList->setDefaultStyle(config_->getSelectionKeys());

  ic_->inputPanel().reset();
  if (isPredictMode) {
    candidateList->setPageSize(4);
    ic_->inputPanel().setAuxUp(Text("[Tabキーで選択]"));
  } else {
    candidateList->setDefaultStyle(config_->getSelectionKeys());
    setCandidateCursorAUX(candidateList.get());
  }

  ic_->inputPanel().setCandidateList(std::move(candidateList));
  isCandidateMode_ = !isPredictMode;
  // kkc_free_candidates(candidatesStructure);
}

void azooKeyState::prepareNormalCandidateList() {
  prepareCandidateList(false, kNormalCandidateListSize);
}

void azooKeyState::preparePredictCandidateList() {
  prepareCandidateList(true, kPredictCandidateListSize);
}

void azooKeyState::advanceCandidateCursor(azooKeyCandidateList *candidateList) {
  candidateList->nextCandidate();
  setCandidateCursorAUX(candidateList);
  auto text = candidateList->azooKeyCandidate(candidateList->cursorIndex())
                  .getPreedit();

  setMultiSegmentPreedit(text, 0);
}

void azooKeyState::backCandidateCursor(azooKeyCandidateList *candidateList) {
  candidateList->prevCandidate();
  setCandidateCursorAUX(candidateList);
  auto text = candidateList->azooKeyCandidate(candidateList->cursorIndex())
                  .getPreedit();
  setMultiSegmentPreedit(text, 0);
}

void azooKeyState::setCandidateCursorAUX(azooKeyCandidateList *candidateList) {
  auto label = "[" + std::to_string(candidateList->globalCursorIndex() + 1) +
               "/" + std::to_string(candidateList->totalSize()) + "]";
  ic_->inputPanel().setAuxUp(Text(label));
}

void azooKeyState::setPreedit(Text &text) {
  if (ic_->capabilityFlags().test(CapabilityFlag::Preedit)) {
    ic_->inputPanel().setClientPreedit(text);
  } else {
    ic_->inputPanel().setPreedit(text);
  }
}

void azooKeyState::setSimplePreedit(const std::string &text) {
  auto preedit = Text(text, TextFormatFlag::Underline);
  preedit.setCursor(0);
  setPreedit(preedit);
}

void azooKeyState::setMultiSegmentPreedit(std::vector<std::string> &texts,
                                          int cursorSegment = 0) {
  auto preedit = Text();
  for (int i = 0; i < int(texts.size()); i++) {
    if (i < cursorSegment) {
      preedit.append(texts[i], TextFormatFlag::NoFlag);
    } else if (i == cursorSegment) {
      preedit.setCursor(preedit.textLength());
      preedit.append(texts[i], TextFormatFlag::HighLight);
      continue;
    } else {
      preedit.append(texts[i], TextFormatFlag::Underline);
    }
  }
  setPreedit(preedit);
}

void azooKeyState::convertToFirstCandidate() {
  if (composingText_ == nullptr) {
    return;
  }
  auto **candidates =
      kkc_get_candidates(composingText_, config_->getKkcConfig(), false, 1);
  if (candidates == nullptr) {
    composingText_ = nullptr;
    return;
  } else if (candidates[0] == nullptr || candidates[1] == nullptr) {
    kkc_free_candidates(candidates);
    return;
  }
  auto preedit = candidates[0] + *candidates[1];
  setSimplePreedit(preedit);
  // kkc_free_candidates(candidates);
}

void azooKeyState::preparePreeditOnKeyEvent() {
  FCITX_DEBUG() << "azooKeyState preparePreeditOnKeyEvent";

  if (composingText_ == nullptr) {
    return;
  }
  auto hiragana = kkc_get_composing_hiragana(composingText_);
  if (hiragana == nullptr) {
    return;
  }

  int len = strlen(hiragana);

  if (len == 0) {
    reset();
    return;
  }

  preparePredictCandidateList();

  if (len <= 3) {
    // one hiragana character: 3 bytes
    // do not convert single hiragana character
    setSimplePreedit(hiragana);
  } else {
    convertToFirstCandidate();
  }
}

void azooKeyState::updateUI() {
  ic_->updateUserInterface(UserInterfaceComponent::InputPanel);
  ic_->updatePreedit();
}

void azooKeyState::loadConfig(std::shared_ptr<azooKeyConfig> &config) {
  if (config_ == nullptr) {
    config_ = config;
  }
}

void azooKeyState::reset() {
  FCITX_DEBUG() << "azooKeyState reset";
  if (composingText_ != nullptr) {
    kkc_free_composing_text_instance(composingText_);
  }
  ic_->inputPanel().reset();
  composingText_ = nullptr;
  isCandidateMode_ = false;
}

}  // namespace fcitx