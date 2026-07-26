#ifndef HAZKEY_SETTINGS_CONTROLLERS_DICTIONARY_TAB_CONTROLLER_H_
#define HAZKEY_SETTINGS_CONTROLLERS_DICTIONARY_TAB_CONTROLLER_H_

#include <QModelIndex>
#include <QObject>
#include <QStandardItemModel>
#include <QString>
#include <optional>
#include <vector>

#include "config.pb.h"
#include "controllers/tab_context.h"

class QWidget;

namespace Ui {
class MainWindow;
}

namespace hazkey::settings {

class DictionaryTabController : public QObject {
    Q_OBJECT

   public:
    DictionaryTabController(Ui::MainWindow* ui, QWidget* window,
                            ServerConnector* server, QObject* parent);
    void setContext(const TabContext& context);
    void connectSignals();
    void loadFromConfig();
    void saveToConfig();

   private slots:
    void onNewEntry();
    void onDeleteEntry();
    void onEntryActivated(const QModelIndex& index);
    void onImportEntries();
    void onExportEntries();

   private:
    void reloadEntriesFromServer();
    void populateModel();
    std::optional<hazkey::config::UserDictionaryEntry> showEntryDialog(
        const hazkey::config::UserDictionaryEntry* existing);

    static QString wordClassDisplayLabel(
        hazkey::config::UserDictionaryEntry::WordClass wordClass);
    static QString wordClassToken(
        hazkey::config::UserDictionaryEntry::WordClass wordClass);
    static hazkey::config::UserDictionaryEntry::WordClass wordClassFromToken(
        const QString& token);

    Ui::MainWindow* ui_;
    QWidget* window_;
    ServerConnector* server_;
    TabContext context_;
    QStandardItemModel model_;
    std::vector<hazkey::config::UserDictionaryEntry> entries_;
};

}  // namespace hazkey::settings

#endif  // HAZKEY_SETTINGS_CONTROLLERS_DICTIONARY_TAB_CONTROLLER_H_