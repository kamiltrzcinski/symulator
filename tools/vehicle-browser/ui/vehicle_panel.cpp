#include "ui/vehicle_panel.hpp"

#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableView>
#include <QVBoxLayout>

#include "models/vehicle_model.hpp"

namespace symulator::tools::vehicle_browser
{

VehiclePanel::VehiclePanel(VehicleModel& model, QWidget* parent)
    : QWidget(parent)
    , model_(model)
{
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(tr("Vehicles"), this));

    table_ = new QTableView(this);
    table_->setModel(&model_);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_->setSortingEnabled(true);
    table_->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(table_);

    auto* button_layout = new QHBoxLayout();
    auto* new_button = new QPushButton(tr("New Vehicle..."), this);
    add_button_ = new QPushButton(tr("Add to Train"), this);
    button_layout->addWidget(new_button);
    button_layout->addWidget(add_button_);
    layout->addLayout(button_layout);

    connect(new_button, &QPushButton::clicked, this, &VehiclePanel::newVehicleRequested);
    connect(add_button_, &QPushButton::clicked, this, &VehiclePanel::addSelectedVehicle);
}

const Vehicle* VehiclePanel::selectedVehicle() const
{
    return model_.vehicleAt(table_->currentIndex().row());
}

void VehiclePanel::filterByVehicleType(qulonglong type_uid)
{
    model_.setFilterVehicleType(UID{static_cast<std::uint64_t>(type_uid)});
}

void VehiclePanel::clearVehicleTypeFilter()
{
    model_.setFilterVehicleType(std::nullopt);
}

void VehiclePanel::addSelectedVehicle()
{
    const Vehicle* vehicle = selectedVehicle();
    if (vehicle != nullptr)
    {
        emit addToTrainRequested(vehicle->uid.value);
    }
}

}  // namespace symulator::tools::vehicle_browser
