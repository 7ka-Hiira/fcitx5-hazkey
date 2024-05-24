#include "azookey.h"

#include "candidatelist.h"

namespace fcitx {

static const std::array<Key, 10> selectionKeys = {
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
  FCITX_INFO() << "Key: " << keyEvent.key().toString();
  auto *state = keyEvent.inputContext()->propertyFor(&factory_);
  state->keyEvent(keyEvent);
}

void azooKeyEngine::reset(const InputMethodEntry &, InputContextEvent &event) {
  auto *state = event.inputContext()->propertyFor(&factory_);
  state->reset();
}

void azooKeyState::keyEvent(KeyEvent &event) {
  FCITX_INFO() << "Key: " << event.key().toString();
  if (composingText_ == nullptr) {
    if (!event.key().isSimple()) {
      return event.filter();
    }
    composingText_ = kkc_get_composing_text();
  } else {
    if (event.key().check(FcitxKey_BackSpace)) {
      kkc_delete_backward(composingText_);
      updatePreedit();
      return event.filterAndAccept();
    }
    if (event.key().check(FcitxKey_Return)) {
      ic_->commitString(event.inputContext()
                            ->inputPanel()
                            .clientPreedit()
                            .toStringForCommit());
      reset();
      return event.filterAndAccept();
    }
    if (event.key().check(FcitxKey_Shift_L) ||
        event.key().check(FcitxKey_Shift_R)) {
      return event.filterAndAccept();
    }
    if (event.key().check(FcitxKey_Escape)) {
      reset();
      return event.filterAndAccept();
    }
    if (event.key().check(FcitxKey_space)) {
      showCandidateList();
      return event.filterAndAccept();
    }
  }

  if (event.key().isSimple()) {
    kkc_input_text(composingText_,
                   const_cast<char *>(event.key().toString().c_str()));
  }

  updatePreedit();
  return event.filterAndAccept();
}

void azooKeyState::showCandidateList() {
  if (composingText_ == nullptr) {
    return;
  }
  char **candidates =
      kkc_get_candidates(composingText_, engine_->getKkcConfig());
  std::vector<std::string> candidatesStr;
  for (int i = 0; candidates[i] != nullptr; i++) {
    candidatesStr.push_back(candidates[i]);
  }

  auto &inputPanel = ic_->inputPanel();
  inputPanel.reset();
  inputPanel.setCandidateList(
      std::make_unique<azooKeyCandidateList>(engine_, ic_, &candidatesStr));
  ic_->updateUserInterface(fcitx::UserInterfaceComponent::InputPanel);

  kkc_free_candidates(candidates);
}

void azooKeyState::updatePreedit() {
  auto &inputPanel = ic_->inputPanel();
  inputPanel.reset();
  if (composingText_ != nullptr) {
    char *converted =
        kkc_get_first_candidate(composingText_, engine_->getKkcConfig());

    if (ic_->capabilityFlags().test(CapabilityFlag::Preedit)) {
      fcitx::Text preedit(converted, fcitx::TextFormatFlag::HighLight);
      inputPanel.setClientPreedit(preedit);
    } else {
      fcitx::Text preedit(converted);
      inputPanel.setPreedit(preedit);
    }
    if (strlen(converted) == 0) {
      kkc_free_composing_text(composingText_);
      composingText_ = nullptr;
    }
    kkc_free_first_candidate(converted);
  }
  ic_->updateUserInterface(fcitx::UserInterfaceComponent::InputPanel);
  ic_->updatePreedit();
}

FCITX_ADDON_FACTORY(azooKeyEngineFactory);

}  // namespace fcitx