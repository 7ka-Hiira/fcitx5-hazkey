#ifndef CONFIG_MACROS_H
#define CONFIG_MACROS_H

#include <QtCore>

#define SET_COMBO(ui_element, enum_value, default_index, ...) \
    do {                                                      \
        int index = default_index;                            \
        switch (enum_value) { __VA_ARGS__ }                   \
        ui_element->setCurrentIndex(index);                   \
    } while (0)

#define GET_COMBO_ENUM(ui_element, enum_type, default_enum, ...) \
    [&]() -> enum_type {                                         \
        int idx = ui_element->currentIndex();                    \
        switch (idx) { __VA_ARGS__ }                             \
        return default_enum;                                     \
    }()

#define SET_CHECKBOX(ui_element, bool_value, default_value) \
    ui_element->setCheckState((bool_value) ? Qt::Checked : Qt::Unchecked)

#define GET_CHECKBOX_BOOL(ui_element) (ui_element->checkState() == Qt::Checked)

#define SET_SPINBOX(ui_element, int_value, default_value) \
    ui_element->setValue(int_value)

#define GET_SPINBOX_INT(ui_element) ui_element->value()

#define ENUM_CASE(enum_val, index_val) \
    case enum_val:                     \
        index = index_val;             \
        break;

#define INDEX_CASE(index_val, enum_val) \
    case index_val:                     \
        return enum_val;

// 設定定義を使った便利マクロ
#define SET_COMBO_FROM_CONFIG(config_struct, ui_element, enum_value) \
    config_struct::setFromEnum(ui_element, enum_value)

#define GET_COMBO_TO_CONFIG(config_struct, ui_element) \
    config_struct::getEnum(ui_element)

// #define SET_CHECKBOX(ui_element, bool_value, default_value) \
    // SET_CHECKBOX(ui_element, bool_value)

#define SET_SPINBOX_WITH_DEFAULT(ui_element, int_value, default_value) \
    SET_SPINBOX(ui_element, int_value)

#endif  // CONFIG_MACROS_H
