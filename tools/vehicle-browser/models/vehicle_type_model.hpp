#pragma once

#include <QAbstractTableModel>

#include <vector>

#include "domain/fleet_types.hpp"

namespace symulator::tools::vehicle_browser
{

class VehicleTypeModel final : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column
    {
        UID_COLUMN,
        NAME_COLUMN,
        FAMILY_COLUMN,
        TYPE_COLUMN,
        SUBTYPE_COLUMN,
        MASS_COLUMN,
        LENGTH_COLUMN,
        COLUMN_COUNT,
    };

    explicit VehicleTypeModel(QObject* parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index,
                                int role = Qt::DisplayRole) const override;
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation,
                                      int role = Qt::DisplayRole) const override;

    void setVehicleTypes(std::vector<VehicleType> vehicle_types);
    [[nodiscard]] const VehicleType* vehicleTypeAt(int row) const noexcept;

private:
    std::vector<VehicleType> vehicle_types_;
};

}  // namespace symulator::tools::vehicle_browser
