#pragma once

#include <QWidget>

#include "domain/fleet_types.hpp"

class QPushButton;
class QSortFilterProxyModel;
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
    void setCanCreateVehicle(bool enabled);

signals:
    void addToTrainRequested(qulonglong vehicle_uid);
    void newVehicleRequested();
    void editVehicleRequested(qulonglong vehicle_uid);

private slots:
    void addSelectedVehicle();
    void editSelectedVehicle();
    void updateActionState();

private:
    VehicleModel& model_;
    QSortFilterProxyModel* proxy_ = nullptr;
    QTableView* table_ = nullptr;
    QPushButton* new_button_ = nullptr;
    QPushButton* edit_button_ = nullptr;
    QPushButton* add_button_ = nullptr;
};

}  // namespace symulator::tools::vehicle_browser
