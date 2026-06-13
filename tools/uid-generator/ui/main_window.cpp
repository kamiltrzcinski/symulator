#include "ui/main_window.hpp"

#include <QFileDialog>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QMessageBox>
#include <QSplitter>
#include <QStatusBar>
#include <QTabWidget>

#include "data/directory_data_source.hpp"
#include "data/i_data_source.hpp"
#include "data/packages_data_source.hpp"
#include "ui/uid_form.hpp"
#include "ui/uid_legend_panel.hpp"
#include "ui/uid_registry_view.hpp"
#include "ui/uid_result_view.hpp"

namespace symulator::tools::uid_generator
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
        loadDefaultPackages();
    }
}

bool MainWindow::runSmokeTest()
{
    if (legend_panel_ == nullptr || !legend_panel_->isVisible())
    {
        showLegend();
    }

    const auto entries = registry_.entries();
    if (entries.empty())
    {
        return legend_panel_ != nullptr;
    }

    const UID occupied = entries.front().uid;
    const UID generated =
        generator_.generate(uid_domain(occupied), uid_kind(occupied), uid_scope(occupied),
                            uid_instance(occupied));
    result_view_->showUid(generated);
    return generated != occupied && validator_.isAvailable(generated) &&
           result_view_->displayedUid() == generated;
}

void MainWindow::openDirectory()
{
    const QString directory =
        QFileDialog::getExistingDirectory(this, tr("Open JSON data directory"));
    if (!directory.isEmpty())
    {
        loadDirectory(directory.toStdWString());
    }
}

void MainWindow::generateUid(int domain, int kind, int scope, int instance)
{
    try
    {
        const UID uid = generator_.generate(
            static_cast<UIDDomain>(domain), static_cast<UIDKind>(kind),
            static_cast<std::uint16_t>(scope), static_cast<std::uint16_t>(instance));
        result_view_->showUid(uid);
        form_->clearError();
    }
    catch (const UidExhaustedException& error)
    {
        form_->showError(QString::fromUtf8(error.what()), true);
    }
    catch (const std::exception& error)
    {
        form_->showError(QString::fromUtf8(error.what()), false);
    }
}

void MainWindow::showLegend()
{
    tabs_->setCurrentWidget(legend_panel_);
}

void MainWindow::buildUi()
{
    setWindowTitle(tr("UID Generator"));
    resize(1050, 700);

    auto* file_menu = menuBar()->addMenu(tr("&File"));
    file_menu->addAction(tr("Open Directory..."), this, &MainWindow::openDirectory);
    file_menu->addSeparator();
    file_menu->addAction(tr("Exit"), this, &QWidget::close);

    auto* help_menu = menuBar()->addMenu(tr("&Help"));
    help_menu->addAction(tr("UID Legend"), this, &MainWindow::showLegend);

    tabs_ = new QTabWidget(this);
    setCentralWidget(tabs_);

    auto* generator_page = new QWidget(tabs_);
    auto* generator_layout = new QHBoxLayout(generator_page);
    form_ = new UidForm(generator_page);
    result_view_ = new UidResultView(clipboard_, generator_page);
    generator_layout->addWidget(form_, 1);
    generator_layout->addWidget(result_view_, 1);
    tabs_->addTab(generator_page, tr("Generator"));

    registry_view_ = new UidRegistryView(tabs_);
    tabs_->addTab(registry_view_, tr("UID Registry"));

    legend_panel_ = new UidLegendPanel(tabs_);
    tabs_->addTab(legend_panel_, tr("UID Legend"));

    connect(form_, &UidForm::generateRequested, this, &MainWindow::generateUid);
}

void MainWindow::loadDefaultPackages()
{
    try
    {
        const PackagesDataSource source;
        populateRegistry(source);
        statusBar()->showMessage(
            tr("Loaded %1 UIDs from packages").arg(static_cast<qulonglong>(registry_.size())));
    }
    catch (const std::exception& error)
    {
        statusBar()->showMessage(tr("No default data loaded: %1").arg(error.what()));
    }
}

void MainWindow::loadDirectory(const std::filesystem::path& directory)
{
    try
    {
        const DirectoryDataSource source(directory);
        populateRegistry(source);
        statusBar()->showMessage(
            tr("Loaded %1 UIDs from %2")
                .arg(static_cast<qulonglong>(registry_.size()))
                .arg(QString::fromStdWString(directory.wstring())));
    }
    catch (const std::exception& error)
    {
        QMessageBox::critical(this, tr("Cannot load data"), QString::fromUtf8(error.what()));
    }
}

void MainWindow::populateRegistry(const IDataSource& source)
{
    UidRegistry replacement;
    for (const auto& type : source.loadVehicleTypes())
    {
        static_cast<void>(replacement.insert(type.uid, type.source_file));
    }
    for (const auto& vehicle : source.loadVehicles())
    {
        static_cast<void>(replacement.insert(vehicle.uid, vehicle.source_file));
        static_cast<void>(replacement.insert(vehicle.type_uid, vehicle.source_file));
        if (vehicle.carrier_id.has_value())
        {
            static_cast<void>(replacement.insert(*vehicle.carrier_id, vehicle.source_file));
        }
    }
    for (const auto& train : source.loadTrains())
    {
        static_cast<void>(replacement.insert(train.uid, train.source_file));
        if (train.carrier_id.has_value())
        {
            static_cast<void>(replacement.insert(*train.carrier_id, train.source_file));
        }
        for (const UID vehicle_uid : train.vehicle_uids)
        {
            static_cast<void>(replacement.insert(vehicle_uid, train.source_file));
        }
    }

    registry_ = std::move(replacement);
    registry_view_->setRegistry(registry_);
}

}  // namespace symulator::tools::uid_generator
