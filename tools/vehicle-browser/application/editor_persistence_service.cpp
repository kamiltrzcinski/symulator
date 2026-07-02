#include "application/editor_persistence_service.hpp"

#include <fstream>
#include <stdexcept>

#include <nlohmann/json.hpp>

namespace symulator::tools::vehicle_browser
{

namespace
{

using json = nlohmann::json;

[[nodiscard]] json loadBaseDocument(const std::filesystem::path& file, bool preserve_existing)
{
    if (!preserve_existing || !std::filesystem::exists(file))
    {
        return json::object();
    }

    std::ifstream input(file);
    if (!input)
    {
        throw std::runtime_error("Cannot open existing JSON file: " + file.string());
    }

    try
    {
        json document = json::parse(input);
        if (!document.is_object())
        {
            throw std::runtime_error("Existing JSON root must be an object: " + file.string());
        }
        return document;
    }
    catch (const std::runtime_error&)
    {
        throw;
    }
    catch (const std::exception& error)
    {
        throw std::runtime_error("Cannot parse existing JSON file " + file.string() + ": " +
                                 error.what());
    }
}

void ensureParentDirectory(const std::filesystem::path& file)
{
    if (!file.parent_path().empty())
    {
        std::filesystem::create_directories(file.parent_path());
    }
}

void writeDocument(const std::filesystem::path& file, const json& document, const char* label)
{
    ensureParentDirectory(file);
    std::ofstream output(file);
    if (!output)
    {
        throw std::runtime_error(std::string("Cannot create ") + label + " JSON file: " +
                                 file.string());
    }
    output << document.dump(2) << '\n';
}

void ensureNonEmpty(const std::string& value, const char* label)
{
    if (value.empty())
    {
        throw std::invalid_argument(std::string(label) + " is required");
    }
}

void ensurePersistableUid(UID uid, const char* label)
{
    if (uid.value == 0 || !uid_is_safe_json_integer(uid))
    {
        throw std::invalid_argument(std::string(label) +
                                    " must be a non-zero JSON-safe UID");
    }
}

void setOptionalString(json& document, const char* key,
                       const std::optional<std::string>& value)
{
    if (value.has_value() && !value->empty())
    {
        document[key] = *value;
    }
    else
    {
        document.erase(key);
    }
}

void setOptionalUid(json& document, const char* key, std::optional<UID> value)
{
    if (value.has_value())
    {
        ensurePersistableUid(*value, key);
        document[key] = value->value;
    }
    else
    {
        document.erase(key);
    }
}

}  // namespace

std::filesystem::path EditorPersistenceService::saveVehicle(
    const VehicleSaveRequest& request) const
{
    ensurePersistableUid(request.uid, "Vehicle UID");
    ensurePersistableUid(request.type_uid, "Vehicle type UID");
    ensureNonEmpty(request.pid, "Vehicle pID");

    json document = loadBaseDocument(request.file, request.preserve_existing_fields);
    document["uid"] = request.uid.value;
    document["type_uid"] = request.type_uid.value;
    document["pID"] = request.pid;
    document["displayName"] =
        request.display_name.empty() ? request.pid : request.display_name;
    setOptionalUid(document, "carrierId", request.carrier_id);
    setOptionalString(document, "inventoryNumber", request.inventory_number);
    setOptionalString(document, "notes", request.notes);

    writeDocument(request.file, document, "vehicle");
    return request.file;
}

UID EditorPersistenceService::saveTrain(const TrainSaveRequest& request) const
{
    ensurePersistableUid(request.uid, "Train UID");
    ensureNonEmpty(request.pid, "Train pID");
    ensureNonEmpty(request.train_category, "Train category");
    if (request.vehicle_uids.empty())
    {
        throw std::runtime_error("Cannot save an empty train consist");
    }

    json document = loadBaseDocument(request.file, request.preserve_existing_fields);
    document["uid"] = request.uid.value;
    document["pID"] = request.pid;
    document["displayName"] =
        request.display_name.empty() ? request.pid : request.display_name;
    document["trainCategory"] = request.train_category;
    setOptionalUid(document, "carrierId", request.carrier_id);

    document["vehicle_uids"] = json::array();
    for (const UID uid : request.vehicle_uids)
    {
        ensurePersistableUid(uid, "Train vehicle UID");
        document["vehicle_uids"].push_back(uid.value);
    }

    writeDocument(request.file, document, "train");
    return request.uid;
}

}  // namespace symulator::tools::vehicle_browser
