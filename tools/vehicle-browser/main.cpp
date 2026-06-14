#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QLocale>
#include <QStringConverter>
#include <QTextStream>

#include <filesystem>
#include <optional>

#ifdef _WIN32
#include <windows.h>
#endif

#include "ui/main_window.hpp"

int main(int argc, char* argv[])
{
#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif
    QLocale::setDefault(QLocale(QLocale::Polish, QLocale::Poland));

    QApplication application(argc, argv);
    QApplication::setApplicationName(QStringLiteral("vehicle-browser"));
    QApplication::setApplicationVersion(QStringLiteral("1.0"));

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("Edycja pojazdów i składów pociągów."));
    const QCommandLineOption help_option(
        QStringList{QStringLiteral("?"), QStringLiteral("h"), QStringLiteral("help")},
        QStringLiteral("Wyświetl tę pomoc."));
    const QCommandLineOption version_option(
        QStringList{QStringLiteral("v"), QStringLiteral("version")},
        QStringLiteral("Wyświetl wersję programu."));

    const QCommandLineOption data_directory_option(
        QStringList{QStringLiteral("d"), QStringLiteral("data-dir")},
        QStringLiteral("Wczytaj rekurencyjnie dane JSON z <katalogu>."),
        QStringLiteral("katalog"));
    const QCommandLineOption smoke_test_option(
        QStringLiteral("smoke-test"),
        QStringLiteral("Uruchom automatyczne testy danych, legendy i kolizji, a następnie zakończ."));
    parser.addOption(help_option);
    parser.addOption(version_option);
    parser.addOption(data_directory_option);
    parser.addOption(smoke_test_option);
    parser.process(application);

    if (parser.isSet(help_option))
    {
        QTextStream output(stdout);
        output.setEncoding(QStringConverter::Utf8);
        output << "Użycie: vehicle-browser [opcje]\n"
                  "Edycja pojazdów i składów pociągów.\n\n"
                  "Opcje:\n"
                  "  -?, -h, --help            Wyświetl tę pomoc.\n"
                  "  -v, --version             Wyświetl wersję programu.\n"
                  "  -d, --data-dir <katalog>  Wczytaj rekurencyjnie dane JSON z katalogu.\n"
                  "  --smoke-test              Uruchom automatyczne testy i zakończ.\n";
        return 0;
    }
    if (parser.isSet(version_option))
    {
        QTextStream output(stdout);
        output.setEncoding(QStringConverter::Utf8);
        output << QApplication::applicationName() << ' '
               << QApplication::applicationVersion() << '\n';
        return 0;
    }

    std::optional<std::filesystem::path> data_directory;
    if (parser.isSet(data_directory_option))
    {
        data_directory = parser.value(data_directory_option).toStdWString();
    }

    symulator::tools::vehicle_browser::MainWindow window(data_directory);
    if (parser.isSet(smoke_test_option))
    {
        window.show();
        application.processEvents();
        return window.runSmokeTest() ? 0 : 1;
    }

    window.show();
    return application.exec();
}
