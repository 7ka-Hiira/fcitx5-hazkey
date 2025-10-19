#include "hazkey_engine.h"

#include <fcitx-utils/macros.h>

#include "hazkey_server_connector.h"
#include "hazkey_state.h"

namespace fcitx {

HazkeyEngine::HazkeyEngine(Instance *instance)
    : instance_(instance), factory_([this](InputContext &ic) {
          return new HazkeyState(this, &ic);
      }) {
    server_ = HazkeyServerConnector();

    instance->inputContextManager().registerProperty("hazkeyState", &factory_);
    // reloadConfig();
}

void HazkeyEngine::keyEvent([[maybe_unused]] const InputMethodEntry &entry,
                            KeyEvent &keyEvent) {
    FCITX_DEBUG() << "keyEvent: " << keyEvent.key().toString();

    auto inputContext = keyEvent.inputContext();
    inputContext->propertyFor(&factory_)->keyEvent(keyEvent);
    inputContext->updatePreedit();
    inputContext->updateUserInterface(UserInterfaceComponent::InputPanel);
}

void HazkeyEngine::activate([[maybe_unused]] const InputMethodEntry &entry,
                            InputContextEvent &event) {
    FCITX_DEBUG() << &entry;
    FCITX_DEBUG() << "HazkeyEngine activate";
    auto inputContext = event.inputContext();
    auto state = inputContext->propertyFor(&factory_);
    state->reset();
    inputContext->updatePreedit();
    inputContext->updateUserInterface(UserInterfaceComponent::InputPanel);
}

void HazkeyEngine::deactivate([[maybe_unused]] const InputMethodEntry &entry,
                              InputContextEvent &event) {
    FCITX_DEBUG() << "HazkeyEngine deactivate";
    auto inputContext = event.inputContext();
    auto state = inputContext->propertyFor(&factory_);
    state->commitPreedit();
    state->reset();
    inputContext->updatePreedit();
    inputContext->updateUserInterface(UserInterfaceComponent::InputPanel);
}

void HazkeyEngine::setConfig(const RawConfig &config) {
    config_.load(config, true);
    safeSaveAsIni(config_, "conf/hazkey.conf");
    reloadConfig();
}

void HazkeyEngine::reloadConfig() {
    // server will directly read config
    //
    readAsIni(config_, "conf/hazkey.conf");
    // server_.setServerConfig(
    //     *config().zenzaiEnabled, *config().zenzaiInferenceLimit,
    //     static_cast<int>(*config().numberStyle),
    //     static_cast<int>(*config().symbolStyle),
    //     static_cast<int>(*config().periodStyle),
    //     static_cast<int>(*config().commaStyle),
    //     static_cast<int>(*config().spaceStyle),
    //     static_cast<int>(*config().diacriticStyle), *config().zenzaiProfile);
}

FCITX_ADDON_FACTORY(HazkeyEngineFactory);

}  // namespace fcitx
