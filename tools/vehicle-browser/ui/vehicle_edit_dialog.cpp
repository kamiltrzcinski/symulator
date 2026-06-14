#include "ui/vehicle_edit_dialog.hpp"

#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include <cctype>
#include <fstream>
#include <nlohmann/json.hpp>
#include <utility>

#include "registry/uid_registry.hpp"
#include "registry/uid_validator.hpp"
#include "services/uid_generator_service.hpp"

namespace symulator::tools::vehicle_browser
{

namespace
{

[[nodiscard]] std::string safeDirectoryName(const QString& value)
{
    std::string result = value.trimmed().toLower().toStdString();
    for (char& character : result)
    {
        if (!std::isalnum(static_cast<unsigned char>(character)) && character != '-' &&
            character != '_')
        {
            character = '_';
        }
    }
    return result;
}

}  // namespace

VehicleEditDialog::VehicleEditDialog(const VehicleType& type,
                                     const UidGeneratorService& generator,
                                     const UidValidator& validator, UidRegistry& registry,
                                     std::filesystem::path default_output_directory,
                                     QWidget* parent)
    : QDialog(parent)
    , type_(type)
    , generator_(generator)
    , validator_(validator)
    , registry_(registry)
    , default_output_directory_(std::move(default_output_directory))
{
    proposed_uid_ = generator_.generate(UIDDomain::ROLLING_STOCK, UIDKind::VEHICLE,
                                        uid_scope(type_.uid), 1);
    buildUi();
}

VehicleEditDialog::VehicleEditDialog(const Vehicle& vehicle, const VehicleType& type,
                                     const UidGeneratorService& generator,
                                     const UidValidator& validator, UidRegistry& registry,
                                     QWidget* parent)
    : QDialog(parent)
    , type_(type)
    , generator_(generator)
    , validator_(validator)
    , registry_(registry)
    , original_vehicle_(vehicle)
    , default_output_directory_(vehicle.source_file.parent_path())
    , proposed_uid_(vehicle.uid)
{
    buildUi();
    side_number_field_->setText(QString::fromStdString(vehicle.pid));
    display_name_field_->setText(QString::fromStdString(vehicle.display_name));
    if (vehicle.carrier_id.has_value())
    {
        carrier_uid_field_->setText(
            QString::number(static_cast<qulonglong>(vehicle.carrier_id->value)));
    }
    if (vehicle.inventory_number.has_value())
    {
        inventory_number_field_->setText(QString::fromStdString(*vehicle.inventory_number));
    }
    if (vehicle.notes.has_value())
    {
        notes_field_->setPlainText(QString::fromStdString(*vehicle.notes));
    }
    updateSaveState();
}

void VehicleEditDialog::buildUi()
{
    setWindowTitle(original_vehicle_.has_value() ? tr("Edytuj pojazd") : tr("Nowy pojazd"));

    auto* layout = new QVBoxLayout(this);
    auto* form = new QFormLayout();

    uid_field_ = new QLineEdit(QString::number(static_cast<qulonglong>(proposed_uid_.value)),
                               this);
    uid_field_->setReadOnly(true);
    form->addRow(tr("UID:"), uid_field_);

    type_uid_field_ =
        new QLineEdit(QString::number(static_cast<qulonglong>(type_.uid.value)), this);
    type_uid_field_->setReadOnly(true);
    form->addRow(tr("UID typu pojazdu:"), type_uid_field_);

    side_number_field_ = new QLineEdit(this);
    form->addRow(tr("Numer boczny:"), side_number_field_);

    display_name_field_ = new QLineEdit(this);
    display_name_field_->setPlaceholderText(tr("Domyślnie taki sam jak numer boczny"));
    form->addRow(tr("Nazwa wyświetlana:"), display_name_field_);

    carrier_uid_field_ = new QLineEdit(this);
    carrier_uid_field_->setPlaceholderText(tr("Opcjonalny numeryczny UID"));
    form->addRow(tr("UID przewoźnika:"), carrier_uid_field_);

    inventory_number_field_ = new QLineEdit(this);
    form->addRow(tr("Numer inwentarzowy:"), inventory_number_field_);

    notes_field_ = new QPlainTextEdit(this);
    form->addRow(tr("Uwagi:"), notes_field_);
    layout->addLayout(form);

    buttons_ = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
    buttons_->button(QDialogButtonBox::Save)->setText(tr("Zapisz"));
    buttons_->button(QDialogButtonBox::Cancel)->setText(tr("Anuluj"));
    layout->addWidget(buttons_);

    connect(side_number_field_, &QLineEdit::textChanged, this,
            &VehicleEditDialog::updateSaveState);
    connect(display_name_field_, &QLineEdit::textChanged, this,
            &VehicleEditDialog::updateSaveState);
    connect(buttons_->button(QDialogButtonBox::Save), &QPushButton::clicked, this,
            &VehicleEditDialog::save);
    connect(buttons_, &QDialogButtonBox::rejected, this, &QDialog::reject);
    updateSaveState();
}

UID VehicleEditDialog::proposedUid() const noexcept
{
    return proposed_uid_;
}

void VehicleEditDialog::setSideNumber(const QString& side_number)
{
    side_number_field_->setText(side_number);
}

std::filesystem::path VehicleEditDialog::saveToDirectory(
    const std::filesystem::path& directory)
{
    const std::filesystem::path output_directory =
        directory / safeDirectoryName(side_number_field_->text());
    return saveToFile(output_directory / "vehicle.json");
}

std::filesystem::path VehicleEditDialog::saveToFile(const std::filesystem::path& file)
{
    const QString side_number = side_number_field_->text().trimmed();
    if (side_number.isEmpty())
    {
        throw std::invalid_argument("Numer boczny jest wymagany");
    }
    if (!proposedUidIsAvailable())
    {
        proposed_uid_ = generator_.generate(UIDDomain::ROLLING_STOCK, UIDKind::VEHICLE,
                                             uid_scope(type_.uid),
                                             uid_instance(proposed_uid_));
        uid_field_->setText(
            QString::number(static_cast<qulonglong>(proposed_uid_.value)));
    }

    const QString display_name = display_name_field_->text().trimmed().isEmpty()
                                     ? side_number
                                     : display_name_field_->text().trimmed();
    nlohmann::json document = nlohmann::json::object();
    if (original_vehicle_.has_value() &&
        std::filesystem::exists(original_vehicle_->source_file))
    {
        std::ifstream input(original_vehicle_->source_file);
        if (input)
        {
            input >> document;
        }
    }
    document["uid"] = proposed_uid_.value;
    document["type_uid"] = type_.uid.value;
    document["pID"] = side_number.toStdString();
    document["displayName"] = display_name.toStdString();

    if (!carrier_uid_field_->text().trimmed().isEmpty())
    {
        bool valid = false;
        const qulonglong carrier = carrier_uid_field_->text().toULongLong(&valid);
        if (!valid)
        {
            throw std::invalid_argument("UID przewoźnika musi być liczbą");
        }
        document["carrierId"] = carrier;
    }
    else
    {
        document.erase("carrierId");
    }
    if (!inventory_number_field_->text().trimmed().isEmpty())
    {
        document["inventoryNumber"] =
            inventory_number_field_->text().trimmed().toStdString();
    }
    else
    {
        document.erase("inventoryNumber");
    }
    if (!notes_field_->toPlainText().trimmed().isEmpty())
    {
        document["notes"] = notes_field_->toPlainText().trimmed().toStdString();
    }
    else
    {
        document.erase("notes");
    }

    if (!file.parent_path().empty())
    {
        std::filesystem::create_directories(file.parent_path());
    }
    std::ofstream output(file);
    if (!output)
    {
        throw std::runtime_error("Nie można utworzyć pliku JSON pojazdu: " +
                                 file.string());
    }
    output << document.dump(2) << '\n';
    if (!original_vehicle_.has_value())
    {
        static_cast<void>(registry_.insert(proposed_uid_, file));
    }
    saved_file_ = file;
    return file;
}

void VehicleEditDialog::updateSaveState()
{
    if (!proposedUidIsAvailable())
    {
        try
        {
            proposed_uid_ = generator_.generate(
                UIDDomain::ROLLING_STOCK, UIDKind::VEHICLE, uid_scope(type_.uid),
                uid_instance(proposed_uid_));
            uid_field_->setText(
                QString::number(static_cast<qulonglong>(proposed_uid_.value)));
        }
        catch (const UidExhaustedException&)
        {
            buttons_->button(QDialogButtonBox::Save)->setEnabled(false);
            return;
        }
    }

    buttons_->button(QDialogButtonBox::Save)->setEnabled(
        !side_number_field_->text().trimmed().isEmpty());
}

bool VehicleEditDialog::proposedUidIsAvailable() const noexcept
{
    return original_vehicle_.has_value() && original_vehicle_->uid == proposed_uid_
               ? true
               : validator_.isAvailable(proposed_uid_);
}

const std::filesystem::path& VehicleEditDialog::savedFile() const noexcept
{
    return saved_file_;
}

void VehicleEditDialog::save()
{
    std::filesystem::path output_file;
    if (original_vehicle_.has_value())
    {
        output_file = original_vehicle_->source_file;
    }
    else
    {
        const QString directory = QFileDialog::getExistingDirectory(
            this, tr("Wybierz katalog zapisu pojazdu"),
            QString::fromStdWString(default_output_directory_.wstring()));
        if (directory.isEmpty())
        {
            return;
        }
        output_file =
            std::filesystem::path(directory.toStdWString()) /
            safeDirectoryName(side_number_field_->text()) / "vehicle.json";
    }

    try
    {
        const auto file = saveToFile(output_file);
        QMessageBox::information(this, tr("Pojazd zapisany"),
                                 tr("Zapisano %1")
                                     .arg(QString::fromStdWString(file.wstring())));
        accept();
    }
    catch (const std::exception& error)
    {
        QMessageBox::critical(this, tr("Nie można zapisać pojazdu"),
                              QString::fromUtf8(error.what()));
    }
}

}  // namespace symulator::tools::vehicle_browser
