#include "mainwindow.h"

#include <QApplication>
#include <QDebug>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QPushButton>
#include <QStyle>
#include <QTabBar>
#include <QTimer>

#include "ui_mainwindow.h"

using hazkey::settings::AboutTabController;
using hazkey::settings::AiTabController;
using hazkey::settings::ConversionTabController;
using hazkey::settings::DictionaryTabController;
using hazkey::settings::InputStyleTabController;
using hazkey::settings::TabContext;
using hazkey::settings::UserInterfaceTabController;

MainWindow::MainWindow(QWidget* parent)
    : QWidget(parent),
      ui_(new Ui::MainWindow),
      server_(ServerConnector()),
      currentProfile_(nullptr),
      networkManager_(std::make_unique<QNetworkAccessManager>(this)) {
    ui_->setupUi(this);

    ui_->inputTableConfigModeTabWidget->tabBar()->setExpanding(true);

    QPushButton* reloadButton =
        ui_->dialogButtonBox->button(QDialogButtonBox::Reset);
    if (reloadButton) {
        reloadButton->setText(tr("Reload"));
        QIcon reloadIcon =
            QApplication::style()->standardIcon(QStyle::SP_BrowserReload);
        reloadButton->setIcon(reloadIcon);
    }

    setupControllers();
    aboutTab_->initialize();
    connectSignals();

    if (!loadCurrentConfig()) {
        setEnabled(false);
        QMessageBox::critical(
            this, tr("Configuration Error"),
            tr("Failed to load configuration. Please check your connection to "
               "the hazkey server."));
    }
}

MainWindow::~MainWindow() { delete ui_; }

void MainWindow::setupControllers() {
    controllerContext_ = TabContext{&currentConfig_, currentProfile_, &server_};

    uiTab_ = std::make_unique<UserInterfaceTabController>(ui_, this);
    conversionTab_ =
        std::make_unique<ConversionTabController>(ui_, this, &server_, this);
    inputStyleTab_ = std::make_unique<InputStyleTabController>(ui_, this);
    dictionaryTab_ =
        std::make_unique<DictionaryTabController>(ui_, this, &server_, this);
    aiTab_ = std::make_unique<AiTabController>(ui_, this, networkManager_.get(),
                                               this);
    aboutTab_ = std::make_unique<AboutTabController>(ui_, this);

    conversionTab_->connectSignals();
    inputStyleTab_->connectSignals();
    dictionaryTab_->connectSignals();
    aiTab_->connectSignals();
}

void MainWindow::bindContexts() {
    controllerContext_.currentConfig = &currentConfig_;
    controllerContext_.currentProfile = currentProfile_;
    controllerContext_.server = &server_;

    uiTab_->setContext(controllerContext_);
    conversionTab_->setContext(controllerContext_);
    inputStyleTab_->setContext(controllerContext_);
    dictionaryTab_->setContext(controllerContext_);
    aiTab_->setContext(controllerContext_);
}

void MainWindow::connectSignals() {
    connect(ui_->dialogButtonBox, &QDialogButtonBox::accepted, this,
            &MainWindow::onApply);
    connect(ui_->dialogButtonBox, &QDialogButtonBox::clicked, this,
            &MainWindow::onButtonClicked);

    QPushButton* resetButton =
        ui_->dialogButtonBox->button(QDialogButtonBox::Reset);
    if (resetButton) {
        connect(resetButton, &QPushButton::clicked, this,
                &MainWindow::onResetConfiguration);
    }
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

bool MainWindow::loadCurrentConfig(bool fetchConfig) {
    if (fetchConfig) {
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
    }

    bindContexts();

    uiTab_->loadFromConfig();
    conversionTab_->loadFromConfig();
    inputStyleTab_->loadFromConfig();
    dictionaryTab_->loadFromConfig();
    aiTab_->loadFromConfig();

    return true;
}

bool MainWindow::saveCurrentConfig() {
    if (!currentProfile_) {
        QMessageBox::warning(this, tr("Error"),
                             tr("No configuration profile loaded."));
        return false;
    }

    uiTab_->saveToConfig();
    conversionTab_->saveToConfig();
    inputStyleTab_->saveToConfig();
    dictionaryTab_->saveToConfig();
    aiTab_->saveToConfig();

    try {
        server_.setCurrentConfig(currentConfig_);
        return true;
    } catch (const std::exception& e) {
        QMessageBox::critical(
            this, tr("Save Error"),
            tr("Failed to save configuration: %1").arg(e.what()));
        return false;
    } catch (...) {
        QMessageBox::critical(
            this, tr("Save Error"),
            tr("An unknown error occurred while saving configuration."));
        return false;
    }
}

void MainWindow::onResetConfiguration() {
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, tr("Reload Configuration"),
        tr("Reloading will discard any unsaved changes. Continue?"),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::No) {
        return;
    }

    if (!server_.beginSession()) {
        QMessageBox::critical(this, tr("Connection Error"),
                              tr("Failed to connect to server."));
        return;
    }

    bool reloadSuccess = server_.reloadZenzaiModelInSession();
    if (!reloadSuccess) {
        qWarning() << "Failed to reload Zenzai model";
    }

    auto configOpt = server_.getConfigInSession();
    server_.endSession();

    if (!configOpt.has_value()) {
        QMessageBox::critical(this, tr("Configuration Error"),
                              tr("Failed to load configuration from server."));
        return;
    }

    currentConfig_ = configOpt.value();
    if (currentConfig_.profiles_size() == 0) {
        QMessageBox::critical(this, tr("Configuration Error"),
                              tr("No profile found in configuration."));
        return;
    }

    currentProfile_ = currentConfig_.mutable_profiles(0);
    if (!currentProfile_) {
        QMessageBox::critical(this, tr("Configuration Error"),
                              tr("Failed to access profile."));
        return;
    }

    if (!loadCurrentConfig(false)) {
        QMessageBox::critical(this, tr("Configuration Error"),
                              tr("Failed to update UI."));
        return;
    }

    QTimer::singleShot(0, this, [this]() {
        QMessageBox::information(
            this, tr("Reload Complete"),
            tr("Configuration has been reloaded successfully."));
    });
}