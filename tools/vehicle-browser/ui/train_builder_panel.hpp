#pragma once

#include <QWidget>

#include <filesystem>
#include <optional>
#include <vector>

#include "domain/fleet_types.hpp"
#include "domain/uid_types.hpp"

class QComboBox;
class QLabel;
class QLineEdit;
class QListView;
class QPushButton;

namespace symulator::tools
{
class UidGeneratorService;
class UidRegistry;
}

namespace symulator::tools::vehicle_browser
{

class TrainModel;

class TrainBuilderPanel final : public QWidget
{
    Q_OBJECT

public:
    TrainBuilderPanel(TrainModel& model, const UidGeneratorService& generator,
                      UidRegistry& registry, QWidget* parent = nullptr);

    void setAvailableData(std::vector<Train> trains, std::vector<Vehicle> vehicles,
                          std::filesystem::path default_output_directory);

    [[nodiscard]] UID saveTrainTo(
        const std::filesystem::path& file, const QString& pid,
        const QString& display_name, const QString& category,
        std::uint16_t first_instance = 1,
        std::optional<UID> existing_uid = std::nullopt,
        std::optional<UID> carrier_id = std::nullopt);

signals:
    void trainSaved(const QString& file);

private slots:
    void selectTrain(int index);
    void newTrain();
    void removeSelected();
    void saveTrain();
    void saveTrainAs();
    void updateActionState();

private:
    void saveCurrent(bool save_as);
    [[nodiscard]] std::optional<UID> parsedCarrierUid() const;
    [[nodiscard]] const Vehicle* findVehicle(UID uid) const;

    TrainModel& model_;
    const UidGeneratorService& generator_;
    UidRegistry& registry_;
    std::vector<Train> trains_;
    std::vector<Vehicle> vehicles_;
    std::filesystem::path default_output_directory_;
    std::optional<std::size_t> current_train_index_;
    std::size_t missing_vehicle_count_ = 0;

    QComboBox* train_selector_ = nullptr;
    QLineEdit* uid_field_ = nullptr;
    QLineEdit* pid_field_ = nullptr;
    QLineEdit* display_name_field_ = nullptr;
    QComboBox* category_field_ = nullptr;
    QLineEdit* carrier_uid_field_ = nullptr;
    QLabel* source_label_ = nullptr;
    QListView* list_ = nullptr;
    QPushButton* remove_button_ = nullptr;
    QPushButton* save_button_ = nullptr;
    QPushButton* save_as_button_ = nullptr;
};

}  // namespace symulator::tools::vehicle_browser
