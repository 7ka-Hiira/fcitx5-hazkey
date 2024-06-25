#ifndef _FCITX5_HAZKEY_HAZKEY_H_
#define _FCITX5_HAZKEY_HAZKEY_H_

#include <fcitx-config/configuration.h>
#include <fcitx-config/iniparser.h>
#include <fcitx-utils/handlertable_details.h>
#include <fcitx-utils/i18n.h>
#include <fcitx-utils/library.h>
#include <fcitx/addonfactory.h>
#include <fcitx/addonmanager.h>
#include <fcitx/inputmethodengine.h>
#include <fcitx/instance.h>
#include <fcitx/menu.h>
#include <iconv.h>

#include "../../azookey-kkc/libhazkey.h"
#include "hazkey_state.h"

namespace fcitx {

/// Config

enum class NumberStyle {
    Halfwidth,
    Fullwidth,
};

FCITX_CONFIG_ENUM_NAME_WITH_I18N(NumberStyle, N_("Halfwidth"), N_("Fullwidth"));

enum class SymbolStyle {
    Halfwidth,
    Fullwidth,
};

FCITX_CONFIG_ENUM_NAME_WITH_I18N(SymbolStyle, N_("Halfwidth"), N_("Fullwidth"));

enum class PeriodStyle {
    FullwidthKuten,
    HalfwidthKuten,
    FullwidthPeriod,
    HalfwidthPeriod,
};

FCITX_CONFIG_ENUM_NAME_WITH_I18N(PeriodStyle, N_("。 (Fullwidth Kuten)"),
                                 N_("｡ (Halfwidth Kuten)"),
                                 N_("． (Fullwidth Period)"),
                                 N_(". (Halfwidth Period)"));

enum class CommaStyle {
    FullwidthToten,
    HalfwidthToten,
    FullwidthComma,
    HalfwidthComma,
};

FCITX_CONFIG_ENUM_NAME_WITH_I18N(CommaStyle, N_("、 (Fullwidth Toten)"),
                                 N_("､ (Halfwidth Toten)"),
                                 N_("， (Fullwidth Comma)"),
                                 N_(", (Halfwidth Comma)"));

enum class SpaceStyle {
    Halfwidth,
    Fullwidth,
};

FCITX_CONFIG_ENUM_NAME_WITH_I18N(SpaceStyle, N_("Halfwidth"), N_("Fullwidth"));

enum class DiacriticStyle {
    Fullwidth,
    Halfwidth,
    Combining,
};

FCITX_CONFIG_ENUM_NAME_WITH_I18N(DiacriticStyle, N_("Fullwidth"),
                                 N_("Halfwidth"), N_("Combining"));

enum class LearnMode {
    None,
};

FCITX_CONFIG_ENUM_NAME_WITH_I18N(LearnMode, N_("None"));

enum class AutoCommitMode {
    None,
    TenWords,
};

FCITX_CONFIG_ENUM_NAME_WITH_I18N(AutoCommitMode, N_("None"), N_("Ten words"));

FCITX_CONFIGURATION(
    HazkeyEngineConfig,
    Option<bool> enablePrediction{this, "EnablePrediction",
                                  _("Enable prediction"), false};
    OptionWithAnnotation<NumberStyle, NumberStyleI18NAnnotation> numberStyle{
        this, "NumberStyle", _("Number style"), NumberStyle::Halfwidth};
    OptionWithAnnotation<SymbolStyle, SymbolStyleI18NAnnotation> symbolStyle{
        this, "SymbolStyle", _("Symbol style"), SymbolStyle::Fullwidth};
    OptionWithAnnotation<PeriodStyle, PeriodStyleI18NAnnotation> periodStyle{
        this, "PeriodStyle", _("Period style"), PeriodStyle::FullwidthKuten};
    OptionWithAnnotation<CommaStyle, CommaStyleI18NAnnotation> commaStyle{
        this, "CommaStyle", _("Comma style"), CommaStyle::FullwidthToten};
    OptionWithAnnotation<SpaceStyle, SpaceStyleI18NAnnotation> spaceStyle{
        this, "SpaceStyle", _("Space style"), SpaceStyle::Fullwidth};
    OptionWithAnnotation<DiacriticStyle, DiacriticStyleI18NAnnotation>
        diacriticStyle{this, "DiacriticStyle", _("Diacritic style"),
                       DiacriticStyle::Fullwidth};
    OptionWithAnnotation<LearnMode, LearnModeI18NAnnotation> learnMode{
        this, "LearnMode", _("Learn mode"), LearnMode::None};
    OptionWithAnnotation<AutoCommitMode, AutoCommitModeI18NAnnotation>
        autoCommitMode{this, "AutoCommit", _("Auto commit"),
                       AutoCommitMode::None};
    Option<bool> zenzaiEnabled{this, "ZenzaiEnabled",
                               _("Enable Zenzai (Experimental)"), false};
    Option<int> zenzaiInferenceLimit{this, "ZenzaiInferenceLimit",
                                     _("Zenzai Inference limit"), 1};
    ExternalOption zenzaiHelp{
        this, "ZenzaiHelp", _("Click to open Zenzai Setup Guide"),
        stringutils::concat("xdg-open "
                            "https://github.com/7ka-Hiira/fcitx5-hazkey/tree/"
                            "main/docs/zenzai.md")};);

/// Engine

class HazkeyEngine : public InputMethodEngineV2 {
   public:
    // constructor
    HazkeyEngine(Instance *instance);

    // handle key event and pass it to HazkeyState
    void keyEvent(const InputMethodEntry &entry, KeyEvent &keyEvent) override;

    // called when input method changes to Hazkey
    void activate(const InputMethodEntry &, InputContextEvent &) override;

    // called when input method changes to another input method
    void deactivate(const InputMethodEntry &, InputContextEvent &) override;

    const KkcConfig *getKkcConfig() const { return kkcConfig_; }

    auto factory() const { return &factory_; }
    auto instance() const { return instance_; }

    const Configuration *getConfig() const override { return &config_; }
    void setConfig(const RawConfig &config) override;
    void reloadConfig() override;

    const HazkeyEngineConfig &config() const { return config_; }

   private:
    HazkeyEngineConfig config_;
    const KkcConfig *kkcConfig_;
    Instance *instance_;
    FactoryFor<HazkeyState> factory_;
    iconv_t conv_;
};

class HazkeyEngineFactory : public AddonFactory {
    AddonInstance *create(AddonManager *manager) override {
        FCITX_UNUSED(manager);
        return new HazkeyEngine(manager->instance());
    }
};

}  // namespace fcitx
#endif  // _FCITX5_HAZKEY_HAZKEY_H_
