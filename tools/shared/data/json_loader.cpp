#include "data/json_loader.hpp"

#include <cstdint>
#include <fstream>
#include <limits>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace symulator::tools
{

namespace
{

using json = nlohmann::json;

[[nodiscard]] json parseJsonFile(const std::filesystem::path& path)
{
    std::ifstream input(path);
    if (!input)
    {
        throw JsonLoadError("Cannot open JSON file: " + path.string());
    }

    try
    {
        const json document = json::parse(input);
        if (!document.is_object())
        {
            throw JsonLoadError("JSON root must be an object: " + path.string());
        }
        return document;
    }
    catch (const JsonLoadError&)
    {
        throw;
    }
    catch (const std::exception& error)
    {
        throw JsonLoadError("Cannot parse JSON file " + path.string() + ": " + error.what());
    }
}

[[nodiscard]] std::string requireString(const json& document, const char* key,
                                        const std::filesystem::path& path)
{
    if (!document.contains(key) || !document.at(key).is_string())
    {
        throw JsonLoadError("Missing or invalid string field '" + std::string(key) + "' in " +
                            path.string());
    }
    return document.at(key).get<std::string>();
}

[[nodiscard]] std::optional<std::string> optionalString(const json& document, const char* key,
                                                        const std::filesystem::path& path)
{
    if (!document.contains(key) || document.at(key).is_null())
    {
        return std::nullopt;
    }
    if (!document.at(key).is_string())
    {
        throw JsonLoadError("Invalid string field '" + std::string(key) + "' in " +
                            path.string());
    }
    return document.at(key).get<std::string>();
}

[[nodiscard]] double requireDouble(const json& document, const char* key,
                                   const std::filesystem::path& path)
{
    if (!document.contains(key) || !document.at(key).is_number())
    {
        throw JsonLoadError("Missing or invalid numeric field '" + std::string(key) + "' in " +
                            path.string());
    }
    return document.at(key).get<double>();
}

[[nodiscard]] std::optional<double> optionalDouble(const json& document, const char* key,
                                                   const std::filesystem::path& path)
{
    if (!document.contains(key) || document.at(key).is_null())
    {
        return std::nullopt;
    }
    if (!document.at(key).is_number())
    {
        throw JsonLoadError("Invalid numeric field '" + std::string(key) + "' in " +
                            path.string());
    }
    return document.at(key).get<double>();
}

[[nodiscard]] int requireInt(const json& document, const char* key,
                             const std::filesystem::path& path)
{
    if (!document.contains(key) || !document.at(key).is_number_integer())
    {
        throw JsonLoadError("Missing or invalid integer field '" + std::string(key) + "' in " +
                            path.string());
    }

    const auto value = document.at(key).get<std::int64_t>();
    if (value < std::numeric_limits<int>::min() || value > std::numeric_limits<int>::max())
    {
        throw JsonLoadError("Integer field '" + std::string(key) + "' is out of range in " +
                            path.string());
    }
    return static_cast<int>(value);
}

[[nodiscard]] std::optional<int> optionalInt(const json& document, const char* key,
                                             const std::filesystem::path& path)
{
    if (!document.contains(key) || document.at(key).is_null())
    {
        return std::nullopt;
    }
    return requireInt(document, key, path);
}

[[nodiscard]] std::optional<bool> optionalBool(const json& document, const char* key,
                                               const std::filesystem::path& path)
{
    if (!document.contains(key) || document.at(key).is_null())
    {
        return std::nullopt;
    }
    if (!document.at(key).is_boolean())
    {
        throw JsonLoadError("Invalid boolean field '" + std::string(key) + "' in " +
                            path.string());
    }
    return document.at(key).get<bool>();
}

[[nodiscard]] UID requireUid(const json& document, const char* key,
                             const std::filesystem::path& path)
{
    if (!document.contains(key) || !document.at(key).is_number_unsigned())
    {
        throw JsonLoadError("Missing or invalid UID field '" + std::string(key) + "' in " +
                            path.string());
    }

    const UID uid{document.at(key).get<std::uint64_t>()};
    if (uid.value == 0 || !uid_is_safe_json_integer(uid))
    {
        throw JsonLoadError("UID field '" + std::string(key) +
                            "' is outside the supported JSON-safe range in " + path.string());
    }
    return uid;
}

[[nodiscard]] std::optional<UID> optionalUid(const json& document, const char* key,
                                             const std::filesystem::path& path)
{
    if (!document.contains(key) || document.at(key).is_null())
    {
        return std::nullopt;
    }
    return requireUid(document, key, path);
}

[[nodiscard]] VehicleType parseVehicleType(const json& document,
                                           const std::filesystem::path& path)
{
    VehicleType result;
    result.uid = requireUid(document, "uid", path);
    result.type_name = requireString(document, "typeName", path);
    result.pkp_series = optionalString(document, "pkpSeries", path);
    result.family = optionalString(document, "family", path);
    result.vehicle_type = requireString(document, "vehicleType", path);
    result.vehicle_subtype = optionalString(document, "vehicleSubtype", path);
    result.length_m = requireDouble(document, "lengthM", path);
    result.axle_count = requireInt(document, "axleCount", path);
    result.mass_empty_t = requireDouble(document, "massEmptyT", path);
    result.mass_gross_t = optionalDouble(document, "massGrossT", path);
    result.max_speed_kmh = requireInt(document, "maxSpeedKmh", path);
    result.braking_lambda_pct = requireInt(document, "brakingLambdaPct", path);
    result.power_kw = optionalDouble(document, "powerKW", path);
    result.traction_force_kn = optionalDouble(document, "tractionForceKN", path);
    result.multiple_coupling_capable =
        optionalBool(document, "multipleCouplingCapable", path);
    result.davis_a = optionalDouble(document, "davisA", path);
    result.davis_b = optionalDouble(document, "davisB", path);
    result.davis_c = optionalDouble(document, "davisC", path);
    result.source_file = path;
    return result;
}

[[nodiscard]] Vehicle parseVehicle(const json& document, const std::filesystem::path& path)
{
    Vehicle result;
    result.uid = requireUid(document, "uid", path);
    result.type_uid = requireUid(document, "type_uid", path);
    result.pid = requireString(document, "pID", path);
    result.display_name = requireString(document, "displayName", path);
    result.vehicle_type = optionalString(document, "vehicleType", path);
    result.vehicle_subtype = optionalString(document, "vehicleSubtype", path);
    result.length_m = optionalDouble(document, "lengthM", path);
    result.axle_count = optionalInt(document, "axleCount", path);
    result.mass_empty_t = optionalDouble(document, "massEmptyT", path);
    result.mass_gross_t = optionalDouble(document, "massGrossT", path);
    result.max_speed_kmh = optionalInt(document, "maxSpeedKmh", path);
    result.braking_lambda_pct = optionalInt(document, "brakingLambdaPct", path);
    result.power_kw = optionalDouble(document, "powerKW", path);
    result.traction_force_kn = optionalDouble(document, "tractionForceKN", path);
    result.davis_a = optionalDouble(document, "davisA", path);
    result.davis_b = optionalDouble(document, "davisB", path);
    result.davis_c = optionalDouble(document, "davisC", path);
    result.traction_status = optionalString(document, "tractionStatus", path);
    result.carrier_id = optionalUid(document, "carrierId", path);
    result.inventory_number = optionalString(document, "inventoryNumber", path);
    result.notes = optionalString(document, "notes", path);
    result.source_file = path;
    return result;
}

[[nodiscard]] Train parseTrain(const json& document, const std::filesystem::path& path)
{
    Train result;
    result.uid = requireUid(document, "uid", path);
    result.pid = requireString(document, "pID", path);
    result.display_name = requireString(document, "displayName", path);
    result.train_category = requireString(document, "trainCategory", path);
    result.carrier_id = optionalUid(document, "carrierId", path);

    if (!document.contains("vehicle_uids") || !document.at("vehicle_uids").is_array())
    {
        throw JsonLoadError("Missing or invalid UID array field 'vehicle_uids' in " +
                            path.string());
    }

    const auto& vehicle_uids = document.at("vehicle_uids");
    result.vehicle_uids.reserve(vehicle_uids.size());
    for (std::size_t index = 0; index < vehicle_uids.size(); ++index)
    {
        const auto& value = vehicle_uids.at(index);
        if (!value.is_number_unsigned())
        {
            throw JsonLoadError("Invalid vehicle UID at index " + std::to_string(index) + " in " +
                                path.string());
        }

        const UID uid{value.get<std::uint64_t>()};
        if (uid.value == 0 || !uid_is_safe_json_integer(uid))
        {
            throw JsonLoadError("Vehicle UID at index " + std::to_string(index) +
                                " is outside the supported JSON-safe range in " + path.string());
        }
        result.vehicle_uids.push_back(uid);
    }

    result.source_file = path;
    return result;
}

}  // namespace

JsonDocumentType JsonLoader::detectDocumentType(const std::filesystem::path& path)
{
    const json document = parseJsonFile(path);
    if (document.contains("vehicle_uids") && document.contains("trainCategory"))
    {
        return JsonDocumentType::TRAIN;
    }
    if (document.contains("type_uid") && document.contains("pID"))
    {
        return JsonDocumentType::VEHICLE;
    }
    if (document.contains("typeName") && document.contains("vehicleType"))
    {
        return JsonDocumentType::VEHICLE_TYPE;
    }
    return JsonDocumentType::UNKNOWN;
}

VehicleType JsonLoader::loadVehicleType(const std::filesystem::path& path)
{
    return parseVehicleType(parseJsonFile(path), path);
}

Vehicle JsonLoader::loadVehicle(const std::filesystem::path& path)
{
    return parseVehicle(parseJsonFile(path), path);
}

Train JsonLoader::loadTrain(const std::filesystem::path& path)
{
    return parseTrain(parseJsonFile(path), path);
}

}  // namespace symulator::tools
