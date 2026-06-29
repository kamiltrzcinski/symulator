#include "data/directory_data_source.hpp"

#include <algorithm>
#include <system_error>
#include <utility>
#include <vector>

#include "data/json_loader.hpp"

namespace symulator::tools
{

namespace
{

void ensureDirectory(const std::filesystem::path& root)
{
    std::error_code error;
    const bool exists = std::filesystem::exists(root, error);
    if (error || !exists)
    {
        throw DataSourceError("Data source directory does not exist: " + root.string());
    }

    error.clear();
    const bool is_directory = std::filesystem::is_directory(root, error);
    if (error || !is_directory)
    {
        throw DataSourceError("Data source path is not a directory: " + root.string());
    }
}

[[nodiscard]] std::vector<std::filesystem::path> collectJsonFiles(
    const std::filesystem::path& root)
{
    ensureDirectory(root);

    std::vector<std::filesystem::path> files;
    std::error_code error;
    std::filesystem::recursive_directory_iterator iterator(
        root, std::filesystem::directory_options::skip_permission_denied, error);
    const std::filesystem::recursive_directory_iterator end;

    while (iterator != end)
    {
        if (!error && iterator->is_regular_file(error) &&
            iterator->path().extension() == ".json")
        {
            files.push_back(iterator->path());
        }

        error.clear();
        iterator.increment(error);
    }

    std::sort(files.begin(), files.end());
    return files;
}

template<typename Record, typename Loader>
[[nodiscard]] std::vector<Record> loadRecognizedFiles(const std::filesystem::path& root,
                                                      JsonDocumentType expected_type,
                                                      Loader loader)
{
    std::vector<Record> records;
    for (const auto& path : collectJsonFiles(root))
    {
        try
        {
            if (JsonLoader::detectDocumentType(path) == expected_type)
            {
                records.push_back(loader(path));
            }
        }
        catch (const JsonLoadError&)
        {
            // Directory mode intentionally skips unrelated, malformed and unreadable JSON.
        }
    }
    return records;
}

}  // namespace

DirectoryDataSource::DirectoryDataSource(std::filesystem::path root)
    : root_(std::move(root))
{
}

void DirectoryDataSource::validate() const
{
    ensureDirectory(root_);
}

std::vector<VehicleType> DirectoryDataSource::loadVehicleTypes() const
{
    return loadRecognizedFiles<VehicleType>(root_, JsonDocumentType::VEHICLE_TYPE,
                                            JsonLoader::loadVehicleType);
}

std::vector<Vehicle> DirectoryDataSource::loadVehicles() const
{
    return loadRecognizedFiles<Vehicle>(root_, JsonDocumentType::VEHICLE,
                                        JsonLoader::loadVehicle);
}

std::vector<Train> DirectoryDataSource::loadTrains() const
{
    return loadRecognizedFiles<Train>(root_, JsonDocumentType::TRAIN, JsonLoader::loadTrain);
}

const std::filesystem::path& DirectoryDataSource::root() const noexcept
{
    return root_;
}

}  // namespace symulator::tools
