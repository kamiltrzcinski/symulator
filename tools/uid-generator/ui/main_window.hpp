#pragma once

#include <QMainWindow>

#include <filesystem>
#include <optional>

#include "registry/uid_registry.hpp"
#include "registry/uid_validator.hpp"
#include "services/clipboard_service.hpp"
#include "services/uid_generator_service.hpp"

class QTabWidget;

namespace symulator::tools
{
class IDataSource;
class UidLegendPanel;
}

namespace symulator::tools::uid_generator
{

class UidForm;
class UidRegistryView;
class UidResultView;

class MainWindow final : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(std::optional<std::filesystem::path> data_directory = std::nullopt,
                        QWidget* parent = nullptr);

    [[nodiscard]] bool runSmokeTest();

private slots:
    void openDirectory();
    void generateUid(int domain, int kind, int scope, int instance);
    void showLegend();

private:
    void buildUi();
    void loadDefaultPackages();
    void loadDirectory(const std::filesystem::path& directory);
    void populateRegistry(const IDataSource& source);

    UidRegistry registry_;
    UidValidator validator_;
    UidGeneratorService generator_;
    UidClipboardService clipboard_;

    QTabWidget* tabs_ = nullptr;
    UidForm* form_ = nullptr;
    UidResultView* result_view_ = nullptr;
    UidRegistryView* registry_view_ = nullptr;
    UidLegendPanel* legend_panel_ = nullptr;
};

}  // namespace symulator::tools::uid_generator
