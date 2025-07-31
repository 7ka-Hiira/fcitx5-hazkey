#ifndef _FCITX5_HAZKEY_HAZKEY_CONFIG_H_
#define _FCITX5_HAZKEY_HAZKEY_CONFIG_H_

#include <fcitx-config/configuration.h>
#include <fcitx-utils/i18n.h>
#include <fcitx-utils/library.h>
#include <fcitx/menu.h>

#include <string>

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

FCITX_CONFIG_ENUM_NAME_WITH_I18N(LearnMode, N_("Disabled"));

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
    Option<bool> zenzaiEnabled{
        this, "ZenzaiEnabled",
        _("Enable Zenzai (Experimental, Requires Vulkan)"), false};
    Option<bool> zenzaiSurroundingTextEnabled{
        this, "ZenzaiSurroundingTextEnabled",
        _("Enable Zenzai contextual converision"), false};
    Option<std::string> zenzaiProfile{this, "ZenzaiProfile",
                                      _("Zenzai Profile"), ""};
    Option<int> zenzaiInferenceLimit{this, "ZenzaiInferenceLimit",
                                     _("Zenzai Inference limit"), 1};
    Option<int> gpuLayers{this, "Zenzai GPU Layers", _("GPU Layers"), 99};
    ExternalOption zenzaiHelp{
        this, "ZenzaiHelp", _("Click to open Zenzai Setup Guide"),
        stringutils::concat("xdg-open "
                            "https://github.com/7ka-Hiira/fcitx5-hazkey/tree/"
                            "main/docs/zenzai.md")};);
}  // namespace fcitx
#endif  // _FCITX5_HAZKEY_HAZKEY_CONFIG_H_
