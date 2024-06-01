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

    if (keyEvent.isRelease() || keyEvent.key().states()) {
        return;
    }

    FCITX_DEBUG() << "keyEvent: " << keyEvent.key().toString();

    keyEvent.inputContext()->propertyFor(&factory_)->keyEvent(keyEvent);
}

void HazukeyEngine::activate(const InputMethodEntry &entry,
                             InputContextEvent &event) {
    FCITX_UNUSED(entry);
    FCITX_DEBUG() << "HazukeyEngine activate";
    auto state = event.inputContext()->propertyFor(&factory_);
    state->loadConfig(config_);
    state->reset();
    state->updateUI();
}

void HazukeyEngine::deactivate(const InputMethodEntry &entry,
                               InputContextEvent &event) {
    FCITX_UNUSED(entry);
    FCITX_DEBUG() << "HazukeyEngine deactivate";
    auto state = event.inputContext()->propertyFor(&factory_);
    state->reset();
    state->updateUI();
}

FCITX_ADDON_FACTORY(HazukeyEngineFactory);

}  // namespace fcitx