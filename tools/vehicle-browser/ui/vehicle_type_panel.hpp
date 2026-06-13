#pragma once

#include <QWidget>

#include "domain/fleet_types.hpp"

class QLineEdit;
class QSortFilterProxyModel;
class QTableView;

namespace symulator::tools::vehicle_browser
{

class VehicleTypeModel;

class VehicleTypePanel final : public QWidget
{
    Q_OBJECT

public:
    explicit VehicleTypePanel(VehicleTypeModel& model, QWidget* parent = nullptr);

    [[nodiscard]] const VehicleType* selectedVehicleType() const;

signals:
    void vehicleTypeSelected(qulonglong uid);
    void selectionCleared();

private slots:
    void emitSelection();

private:
    VehicleTypeModel& model_;
    QSortFilterProxyModel* proxy_ = nullptr;
    QLineEdit* filter_ = nullptr;
    QTableView* table_ = nullptr;
};

}  // namespace symulator::tools::vehicle_browser
