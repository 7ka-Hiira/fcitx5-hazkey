#include "hazukey.h"

namespace fcitx {

HazukeyEngine::HazukeyEngine(Instance *instance)
    : instance_(instance),
      factory_([this](InputContext &ic) { return new HazukeyState(&ic); }) {
    instance->inputContextManager().registerProperty("hazukeyState", &factory_);
    config_ = std::make_shared<HazukeyConfig>();
}

void HazukeyEngine::keyEvent(const InputMethodEntry &entry,
                             KeyEvent &keyEvent) {
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

void HazukeyEngine::activate(const InputMethodEntry &entry,
                             InputContextEvent &event) {
    FCITX_UNUSED(entry);
    FCITX_DEBUG() << "HazukeyEngine activate";
    auto inputContext = event.inputContext();
    auto state = inputContext->propertyFor(&factory_);
    state->loadConfig(config_);
    state->reset();
    inputContext->updatePreedit();
    inputContext->updateUserInterface(UserInterfaceComponent::InputPanel);
}

void HazukeyEngine::deactivate(const InputMethodEntry &entry,
                               InputContextEvent &event) {
    FCITX_UNUSED(entry);
    FCITX_DEBUG() << "HazukeyEngine deactivate";
    auto inputContext = event.inputContext();
    auto state = inputContext->propertyFor(&factory_);
    state->reset();
    inputContext->updatePreedit();
    inputContext->updateUserInterface(UserInterfaceComponent::InputPanel);
}

FCITX_ADDON_FACTORY(HazukeyEngineFactory);

}  // namespace fcitx