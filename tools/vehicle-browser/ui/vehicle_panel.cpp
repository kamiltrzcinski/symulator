#include "ui/vehicle_panel.hpp"

#include <QHeaderView>
#include <QHBoxLayout>
#include <QItemSelectionModel>
#include <QLabel>
#include <QPushButton>
#include <QSortFilterProxyModel>
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
    layout->addWidget(new QLabel(tr("Pojazdy"), this));

    proxy_ = new QSortFilterProxyModel(this);
    proxy_->setSourceModel(&model_);
    proxy_->setSortCaseSensitivity(Qt::CaseInsensitive);

    table_ = new QTableView(this);
    table_->setModel(proxy_);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_->setSortingEnabled(true);
    table_->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(table_);

    auto* button_layout = new QHBoxLayout();
    new_button_ = new QPushButton(tr("Nowy pojazd..."), this);
    edit_button_ = new QPushButton(tr("Edytuj pojazd..."), this);
    add_button_ = new QPushButton(tr("Dodaj do składu"), this);
    button_layout->addWidget(new_button_);
    button_layout->addWidget(edit_button_);
    button_layout->addWidget(add_button_);
    layout->addLayout(button_layout);

    connect(new_button_, &QPushButton::clicked, this, &VehiclePanel::newVehicleRequested);
    connect(edit_button_, &QPushButton::clicked, this, &VehiclePanel::editSelectedVehicle);
    connect(add_button_, &QPushButton::clicked, this, &VehiclePanel::addSelectedVehicle);
    connect(table_, &QTableView::doubleClicked, this, &VehiclePanel::editSelectedVehicle);
    connect(table_->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            &VehiclePanel::updateActionState);
    connect(proxy_, &QAbstractItemModel::modelReset, this,
            &VehiclePanel::updateActionState);
    new_button_->setEnabled(false);
    updateActionState();
}

const Vehicle* VehiclePanel::selectedVehicle() const
{
    const QModelIndex proxy_index = table_->currentIndex();
    if (!proxy_index.isValid())
    {
        return nullptr;
    }
    return model_.vehicleAt(proxy_->mapToSource(proxy_index).row());
}

void VehiclePanel::filterByVehicleType(qulonglong type_uid)
{
    model_.setFilterVehicleType(UID{static_cast<std::uint64_t>(type_uid)});
    updateActionState();
}

void VehiclePanel::clearVehicleTypeFilter()
{
    model_.setFilterVehicleType(std::nullopt);
    updateActionState();
}

void VehiclePanel::setCanCreateVehicle(bool enabled)
{
    new_button_->setEnabled(enabled);
}

void VehiclePanel::addSelectedVehicle()
{
    const Vehicle* vehicle = selectedVehicle();
    if (vehicle != nullptr)
    {
        emit addToTrainRequested(vehicle->uid.value);
    }
}

void VehiclePanel::editSelectedVehicle()
{
    const Vehicle* vehicle = selectedVehicle();
    if (vehicle != nullptr)
    {
        emit editVehicleRequested(vehicle->uid.value);
    }
}

void VehiclePanel::updateActionState()
{
    const bool has_selection = selectedVehicle() != nullptr;
    edit_button_->setEnabled(has_selection);
    add_button_->setEnabled(has_selection);
}

}  // namespace symulator::tools::vehicle_browser
