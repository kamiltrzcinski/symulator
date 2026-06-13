#pragma once

#include <QMainWindow>

#include <filesystem>
#include <optional>

#include "application/browser_data_controller.hpp"
#include "models/train_model.hpp"
#include "models/vehicle_model.hpp"
#include "models/vehicle_type_model.hpp"
#include "registry/uid_registry.hpp"
#include "registry/uid_validator.hpp"
#include "services/uid_generator_service.hpp"

namespace symulator::tools::vehicle_browser
{

class TrainBuilderPanel;
class VehiclePanel;
class VehicleTypePanel;

class MainWindow final : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(std::optional<std::filesystem::path> data_directory = std::nullopt,
                        QWidget* parent = nullptr);

    [[nodiscard]] bool runSmokeTest();

private slots:
    void openDirectory();
    void showUidLegend();
    void addSelectedVehicleToTrain(qulonglong vehicle_uid);
    void createVehicle();

private:
    void buildUi();
    void applyData(BrowserDataSet data);
    void loadPackages();
    void loadDirectory(const std::filesystem::path& directory);
    [[nodiscard]] const Vehicle* findVehicle(UID uid) const;

    BrowserDataController data_controller_;
    BrowserDataSet data_;
    UidRegistry registry_;
    UidValidator validator_;
    UidGeneratorService generator_;
    VehicleTypeModel vehicle_type_model_;
    VehicleModel vehicle_model_;
    TrainModel train_model_;

    VehicleTypePanel* vehicle_type_panel_ = nullptr;
    VehiclePanel* vehicle_panel_ = nullptr;
    TrainBuilderPanel* train_builder_panel_ = nullptr;
};

}  // namespace symulator::tools::vehicle_browser
