#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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

   private:
    void loadCurrentConfig();
    Ui::MainWindow *ui_;
    ServerConnector server_;
    hazkey::config::CurrentConfig currentConfig_;
    hazkey::config::ConfigProfile configProfile_;
};
#endif  // MAINWINDOW_H
