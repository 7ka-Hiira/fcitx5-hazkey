#include "userdicttab.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextStream>
#include <QVBoxLayout>

QString UserDictTab::filePath() {
    QString xdg = qEnvironmentVariable("XDG_CONFIG_HOME");
    QString base;
    if (!xdg.isEmpty()) {
        base = xdg + "/hazkey";
    } else {
        base = QDir::homePath() + "/.config/hazkey";
    }
    QDir().mkpath(base);
    return base + "/user_dictionary.tsv";
}

UserDictTab::UserDictTab(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);

    table_ = new QTableWidget(this);
    table_->setColumnCount(3);
    table_->setHorizontalHeaderLabels(
        {tr("Reading"), tr("Word"), tr("Comment")});
    table_->horizontalHeader()->setStretchLastSection(true);
    table_->horizontalHeader()->setSectionResizeMode(
        0, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(
        1, QHeaderView::ResizeToContents);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->verticalHeader()->setVisible(false);
    layout->addWidget(table_);

    auto* buttonRow = new QHBoxLayout();
    addButton_ = new QPushButton(tr("Add"), this);
    editButton_ = new QPushButton(tr("Edit"), this);
    deleteButton_ = new QPushButton(tr("Delete"), this);
    editButton_->setEnabled(false);
    deleteButton_->setEnabled(false);
    buttonRow->addWidget(addButton_);
    buttonRow->addWidget(editButton_);
    buttonRow->addWidget(deleteButton_);
    buttonRow->addStretch();
    layout->addLayout(buttonRow);

    connect(addButton_, &QPushButton::clicked, this, &UserDictTab::onAdd);
    connect(editButton_, &QPushButton::clicked, this, &UserDictTab::onEdit);
    connect(deleteButton_, &QPushButton::clicked, this, &UserDictTab::onDelete);
    connect(table_, &QTableWidget::itemSelectionChanged, this,
            &UserDictTab::onSelectionChanged);

    loadFromDisk();
    refreshTable();
}

void UserDictTab::loadFromDisk() {
    entries_.clear();
    QFile file(filePath());
    if (!file.exists()) return;
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    while (!in.atEnd()) {
        const QString line = in.readLine();
        if (line.isEmpty() || line.startsWith('#')) continue;
        const QStringList cols = line.split('\t');
        if (cols.size() < 2) continue;
        UserDictEntry e;
        e.reading = cols[0].trimmed();
        e.word = cols[1];
        e.comment = cols.size() >= 3 ? cols[2] : QString();
        if (e.reading.isEmpty() || e.word.isEmpty()) continue;
        entries_.append(e);
    }
}

bool UserDictTab::saveToDisk() {
    const QString path = filePath();
    // QSaveFile writes to a temporary file and atomically renames it over the
    // target on commit(), so a crash mid-write never corrupts the dictionary.
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(
            this, tr("User Dictionary"),
            tr("Failed to save user dictionary to %1").arg(path));
        return false;
    }
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << "# reading<TAB>word<TAB>comment\n";
    for (const auto& e : entries_) {
        out << e.reading << '\t' << e.word;
        if (!e.comment.isEmpty()) out << '\t' << e.comment;
        out << '\n';
    }
    if (!file.commit()) {
        QMessageBox::warning(
            this, tr("User Dictionary"),
            tr("Failed to save user dictionary to %1").arg(path));
        return false;
    }
    return true;
}

void UserDictTab::refreshTable() {
    table_->setRowCount(entries_.size());
    for (int i = 0; i < entries_.size(); ++i) {
        table_->setItem(i, 0, new QTableWidgetItem(entries_[i].reading));
        table_->setItem(i, 1, new QTableWidgetItem(entries_[i].word));
        table_->setItem(i, 2, new QTableWidgetItem(entries_[i].comment));
    }
    onSelectionChanged();
}

void UserDictTab::onSelectionChanged() {
    const bool hasSel = !table_->selectedItems().isEmpty();
    editButton_->setEnabled(hasSel);
    deleteButton_->setEnabled(hasSel);
}

bool UserDictTab::editEntryDialog(UserDictEntry& entry, const QString& title) {
    QDialog dialog(this);
    dialog.setWindowTitle(title);
    auto* form = new QFormLayout();
    auto* readingEdit = new QLineEdit(entry.reading, &dialog);
    auto* wordEdit = new QLineEdit(entry.word, &dialog);
    auto* commentEdit = new QLineEdit(entry.comment, &dialog);
    form->addRow(tr("Reading (hiragana)"), readingEdit);
    form->addRow(tr("Word"), wordEdit);
    form->addRow(tr("Comment"), commentEdit);

    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    auto* layout = new QVBoxLayout(&dialog);
    layout->addLayout(form);
    layout->addWidget(buttons);

    if (dialog.exec() != QDialog::Accepted) return false;
    const QString reading = readingEdit->text().trimmed();
    const QString word = wordEdit->text();
    if (reading.isEmpty() || word.isEmpty()) {
        QMessageBox::warning(this, tr("User Dictionary"),
                             tr("Reading and Word must not be empty."));
        return false;
    }
    if (reading.contains('\t') || word.contains('\t') ||
        commentEdit->text().contains('\t') || reading.contains('\n') ||
        word.contains('\n')) {
        QMessageBox::warning(this, tr("User Dictionary"),
                             tr("Tab and newline characters are not allowed."));
        return false;
    }
    entry.reading = reading;
    entry.word = word;
    entry.comment = commentEdit->text();
    return true;
}

void UserDictTab::onAdd() {
    UserDictEntry e;
    if (!editEntryDialog(e, tr("Add Word"))) return;
    entries_.append(e);
    if (!saveToDisk()) {
        entries_.removeLast();
        return;
    }
    refreshTable();
}

void UserDictTab::onEdit() {
    const int row = table_->currentRow();
    if (row < 0 || row >= entries_.size()) return;
    UserDictEntry e = entries_[row];
    if (!editEntryDialog(e, tr("Edit Word"))) return;
    const UserDictEntry old = entries_[row];
    entries_[row] = e;
    if (!saveToDisk()) {
        entries_[row] = old;
        return;
    }
    refreshTable();
    table_->selectRow(row);
}

void UserDictTab::onDelete() {
    const int row = table_->currentRow();
    if (row < 0 || row >= entries_.size()) return;
    if (QMessageBox::question(
            this, tr("User Dictionary"),
            tr("Delete \"%1\" → \"%2\"?")
                .arg(entries_[row].reading, entries_[row].word)) !=
        QMessageBox::Yes) {
        return;
    }
    const UserDictEntry removed = entries_[row];
    entries_.removeAt(row);
    if (!saveToDisk()) {
        entries_.insert(row, removed);
        return;
    }
    refreshTable();
}
