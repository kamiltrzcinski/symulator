#pragma once

#include <filesystem>
#include <vector>

#include "domain/fleet_types.hpp"

namespace symulator::tools
{
class IDataSource;
class UidRegistry;
}

namespace symulator::tools::vehicle_browser
{

struct BrowserDataSet
{
    std::vector<VehicleType> vehicle_types;
    std::vector<Vehicle> vehicles;
    std::vector<Train> trains;
};

class BrowserDataController
{
public:
    [[nodiscard]] BrowserDataSet loadPackages(const std::filesystem::path& packages_root,
                                              UidRegistry& registry) const;
    [[nodiscard]] BrowserDataSet loadDirectory(const std::filesystem::path& directory,
                                               UidRegistry& registry) const;

private:
    [[nodiscard]] static BrowserDataSet loadSource(const IDataSource& source,
                                                   UidRegistry& registry);
};

}  // namespace symulator::tools::vehicle_browser
