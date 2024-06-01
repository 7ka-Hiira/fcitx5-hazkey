#include "azookey.h"

namespace fcitx {

azooKeyEngine::azooKeyEngine(Instance *instance)
    : instance_(instance),
      factory_([this](InputContext &ic) { return new azooKeyState(&ic); }) {
    instance->inputContextManager().registerProperty("azookeyState", &factory_);
    config_ = std::make_shared<azooKeyConfig>();
}

void azooKeyEngine::keyEvent(const InputMethodEntry &entry,
                             KeyEvent &keyEvent) {
    FCITX_UNUSED(entry);

    if (keyEvent.isRelease() || keyEvent.key().states()) {
        return;
    }

    FCITX_DEBUG() << "keyEvent: " << keyEvent.key().toString();

    keyEvent.inputContext()->propertyFor(&factory_)->keyEvent(keyEvent);
}

void azooKeyEngine::activate(const InputMethodEntry &entry,
                             InputContextEvent &event) {
    FCITX_UNUSED(entry);
    FCITX_DEBUG() << "azooKeyEngine activate";
    auto state = event.inputContext()->propertyFor(&factory_);
    state->loadConfig(config_);
    state->reset();
    state->updateUI();
}

void azooKeyEngine::deactivate(const InputMethodEntry &entry,
                               InputContextEvent &event) {
    FCITX_UNUSED(entry);
    FCITX_DEBUG() << "azooKeyEngine deactivate";
    auto state = event.inputContext()->propertyFor(&factory_);
    state->reset();
    state->updateUI();
}

FCITX_ADDON_FACTORY(azooKeyEngineFactory);

}  // namespace fcitx