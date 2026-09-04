#ifndef USERDICTTAB_H
#define USERDICTTAB_H

#include <QString>
#include <QVector>
#include <QWidget>

class QTableWidget;
class QPushButton;

struct UserDictEntry {
    QString reading;
    QString word;
    QString comment;
};

/// "User Dictionary" tab. Reads/writes ~/.config/hazkey/user_dictionary.tsv
/// directly. The hazkey-server picks up changes via mtime polling, so no
/// IPC round-trip is required when entries are saved.
class UserDictTab : public QWidget {
    Q_OBJECT
   public:
    explicit UserDictTab(QWidget* parent = nullptr);

   private slots:
    void onAdd();
    void onEdit();
    void onDelete();
    void onSelectionChanged();

   private:
    static QString filePath();
    void loadFromDisk();
    bool saveToDisk();
    void refreshTable();
    bool editEntryDialog(UserDictEntry& entry, const QString& title);

    QTableWidget* table_;
    QPushButton* addButton_;
    QPushButton* editButton_;
    QPushButton* deleteButton_;
    QVector<UserDictEntry> entries_;
};

#endif  // USERDICTTAB_H
