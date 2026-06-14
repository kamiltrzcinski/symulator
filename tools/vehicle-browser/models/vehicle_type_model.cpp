#include "models/vehicle_type_model.hpp"

namespace symulator::tools::vehicle_browser
{

VehicleTypeModel::VehicleTypeModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

int VehicleTypeModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(vehicle_types_.size());
}

int VehicleTypeModel::columnCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : COLUMN_COUNT;
}

QVariant VehicleTypeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 ||
        index.row() >= static_cast<int>(vehicle_types_.size()))
    {
        return {};
    }

    const auto& type = vehicle_types_[static_cast<std::size_t>(index.row())];
    if (role == Qt::UserRole)
    {
        return QVariant::fromValue<qulonglong>(type.uid.value);
    }
    if (role != Qt::DisplayRole && role != Qt::EditRole)
    {
        return {};
    }

    switch (index.column())
    {
    case UID_COLUMN:
        return QVariant::fromValue<qulonglong>(type.uid.value);
    case NAME_COLUMN:
        return QString::fromStdString(type.type_name);
    case FAMILY_COLUMN:
        return type.family.has_value() ? QString::fromStdString(*type.family) : QString();
    case TYPE_COLUMN:
        return QString::fromStdString(type.vehicle_type);
    case SUBTYPE_COLUMN:
        return type.vehicle_subtype.has_value() ? QString::fromStdString(*type.vehicle_subtype)
                                                : QString();
    case MASS_COLUMN:
        return type.mass_gross_t.value_or(type.mass_empty_t);
    case LENGTH_COLUMN:
        return type.length_m;
    default:
        return {};
    }
}

QVariant VehicleTypeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
    {
        return QAbstractTableModel::headerData(section, orientation, role);
    }

    switch (section)
    {
    case UID_COLUMN:
        return tr("UID");
    case NAME_COLUMN:
        return tr("Nazwa");
    case FAMILY_COLUMN:
        return tr("Rodzina / producent");
    case TYPE_COLUMN:
        return tr("Rodzaj pojazdu");
    case SUBTYPE_COLUMN:
        return tr("Trakcja / podtyp");
    case MASS_COLUMN:
        return tr("Masa [t]");
    case LENGTH_COLUMN:
        return tr("Długość [m]");
    default:
        return {};
    }
}

void VehicleTypeModel::setVehicleTypes(std::vector<VehicleType> vehicle_types)
{
    beginResetModel();
    vehicle_types_ = std::move(vehicle_types);
    endResetModel();
}

const VehicleType* VehicleTypeModel::vehicleTypeAt(int row) const noexcept
{
    if (row < 0 || row >= static_cast<int>(vehicle_types_.size()))
    {
        return nullptr;
    }
    return &vehicle_types_[static_cast<std::size_t>(row)];
}

}  // namespace symulator::tools::vehicle_browser
