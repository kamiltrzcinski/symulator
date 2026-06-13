#pragma once

#include <QAbstractTableModel>

#include <optional>
#include <vector>

#include "domain/fleet_types.hpp"

namespace symulator::tools::vehicle_browser
{

class VehicleModel final : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column
    {
        UID_COLUMN,
        TYPE_UID_COLUMN,
        SIDE_NUMBER_COLUMN,
        DISPLAY_NAME_COLUMN,
        CARRIER_COLUMN,
        COLUMN_COUNT,
    };

    explicit VehicleModel(QObject* parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index,
                                int role = Qt::DisplayRole) const override;
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation,
                                      int role = Qt::DisplayRole) const override;

    void setVehicles(std::vector<Vehicle> vehicles);
    void setFilterVehicleType(std::optional<UID> type_uid);
    [[nodiscard]] const Vehicle* vehicleAt(int row) const noexcept;

private:
    void rebuildVisibleRows();

    std::vector<Vehicle> vehicles_;
    std::vector<std::size_t> visible_rows_;
    std::optional<UID> filter_type_uid_;
};

}  // namespace symulator::tools::vehicle_browser
