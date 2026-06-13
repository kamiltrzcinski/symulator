#include "models/train_model.hpp"

#include <algorithm>

namespace symulator::tools::vehicle_browser
{

TrainModel::TrainModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int TrainModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(vehicles_.size());
}

QVariant TrainModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 ||
        index.row() >= static_cast<int>(vehicles_.size()))
    {
        return {};
    }

    const auto& vehicle = vehicles_[static_cast<std::size_t>(index.row())];
    if (role == Qt::UserRole)
    {
        return QVariant::fromValue<qulonglong>(vehicle.uid.value);
    }
    if (role == Qt::DisplayRole)
    {
        return QStringLiteral("%1 - %2")
            .arg(QString::fromStdString(vehicle.pid))
            .arg(QString::fromStdString(vehicle.display_name));
    }
    return {};
}

Qt::ItemFlags TrainModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags result = QAbstractListModel::flags(index);
    if (index.isValid())
    {
        result |= Qt::ItemIsDragEnabled;
    }
    result |= Qt::ItemIsDropEnabled;
    return result;
}

Qt::DropActions TrainModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

bool TrainModel::moveRows(const QModelIndex& source_parent, int source_row, int count,
                          const QModelIndex& destination_parent, int destination_child)
{
    if (source_parent.isValid() || destination_parent.isValid() || count != 1 ||
        source_row < 0 || source_row >= static_cast<int>(vehicles_.size()) ||
        destination_child < 0 || destination_child > static_cast<int>(vehicles_.size()) ||
        destination_child == source_row || destination_child == source_row + 1)
    {
        return false;
    }

    if (!beginMoveRows(source_parent, source_row, source_row, destination_parent,
                       destination_child))
    {
        return false;
    }

    Vehicle moved = std::move(vehicles_[static_cast<std::size_t>(source_row)]);
    vehicles_.erase(vehicles_.begin() + source_row);
    const int adjusted_destination =
        destination_child > source_row ? destination_child - 1 : destination_child;
    vehicles_.insert(vehicles_.begin() + adjusted_destination, std::move(moved));
    endMoveRows();
    return true;
}

void TrainModel::appendVehicle(const Vehicle& vehicle)
{
    const int row = static_cast<int>(vehicles_.size());
    beginInsertRows({}, row, row);
    vehicles_.push_back(vehicle);
    endInsertRows();
}

bool TrainModel::removeVehicle(int row)
{
    if (row < 0 || row >= static_cast<int>(vehicles_.size()))
    {
        return false;
    }
    beginRemoveRows({}, row, row);
    vehicles_.erase(vehicles_.begin() + row);
    endRemoveRows();
    return true;
}

void TrainModel::clear()
{
    beginResetModel();
    vehicles_.clear();
    endResetModel();
}

const std::vector<Vehicle>& TrainModel::vehicles() const noexcept
{
    return vehicles_;
}

}  // namespace symulator::tools::vehicle_browser
