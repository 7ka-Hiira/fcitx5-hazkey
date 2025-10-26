#include <QApplication>
#include <QLocale>
#include <QLibraryInfo>
#include <QTranslator>

#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // Load Qt base translations for standard widgets (e.g., file dialogs)
    QTranslator qtTranslator;
    if (qtTranslator.load(QLocale::system(), "qtbase", "_",
                          QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        a.installTranslator(&qtTranslator);
    }

    // Load application translations embedded in resources (:/i18n)
    QTranslator appTranslator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "hazkey-settings_" + QLocale(locale).name();
        if (appTranslator.load(":/i18n/" + baseName)) {
            a.installTranslator(&appTranslator);
            break;
        }
    }

    MainWindow w;
    w.show();
    return a.exec();
}
