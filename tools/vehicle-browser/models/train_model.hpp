#pragma once

#include <QAbstractListModel>

#include <vector>

#include "domain/fleet_types.hpp"

namespace symulator::tools::vehicle_browser
{

class TrainModel final : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit TrainModel(QObject* parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index,
                                int role = Qt::DisplayRole) const override;
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index) const override;
    [[nodiscard]] Qt::DropActions supportedDropActions() const override;
    [[nodiscard]] bool moveRows(const QModelIndex& source_parent, int source_row, int count,
                                const QModelIndex& destination_parent,
                                int destination_child) override;

    void appendVehicle(const Vehicle& vehicle);
    void setVehicles(std::vector<Vehicle> vehicles);
    [[nodiscard]] bool removeVehicle(int row);
    void clear();
    [[nodiscard]] const std::vector<Vehicle>& vehicles() const noexcept;

private:
    std::vector<Vehicle> vehicles_;
};

}  // namespace symulator::tools::vehicle_browser
