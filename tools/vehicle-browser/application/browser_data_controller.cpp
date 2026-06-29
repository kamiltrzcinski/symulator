#include "application/browser_data_controller.hpp"

#include "data/directory_data_source.hpp"
#include "data/i_data_source.hpp"
#include "data/packages_data_source.hpp"
#include "registry/uid_registry.hpp"

namespace symulator::tools::vehicle_browser
{

BrowserDataSet BrowserDataController::loadPackages(const std::filesystem::path& packages_root,
                                                   UidRegistry& registry) const
{
    return loadSource(PackagesDataSource(packages_root), registry);
}

BrowserDataSet BrowserDataController::loadDirectory(const std::filesystem::path& directory,
                                                    UidRegistry& registry) const
{
    return loadSource(DirectoryDataSource(directory), registry);
}

BrowserDataSet BrowserDataController::loadSource(const IDataSource& source, UidRegistry& registry)
{
    source.validate();

    BrowserDataSet data;
    data.vehicle_types = source.loadVehicleTypes();
    data.vehicles = source.loadVehicles();
    data.trains = source.loadTrains();

    UidRegistry replacement;
    for (const auto& type : data.vehicle_types)
    {
        static_cast<void>(replacement.insert(type.uid, type.source_file));
    }
    for (const auto& vehicle : data.vehicles)
    {
        static_cast<void>(replacement.insert(vehicle.uid, vehicle.source_file));
        static_cast<void>(replacement.insert(vehicle.type_uid, vehicle.source_file));
        if (vehicle.carrier_id.has_value())
        {
            static_cast<void>(replacement.insert(*vehicle.carrier_id, vehicle.source_file));
        }
    }
    for (const auto& train : data.trains)
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
    registry = std::move(replacement);
    return data;
}

}  // namespace symulator::tools::vehicle_browser
