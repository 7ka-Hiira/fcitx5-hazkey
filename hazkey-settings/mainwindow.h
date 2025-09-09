#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAbstractButton>
#include <QWidget>

#include "serverconnector.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QWidget {
    Q_OBJECT

   public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

   private slots:
    void onButtonClicked(QAbstractButton *button);
    void onApply();
    void onUseHistoryToggled(bool enabled);
    void onEnableTable();
    void onDisableTable();
    void onTableMoveUp();
    void onTableMoveDown();
    void onEnabledTableSelectionChanged();
    void onAvailableTableSelectionChanged();
    void onEnableKeymap();
    void onDisableKeymap();
    void onKeymapMoveUp();
    void onKeymapMoveDown();
    void onEnabledKeymapSelectionChanged();
    void onAvailableKeymapSelectionChanged();

   private:
    void connectSignals();
    bool loadCurrentConfig();
    bool saveCurrentConfig();
    void setupInputTableLists();
    void loadInputTables();
    void saveInputTables();
    void updateTableButtonStates();
    void setupKeymapLists();
    void loadKeymaps();
    void saveKeymaps();
    void updateKeymapButtonStates();
    Ui::MainWindow *ui_;
    ServerConnector server_;
    hazkey::config::CurrentConfig currentConfig_;
    hazkey::config::Profile *currentProfile_;
};
#endif  // MAINWINDOW_H
