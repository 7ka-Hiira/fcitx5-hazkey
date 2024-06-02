#include "hazkey.h"

namespace fcitx {

HazkeyEngine::HazkeyEngine(Instance *instance)
    : instance_(instance),
      factory_([this](InputContext &ic) { return new HazkeyState(&ic); }) {
    instance->inputContextManager().registerProperty("hazkeyState", &factory_);
    config_ = std::make_shared<HazkeyConfig>();
}

void HazkeyEngine::keyEvent(const InputMethodEntry &entry, KeyEvent &keyEvent) {
    FCITX_UNUSED(entry);
    if (keyEvent.isRelease()) {
        return;
    }
    FCITX_DEBUG() << "keyEvent: " << keyEvent.key().toString();

    auto inputContext = keyEvent.inputContext();
    inputContext->propertyFor(&factory_)->keyEvent(keyEvent);
    inputContext->updatePreedit();
    inputContext->updateUserInterface(UserInterfaceComponent::InputPanel);
}

void HazkeyEngine::activate(const InputMethodEntry &entry,
                            InputContextEvent &event) {
    FCITX_UNUSED(entry);
    FCITX_DEBUG() << "HazkeyEngine activate";
    auto inputContext = event.inputContext();
    auto state = inputContext->propertyFor(&factory_);
    state->loadConfig(config_);
    state->reset();
    inputContext->updatePreedit();
    inputContext->updateUserInterface(UserInterfaceComponent::InputPanel);
}

void HazkeyEngine::deactivate(const InputMethodEntry &entry,
                              InputContextEvent &event) {
    FCITX_UNUSED(entry);
    FCITX_DEBUG() << "HazkeyEngine deactivate";
    auto inputContext = event.inputContext();
    auto state = inputContext->propertyFor(&factory_);
    state->reset();
    inputContext->updatePreedit();
    inputContext->updateUserInterface(UserInterfaceComponent::InputPanel);
}

FCITX_ADDON_FACTORY(HazkeyEngineFactory);

}  // namespace fcitx