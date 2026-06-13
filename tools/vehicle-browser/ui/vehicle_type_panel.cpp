#include "ui/vehicle_type_panel.hpp"

#include <QHeaderView>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QVBoxLayout>

#include "models/vehicle_type_model.hpp"

namespace symulator::tools::vehicle_browser
{

VehicleTypePanel::VehicleTypePanel(VehicleTypeModel& model, QWidget* parent)
    : QWidget(parent)
    , model_(model)
{
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(tr("Vehicle types"), this));

    filter_ = new QLineEdit(this);
    filter_->setPlaceholderText(tr("Filter vehicle types..."));
    layout->addWidget(filter_);

    proxy_ = new QSortFilterProxyModel(this);
    proxy_->setSourceModel(&model_);
    proxy_->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxy_->setFilterKeyColumn(-1);

    table_ = new QTableView(this);
    table_->setModel(proxy_);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_->setSortingEnabled(true);
    table_->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(table_);

    connect(filter_, &QLineEdit::textChanged, proxy_,
            &QSortFilterProxyModel::setFilterFixedString);
    connect(table_->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            &VehicleTypePanel::emitSelection);
}

const VehicleType* VehicleTypePanel::selectedVehicleType() const
{
    const QModelIndex proxy_index = table_->currentIndex();
    if (!proxy_index.isValid())
    {
        return nullptr;
    }
    return model_.vehicleTypeAt(proxy_->mapToSource(proxy_index).row());
}

void VehicleTypePanel::emitSelection()
{
    const VehicleType* type = selectedVehicleType();
    if (type == nullptr)
    {
        emit selectionCleared();
        return;
    }
    emit vehicleTypeSelected(type->uid.value);
}

}  // namespace symulator::tools::vehicle_browser
