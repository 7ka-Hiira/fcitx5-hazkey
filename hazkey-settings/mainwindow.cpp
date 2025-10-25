#include "mainwindow.h"

#include <qlabel.h>

#include <QAbstractButton>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>

#include "./ui_mainwindow.h"
#include "config_definitions.h"
#include "serverconnector.h"

MainWindow::MainWindow(QWidget* parent)
    : QWidget(parent), ui_(new Ui::MainWindow), server_(ServerConnector()) {
    ui_->setupUi(this);

    // Expand table settings mode change tab
    ui_->inputTableConfigModeTabWidget->tabBar()->setExpanding(true);

    // Connect UI signals
    connectSignals();

    // Setup input table lists
    setupInputTableLists();

    // Setup keymap lists
    setupKeymapLists();

    // Load configuration
    if (!loadCurrentConfig()) {
        // If config loading fails, disable UI elements
        setEnabled(false);
        QMessageBox::critical(this, "Configuration Error",
                              "Failed to load configuration. Please check your "
                              "connection to the hazkey server.");
    }
}

void MainWindow::connectSignals() {
    // Connect dialog buttons
    connect(ui_->dialogButtonBox, &QDialogButtonBox::accepted, this,
            &MainWindow::onApply);
    connect(ui_->dialogButtonBox, &QDialogButtonBox::clicked, this,
            &MainWindow::onButtonClicked);

    // Connect history checkbox to enable/disable dependent controls
    connect(ui_->useHistory, &QCheckBox::toggled, this,
            &MainWindow::onUseHistoryToggled);

    // Connect input table management buttons
    connect(ui_->enableTable, &QToolButton::clicked, this,
            &MainWindow::onEnableTable);
    connect(ui_->disableTable, &QToolButton::clicked, this,
            &MainWindow::onDisableTable);
    connect(ui_->tableMoveUp, &QToolButton::clicked, this,
            &MainWindow::onTableMoveUp);
    connect(ui_->tableMoveDown, &QToolButton::clicked, this,
            &MainWindow::onTableMoveDown);

    // Connect list selection changes to update button states
    connect(ui_->enabledTableList, &QListWidget::itemSelectionChanged, this,
            &MainWindow::onEnabledTableSelectionChanged);
    connect(ui_->availableTableList, &QListWidget::itemSelectionChanged, this,
            &MainWindow::onAvailableTableSelectionChanged);

    // Connect keymap management buttons
    connect(ui_->enableKeymap, &QToolButton::clicked, this,
            &MainWindow::onEnableKeymap);
    connect(ui_->disableKeymap, &QToolButton::clicked, this,
            &MainWindow::onDisableKeymap);
    connect(ui_->keymapMoveUp, &QToolButton::clicked, this,
            &MainWindow::onKeymapMoveUp);
    connect(ui_->keymapMoveDown, &QToolButton::clicked, this,
            &MainWindow::onKeymapMoveDown);

    // Connect keymap list selection changes to update button states
    connect(ui_->enabledKeymapList, &QListWidget::itemSelectionChanged, this,
            &MainWindow::onEnabledKeymapSelectionChanged);
    connect(ui_->availableKeymapList, &QListWidget::itemSelectionChanged, this,
            &MainWindow::onAvailableKeymapSelectionChanged);
}

void MainWindow::onButtonClicked(QAbstractButton* button) {
    QDialogButtonBox::StandardButton standardButton =
        ui_->dialogButtonBox->standardButton(button);

    switch (standardButton) {
        case QDialogButtonBox::Ok:
            if (saveCurrentConfig()) {
                close();
            }
            break;
        case QDialogButtonBox::Apply:
            saveCurrentConfig();
            break;
        case QDialogButtonBox::Cancel:
            close();
            break;
        default:
            break;
    }
}

void MainWindow::onApply() { saveCurrentConfig(); }

void MainWindow::onUseHistoryToggled(bool enabled) {
    ui_->stopStoreNewHistory->setEnabled(enabled);
}

bool MainWindow::loadCurrentConfig() {
    auto configOpt = server_.getConfig();

    if (!configOpt.has_value()) {
        return false;
    }

    currentConfig_ = configOpt.value();
    if (currentConfig_.profiles_size() == 0) {
        return false;
    }

    currentProfile_ = currentConfig_.mutable_profiles(0);
    if (!currentProfile_) {
        return false;
    }

    if (!currentConfig_.is_zenzai_available()) {
        ui_->enableZenzai->setEnabled(false);
        ui_->zenzaiContextualConversion->setEnabled(false);
        ui_->zenzaiInferenceLimit->setEnabled(false);
        ui_->zenzaiUserPlofile->setEnabled(false);
        QLabel* warningLabel =
            new QLabel("<b>Warning:</b> Zenzai support not installed.");
        warningLabel->setStyleSheet("background-color: yellow; padding: 5px;");
        ui_->aiTabScrollContentsLayout->insertWidget(1, warningLabel);
    }

    SET_COMBO_FROM_CONFIG(ConfigDefs::AutoConvertMode, ui_->autoConvertion,
                          currentProfile_->auto_convert_mode());
    SET_COMBO_FROM_CONFIG(ConfigDefs::AuxTextMode, ui_->auxiliaryText,
                          currentProfile_->aux_text_mode());
    SET_COMBO_FROM_CONFIG(ConfigDefs::SuggestionListMode, ui_->suggestionList,
                          currentProfile_->suggestion_list_mode());

    SET_SPINBOX(ui_->numSuggestion, currentProfile_->num_suggestions(),
                ConfigDefs::SpinboxDefaults::NUM_SUGGESTIONS);
    SET_SPINBOX(ui_->numCandidatesPerPage,
                currentProfile_->num_candidates_per_page(),
                ConfigDefs::SpinboxDefaults::NUM_CANDIDATES_PER_PAGE);
    SET_SPINBOX(ui_->zenzaiInferenceLimit,
                currentProfile_->zenzai_infer_limit(),
                ConfigDefs::SpinboxDefaults::ZENZAI_INFERENCE_LIMIT);

    SET_CHECKBOX(ui_->useHistory, currentProfile_->use_input_history(),
                 ConfigDefs::CheckboxDefaults::USE_HISTORY);
    SET_CHECKBOX(ui_->stopStoreNewHistory,
                 currentProfile_->stop_store_new_history(),
                 ConfigDefs::CheckboxDefaults::STOP_STORE_NEW_HISTORY);
    SET_CHECKBOX(ui_->enableZenzai, currentProfile_->zenzai_enable(),
                 ConfigDefs::CheckboxDefaults::ENABLE_ZENZAI);
    SET_CHECKBOX(ui_->zenzaiContextualConversion,
                 currentProfile_->zenzai_contextual_mode(),
                 ConfigDefs::CheckboxDefaults::ZENZAI_CONTEXTUAL);

    auto specialConversions = &currentProfile_->special_conversion_mode();
    SET_CHECKBOX(ui_->halfwidthKatakanaConversion,
                 specialConversions->halfwidth_katakana(),
                 ConfigDefs::CheckboxDefaults::HALFWIDTH_KATAKANA);
    SET_CHECKBOX(ui_->extendedEmojiConversion,
                 specialConversions->extended_emoji(),
                 ConfigDefs::CheckboxDefaults::EXTENDED_EMOJI);
    SET_CHECKBOX(ui_->commaSeparatedNumCoversion,
                 specialConversions->comma_separated_number(),
                 ConfigDefs::CheckboxDefaults::COMMA_SEPARATED_NUMBER);
    SET_CHECKBOX(ui_->calendarConversion, specialConversions->calendar(),
                 ConfigDefs::CheckboxDefaults::CALENDER);
    SET_CHECKBOX(ui_->timeConversion, specialConversions->time(),
                 ConfigDefs::CheckboxDefaults::TIME);
    SET_CHECKBOX(ui_->mailDomainConversion, specialConversions->mail_domain(),
                 ConfigDefs::CheckboxDefaults::MAIL_DOMAIN);
    SET_CHECKBOX(ui_->unicodeCodePointConversion,
                 specialConversions->unicode_codepoint(),
                 ConfigDefs::CheckboxDefaults::UNICODE_CODEPOINT);
    SET_CHECKBOX(ui_->romanTypographyConversion,
                 specialConversions->roman_typography(),
                 ConfigDefs::CheckboxDefaults::ROMAN_TYPOGRAPHY);
    SET_CHECKBOX(ui_->hazkeyVersionConversion,
                 specialConversions->hazkey_version(),
                 ConfigDefs::CheckboxDefaults::HAZKEY_VERSION);

    ui_->stopStoreNewHistory->setEnabled(currentProfile_->use_input_history());

    SET_LINEEDIT(ui_->submodeEntryPointChars,
                 currentProfile_->submode_entry_point_chars(),
                 "ABCDEFGHIJKLMNOPQRSTUVWXYZ");

    // Load input table configuration
    loadInputTables();

    // Load keymap configuration
    loadKeymaps();

    return true;
}

bool MainWindow::saveCurrentConfig() {
    if (!currentProfile_) {
        QMessageBox::warning(this, "Error", "No configuration profile loaded.");
        return false;
    }

    currentProfile_->set_auto_convert_mode(
        GET_COMBO_TO_CONFIG(ConfigDefs::AutoConvertMode, ui_->autoConvertion));
    currentProfile_->set_aux_text_mode(
        GET_COMBO_TO_CONFIG(ConfigDefs::AuxTextMode, ui_->auxiliaryText));
    currentProfile_->set_suggestion_list_mode(GET_COMBO_TO_CONFIG(
        ConfigDefs::SuggestionListMode, ui_->suggestionList));

    currentProfile_->set_num_suggestions(GET_SPINBOX_INT(ui_->numSuggestion));
    currentProfile_->set_num_candidates_per_page(
        GET_SPINBOX_INT(ui_->numCandidatesPerPage));
    currentProfile_->set_zenzai_infer_limit(
        GET_SPINBOX_INT(ui_->zenzaiInferenceLimit));

    currentProfile_->set_use_input_history(GET_CHECKBOX_BOOL(ui_->useHistory));
    currentProfile_->set_stop_store_new_history(
        GET_CHECKBOX_BOOL(ui_->stopStoreNewHistory));
    currentProfile_->set_zenzai_enable(GET_CHECKBOX_BOOL(ui_->enableZenzai));
    currentProfile_->set_zenzai_contextual_mode(
        GET_CHECKBOX_BOOL(ui_->zenzaiContextualConversion));

    auto* specialConversions =
        currentProfile_->mutable_special_conversion_mode();
    specialConversions->set_halfwidth_katakana(
        GET_CHECKBOX_BOOL(ui_->halfwidthKatakanaConversion));
    specialConversions->set_extended_emoji(
        GET_CHECKBOX_BOOL(ui_->extendedEmojiConversion));
    specialConversions->set_comma_separated_number(
        GET_CHECKBOX_BOOL(ui_->commaSeparatedNumCoversion));
    specialConversions->set_calendar(
        GET_CHECKBOX_BOOL(ui_->calendarConversion));
    specialConversions->set_time(GET_CHECKBOX_BOOL(ui_->timeConversion));
    specialConversions->set_mail_domain(
        GET_CHECKBOX_BOOL(ui_->mailDomainConversion));
    specialConversions->set_unicode_codepoint(
        GET_CHECKBOX_BOOL(ui_->unicodeCodePointConversion));
    specialConversions->set_roman_typography(
        GET_CHECKBOX_BOOL(ui_->romanTypographyConversion));
    specialConversions->set_hazkey_version(
        GET_CHECKBOX_BOOL(ui_->hazkeyVersionConversion));

    currentProfile_->set_submode_entry_point_chars(
        GET_LINEEDIT_STRING(ui_->submodeEntryPointChars));

    // Save input table configuration
    saveInputTables();

    // Save keymap configuration
    saveKeymaps();

    // Save to server
    try {
        server_.setCurrentConfig(currentConfig_);
        return true;
    } catch (const std::exception& e) {
        QMessageBox::critical(
            this, "Save Error",
            QString("Failed to save configuration: %1").arg(e.what()));
        return false;
    } catch (...) {
        QMessageBox::critical(
            this, "Save Error",
            "An unknown error occurred while saving configuration.");
        return false;
    }
}

void MainWindow::setupInputTableLists() {
    // Enable double-click to move items between lists
    connect(ui_->enabledTableList, &QListWidget::itemDoubleClicked, this,
            &MainWindow::onDisableTable);
    connect(ui_->availableTableList, &QListWidget::itemDoubleClicked, this,
            &MainWindow::onEnableTable);

    // Update button states initially
    updateTableButtonStates();
}

void MainWindow::loadInputTables() {
    if (!currentProfile_) {
        return;
    }

    // Clear existing items
    ui_->enabledTableList->clear();
    ui_->availableTableList->clear();

    // Create sets for quick lookup
    QSet<QString> enabledTableNames;

    // Load enabled tables
    for (int i = 0; i < currentProfile_->enabled_tables_size(); ++i) {
        const auto& enabledTable = currentProfile_->enabled_tables(i);
        QString tableName = QString::fromStdString(enabledTable.name());
        enabledTableNames.insert(tableName);

        QListWidgetItem* item = new QListWidgetItem(tableName);

        // Check if this table is available
        bool isAvailable = false;
        bool isBuiltIn = false;

        for (int j = 0; j < currentConfig_.available_tables_size(); ++j) {
            const auto& availableTable = currentConfig_.available_tables(j);
            if (availableTable.name() == enabledTable.name()) {
                isAvailable = true;
                isBuiltIn = availableTable.is_built_in();
                break;
            }
        }

        // Set item appearance based on status
        if (!isAvailable) {
            item->setText(tableName + " [not found]");
            item->setForeground(QColor(Qt::red));
        } else if (isBuiltIn) {
            item->setText(tableName + " [built-in]");
        }

        // Store original name and metadata
        item->setData(Qt::UserRole, tableName);
        item->setData(Qt::UserRole + 1, isBuiltIn);
        item->setData(Qt::UserRole + 2, isAvailable);

        ui_->enabledTableList->addItem(item);
    }

    // Load available tables (excluding already enabled ones)
    for (int i = 0; i < currentConfig_.available_tables_size(); ++i) {
        const auto& availableTable = currentConfig_.available_tables(i);
        QString tableName = QString::fromStdString(availableTable.name());

        if (!enabledTableNames.contains(tableName)) {
            QListWidgetItem* item = new QListWidgetItem(tableName);

            if (availableTable.is_built_in()) {
                item->setText(tableName + " [built-in]");
            }

            // Store metadata
            item->setData(Qt::UserRole, tableName);
            item->setData(Qt::UserRole + 1, availableTable.is_built_in());
            item->setData(Qt::UserRole + 2, true);  // available

            ui_->availableTableList->addItem(item);
        }
    }

    updateTableButtonStates();
}

void MainWindow::saveInputTables() {
    if (!currentProfile_) {
        return;
    }

    // Clear existing enabled tables
    currentProfile_->clear_enabled_tables();

    // Save enabled tables in order
    for (int i = 0; i < ui_->enabledTableList->count(); ++i) {
        QListWidgetItem* item = ui_->enabledTableList->item(i);
        QString tableName = item->data(Qt::UserRole).toString();
        bool isBuiltIn = item->data(Qt::UserRole + 1).toBool();
        bool isAvailable = item->data(Qt::UserRole + 2).toBool();

        auto* enabledTable = currentProfile_->add_enabled_tables();
        enabledTable->set_name(tableName.toStdString());
        enabledTable->set_is_built_in(isBuiltIn);

        // Find filename from available tables if available
        if (isAvailable) {
            for (int j = 0; j < currentConfig_.available_tables_size(); ++j) {
                const auto& availableTable = currentConfig_.available_tables(j);
                if (availableTable.name() == tableName.toStdString()) {
                    enabledTable->set_filename(availableTable.filename());
                    break;
                }
            }
        }
    }
}

void MainWindow::onEnableTable() {
    QListWidgetItem* item = ui_->availableTableList->currentItem();
    if (!item) {
        return;
    }

    // Move item from available to enabled list
    int row = ui_->availableTableList->row(item);
    ui_->availableTableList->takeItem(row);
    ui_->enabledTableList->addItem(item);

    updateTableButtonStates();
}

void MainWindow::onDisableTable() {
    QListWidgetItem* item = ui_->enabledTableList->currentItem();
    if (!item) {
        return;
    }

    // Only move to available list if the table is actually available
    bool isAvailable = item->data(Qt::UserRole + 2).toBool();

    int row = ui_->enabledTableList->row(item);
    ui_->enabledTableList->takeItem(row);

    if (isAvailable) {
        // Reset display text for available list
        QString tableName = item->data(Qt::UserRole).toString();
        bool isBuiltIn = item->data(Qt::UserRole + 1).toBool();

        if (isBuiltIn) {
            item->setText(tableName + " [built-in]");
        } else {
            item->setText(tableName);
        }
        item->setForeground(QColor());  // Reset color

        ui_->availableTableList->addItem(item);
    } else {
        // Delete item if table is not available
        delete item;
    }

    updateTableButtonStates();
}

void MainWindow::onTableMoveUp() {
    QListWidgetItem* item = ui_->enabledTableList->currentItem();
    if (!item) {
        return;
    }

    int row = ui_->enabledTableList->row(item);
    if (row > 0) {
        ui_->enabledTableList->takeItem(row);
        ui_->enabledTableList->insertItem(row - 1, item);
        ui_->enabledTableList->setCurrentItem(item);
    }

    updateTableButtonStates();
}

void MainWindow::onTableMoveDown() {
    QListWidgetItem* item = ui_->enabledTableList->currentItem();
    if (!item) {
        return;
    }

    int row = ui_->enabledTableList->row(item);
    if (row < ui_->enabledTableList->count() - 1) {
        ui_->enabledTableList->takeItem(row);
        ui_->enabledTableList->insertItem(row + 1, item);
        ui_->enabledTableList->setCurrentItem(item);
    }

    updateTableButtonStates();
}

void MainWindow::onEnabledTableSelectionChanged() { updateTableButtonStates(); }

void MainWindow::onAvailableTableSelectionChanged() {
    updateTableButtonStates();
}

void MainWindow::updateTableButtonStates() {
    QListWidgetItem* enabledItem = ui_->enabledTableList->currentItem();
    QListWidgetItem* availableItem = ui_->availableTableList->currentItem();

    // Enable/disable buttons based on selection and position
    ui_->disableTable->setEnabled(enabledItem != nullptr);
    ui_->enableTable->setEnabled(availableItem != nullptr);

    if (enabledItem) {
        int row = ui_->enabledTableList->row(enabledItem);
        ui_->tableMoveUp->setEnabled(row > 0);
        ui_->tableMoveDown->setEnabled(row <
                                       ui_->enabledTableList->count() - 1);
    } else {
        ui_->tableMoveUp->setEnabled(false);
        ui_->tableMoveDown->setEnabled(false);
    }
}

void MainWindow::setupKeymapLists() {
    // Enable double-click to move items between lists
    connect(ui_->enabledKeymapList, &QListWidget::itemDoubleClicked, this,
            &MainWindow::onDisableKeymap);
    connect(ui_->availableKeymapList, &QListWidget::itemDoubleClicked, this,
            &MainWindow::onEnableKeymap);

    // Update button states initially
    updateKeymapButtonStates();
}

void MainWindow::loadKeymaps() {
    if (!currentProfile_) {
        return;
    }

    // Clear existing items
    ui_->enabledKeymapList->clear();
    ui_->availableKeymapList->clear();

    // Create sets for quick lookup
    QSet<QString> enabledKeymapNames;

    // Load enabled keymaps
    for (int i = 0; i < currentProfile_->enabled_keymaps_size(); ++i) {
        const auto& enabledKeymap = currentProfile_->enabled_keymaps(i);
        QString keymapName = QString::fromStdString(enabledKeymap.name());
        enabledKeymapNames.insert(keymapName);

        QListWidgetItem* item = new QListWidgetItem(keymapName);

        // Check if this keymap is available
        bool isAvailable = false;
        bool isBuiltIn = false;

        for (int j = 0; j < currentConfig_.available_keymaps_size(); ++j) {
            const auto& availableKeymap = currentConfig_.available_keymaps(j);
            if (availableKeymap.name() == enabledKeymap.name()) {
                isAvailable = true;
                isBuiltIn = availableKeymap.is_built_in();
                break;
            }
        }

        // Set item appearance based on status
        if (!isAvailable) {
            item->setText(keymapName + " [not found]");
            item->setForeground(QColor(Qt::red));
        } else if (isBuiltIn) {
            item->setText(keymapName + " [built-in]");
        }

        // Store original name and metadata
        item->setData(Qt::UserRole, keymapName);
        item->setData(Qt::UserRole + 1, isBuiltIn);
        item->setData(Qt::UserRole + 2, isAvailable);

        ui_->enabledKeymapList->addItem(item);
    }

    // Load available keymaps (excluding already enabled ones)
    for (int i = 0; i < currentConfig_.available_keymaps_size(); ++i) {
        const auto& availableKeymap = currentConfig_.available_keymaps(i);
        QString keymapName = QString::fromStdString(availableKeymap.name());

        if (!enabledKeymapNames.contains(keymapName)) {
            QListWidgetItem* item = new QListWidgetItem(keymapName);

            if (availableKeymap.is_built_in()) {
                item->setText(keymapName + " [built-in]");
            }

            // Store metadata
            item->setData(Qt::UserRole, keymapName);
            item->setData(Qt::UserRole + 1, availableKeymap.is_built_in());
            item->setData(Qt::UserRole + 2, true);  // available

            ui_->availableKeymapList->addItem(item);
        }
    }

    updateKeymapButtonStates();
}

void MainWindow::saveKeymaps() {
    if (!currentProfile_) {
        return;
    }

    // Clear existing enabled keymaps
    currentProfile_->clear_enabled_keymaps();

    // Save enabled keymaps in order
    for (int i = 0; i < ui_->enabledKeymapList->count(); ++i) {
        QListWidgetItem* item = ui_->enabledKeymapList->item(i);
        QString keymapName = item->data(Qt::UserRole).toString();
        bool isBuiltIn = item->data(Qt::UserRole + 1).toBool();
        bool isAvailable = item->data(Qt::UserRole + 2).toBool();

        auto* enabledKeymap = currentProfile_->add_enabled_keymaps();
        enabledKeymap->set_name(keymapName.toStdString());
        enabledKeymap->set_is_built_in(isBuiltIn);

        // Find filename from available keymaps if available
        if (isAvailable) {
            for (int j = 0; j < currentConfig_.available_keymaps_size(); ++j) {
                const auto& availableKeymap =
                    currentConfig_.available_keymaps(j);
                if (availableKeymap.name() == keymapName.toStdString()) {
                    enabledKeymap->set_filename(availableKeymap.filename());
                    break;
                }
            }
        }
    }
}

void MainWindow::onEnableKeymap() {
    QListWidgetItem* item = ui_->availableKeymapList->currentItem();
    if (!item) {
        return;
    }

    // Move item from available to enabled list
    int row = ui_->availableKeymapList->row(item);
    ui_->availableKeymapList->takeItem(row);
    ui_->enabledKeymapList->addItem(item);

    updateKeymapButtonStates();
}

void MainWindow::onDisableKeymap() {
    QListWidgetItem* item = ui_->enabledKeymapList->currentItem();
    if (!item) {
        return;
    }

    // Only move to available list if the keymap is actually available
    bool isAvailable = item->data(Qt::UserRole + 2).toBool();

    int row = ui_->enabledKeymapList->row(item);
    ui_->enabledKeymapList->takeItem(row);

    if (isAvailable) {
        // Reset display text for available list
        QString keymapName = item->data(Qt::UserRole).toString();
        bool isBuiltIn = item->data(Qt::UserRole + 1).toBool();

        if (isBuiltIn) {
            item->setText(keymapName + " [built-in]");
        } else {
            item->setText(keymapName);
        }
        item->setForeground(QColor());  // Reset color

        ui_->availableKeymapList->addItem(item);
    } else {
        // Delete item if keymap is not available
        delete item;
    }

    updateKeymapButtonStates();
}

void MainWindow::onKeymapMoveUp() {
    QListWidgetItem* item = ui_->enabledKeymapList->currentItem();
    if (!item) {
        return;
    }

    int row = ui_->enabledKeymapList->row(item);
    if (row > 0) {
        ui_->enabledKeymapList->takeItem(row);
        ui_->enabledKeymapList->insertItem(row - 1, item);
        ui_->enabledKeymapList->setCurrentItem(item);
    }

    updateKeymapButtonStates();
}

void MainWindow::onKeymapMoveDown() {
    QListWidgetItem* item = ui_->enabledKeymapList->currentItem();
    if (!item) {
        return;
    }

    int row = ui_->enabledKeymapList->row(item);
    if (row < ui_->enabledKeymapList->count() - 1) {
        ui_->enabledKeymapList->takeItem(row);
        ui_->enabledKeymapList->insertItem(row + 1, item);
        ui_->enabledKeymapList->setCurrentItem(item);
    }

    updateKeymapButtonStates();
}

void MainWindow::onEnabledKeymapSelectionChanged() {
    updateKeymapButtonStates();
}

void MainWindow::onAvailableKeymapSelectionChanged() {
    updateKeymapButtonStates();
}

void MainWindow::updateKeymapButtonStates() {
    QListWidgetItem* enabledItem = ui_->enabledKeymapList->currentItem();
    QListWidgetItem* availableItem = ui_->availableKeymapList->currentItem();

    // Enable/disable buttons based on selection and position
    ui_->disableKeymap->setEnabled(enabledItem != nullptr);
    ui_->enableKeymap->setEnabled(availableItem != nullptr);

    if (enabledItem) {
        int row = ui_->enabledKeymapList->row(enabledItem);
        ui_->keymapMoveUp->setEnabled(row > 0);
        ui_->keymapMoveDown->setEnabled(row <
                                        ui_->enabledKeymapList->count() - 1);
    } else {
        ui_->keymapMoveUp->setEnabled(false);
        ui_->keymapMoveDown->setEnabled(false);
    }
}

MainWindow::~MainWindow() { delete ui_; }
