#include "mainwindow.h"

#include <qlabel.h>

#include <QAbstractButton>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>

#include "./ui_mainwindow.h"
#include "config_definitions.h"
#include "serverconnector.h"

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent), ui_(new Ui::MainWindow), server_(ServerConnector()) {
    ui_->setupUi(this);

    // Manage settings button
    QMenu *manageSettingsMenu = new QMenu(this);
    QAction *importAction = manageSettingsMenu->addAction("Import");
    QAction *exportAction = manageSettingsMenu->addAction("Export");
    QAction *restoreDefaultAction =
        manageSettingsMenu->addAction("Restore Default");
    QPushButton *manageSettingsButton = new QPushButton("Manage Settings");
    manageSettingsButton->setMenu(manageSettingsMenu);
    ui_->dialogButtonBox->addButton(manageSettingsButton,
                                    QDialogButtonBox::ResetRole);

    // Connect menu actions (placeholder implementations)
    connect(importAction, &QAction::triggered, this, [this]() {
        QMessageBox::information(this, "Import",
                                 "Import functionality not yet implemented.");
    });
    connect(exportAction, &QAction::triggered, this, [this]() {
        QMessageBox::information(this, "Export",
                                 "Export functionality not yet implemented.");
    });
    connect(restoreDefaultAction, &QAction::triggered, this, [this]() {
        QMessageBox::information(
            this, "Restore Default",
            "Restore Default functionality not yet implemented.");
    });

    // Expand table settings mode change tab
    ui_->inputTableConfigModeTabWidget->tabBar()->setExpanding(true);

    // Connect UI signals
    connectSignals();

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
}

void MainWindow::onButtonClicked(QAbstractButton *button) {
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
        QLabel *warningLabel =
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

    auto *specialConversions =
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

    // Save to server
    try {
        server_.setCurrentConfig(currentConfig_);
        return true;
    } catch (const std::exception &e) {
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

MainWindow::~MainWindow() { delete ui_; }
