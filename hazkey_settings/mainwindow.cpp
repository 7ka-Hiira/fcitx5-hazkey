#include "mainwindow.h"
#include <QMenu>

#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // Manage settings button
    QMenu *manageSettingsMenu = new QMenu(this);
    QAction *importAction = manageSettingsMenu->addAction("Import");
    QAction *exportAction = manageSettingsMenu->addAction("Export");
    QAction *restoreDefaultAction = manageSettingsMenu->addAction("Restore Default");
    QPushButton* manageSettingsButton = new QPushButton("Manage Settings");
    manageSettingsButton->setMenu(manageSettingsMenu);
    ui->dialogButtonBox->addButton(manageSettingsButton, QDialogButtonBox::ResetRole);

    // Advanced layout list action button
    QMenu *manageTableMenu = new QMenu(this);
    QAction *importTableAction = manageTableMenu->addAction("Import selected table");
    QAction *exportTableAction = manageTableMenu->addAction("Export selected table");
    ui->tableMoreActions->setMenu(manageTableMenu);

    // Expand table settings mode change tab
    ui->inputTableConfigModeTabWidget->tabBar()->setExpanding(true);

    // Expand table editor
    ui->keymapEditorTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

MainWindow::~MainWindow() { delete ui; }
