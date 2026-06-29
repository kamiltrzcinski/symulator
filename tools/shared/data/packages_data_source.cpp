#include "data/packages_data_source.hpp"

#include <algorithm>
#include <array>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "data/json_loader.hpp"

namespace symulator::tools
{

namespace
{

void ensureDirectory(const std::filesystem::path& path, const char* label)
{
    std::error_code error;
    const bool exists = std::filesystem::exists(path, error);
    if (error || !exists)
    {
        throw DataSourceError(std::string(label) + " does not exist: " + path.string());
    }

    error.clear();
    const bool is_directory = std::filesystem::is_directory(path, error);
    if (error || !is_directory)
    {
        throw DataSourceError(std::string(label) + " is not a directory: " + path.string());
    }
}

void ensurePackageStructure(const std::filesystem::path& root)
{
    ensureDirectory(root, "Packages directory");
    for (const char* directory : std::array{"vehicle-types", "vehicles", "trains"})
    {
        ensureDirectory(root / directory, "Required packages subdirectory");
    }
}

[[nodiscard]] std::vector<std::filesystem::path> collectFiles(
    const std::filesystem::path& root, bool vehicle_files_only)
{
    ensureDirectory(root, "Packages data directory");

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

void PackagesDataSource::validate() const
{
    ensurePackageStructure(root_);
}

std::vector<VehicleType> PackagesDataSource::loadVehicleTypes() const
{
    validate();
    return loadFiles<VehicleType>(collectFiles(root_ / "vehicle-types", false),
                                  JsonLoader::loadVehicleType);
}

std::vector<Vehicle> PackagesDataSource::loadVehicles() const
{
    validate();
    return loadFiles<Vehicle>(collectFiles(root_ / "vehicles", true), JsonLoader::loadVehicle);
}

std::vector<Train> PackagesDataSource::loadTrains() const
{
    validate();
    return loadFiles<Train>(collectFiles(root_ / "trains", false), JsonLoader::loadTrain);
}

const std::filesystem::path& PackagesDataSource::root() const noexcept
{
    return root_;
}

}  // namespace symulator::tools
