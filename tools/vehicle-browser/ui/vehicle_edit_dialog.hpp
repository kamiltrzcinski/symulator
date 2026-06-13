#pragma once

#include <QDialog>

#include <filesystem>

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
                      QWidget* parent = nullptr);

    [[nodiscard]] UID proposedUid() const noexcept;
    void setSideNumber(const QString& side_number);
    [[nodiscard]] std::filesystem::path saveToDirectory(
        const std::filesystem::path& directory);

private slots:
    void updateSaveState();
    void chooseDirectoryAndSave();

private:
    const VehicleType& type_;
    const UidGeneratorService& generator_;
    const UidValidator& validator_;
    UidRegistry& registry_;
    UID proposed_uid_{};

    QLineEdit* uid_field_ = nullptr;
    QLineEdit* type_uid_field_ = nullptr;
    QLineEdit* side_number_field_ = nullptr;
    QLineEdit* carrier_uid_field_ = nullptr;
    QLineEdit* inventory_number_field_ = nullptr;
    QPlainTextEdit* notes_field_ = nullptr;
    QDialogButtonBox* buttons_ = nullptr;
};

}  // namespace symulator::tools::vehicle_browser
