#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>

#include <filesystem>
#include <optional>

#include "ui/main_window.hpp"

int main(int argc, char* argv[])
{
    QApplication application(argc, argv);
    QApplication::setApplicationName(QStringLiteral("vehicle-browser"));
    QApplication::setApplicationVersion(QStringLiteral("1.0"));

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("Browse Vehicles and compose Train consists."));
    parser.addHelpOption();
    parser.addVersionOption();

    const QCommandLineOption data_directory_option(
        QStringList{QStringLiteral("d"), QStringLiteral("data-dir")},
        QStringLiteral("Read JSON data recursively from <directory>."),
        QStringLiteral("directory"));
    const QCommandLineOption smoke_test_option(
        QStringLiteral("smoke-test"),
        QStringLiteral("Run automated data, legend and collision checks, then exit."));
    parser.addOption(data_directory_option);
    parser.addOption(smoke_test_option);
    parser.process(application);

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
