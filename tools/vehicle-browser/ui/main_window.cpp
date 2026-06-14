#include "ui/main_window.hpp"

#include <QDialog>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QSplitter>
#include <QStatusBar>
#include <QTableWidget>
#include <QToolBar>
#include <QVBoxLayout>

#include "ui/train_builder_panel.hpp"
#include "ui/uid_legend_panel.hpp"
#include "ui/vehicle_edit_dialog.hpp"
#include "ui/vehicle_panel.hpp"
#include "ui/vehicle_type_panel.hpp"

namespace symulator::tools::vehicle_browser
{

MainWindow::MainWindow(std::optional<std::filesystem::path> data_directory, QWidget* parent)
    : QMainWindow(parent)
    , validator_(registry_)
    , generator_(validator_)
{
    buildUi();
    if (data_directory.has_value())
    {
        loadDirectory(*data_directory);
    }
    else
    {
        loadPackages();
    }
}

bool MainWindow::runSmokeTest()
{
    auto* legend = new UidLegendPanel();
    const bool legend_ready = legend->findChild<QTableWidget*>() != nullptr;
    delete legend;

    if (data_.vehicle_types.empty())
    {
        return legend_ready;
    }

    const VehicleType& type = data_.vehicle_types.front();
    const UID first_candidate =
        make_uid(UIDDomain::ROLLING_STOCK, UIDKind::VEHICLE, uid_scope(type.uid), 1);
    VehicleEditDialog dialog(type, generator_, validator_, registry_);
    const UID proposed = dialog.proposedUid();
    const bool collision_was_avoided =
        validator_.isAvailable(proposed) &&
        (!registry_.contains(first_candidate) || proposed != first_candidate);
    dialog.accept();
    return legend_ready && vehicle_type_model_.rowCount() > 0 &&
           vehicle_model_.rowCount() > 0 && collision_was_avoided;
}

void MainWindow::openDirectory()
{
    const QString directory =
        QFileDialog::getExistingDirectory(this, tr("Otwórz katalog z danymi JSON"));
    if (!directory.isEmpty())
    {
        loadDirectory(directory.toStdWString());
    }
}

void MainWindow::showUidLegend()
{
    auto* dialog = new QDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle(tr("Legenda UID"));
    dialog->resize(900, 600);
    auto* layout = new QVBoxLayout(dialog);
    layout->addWidget(new UidLegendPanel(dialog));
    dialog->show();
}

void MainWindow::addSelectedVehicleToTrain(qulonglong vehicle_uid)
{
    const Vehicle* vehicle = findVehicle(UID{static_cast<std::uint64_t>(vehicle_uid)});
    if (vehicle != nullptr)
    {
        train_model_.appendVehicle(*vehicle);
    }
}

void MainWindow::createVehicle()
{
    const VehicleType* type = vehicle_type_panel_->selectedVehicleType();
    if (type == nullptr)
    {
        QMessageBox::information(this, tr("Wybierz typ pojazdu"),
                                 tr("Najpierw wybierz typ pojazdu."));
        return;
    }

    VehicleEditDialog dialog(*type, generator_, validator_, registry_,
                             active_source_root_ / "vehicles", this);
    if (dialog.exec() == QDialog::Accepted)
    {
        reloadActiveSource();
        statusBar()->showMessage(
            tr("Pojazd zapisano i odświeżono dane: %1")
                .arg(QString::fromStdWString(dialog.savedFile().wstring())));
    }
}

void MainWindow::editVehicle(qulonglong vehicle_uid)
{
    const Vehicle* vehicle = findVehicle(UID{static_cast<std::uint64_t>(vehicle_uid)});
    if (vehicle == nullptr)
    {
        QMessageBox::warning(this, tr("Nie znaleziono pojazdu"),
                             tr("Wybrany pojazd nie jest już dostępny w aktywnych danych."));
        return;
    }

    const VehicleType* type = findVehicleType(vehicle->type_uid);
    if (type == nullptr)
    {
        QMessageBox::warning(this, tr("Brak typu pojazdu"),
                             tr("Nie można edytować pojazdu, ponieważ jego typ nie został "
                                "wczytany."));
        return;
    }

    VehicleEditDialog dialog(*vehicle, *type, generator_, validator_, registry_, this);
    if (dialog.exec() == QDialog::Accepted)
    {
        reloadActiveSource();
        statusBar()->showMessage(
            tr("Zapisano zmiany pojazdu: %1")
                .arg(QString::fromStdWString(dialog.savedFile().wstring())));
    }
}

void MainWindow::buildUi()
{
    setWindowTitle(tr("Edytor taboru i składów"));
    resize(1650, 850);

    auto* file_menu = menuBar()->addMenu(tr("&Plik"));
    file_menu->addAction(tr("Otwórz katalog..."), this, &MainWindow::openDirectory);
    file_menu->addAction(tr("Odśwież dane"), this, &MainWindow::reloadActiveSource);
    file_menu->addSeparator();
    file_menu->addAction(tr("Zakończ"), this, &QWidget::close);

    auto* help_menu = menuBar()->addMenu(tr("P&omoc"));
    help_menu->addAction(tr("Legenda UID"), this, &MainWindow::showUidLegend);

    auto* toolbar = addToolBar(tr("Dane"));
    toolbar->addAction(tr("Otwórz katalog..."), this, &MainWindow::openDirectory);
    toolbar->addAction(tr("Odśwież"), this, &MainWindow::reloadActiveSource);
    toolbar->addAction(tr("Nowy pojazd..."), this, &MainWindow::createVehicle);

    auto* splitter = new QSplitter(Qt::Horizontal, this);
    vehicle_type_panel_ = new VehicleTypePanel(vehicle_type_model_, splitter);
    vehicle_panel_ = new VehiclePanel(vehicle_model_, splitter);
    train_builder_panel_ =
        new TrainBuilderPanel(train_model_, generator_, registry_, splitter);
    splitter->addWidget(vehicle_type_panel_);
    splitter->addWidget(vehicle_panel_);
    splitter->addWidget(train_builder_panel_);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 2);
    splitter->setStretchFactor(2, 2);
    setCentralWidget(splitter);

    connect(vehicle_type_panel_, &VehicleTypePanel::vehicleTypeSelected, vehicle_panel_,
            &VehiclePanel::filterByVehicleType);
    connect(vehicle_type_panel_, &VehicleTypePanel::vehicleTypeSelected, this,
            [this](qulonglong) { vehicle_panel_->setCanCreateVehicle(true); });
    connect(vehicle_type_panel_, &VehicleTypePanel::selectionCleared, vehicle_panel_,
            &VehiclePanel::clearVehicleTypeFilter);
    connect(vehicle_type_panel_, &VehicleTypePanel::selectionCleared, this,
            [this] { vehicle_panel_->setCanCreateVehicle(false); });
    connect(vehicle_panel_, &VehiclePanel::addToTrainRequested, this,
            &MainWindow::addSelectedVehicleToTrain);
    connect(vehicle_panel_, &VehiclePanel::newVehicleRequested, this,
            &MainWindow::createVehicle);
    connect(vehicle_panel_, &VehiclePanel::editVehicleRequested, this,
            &MainWindow::editVehicle);
    connect(train_builder_panel_, &TrainBuilderPanel::trainSaved, this,
            &MainWindow::reloadActiveSource);
}

void MainWindow::applyData(BrowserDataSet data_set)
{
    data_ = std::move(data_set);
    vehicle_type_model_.setVehicleTypes(data_.vehicle_types);
    vehicle_model_.setVehicles(data_.vehicles);
    train_builder_panel_->setAvailableData(data_.trains, data_.vehicles,
                                           active_source_root_ / "trains");
    statusBar()->showMessage(
        tr("Wczytano: %1 typów pojazdów, %2 pojazdów i %3 składów")
            .arg(static_cast<qulonglong>(data_.vehicle_types.size()))
            .arg(static_cast<qulonglong>(data_.vehicles.size()))
            .arg(static_cast<qulonglong>(data_.trains.size())));
}

void MainWindow::loadPackages()
{
    packages_mode_ = true;
    active_source_root_ = std::filesystem::current_path() / "packages";
    try
    {
        applyData(data_controller_.loadPackages(active_source_root_, registry_));
    }
    catch (const std::exception& error)
    {
        statusBar()->showMessage(tr("Nie wczytano domyślnych pakietów: %1")
                                     .arg(QString::fromUtf8(error.what())));
    }
}

void MainWindow::loadDirectory(const std::filesystem::path& directory)
{
    packages_mode_ = false;
    active_source_root_ = directory;
    try
    {
        applyData(data_controller_.loadDirectory(directory, registry_));
    }
    catch (const std::exception& error)
    {
        QMessageBox::critical(this, tr("Nie można wczytać danych"),
                              QString::fromUtf8(error.what()));
    }
}

void MainWindow::reloadActiveSource()
{
    if (packages_mode_)
    {
        loadPackages();
    }
    else
    {
        loadDirectory(active_source_root_);
    }
}

const Vehicle* MainWindow::findVehicle(UID uid) const
{
    for (const auto& vehicle : data_.vehicles)
    {
        if (vehicle.uid == uid)
        {
            return &vehicle;
        }
    }
    return nullptr;
}

const VehicleType* MainWindow::findVehicleType(UID uid) const
{
    for (const auto& type : data_.vehicle_types)
    {
        if (type.uid == uid)
        {
            return &type;
        }
    }
    return nullptr;
}

}  // namespace symulator::tools::vehicle_browser
