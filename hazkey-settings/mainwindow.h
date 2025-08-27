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

   private:
    void connectSignals();
    bool loadCurrentConfig();
    bool saveCurrentConfig();
    Ui::MainWindow *ui_;
    ServerConnector server_;
    hazkey::config::CurrentConfig currentConfig_;
    hazkey::config::Profile *currentProfile_;
};
#endif  // MAINWINDOW_H
