#ifndef CONFIG_DEFINITIONS_H
#define CONFIG_DEFINITIONS_H

#include <QComboBox>
#include <QtCore>

#include "base.pb.h"
#include "config_macros.h"

#define CONFIG_COMBO_DEFINITION(name, default_index, ...)                      \
    struct name##_Config {                                                     \
        static constexpr int DEFAULT_INDEX = default_index;                    \
        static void setFromEnum(QComboBox* combo, int enumValue) {             \
            SET_COMBO_WITH_DEFAULT(combo, enumValue, DEFAULT_INDEX,            \
                                   __VA_ARGS__)                                \
        }                                                                      \
        template <typename EnumType>                                           \
        static EnumType getEnum(QComboBox* combo, EnumType defaultEnum) {      \
            return GET_COMBO_ENUM_WITH_DEFAULT(combo, EnumType, defaultEnum,   \
                                               MAKE_INDEX_CASES(__VA_ARGS__)); \
        }                                                                      \
    };

#define ENUM_CASE(enum_val, index_val) \
    case enum_val:                     \
        index = index_val;             \
        break;

#define INDEX_CASE(index_val, enum_val) \
    case index_val:                     \
        return enum_val;

#define MAKE_INDEX_CASES(...) CONVERT_ENUM_CASES_TO_INDEX_CASES(__VA_ARGS__)

namespace ConfigDefs {
struct AutoConvertMode {
    static constexpr int DEFAULT_INDEX = 1;  // AUTO_CONVERT_FOR_MULTIPLE_CHARS
    using EnumType = hazkey::config::Profile_AutoConvertMode;
    static constexpr EnumType DEFAULT_ENUM =
        hazkey::config::Profile_AutoConvertMode_AUTO_CONVERT_FOR_MULTIPLE_CHARS;

    static void setFromEnum(QComboBox* combo, int enumValue) {
        SET_COMBO(
            combo, enumValue, DEFAULT_INDEX,
            ENUM_CASE(
                hazkey::config::Profile_AutoConvertMode_AUTO_CONVERT_ALWAYS, 0)
                ENUM_CASE(
                    hazkey::config::
                        Profile_AutoConvertMode_AUTO_CONVERT_FOR_MULTIPLE_CHARS,
                    1)
                    ENUM_CASE(hazkey::config::
                                  Profile_AutoConvertMode_AUTO_CONVERT_DISABLED,
                              2));
    }

    static EnumType getEnum(QComboBox* combo) {
        return GET_COMBO_ENUM(
            combo, EnumType, DEFAULT_ENUM,
            INDEX_CASE(
                0, hazkey::config::Profile_AutoConvertMode_AUTO_CONVERT_ALWAYS)
                INDEX_CASE(
                    1,
                    hazkey::config::
                        Profile_AutoConvertMode_AUTO_CONVERT_FOR_MULTIPLE_CHARS)
                    INDEX_CASE(
                        2, hazkey::config::
                               Profile_AutoConvertMode_AUTO_CONVERT_DISABLED));
    }
};

struct AuxTextMode {
    static constexpr int DEFAULT_INDEX =
        1;  // AUX_TEXT_SHOW_WHEN_CURSOR_NOT_AT_END
    using EnumType = hazkey::config::Profile_AuxTextMode;
    static constexpr EnumType DEFAULT_ENUM = hazkey::config::
        Profile_AuxTextMode_AUX_TEXT_SHOW_WHEN_CURSOR_NOT_AT_END;

    static void setFromEnum(QComboBox* combo, int enumValue) {
        SET_COMBO(
            combo, enumValue, DEFAULT_INDEX,
            ENUM_CASE(hazkey::config::Profile_AuxTextMode_AUX_TEXT_SHOW_ALWAYS,
                      0)
                ENUM_CASE(
                    hazkey::config::
                        Profile_AuxTextMode_AUX_TEXT_SHOW_WHEN_CURSOR_NOT_AT_END,
                    1)
                    ENUM_CASE(
                        hazkey::config::Profile_AuxTextMode_AUX_TEXT_DISABLED,
                        2));
    }

    static EnumType getEnum(QComboBox* combo) {
        return GET_COMBO_ENUM(
            combo, EnumType, DEFAULT_ENUM,
            INDEX_CASE(0,
                       hazkey::config::Profile_AuxTextMode_AUX_TEXT_SHOW_ALWAYS)
                INDEX_CASE(
                    1,
                    hazkey::config::
                        Profile_AuxTextMode_AUX_TEXT_SHOW_WHEN_CURSOR_NOT_AT_END)
                    INDEX_CASE(
                        2,
                        hazkey::config::Profile_AuxTextMode_AUX_TEXT_DISABLED));
    }
};

struct SuggestionListMode {
    static constexpr int DEFAULT_INDEX =
        1;  // SUGGESTION_LIST_SHOW_PREDICTIVE_RESULTS
    using EnumType = hazkey::config::Profile_SuggestionListMode;
    static constexpr EnumType DEFAULT_ENUM = hazkey::config::
        Profile_SuggestionListMode_SUGGESTION_LIST_SHOW_PREDICTIVE_RESULTS;

    static void setFromEnum(QComboBox* combo, int enumValue) {
        SET_COMBO(
            combo, enumValue, DEFAULT_INDEX,
            ENUM_CASE(
                hazkey::config::
                    Profile_SuggestionListMode_SUGGESTION_LIST_SHOW_NORMAL_RESULTS,
                0)
                ENUM_CASE(
                    hazkey::config::
                        Profile_SuggestionListMode_SUGGESTION_LIST_SHOW_PREDICTIVE_RESULTS,
                    1)
                    ENUM_CASE(
                        hazkey::config::
                            Profile_SuggestionListMode_SUGGESTION_LIST_DISABLED,
                        2));
    }

    static EnumType getEnum(QComboBox* combo) {
        return GET_COMBO_ENUM(
            combo, EnumType, DEFAULT_ENUM,
            INDEX_CASE(
                0,
                hazkey::config::
                    Profile_SuggestionListMode_SUGGESTION_LIST_SHOW_NORMAL_RESULTS)
                INDEX_CASE(
                    1,
                    hazkey::config::
                        Profile_SuggestionListMode_SUGGESTION_LIST_SHOW_PREDICTIVE_RESULTS)
                    INDEX_CASE(
                        2,
                        hazkey::config::
                            Profile_SuggestionListMode_SUGGESTION_LIST_DISABLED));
    }
};

struct CheckboxDefaults {
    static constexpr bool USE_HISTORY = false;
    static constexpr bool STOP_STORE_NEW_HISTORY = false;
    static constexpr bool ENABLE_ZENZAI = false;
    static constexpr bool ZENZAI_CONTEXTUAL = false;
    static constexpr bool HALFWIDTH_KATAKANA = false;
    static constexpr bool EXTENDED_EMOJI = false;
    static constexpr bool COMMA_SEPARATED_NUMBER = false;
    static constexpr bool CALENDER = false;
    static constexpr bool TIME = false;
    static constexpr bool MAIL_DOMAIN = false;
    static constexpr bool UNICODE_CODEPOINT = false;
    static constexpr bool ROMAN_TYPOGRAPHY = false;
    static constexpr bool HAZKEY_VERSION = false;
};

struct SpinboxDefaults {
    static constexpr int NUM_SUGGESTIONS = 5;
    static constexpr int NUM_CANDIDATES_PER_PAGE = 10;
    static constexpr int ZENZAI_INFERENCE_LIMIT = 100;
};
}  // namespace ConfigDefs

#endif  // CONFIG_DEFINITIONS_H
