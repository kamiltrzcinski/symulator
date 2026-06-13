#include "data/packages_data_source.hpp"

#include <algorithm>
#include <system_error>
#include <utility>
#include <vector>

#include "data/json_loader.hpp"

namespace symulator::tools
{

namespace
{

[[nodiscard]] std::vector<std::filesystem::path> collectFiles(
    const std::filesystem::path& root, bool vehicle_files_only)
{
    if (!std::filesystem::exists(root))
    {
        return {};
    }

    std::vector<std::filesystem::path> files;
    std::error_code error;
    std::filesystem::recursive_directory_iterator iterator(
        root, std::filesystem::directory_options::skip_permission_denied, error);
    const std::filesystem::recursive_directory_iterator end;

    while (iterator != end)
    {
        if (!error && iterator->is_regular_file(error))
        {
            const auto& path = iterator->path();
            const bool matches = vehicle_files_only ? path.filename() == "vehicle.json"
                                                    : path.extension() == ".json";
            if (matches)
            {
                files.push_back(path);
            }
        }

        error.clear();
        iterator.increment(error);
    }

    std::sort(files.begin(), files.end());
    return files;
}

template<typename Record, typename Loader>
[[nodiscard]] std::vector<Record> loadFiles(const std::vector<std::filesystem::path>& files,
                                            Loader loader)
{
    std::vector<Record> records;
    records.reserve(files.size());
    for (const auto& path : files)
    {
        try
        {
            records.push_back(loader(path));
        }
        catch (const JsonLoadError&)
        {
            // A single malformed or unreadable package must not hide valid records.
        }
    }
    return records;
}

}  // namespace

PackagesDataSource::PackagesDataSource(std::filesystem::path packages_root)
    : root_(std::move(packages_root))
{
}

std::vector<VehicleType> PackagesDataSource::loadVehicleTypes() const
{
    return loadFiles<VehicleType>(collectFiles(root_ / "vehicle-types", false),
                                  JsonLoader::loadVehicleType);
}

std::vector<Vehicle> PackagesDataSource::loadVehicles() const
{
    return loadFiles<Vehicle>(collectFiles(root_ / "vehicles", true), JsonLoader::loadVehicle);
}

std::vector<Train> PackagesDataSource::loadTrains() const
{
    return loadFiles<Train>(collectFiles(root_ / "trains", false), JsonLoader::loadTrain);
}

const std::filesystem::path& PackagesDataSource::root() const noexcept
{
    return root_;
}

}  // namespace symulator::tools
