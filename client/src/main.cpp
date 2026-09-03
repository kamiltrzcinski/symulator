#include <QApplication>
#include <QFile>
#include <QString>
#include <QDebug>

#include "ui/thales/thales_command_processor.hpp"
#include "ui/thales/thales_main_window.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Załadowanie pliku stylów QSS
    QFile styleFile("client/resources/style/thales.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        app.setStyleSheet(QLatin1String(styleFile.readAll()));
        styleFile.close();
    } else {
        qWarning() << "Nie udało się załadować pliku stylów:" << styleFile.fileName();
    }

    // Inicjalizacja procesora poleceń i głównego okna
    ThalesCommandProcessor processor;
    ThalesMainWindow mainWindow(&processor);

    mainWindow.show();

    return app.exec();
}
