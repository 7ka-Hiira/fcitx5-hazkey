#include "azookey.h"

#include "candidatelist.h"

namespace fcitx {

static const KeyList selectionKeys = {
    Key{FcitxKey_1}, Key{FcitxKey_2}, Key{FcitxKey_3}, Key{FcitxKey_4},
    Key{FcitxKey_5}, Key{FcitxKey_6}, Key{FcitxKey_7}, Key{FcitxKey_8},
    Key{FcitxKey_9}, Key{FcitxKey_0},
};

azooKeyEngine::azooKeyEngine(Instance *instance)
    : instance_(instance), factory_([this](InputContext &ic) {
        return new azooKeyState(&ic, this);
      }) {
  kkc_config_ = kkc_get_config();
  instance->inputContextManager().registerProperty("azookeyState", &factory_);
}

void azooKeyEngine::keyEvent(const InputMethodEntry &entry,
                             KeyEvent &keyEvent) {
  FCITX_UNUSED(entry);
  if (keyEvent.isRelease() || keyEvent.key().states()) {
    return;
  }

  auto *state = keyEvent.inputContext()->propertyFor(&factory_);
  auto candidateList = std::dynamic_pointer_cast<CommonCandidateList>(
      keyEvent.inputContext()->inputPanel().candidateList());
  if (state->isCandidateMode()) {
    state->candidateKeyEvent(keyEvent, candidateList);
  } else if (state->isShowingPreedit()) {
    state->preeditKeyEvent(keyEvent, candidateList);
  } else {
    state->keyEvent(keyEvent);
  }
}

void azooKeyState::keyEvent(KeyEvent &event) {
  if (event.key().isSimple()) {
    composingText_ = kkc_get_composing_text();
    kkc_input_text(
        composingText_,
        const_cast<char *>(Key::keySymToUTF8(event.key().sym()).c_str()));
    updateUI();
    return event.filterAndAccept();
  }
  return event.filter();
}

void azooKeyState::candidateKeyEvent(
    KeyEvent &event, std::shared_ptr<CommonCandidateList> candidateList) {
  if (event.key().check(FcitxKey_Return)) {
    candidateList->candidate(candidateList->cursorIndex()).select(ic_);
    reset();
  } else if (event.key().check(FcitxKey_BackSpace)) {
    hideList();
    updateUI();
    return event.accept();
  } else if (event.key().check(FcitxKey_space) ||
             event.key().check(FcitxKey_Tab)) {
    candidateList->nextCandidate();
    ic_->updateUserInterface(UserInterfaceComponent::InputPanel);
    auto &inputPanel = ic_->inputPanel();
    auto text = candidateList->candidate(candidateList->cursorIndex()).text();
    if (ic_->capabilityFlags().test(CapabilityFlag::Preedit)) {
      inputPanel.setClientPreedit(text);
    } else {
      inputPanel.setPreedit(text);
    }
    ic_->updatePreedit();
    return event.filterAndAccept();
  } else if (event.key().check(FcitxKey_Escape)) {
    reset();
  } else if (event.key().checkKeyList(selectionKeys)) {
    auto index = event.key().keyListIndex(selectionKeys);
    candidateList->candidate(index).select(ic_);
    reset();
  } else if (event.key().isSimple()) {
    kkc_input_text(
        composingText_,
        const_cast<char *>(Key::keySymToUTF8(event.key().sym()).c_str()));
    return event.filterAndAccept();
  } else {
    return event.filterAndAccept();
  }
  updateUI();
  return event.filter();
}

void azooKeyState::preeditKeyEvent(
    KeyEvent &event,
    std::shared_ptr<CommonCandidateList> PreeditCandidateList) {
  if (event.key().check(FcitxKey_Return)) {
    ic_->commitString(
        event.inputContext()->inputPanel().clientPreedit().toStringForCommit());
    reset();
  } else if (event.key().check(FcitxKey_BackSpace)) {
    kkc_delete_backward(composingText_);
    isCandidateMode_ = false;
  } else if (event.key().check(FcitxKey_Tab)) {
    PreeditCandidateList->setCursorIndex(1);
    PreeditCandidateList->setPageSize(9);
    ic_->updateUserInterface(UserInterfaceComponent::InputPanel);
    isCandidateMode_ = true;
    return event.filterAndAccept();
  } else if (event.key().check(FcitxKey_space)) {
    showCandidateList();
    return event.filterAndAccept();
  } else if (event.key().check(FcitxKey_Escape)) {
    reset();
  } else if (event.key().isSimple()) {
    kkc_input_text(
        composingText_,
        const_cast<char *>(Key::keySymToUTF8(event.key().sym()).c_str()));
  }
  updateUI();
  return event.filterAndAccept();
}

void azooKeyEngine::activate(const InputMethodEntry &entry,
                             InputContextEvent &event) {
  FCITX_UNUSED(entry);
  auto *state = event.inputContext()->propertyFor(&factory_);
  state->reset();
}

void azooKeyEngine::deactivate(const InputMethodEntry &entry,
                               InputContextEvent &event) {
  FCITX_UNUSED(entry);
  auto *state = event.inputContext()->propertyFor(&factory_);
  state->reset();
}

void azooKeyState::showCandidateList() {
  if (composingText_ == nullptr) {
    return;
  }
  char **candidates =
      kkc_get_candidates(composingText_, engine_->getKkcConfig(), false, 9);

  auto candidateList = std::make_unique<CommonCandidateList>();
  candidateList->setLayoutHint(CandidateLayoutHint::Vertical);
  candidateList->setSelectionKey(selectionKeys);
  for (int i = 0; candidates[i] != nullptr; i++) {
    candidateList->append(
        std::make_unique<azooKeyCandidateWord>(engine_, candidates[i]));
  }
  candidateList->setPageSize(9);
  candidateList->setCursorPositionAfterPaging(
      CursorPositionAfterPaging::ResetToFirst);
  candidateList->setCursorIndex(0);
  auto &inputPanel = ic_->inputPanel();
  inputPanel.reset();
  inputPanel.setCandidateList(std::move(candidateList));
  ic_->updateUserInterface(UserInterfaceComponent::InputPanel);
  isCandidateMode_ = true;
  kkc_free_candidates(candidates);
}

void azooKeyState::showPredictCandidateList() {
  if (composingText_ == nullptr) {
    return;
  }
  char **candidates =
      kkc_get_candidates(composingText_, engine_->getKkcConfig(), true, 9);
  if (candidates == nullptr) {
    composingText_ = nullptr;
    return;
  }
  auto candidateList = std::make_unique<CommonCandidateList>();
  candidateList->setLayoutHint(CandidateLayoutHint::Vertical);
  candidateList->setSelectionKey(selectionKeys);
  for (int i = 0; candidates[i] != nullptr; i++) {
    candidateList->append(
        std::make_unique<azooKeyCandidateWord>(engine_, candidates[i]));
  }
  candidateList->setPageSize(4);
  candidateList->setCursorPositionAfterPaging(
      CursorPositionAfterPaging::ResetToFirst);
  auto &inputPanel = ic_->inputPanel();
  inputPanel.setAuxUp(Text("[Tabキーで選択]"));
  inputPanel.setCandidateList(std::move(candidateList));
  ic_->updateUserInterface(UserInterfaceComponent::InputPanel);
  isCandidateMode_ = false;
  kkc_free_candidates(candidates);
}

void azooKeyState::hideList() {
  auto &inputPanel = ic_->inputPanel();
  inputPanel.reset();
  isCandidateMode_ = false;
  ic_->updateUserInterface(UserInterfaceComponent::InputPanel);
}

void azooKeyState::updateUI() {
  auto &inputPanel = ic_->inputPanel();
  inputPanel.reset();
  if (composingText_ != nullptr) {
    auto convertedList =
        kkc_get_candidates(composingText_, engine_->getKkcConfig(), false, 1);
    if (convertedList == nullptr) {
      kkc_free_composing_text(std::move(composingText_));
      composingText_ = nullptr;
    } else {
      convertedPtr_ = convertedList[0];
      if (ic_->capabilityFlags().test(CapabilityFlag::Preedit)) {
        fcitx::Text preedit(convertedPtr_, fcitx::TextFormatFlag::NoFlag);
        inputPanel.setClientPreedit(preedit);
      } else {
        fcitx::Text preedit(convertedPtr_);
        inputPanel.setPreedit(preedit);
      }
      kkc_free_candidates(convertedList);
      if (!isCandidateMode_) {
        showPredictCandidateList();
      }
    }
  }
  ic_->updateUserInterface(fcitx::UserInterfaceComponent::InputPanel);
  ic_->updatePreedit();
}

FCITX_ADDON_FACTORY(azooKeyEngineFactory);

}  // namespace fcitx