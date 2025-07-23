#include "hazkey_engine.h"

#include "hazkey_state.h"

namespace fcitx {

HazkeyEngine::HazkeyEngine(Instance *instance)
    : instance_(instance), factory_([this](InputContext &ic) {
          return new HazkeyState(this, &ic);
      }) {
        socket_ = connect_server();

    instance->inputContextManager().registerProperty("hazkeyState", &factory_);
        // kkc_get_config(*config().zenzaiEnabled, *config().zenzaiInferenceLimit,
        //                static_cast<int>(*config().numberStyle),
        //                static_cast<int>(*config().symbolStyle),
        //                static_cast<int>(*config().periodStyle),
        //                static_cast<int>(*config().commaStyle),
        //                static_cast<int>(*config().spaceStyle),
        //                static_cast<int>(*config().diacriticStyle),
        //                static_cast<int>(*config().gpuLayers),
        //                const_cast<char *>(config().zenzaiProfile->c_str()));
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

void HazkeyEngine::activate(const InputMethodEntry &entry,
                            InputContextEvent &event) {
    FCITX_UNUSED(entry);
    FCITX_DEBUG() << "HazkeyEngine activate";
    auto inputContext = event.inputContext();
    auto state = inputContext->propertyFor(&factory_);
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

void HazkeyEngine::setConfig(const RawConfig &config) {
    config_.load(config, true);
    safeSaveAsIni(config_, "conf/hazkey.conf");
    reloadConfig();
}

void HazkeyEngine::reloadConfig() {
    // readAsIni(config_, "conf/hazkey.conf");
    //     kkc_get_config(*config().zenzaiEnabled, *config().zenzaiInferenceLimit,
    //                    static_cast<int>(*config().numberStyle),
    //                    static_cast<int>(*config().symbolStyle),
    //                    static_cast<int>(*config().periodStyle),
    //                    static_cast<int>(*config().commaStyle),
    //                    static_cast<int>(*config().spaceStyle),
    //                    static_cast<int>(*config().diacriticStyle),
    //                    static_cast<int>(*config().gpuLayers),
    //                    const_cast<char *>(config().zenzaiProfile->c_str()));
}

FCITX_ADDON_FACTORY(HazkeyEngineFactory);

}  // namespace fcitx
