#pragma once

#include <QWidget>

#include "domain/fleet_types.hpp"

class QPushButton;
class QTableView;

namespace symulator::tools::vehicle_browser
{

class VehicleModel;

class VehiclePanel final : public QWidget
{
    Q_OBJECT

public:
    explicit VehiclePanel(VehicleModel& model, QWidget* parent = nullptr);

    [[nodiscard]] const Vehicle* selectedVehicle() const;

public slots:
    void filterByVehicleType(qulonglong type_uid);
    void clearVehicleTypeFilter();

signals:
    void addToTrainRequested(qulonglong vehicle_uid);
    void newVehicleRequested();

private slots:
    void addSelectedVehicle();

private:
    VehicleModel& model_;
    QTableView* table_ = nullptr;
    QPushButton* add_button_ = nullptr;
};

}  // namespace symulator::tools::vehicle_browser
