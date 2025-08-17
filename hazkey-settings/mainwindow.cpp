#include "mainwindow.h"
#include <QMenu>

#include "./ui_mainwindow.h"
#include "serverconnector.h"

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent), ui_(new Ui::MainWindow), server_(ServerConnector()) {
    ui_->setupUi(this);

    // Manage settings button
    QMenu *manageSettingsMenu = new QMenu(this);
    QAction *importAction = manageSettingsMenu->addAction("Import");
    QAction *exportAction = manageSettingsMenu->addAction("Export");
    QAction *restoreDefaultAction = manageSettingsMenu->addAction("Restore Default");
    QPushButton* manageSettingsButton = new QPushButton("Manage Settings");
    manageSettingsButton->setMenu(manageSettingsMenu);
    ui_->dialogButtonBox->addButton(manageSettingsButton, QDialogButtonBox::ResetRole);

    // Advanced layout list action button
    QMenu *manageTableMenu = new QMenu(this);
    QAction *importTableAction = manageTableMenu->addAction("Import selected table");
    QAction *exportTableAction = manageTableMenu->addAction("Export selected table");
    ui_->tableMoreActions->setMenu(manageTableMenu);

    // Expand table settings mode change tab
    ui_->inputTableConfigModeTabWidget->tabBar()->setExpanding(true);

    // Expand table editor
    ui_->keymapEditorTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);


    // Connect to server
    server_.connect_server();

    loadCurrentConfig();
}

void MainWindow::loadCurrentConfig() {
    auto currentConfig = server_.getCurrentConfig().value();

    auto currentProfile = currentConfig.configs().Get(0);

    int autoConvertModeCurrentIndex;
    switch (currentProfile.auto_convert_mode()) {
        // FIXME: wrong index
        case hazkey::config::ConfigProfile_AutoConvertMode_AUTO_CONVERT_ALWAYS:
            autoConvertModeCurrentIndex = 0;
            break;
        case hazkey::config::ConfigProfile_AutoConvertMode_AUTO_CONVERT_DISABLED:
            autoConvertModeCurrentIndex = 2;
            break;
        case hazkey::config::ConfigProfile_AutoConvertMode_AUTO_CONVERT_FOR_MULTIPLE_CHARS:
        default:
            autoConvertModeCurrentIndex = 1;
    }
    ui_->autoConvertion->setCurrentIndex(autoConvertModeCurrentIndex);

    int auxTextModeCurrentIndex;
    switch (currentProfile.aux_text_mode()) {
        case hazkey::config::ConfigProfile_AuxTextMode_AUX_TEXT_SHOW_ALWAYS:
            auxTextModeCurrentIndex = 0;
            break;
        case hazkey::config::ConfigProfile_AuxTextMode_AUX_TEXT_DISABLED:
            auxTextModeCurrentIndex = 2;
            break;
        case hazkey::config::ConfigProfile_AuxTextMode_AUX_TEXT_SHOW_WHEN_CURSOR_NOT_AT_END:
        default:
            auxTextModeCurrentIndex = 1;
    }
    ui_->auxiliaryText->setCurrentIndex(auxTextModeCurrentIndex);

    int suggestionListModeCurrentIndex;
    switch (currentProfile.suggestion_list_mode()) {
        case hazkey::config::ConfigProfile_SuggestionListMode_SUGGESTION_LIST_SHOW_NORMAL_RESULTS:
            suggestionListModeCurrentIndex = 0;
            break;
        case hazkey::config::ConfigProfile_SuggestionListMode_SUGGESTION_LIST_SHOW_PREDICTIVE_RESULTS:
            suggestionListModeCurrentIndex = 1;
            break;
        case hazkey::config::ConfigProfile_SuggestionListMode_SUGGESTION_LIST_DISABLED:
        default:
            suggestionListModeCurrentIndex = 2;
    }
    ui_->suggestionList->setCurrentIndex(suggestionListModeCurrentIndex);

    ui_->numSuggestion->setValue(currentProfile.num_suggestions());
    ui_->numCandidatesPerPage->setValue(currentProfile.num_candidates_per_page());

    auto useHistoryState = currentProfile.use_input_history() ? Qt::Checked : Qt::CheckState::Unchecked;
    ui_->useHistory->setCheckState(useHistoryState);

    ui_->stopStoreNewHistory->setEnabled(currentProfile.use_input_history());

    auto stopStoreHistoryState = currentProfile.stop_store_new_history() ? Qt::Checked : Qt::CheckState::Unchecked;
    ui_->stopStoreNewHistory->setCheckState(stopStoreHistoryState);

    auto specialConversions = &currentProfile.special_conversion_mode();
    auto halfwidthKatakana = specialConversions->halfwidth_katakana() ? Qt::Checked : Qt::CheckState::Unchecked;
    ui_->halfwidthKatakanaConversion->setCheckState(halfwidthKatakana);
    auto extendedEmoji = specialConversions->extended_emoji() ? Qt::Checked : Qt::CheckState::Unchecked;
    ui_->extendedEmojiConversion->setCheckState(extendedEmoji);
    auto commaSeparatedNumber = specialConversions->comma_separated_number() ? Qt::Checked : Qt::CheckState::Unchecked;
    ui_->commaSeparatedNumCoversion->setCheckState(commaSeparatedNumber);
    auto calenderConversion = specialConversions->calender() ? Qt::Checked : Qt::CheckState::Unchecked;
    ui_->calenderConversion->setCheckState(calenderConversion);
    auto timeConversion = specialConversions->time() ? Qt::Checked : Qt::CheckState::Unchecked;
    ui_->timeConversion->setCheckState(timeConversion);
    auto mailDomainConversion = specialConversions->mail_domain() ? Qt::Checked : Qt::CheckState::Unchecked;
    ui_->mailDomainConversion->setCheckState(mailDomainConversion);
    auto unicodeConversion = specialConversions->unicode_codepoint() ? Qt::Checked : Qt::CheckState::Unchecked;
    ui_->unicodeCodePointConversion->setCheckState(unicodeConversion);
    auto romanTypographyConversion = specialConversions->roman_typography() ? Qt::Checked : Qt::CheckState::Unchecked;
    ui_->romanTypographyConversion->setCheckState(romanTypographyConversion);
    auto versionConversion = specialConversions->hazkey_version() ? Qt::Checked : Qt::CheckState::Unchecked;
    ui_->hazkeyVersionConversion->setCheckState(versionConversion);

    auto enableZenzai = currentProfile.zenzai_enable() ? Qt::Checked : Qt::CheckState::Unchecked;
    ui_->enableZenzai->setCheckState(enableZenzai);

    auto enableZenzaiContextual = currentProfile.zenzai_contextual_mode() ? Qt::Checked : Qt::CheckState::Unchecked;
    ui_->zenzaiContextualConversion->setCheckState(enableZenzaiContextual);

    ui_->zenzaiInferenceLimit->setValue(currentProfile.zenzai_infer_limit());

    auto zenzaiProfile = std::string();
    auto zenzaiVersionConfig = currentProfile.zenzai_version_config();
    if (zenzaiVersionConfig.has_v2()) {
        zenzaiProfile = zenzaiVersionConfig.v2().profile();
    } else if (zenzaiVersionConfig.has_v3()) {
        zenzaiProfile = zenzaiVersionConfig.v3().profile();
    }

    auto enableRichSuggestion = currentProfile.use_rich_suggestion() ? Qt::Checked : Qt::CheckState::Unchecked;
    ui_->richSuggestionCheckBox->setCheckState(enableRichSuggestion);

    auto useZenzaiCustomModel = currentProfile.use_zenzai_custom_weight() ? Qt::Checked : Qt::CheckState::Unchecked;
    ui_->useCustomZenzaiModel->setCheckState(useZenzaiCustomModel);

    auto zenzaiCustomModelPath = currentProfile.zenzai_weight_path();
    ui_->customZenzaiModelPath->setText(QString::fromStdString(zenzaiCustomModelPath));
}

MainWindow::~MainWindow() { delete ui_; }
