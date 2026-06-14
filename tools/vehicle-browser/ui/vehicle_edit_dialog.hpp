#pragma once

#include <QDialog>

#include <filesystem>
#include <optional>

#include "domain/fleet_types.hpp"

class QDialogButtonBox;
class QLineEdit;
class QPlainTextEdit;

namespace symulator::tools
{
class UidGeneratorService;
class UidRegistry;
class UidValidator;
}

namespace symulator::tools::vehicle_browser
{

class VehicleEditDialog final : public QDialog
{
    Q_OBJECT

public:
    VehicleEditDialog(const VehicleType& type, const UidGeneratorService& generator,
                      const UidValidator& validator, UidRegistry& registry,
                      std::filesystem::path default_output_directory = {},
                      QWidget* parent = nullptr);
    VehicleEditDialog(const Vehicle& vehicle, const VehicleType& type,
                      const UidGeneratorService& generator,
                      const UidValidator& validator, UidRegistry& registry,
                      QWidget* parent = nullptr);

    [[nodiscard]] UID proposedUid() const noexcept;
    void setSideNumber(const QString& side_number);
    [[nodiscard]] std::filesystem::path saveToDirectory(
        const std::filesystem::path& directory);
    [[nodiscard]] std::filesystem::path saveToFile(const std::filesystem::path& file);
    [[nodiscard]] const std::filesystem::path& savedFile() const noexcept;

private slots:
    void updateSaveState();
    void save();

private:
    void buildUi();
    [[nodiscard]] bool proposedUidIsAvailable() const noexcept;

    const VehicleType& type_;
    const UidGeneratorService& generator_;
    const UidValidator& validator_;
    UidRegistry& registry_;
    std::optional<Vehicle> original_vehicle_;
    std::filesystem::path default_output_directory_;
    std::filesystem::path saved_file_;
    UID proposed_uid_{};

    QLineEdit* uid_field_ = nullptr;
    QLineEdit* type_uid_field_ = nullptr;
    QLineEdit* side_number_field_ = nullptr;
    QLineEdit* display_name_field_ = nullptr;
    QLineEdit* carrier_uid_field_ = nullptr;
    QLineEdit* inventory_number_field_ = nullptr;
    QPlainTextEdit* notes_field_ = nullptr;
    QDialogButtonBox* buttons_ = nullptr;
};

}  // namespace symulator::tools::vehicle_browser
