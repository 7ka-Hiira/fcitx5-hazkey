#include "azookey_state.h"

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

  char **candidates = kkc_get_candidates(
      composingText_, config_->getKkcConfig(), isPredictMode, nBest);
  auto candidateList = std::make_unique<azooKeyCandidateList>();
  for (int i = 0; candidates[i] != nullptr; i++) {
    candidateList->append(
        std::make_unique<azooKeyCandidateWord>(candidates[i]));
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
  kkc_free_candidates(candidates);
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
  auto text = candidateList->candidate(candidateList->cursorIndex()).text();
  setPreedit(text.toString());
}

void azooKeyState::backCandidateCursor(azooKeyCandidateList *candidateList) {
  candidateList->prevCandidate();
  setCandidateCursorAUX(candidateList);
  auto text = candidateList->candidate(candidateList->cursorIndex()).text();
  setPreedit(text.toString());
}

void azooKeyState::setCandidateCursorAUX(azooKeyCandidateList *candidateList) {
  auto label = "[" + std::to_string(candidateList->cursorIndex() + 1) + "/" +
               std::to_string(candidateList->totalSize()) + "]";
  ic_->inputPanel().setAuxUp(Text(label));
}

void azooKeyState::setPreedit(const std::string &text) {
  if (ic_->capabilityFlags().test(CapabilityFlag::Preedit)) {
    auto preedit = Text(text, TextFormatFlag::Underline);
    preedit.setCursor(0);
    ic_->inputPanel().setClientPreedit(preedit);
  } else {
    auto preedit = Text(text);
    preedit.setCursor(0);
    ic_->inputPanel().setPreedit(Text(text));
  }
}

void azooKeyState::convertToFirstCandidate() {
  if (composingText_ == nullptr) {
    return;
  }
  char **candidates =
      kkc_get_candidates(composingText_, config_->getKkcConfig(), false, 1);
  if (candidates == nullptr) {
    composingText_ = nullptr;
    return;
  }

  setPreedit(candidates[0]);
  kkc_free_candidates(candidates);
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
    setPreedit(hiragana);
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