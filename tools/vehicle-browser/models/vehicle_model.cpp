#include "models/vehicle_model.hpp"

namespace symulator::tools::vehicle_browser
{

VehicleModel::VehicleModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

int VehicleModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(visible_rows_.size());
}

int VehicleModel::columnCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : COLUMN_COUNT;
}

QVariant VehicleModel::data(const QModelIndex& index, int role) const
{
    const Vehicle* vehicle = vehicleAt(index.row());
    if (!index.isValid() || vehicle == nullptr)
    {
        return {};
    }

    if (role == Qt::UserRole)
    {
        return QVariant::fromValue<qulonglong>(vehicle->uid.value);
    }
    if (role != Qt::DisplayRole && role != Qt::EditRole)
    {
        return {};
    }

    switch (index.column())
    {
    case UID_COLUMN:
        return QVariant::fromValue<qulonglong>(vehicle->uid.value);
    case TYPE_UID_COLUMN:
        return QVariant::fromValue<qulonglong>(vehicle->type_uid.value);
    case SIDE_NUMBER_COLUMN:
        return QString::fromStdString(vehicle->pid);
    case DISPLAY_NAME_COLUMN:
        return QString::fromStdString(vehicle->display_name);
    case CARRIER_COLUMN:
        return vehicle->carrier_id.has_value()
                   ? QVariant::fromValue<qulonglong>(vehicle->carrier_id->value)
                   : QVariant();
    default:
        return {};
    }
}

QVariant VehicleModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
    {
        return QAbstractTableModel::headerData(section, orientation, role);
    }

    switch (section)
    {
    case UID_COLUMN:
        return tr("UID");
    case TYPE_UID_COLUMN:
        return tr("VehicleType UID");
    case SIDE_NUMBER_COLUMN:
        return tr("Side number");
    case DISPLAY_NAME_COLUMN:
        return tr("Display name");
    case CARRIER_COLUMN:
        return tr("Carrier UID");
    default:
        return {};
    }
}

void VehicleModel::setVehicles(std::vector<Vehicle> vehicles)
{
    beginResetModel();
    vehicles_ = std::move(vehicles);
    rebuildVisibleRows();
    endResetModel();
}

void VehicleModel::setFilterVehicleType(std::optional<UID> type_uid)
{
    beginResetModel();
    filter_type_uid_ = type_uid;
    rebuildVisibleRows();
    endResetModel();
}

const Vehicle* VehicleModel::vehicleAt(int row) const noexcept
{
    if (row < 0 || row >= static_cast<int>(visible_rows_.size()))
    {
        return nullptr;
    }
    return &vehicles_[visible_rows_[static_cast<std::size_t>(row)]];
}

void VehicleModel::rebuildVisibleRows()
{
    visible_rows_.clear();
    for (std::size_t index = 0; index < vehicles_.size(); ++index)
    {
        if (!filter_type_uid_.has_value() || vehicles_[index].type_uid == *filter_type_uid_)
        {
            visible_rows_.push_back(index);
        }
    }
}

}  // namespace symulator::tools::vehicle_browser
