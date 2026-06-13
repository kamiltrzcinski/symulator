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
                                     QWidget* parent)
    : QDialog(parent)
    , type_(type)
    , generator_(generator)
    , validator_(validator)
    , registry_(registry)
{
    setWindowTitle(tr("New Vehicle"));
    proposed_uid_ = generator_.generate(UIDDomain::ROLLING_STOCK, UIDKind::VEHICLE,
                                        uid_scope(type_.uid), 1);

    auto* layout = new QVBoxLayout(this);
    auto* form = new QFormLayout();

    uid_field_ = new QLineEdit(QString::number(static_cast<qulonglong>(proposed_uid_.value)),
                               this);
    uid_field_->setReadOnly(true);
    form->addRow(tr("UID:"), uid_field_);

    type_uid_field_ =
        new QLineEdit(QString::number(static_cast<qulonglong>(type_.uid.value)), this);
    type_uid_field_->setReadOnly(true);
    form->addRow(tr("VehicleType UID:"), type_uid_field_);

    side_number_field_ = new QLineEdit(this);
    form->addRow(tr("Side number:"), side_number_field_);

    carrier_uid_field_ = new QLineEdit(this);
    carrier_uid_field_->setPlaceholderText(tr("Optional numeric UID"));
    form->addRow(tr("Carrier UID:"), carrier_uid_field_);

    inventory_number_field_ = new QLineEdit(this);
    form->addRow(tr("Inventory number:"), inventory_number_field_);

    notes_field_ = new QPlainTextEdit(this);
    form->addRow(tr("Notes:"), notes_field_);
    layout->addLayout(form);

    buttons_ = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttons_);

    connect(side_number_field_, &QLineEdit::textChanged, this,
            &VehicleEditDialog::updateSaveState);
    connect(buttons_->button(QDialogButtonBox::Save), &QPushButton::clicked, this,
            &VehicleEditDialog::chooseDirectoryAndSave);
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
    if (side_number_field_->text().trimmed().isEmpty())
    {
        throw std::invalid_argument("Side number is required");
    }
    if (!validator_.isAvailable(proposed_uid_))
    {
        proposed_uid_ = generator_.generate(UIDDomain::ROLLING_STOCK, UIDKind::VEHICLE,
                                             uid_scope(type_.uid),
                                             uid_instance(proposed_uid_));
        uid_field_->setText(
            QString::number(static_cast<qulonglong>(proposed_uid_.value)));
    }

    nlohmann::json document{
        {"uid", proposed_uid_.value},
        {"type_uid", type_.uid.value},
        {"pID", side_number_field_->text().trimmed().toStdString()},
        {"displayName", side_number_field_->text().trimmed().toStdString()},
    };

    if (!carrier_uid_field_->text().trimmed().isEmpty())
    {
        bool valid = false;
        const qulonglong carrier = carrier_uid_field_->text().toULongLong(&valid);
        if (!valid)
        {
            throw std::invalid_argument("Carrier UID must be a number");
        }
        document["carrierId"] = carrier;
    }
    if (!inventory_number_field_->text().trimmed().isEmpty())
    {
        document["inventoryNumber"] =
            inventory_number_field_->text().trimmed().toStdString();
    }
    if (!notes_field_->toPlainText().trimmed().isEmpty())
    {
        document["notes"] = notes_field_->toPlainText().trimmed().toStdString();
    }

    const std::filesystem::path output_directory =
        directory / safeDirectoryName(side_number_field_->text());
    const std::filesystem::path output_file = output_directory / "vehicle.json";
    std::filesystem::create_directories(output_directory);
    std::ofstream output(output_file);
    if (!output)
    {
        throw std::runtime_error("Cannot create vehicle JSON file: " + output_file.string());
    }
    output << document.dump(2) << '\n';
    static_cast<void>(registry_.insert(proposed_uid_, output_file));
    return output_file;
}

void VehicleEditDialog::updateSaveState()
{
    if (!validator_.isAvailable(proposed_uid_))
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

void VehicleEditDialog::chooseDirectoryAndSave()
{
    const QString directory =
        QFileDialog::getExistingDirectory(this, tr("Select Vehicle output directory"));
    if (directory.isEmpty())
    {
        return;
    }

    try
    {
        const auto file = saveToDirectory(directory.toStdWString());
        QMessageBox::information(this, tr("Vehicle saved"),
                                 tr("Saved %1").arg(QString::fromStdWString(file.wstring())));
        accept();
    }
    catch (const std::exception& error)
    {
        QMessageBox::critical(this, tr("Cannot save Vehicle"),
                              QString::fromUtf8(error.what()));
    }
}

}  // namespace symulator::tools::vehicle_browser
