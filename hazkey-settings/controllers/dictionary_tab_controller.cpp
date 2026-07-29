#include "controllers/dictionary_tab_controller.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFile>
#include <QFileDialog>
#include <QFormLayout>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QStringList>
#include <QTableView>
#include <QTextStream>
#include <QUuid>
#include <algorithm>
#include <array>
#include <functional>

#include "config_definitions.h"
#include "config_macros.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

namespace hazkey::settings {

namespace {

struct WordClassInfo {
    hazkey::config::UserDictionaryEntry::WordClass value;
    const char* token;
    const char* label;
};

// Stable, locale-independent tokens are used for the TSV import/export
// format so files stay portable regardless of the UI language; `label` is
// the translatable string shown in the dialog and the entry list.
const std::array<WordClassInfo, 9>& wordClassTable() {
    static const std::array<WordClassInfo, 9> table{{
        {hazkey::config::UserDictionaryEntry::GENERAL_NOUN, "general_noun",
         QT_TR_NOOP("General Noun")},
        {hazkey::config::UserDictionaryEntry::PROPER_NOUN, "proper_noun",
         QT_TR_NOOP("Proper Noun")},
        {hazkey::config::UserDictionaryEntry::PERSON_NAME, "person",
         QT_TR_NOOP("Person Name")},
        {hazkey::config::UserDictionaryEntry::PERSON_FAMILY_NAME,
         "person_family_name", QT_TR_NOOP("Family Name")},
        {hazkey::config::UserDictionaryEntry::PERSON_GIVEN_NAME,
         "person_given_name", QT_TR_NOOP("Given Name")},
        {hazkey::config::UserDictionaryEntry::ORGANIZATION_NAME,
         "organization", QT_TR_NOOP("Organization Name")},
        {hazkey::config::UserDictionaryEntry::PLACE_NAME, "place",
         QT_TR_NOOP("Place Name")},
        {hazkey::config::UserDictionaryEntry::NUMBER, "number",
         QT_TR_NOOP("Number")},
        {hazkey::config::UserDictionaryEntry::SYMBOL, "symbol",
         QT_TR_NOOP("Symbol")},
    }};
    return table;
}

}  // namespace

DictionaryTabController::DictionaryTabController(Ui::MainWindow* ui,
                                                  QWidget* window,
                                                  ServerConnector* server,
                                                  QObject* parent)
    : QObject(parent), ui_(ui), window_(window), server_(server) {
    // setHorizontalHeaderLabels() grows the (initially column-less) model to
    // match, so the column count doesn't need to be set separately.
    model_.setHorizontalHeaderLabels(
        {tr("Reading"), tr("Word"), tr("Class")});
    ui_->userDictViewer->setModel(&model_);
    ui_->userDictViewer->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui_->userDictViewer->setSelectionMode(
        QAbstractItemView::ExtendedSelection);
    ui_->userDictViewer->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui_->userDictViewer->horizontalHeader()->setStretchLastSection(true);
}

void DictionaryTabController::setContext(const TabContext& context) {
    context_ = context;
}

void DictionaryTabController::connectSignals() {
    connect(ui_->userDictNewEntry, &QPushButton::clicked, this,
            &DictionaryTabController::onNewEntry);
    connect(ui_->userDictDeleteEntry, &QPushButton::clicked, this,
            &DictionaryTabController::onDeleteEntry);
    connect(ui_->userDictImport, &QPushButton::clicked, this,
            &DictionaryTabController::onImportEntries);
    connect(ui_->userDictExport, &QPushButton::clicked, this,
            &DictionaryTabController::onExportEntries);
    connect(ui_->userDictViewer, &QTableView::doubleClicked, this,
            &DictionaryTabController::onEntryActivated);
}

void DictionaryTabController::loadFromConfig() {
    if (context_.currentProfile == nullptr) {
        return;
    }

    SET_CHECKBOX(ui_->useUserDict,
                 context_.currentProfile->use_user_dictionary(),
                 ConfigDefs::CheckboxDefaults::USE_USER_DICT);

    reloadEntriesFromServer();
}

void DictionaryTabController::saveToConfig() {
    if (context_.currentProfile == nullptr) {
        return;
    }

    context_.currentProfile->set_use_user_dictionary(
        GET_CHECKBOX_BOOL(ui_->useUserDict));

    if (server_ != nullptr && !server_->setUserDictionary(entries_)) {
        QMessageBox::warning(
            window_, tr("Error"),
            tr("Failed to save the user dictionary. Please check that the "
               "hazkey server is running."));
    }
}

void DictionaryTabController::reloadEntriesFromServer() {
    entries_.clear();
    if (server_ != nullptr) {
        auto result = server_->getUserDictionary();
        if (result.has_value()) {
            const auto& fetched = result->entries();
            entries_.assign(fetched.begin(), fetched.end());
        }
    }
    populateModel();
}

void DictionaryTabController::populateModel() {
    model_.removeRows(0, model_.rowCount());
    for (const auto& entry : entries_) {
        auto* readingItem =
            new QStandardItem(QString::fromStdString(entry.reading()));
        auto* wordItem =
            new QStandardItem(QString::fromStdString(entry.word()));
        auto* classItem =
            new QStandardItem(wordClassDisplayLabel(entry.word_class()));
        readingItem->setEditable(false);
        wordItem->setEditable(false);
        classItem->setEditable(false);
        model_.appendRow({readingItem, wordItem, classItem});
    }
}

void DictionaryTabController::onNewEntry() {
    auto result = showEntryDialog(nullptr);
    if (!result.has_value()) {
        return;
    }
    entries_.push_back(result.value());
    populateModel();
}

void DictionaryTabController::onDeleteEntry() {
    if (ui_->userDictViewer->selectionModel() == nullptr) {
        return;
    }
    QModelIndexList selected =
        ui_->userDictViewer->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        return;
    }

    auto reply = QMessageBox::question(
        window_, tr("Delete Entry"),
        tr("Are you sure you want to delete the selected dictionary "
           "entries?"),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (reply != QMessageBox::Yes) {
        return;
    }

    std::vector<int> rows;
    rows.reserve(static_cast<size_t>(selected.size()));
    for (const auto& index : selected) {
        rows.push_back(index.row());
    }
    std::sort(rows.begin(), rows.end(), std::greater<int>());
    for (int row : rows) {
        if (row >= 0 && row < static_cast<int>(entries_.size())) {
            entries_.erase(entries_.begin() + row);
        }
    }
    populateModel();
}

void DictionaryTabController::onEntryActivated(const QModelIndex& index) {
    if (!index.isValid()) {
        return;
    }
    int row = index.row();
    if (row < 0 || row >= static_cast<int>(entries_.size())) {
        return;
    }

    auto result = showEntryDialog(&entries_[static_cast<size_t>(row)]);
    if (!result.has_value()) {
        return;
    }
    entries_[static_cast<size_t>(row)] = result.value();
    populateModel();
}

void DictionaryTabController::onImportEntries() {
    QString filePath = QFileDialog::getOpenFileName(
        window_, tr("Import User Dictionary"), QString(),
        tr("Text files (*.txt *.tsv);;All files (*)"));
    if (filePath.isEmpty()) {
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(window_, tr("Error"),
                              tr("Failed to open the file."));
        return;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);

    int importedCount = 0;
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.trimmed().isEmpty()) {
            continue;
        }

        QStringList fields = line.split('\t');
        if (fields.size() < 2) {
            continue;
        }

        QString reading = fields.at(0).trimmed();
        QString word = fields.at(1).trimmed();
        if (reading.isEmpty() || word.isEmpty()) {
            continue;
        }

        hazkey::config::UserDictionaryEntry entry;
        entry.set_id(
            QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString());
        entry.set_reading(reading.toStdString());
        entry.set_word(word.toStdString());
        entry.set_word_class(fields.size() >= 3
                                  ? wordClassFromToken(fields.at(2).trimmed())
                                  : hazkey::config::UserDictionaryEntry::GENERAL_NOUN);

        entries_.push_back(entry);
        ++importedCount;
    }
    file.close();

    populateModel();

    QMessageBox::information(
        window_, tr("Import Complete"),
        tr("Imported %1 dictionary entries.").arg(importedCount));
}

void DictionaryTabController::onExportEntries() {
    QString filePath = QFileDialog::getSaveFileName(
        window_, tr("Export User Dictionary"),
        QStringLiteral("user_dictionary.txt"),
        tr("Text files (*.txt *.tsv);;All files (*)"));
    if (filePath.isEmpty()) {
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(window_, tr("Error"),
                              tr("Failed to save the file."));
        return;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    for (const auto& entry : entries_) {
        out << QString::fromStdString(entry.reading()) << '\t'
            << QString::fromStdString(entry.word()) << '\t'
            << wordClassToken(entry.word_class()) << '\n';
    }
    file.close();

    QMessageBox::information(
        window_, tr("Export Complete"),
        tr("The user dictionary has been exported successfully."));
}

std::optional<hazkey::config::UserDictionaryEntry>
DictionaryTabController::showEntryDialog(
    const hazkey::config::UserDictionaryEntry* existing) {
    QDialog dialog(window_);
    dialog.setWindowTitle(existing != nullptr ? tr("Edit Dictionary Entry")
                                               : tr("New Dictionary Entry"));

    auto* layout = new QFormLayout(&dialog);

    auto* readingEdit = new QLineEdit(&dialog);
    auto* wordEdit = new QLineEdit(&dialog);
    auto* classCombo = new QComboBox(&dialog);

    for (const auto& info : wordClassTable()) {
        classCombo->addItem(tr(info.label), static_cast<int>(info.value));
    }

    if (existing != nullptr) {
        readingEdit->setText(QString::fromStdString(existing->reading()));
        wordEdit->setText(QString::fromStdString(existing->word()));
        int index =
            classCombo->findData(static_cast<int>(existing->word_class()));
        if (index >= 0) {
            classCombo->setCurrentIndex(index);
        }
    }

    layout->addRow(tr("Reading:"), readingEdit);
    layout->addRow(tr("Word:"), wordEdit);
    layout->addRow(tr("Class:"), classCombo);

    auto* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout->addRow(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    while (true) {
        if (dialog.exec() != QDialog::Accepted) {
            return std::nullopt;
        }

        QString reading = readingEdit->text().trimmed();
        QString word = wordEdit->text().trimmed();

        if (reading.isEmpty() || word.isEmpty()) {
            QMessageBox::warning(
                &dialog, tr("Invalid Entry"),
                tr("Please fill in both the reading and the word."));
            continue;
        }

        hazkey::config::UserDictionaryEntry entry;
        entry.set_id(existing != nullptr
                         ? existing->id()
                         : QUuid::createUuid()
                               .toString(QUuid::WithoutBraces)
                               .toStdString());
        entry.set_reading(reading.toStdString());
        entry.set_word(word.toStdString());
        entry.set_word_class(
            static_cast<hazkey::config::UserDictionaryEntry::WordClass>(
                classCombo->currentData().toInt()));
        return entry;
    }
}

QString DictionaryTabController::wordClassDisplayLabel(
    hazkey::config::UserDictionaryEntry::WordClass wordClass) {
    for (const auto& info : wordClassTable()) {
        if (info.value == wordClass) {
            return tr(info.label);
        }
    }
    return tr(wordClassTable().front().label);
}

QString DictionaryTabController::wordClassToken(
    hazkey::config::UserDictionaryEntry::WordClass wordClass) {
    for (const auto& info : wordClassTable()) {
        if (info.value == wordClass) {
            return QString::fromLatin1(info.token);
        }
    }
    return QString::fromLatin1(wordClassTable().front().token);
}

hazkey::config::UserDictionaryEntry::WordClass
DictionaryTabController::wordClassFromToken(const QString& token) {
    QString normalized = token.trimmed().toLower();
    for (const auto& info : wordClassTable()) {
        if (normalized == QString::fromLatin1(info.token)) {
            return info.value;
        }
    }
    return hazkey::config::UserDictionaryEntry::GENERAL_NOUN;
}

}  // namespace hazkey::settings