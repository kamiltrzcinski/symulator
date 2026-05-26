#include "engine/core/fleet_registry.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <limits>
#include <sstream>

#include <nlohmann/json.hpp>

namespace engine::core
{

using json = nlohmann::json;

namespace
{

[[nodiscard]] std::string to_upper(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::toupper(ch)); });
    return s;
}

[[nodiscard]] bool is_json_file(const std::filesystem::path& path)
{
    return std::filesystem::is_regular_file(path) && path.extension() == ".json";
}

[[nodiscard]] std::vector<std::filesystem::path> collect_json_files(
    const std::filesystem::path& root)
{
    if (!std::filesystem::exists(root))
    {
        throw FleetLoadError("Missing directory: " + root.string());
    }

    std::vector<std::filesystem::path> files;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(root))
    {
        if (is_json_file(entry.path()))
        {
            files.push_back(entry.path());
        }
    }

    std::sort(files.begin(), files.end());
    return files;
}

[[nodiscard]] std::vector<std::filesystem::path> collect_vehicle_files(
    const std::filesystem::path& root)
{
    if (!std::filesystem::exists(root))
    {
        throw FleetLoadError("Missing directory: " + root.string());
    }

    std::vector<std::filesystem::path> files;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(root))
    {
        if (std::filesystem::is_regular_file(entry.path()) &&
            entry.path().filename() == "vehicle.json")
        {
            files.push_back(entry.path());
        }
    }

    std::sort(files.begin(), files.end());
    return files;
}

[[nodiscard]] json parse_json_file(const std::filesystem::path& path)
{
    std::ifstream f(path);
    if (!f)
    {
        throw FleetLoadError("Cannot open file: " + path.string());
    }

    try
    {
        return json::parse(f);
    }
    catch (const std::exception& ex)
    {
        throw FleetLoadError("JSON parse error in " + path.string() + ": " + ex.what());
    }
}

[[nodiscard]] std::string require_string(const json& j, const char* key,
                                         const std::filesystem::path& path)
{
    if (!j.contains(key) || !j.at(key).is_string())
    {
        throw FleetLoadError("Missing or invalid string field '" + std::string(key) + "' in " +
                             path.string());
    }
    return j.at(key).get<std::string>();
}

[[nodiscard]] std::string optional_string(const json& j, const char* key)
{
    if (!j.contains(key) || j.at(key).is_null())
    {
        return "";
    }
    if (!j.at(key).is_string())
    {
        return "";
    }
    return j.at(key).get<std::string>();
}

[[nodiscard]] int require_int(const json& j, const char* key, const std::filesystem::path& path)
{
    if (!j.contains(key) || !j.at(key).is_number())
    {
        throw FleetLoadError("Missing or invalid numeric field '" + std::string(key) + "' in " +
                             path.string());
    }
    return j.at(key).get<int>();
}

[[nodiscard]] float require_float(const json& j, const char* key, const std::filesystem::path& path)
{
    if (!j.contains(key) || !j.at(key).is_number())
    {
        throw FleetLoadError("Missing or invalid numeric field '" + std::string(key) + "' in " +
                             path.string());
    }
    return j.at(key).get<float>();
}

[[nodiscard]] std::optional<float> optional_float(const json& j, const char* key,
                                                  const std::filesystem::path& path)
{
    if (!j.contains(key) || j.at(key).is_null())
    {
        return std::nullopt;
    }
    if (!j.at(key).is_number())
    {
        throw FleetLoadError("Invalid numeric field '" + std::string(key) + "' in " +
                             path.string());
    }
    return j.at(key).get<float>();
}

[[nodiscard]] std::optional<bool> optional_bool(const json& j, const char* key,
                                                const std::filesystem::path& path)
{
    if (!j.contains(key) || j.at(key).is_null())
    {
        return std::nullopt;
    }
    if (!j.at(key).is_boolean())
    {
        throw FleetLoadError("Invalid boolean field '" + std::string(key) + "' in " +
                             path.string());
    }
    return j.at(key).get<bool>();
}

[[nodiscard]] bool is_locomotive_type(const std::string& vehicle_type)
{
    return to_upper(vehicle_type) == "LOCOMOTIVE";
}

[[nodiscard]] bool is_traction_capable(const std::string& vehicle_type,
                                       const std::string& vehicle_subtype)
{
    const std::string type = to_upper(vehicle_type);
    const std::string subtype = to_upper(vehicle_subtype);

    if (type == "LOCOMOTIVE")
    {
        return true;
    }

    if ((type == "EMU_UNIT" || type == "DMU_UNIT") && subtype == "MOTOR")
    {
        return true;
    }

    return false;
}

[[nodiscard]] TractionStatus parse_traction_status(const std::string& value,
                                                   const std::filesystem::path& path)
{
    const std::string upper = to_upper(value);
    if (upper == "OPERATIONAL")
    {
        return TractionStatus::OPERATIONAL;
    }
    if (upper == "DEFECTIVE")
    {
        return TractionStatus::DEFECTIVE;
    }

    throw FleetLoadError("Invalid tractionStatus value '" + value + "' in " + path.string());
}

[[nodiscard]] std::optional<TractionStatus> optional_traction_status(
    const json& j, const char* key, const std::filesystem::path& path)
{
    if (!j.contains(key) || j.at(key).is_null())
    {
        return std::nullopt;
    }
    if (!j.at(key).is_string())
    {
        throw FleetLoadError("Invalid string field '" + std::string(key) + "' in " + path.string());
    }

    return parse_traction_status(j.at(key).get<std::string>(), path);
}

[[nodiscard]] TrainCategory parse_train_category(const std::string& value,
                                                 const std::filesystem::path& path)
{
    const std::string upper = to_upper(value);
    if (upper == "PASSENGER")
    {
        return TrainCategory::PASSENGER;
    }
    if (upper == "FREIGHT")
    {
        return TrainCategory::FREIGHT;
    }
    if (upper == "MAINTENANCE")
    {
        return TrainCategory::MAINTENANCE;
    }

    throw FleetLoadError("Invalid trainCategory value '" + value + "' in " + path.string());
}

[[nodiscard]] std::optional<TrainCategory> category_from_folder(
    const std::filesystem::path& file, const std::filesystem::path& trains_root)
{
    const auto rel = std::filesystem::relative(file.parent_path(), trains_root);
    if (rel.empty())
    {
        return std::nullopt;
    }

    const auto first = (*rel.begin()).string();
    const std::string lower = to_upper(first);

    if (lower == "PASSENGER" || lower == "LOCAL")
    {
        return TrainCategory::PASSENGER;
    }
    if (lower == "FREIGHT")
    {
        return TrainCategory::FREIGHT;
    }
    if (lower == "SHUNTING")
    {
        return TrainCategory::MAINTENANCE;
    }

    return std::nullopt;
}

[[nodiscard]] float resolve_float_override(const json& j, const char* key, float fallback,
                                           const std::filesystem::path& path)
{
    if (!j.contains(key) || j.at(key).is_null())
    {
        return fallback;
    }
    if (!j.at(key).is_number())
    {
        throw FleetLoadError("Invalid numeric override '" + std::string(key) + "' in " +
                             path.string());
    }
    return j.at(key).get<float>();
}

[[nodiscard]] int resolve_int_override(const json& j, const char* key, int fallback,
                                       const std::filesystem::path& path)
{
    if (!j.contains(key) || j.at(key).is_null())
    {
        return fallback;
    }
    if (!j.at(key).is_number())
    {
        throw FleetLoadError("Invalid integer override '" + std::string(key) + "' in " +
                             path.string());
    }
    return j.at(key).get<int>();
}

[[nodiscard]] std::optional<float> resolve_optional_float_override(
    const json& j, const char* key, const std::optional<float>& fallback,
    const std::filesystem::path& path)
{
    if (!j.contains(key))
    {
        return fallback;
    }
    if (j.at(key).is_null())
    {
        return std::nullopt;
    }
    if (!j.at(key).is_number())
    {
        throw FleetLoadError("Invalid numeric override '" + std::string(key) + "' in " +
                             path.string());
    }
    return j.at(key).get<float>();
}

}  // namespace

void FleetRegistry::load(const std::filesystem::path& data_root)
{
    types_.clear();
    vehicles_.clear();
    consists_.clear();

    load_types_(data_root / "vehicle_types");
    load_vehicles_(data_root / "vehicles");
    load_consists_(data_root / "trains");
}

const VehicleType& FleetRegistry::get_type(const GID& type_id) const
{
    const auto it = types_.find(type_id);
    if (it == types_.end())
    {
        throw FleetLoadError("VehicleType not found: " + type_id.value);
    }
    return it->second;
}

const Vehicle& FleetRegistry::get_vehicle(const GID& gid) const
{
    const auto it = vehicles_.find(gid);
    if (it == vehicles_.end())
    {
        throw FleetLoadError("Vehicle not found: " + gid.value);
    }
    return it->second;
}

const TrainConsist& FleetRegistry::get_consist(const GID& gid) const
{
    const auto it = consists_.find(gid);
    if (it == consists_.end())
    {
        throw FleetLoadError("TrainConsist not found: " + gid.value);
    }
    return it->second;
}

void FleetRegistry::load_types_(const std::filesystem::path& types_dir)
{
    const auto files = collect_json_files(types_dir);
    for (const auto& path : files)
    {
        const json j = parse_json_file(path);

        VehicleType type{};
        type.type_id = GID{require_string(j, "typeID", path)};
        type.type_name = require_string(j, "typeName", path);
        type.pkp_series = optional_string(j, "pkpSeries");
        type.family = optional_string(j, "family");
        type.vehicle_type = require_string(j, "vehicleType", path);
        type.vehicle_subtype = optional_string(j, "vehicleSubtype");
        type.length_m = require_float(j, "lengthM", path);
        type.axle_count = require_int(j, "axleCount", path);
        type.mass_empty_t = require_float(j, "massEmptyT", path);
        type.mass_gross_t = optional_float(j, "massGrossT", path);
        type.max_speed_kmh = require_int(j, "maxSpeedKmh", path);
        type.braking_lambda_pct = require_int(j, "brakingLambdaPct", path);
        type.power_kw = optional_float(j, "powerKW", path);
        type.traction_force_kn = optional_float(j, "tractionForceKN", path);
        type.multiple_coupling_capable = optional_bool(j, "multipleCouplingCapable", path);

        if (!is_traction_capable(type.vehicle_type, type.vehicle_subtype) &&
            type.multiple_coupling_capable.has_value())
        {
            throw FleetLoadError(
                "multipleCouplingCapable is only valid for traction-capable type categories in " +
                path.string());
        }

        const bool has_davis_a = j.contains("davisA") && !j.at("davisA").is_null();
        const bool has_davis_b = j.contains("davisB") && !j.at("davisB").is_null();
        const bool has_davis_c = j.contains("davisC") && !j.at("davisC").is_null();

        if (has_davis_a || has_davis_b || has_davis_c)
        {
            if (!(has_davis_a && has_davis_b && has_davis_c))
            {
                throw FleetLoadError("Incomplete Davis coefficients in " + path.string());
            }

            type.davis.a = require_float(j, "davisA", path);
            type.davis.b = require_float(j, "davisB", path);
            type.davis.c = require_float(j, "davisC", path);
        }
        else
        {
            type.davis =
                davis_defaults_(type.vehicle_type, type.vehicle_subtype, type.max_speed_kmh);
        }

        const auto [it, inserted] = types_.emplace(type.type_id, std::move(type));
        if (!inserted)
        {
            throw FleetLoadError("Duplicate typeID: " + it->first.value + " in " + path.string());
        }
    }
}

void FleetRegistry::load_vehicles_(const std::filesystem::path& vehicles_dir)
{
    const auto files = collect_vehicle_files(vehicles_dir);
    for (const auto& path : files)
    {
        const json j = parse_json_file(path);

        Vehicle vehicle{};
        vehicle.gid = GID{require_string(j, "gID", path)};
        vehicle.pid = require_string(j, "pID", path);
        vehicle.type_id = GID{require_string(j, "typeID", path)};
        vehicle.display_name = require_string(j, "displayName", path);

        const auto type_it = types_.find(vehicle.type_id);
        if (type_it == types_.end())
        {
            throw FleetLoadError("Vehicle references unknown typeID '" + vehicle.type_id.value +
                                 "' in " + path.string());
        }
        const VehicleType& type = type_it->second;

        const std::string vehicle_type_override = optional_string(j, "vehicleType");
        if (!vehicle_type_override.empty() && vehicle_type_override != type.vehicle_type)
        {
            throw FleetLoadError("VehicleType override mismatch in " + path.string());
        }
        const std::string vehicle_subtype_override = optional_string(j, "vehicleSubtype");
        if (!vehicle_subtype_override.empty() && vehicle_subtype_override != type.vehicle_subtype)
        {
            throw FleetLoadError("VehicleSubtype override mismatch in " + path.string());
        }

        vehicle.vehicle_type = type.vehicle_type;
        vehicle.vehicle_subtype = type.vehicle_subtype;
        vehicle.length_m = resolve_float_override(j, "lengthM", type.length_m, path);
        vehicle.axle_count = resolve_int_override(j, "axleCount", type.axle_count, path);
        vehicle.mass_empty_t = resolve_float_override(j, "massEmptyT", type.mass_empty_t, path);

        const std::optional<float> mass_gross_override = optional_float(j, "massGrossT", path);
        vehicle.effective_mass_t =
            mass_gross_override.value_or(type.mass_gross_t.value_or(vehicle.mass_empty_t));

        vehicle.max_speed_kmh = resolve_int_override(j, "maxSpeedKmh", type.max_speed_kmh, path);
        vehicle.braking_lambda_pct =
            resolve_int_override(j, "brakingLambdaPct", type.braking_lambda_pct, path);

        vehicle.power_kw = resolve_optional_float_override(j, "powerKW", type.power_kw, path);
        vehicle.traction_force_kn =
            resolve_optional_float_override(j, "tractionForceKN", type.traction_force_kn, path);
        vehicle.multiple_coupling_capable = type.multiple_coupling_capable;

        vehicle.traction_capable =
            is_traction_capable(vehicle.vehicle_type, vehicle.vehicle_subtype);
        const std::optional<TractionStatus> traction_status_override =
            optional_traction_status(j, "tractionStatus", path);

        if (vehicle.traction_capable)
        {
            vehicle.traction_status =
                traction_status_override.value_or(TractionStatus::OPERATIONAL);
        }
        else
        {
            if (traction_status_override.has_value())
            {
                throw FleetLoadError(
                    "tractionStatus is only valid for traction-capable vehicles in " +
                    path.string());
            }
            vehicle.traction_status = std::nullopt;
        }

        const bool has_any_davis_key =
            j.contains("davisA") || j.contains("davisB") || j.contains("davisC");
        if (has_any_davis_key)
        {
            vehicle.davis.a = require_float(j, "davisA", path);
            vehicle.davis.b = require_float(j, "davisB", path);
            vehicle.davis.c = require_float(j, "davisC", path);
        }
        else
        {
            vehicle.davis = type.davis;
        }

        const auto [it, inserted] = vehicles_.emplace(vehicle.gid, std::move(vehicle));
        if (!inserted)
        {
            throw FleetLoadError("Duplicate vehicle gID: " + it->first.value + " in " +
                                 path.string());
        }
    }
}

void FleetRegistry::load_consists_(const std::filesystem::path& consists_dir)
{
    const auto files = collect_json_files(consists_dir);
    for (const auto& path : files)
    {
        const json j = parse_json_file(path);

        TrainConsist consist{};
        consist.gid = GID{require_string(j, "gID", path)};
        consist.pid = require_string(j, "pID", path);
        consist.display_name = require_string(j, "displayName", path);
        consist.train_category =
            parse_train_category(require_string(j, "trainCategory", path), path);

        if (!j.contains("vehicles") || !j.at("vehicles").is_array())
        {
            throw FleetLoadError("Missing or invalid array field 'vehicles' in " + path.string());
        }

        const auto folder_category = category_from_folder(path, consists_dir);
        if (folder_category.has_value() && *folder_category != consist.train_category)
        {
            throw FleetLoadError("trainCategory does not match folder in " + path.string());
        }

        consist.total_length_m = 0.0f;
        consist.total_axles = 0;
        consist.total_mass_t = 0.0f;
        consist.max_speed_kmh = std::numeric_limits<float>::max();
        consist.total_traction_kn = 0.0f;
        consist.total_power_kw = 0.0f;

        double lambda_mass_sum = 0.0;
        std::vector<const Vehicle*> operational_locomotives;

        for (const auto& item : j.at("vehicles"))
        {
            if (!item.is_string())
            {
                throw FleetLoadError("Invalid vehicle gID entry in " + path.string());
            }

            const GID vehicle_gid{item.get<std::string>()};
            consist.vehicle_gids.push_back(vehicle_gid);

            const auto vehicle_it = vehicles_.find(vehicle_gid);
            if (vehicle_it == vehicles_.end())
            {
                throw FleetLoadError("Consist references unknown vehicle gID '" +
                                     vehicle_gid.value + "' in " + path.string());
            }

            const Vehicle& vehicle = vehicle_it->second;
            consist.total_length_m += vehicle.length_m;
            consist.total_axles += vehicle.axle_count;
            consist.total_mass_t += vehicle.effective_mass_t;
            consist.max_speed_kmh =
                std::min(consist.max_speed_kmh, static_cast<float>(vehicle.max_speed_kmh));

            const bool operational = !vehicle.traction_status.has_value() ||
                                     *vehicle.traction_status == TractionStatus::OPERATIONAL;
            if (vehicle.traction_capable && operational)
            {
                if (is_locomotive_type(vehicle.vehicle_type))
                {
                    operational_locomotives.push_back(&vehicle);
                }
                else
                {
                    consist.total_traction_kn += vehicle.traction_force_kn.value_or(0.0f);
                    consist.total_power_kw += vehicle.power_kw.value_or(0.0f);
                }
            }

            lambda_mass_sum +=
                static_cast<double>(vehicle.braking_lambda_pct) * vehicle.effective_mass_t;
        }

        if (!operational_locomotives.empty())
        {
            const auto add_locomotive_traction = [&consist](const Vehicle& vehicle)
            {
                consist.total_traction_kn += vehicle.traction_force_kn.value_or(0.0f);
                consist.total_power_kw += vehicle.power_kw.value_or(0.0f);
            };

            if (operational_locomotives.size() == 1)
            {
                add_locomotive_traction(*operational_locomotives.front());
            }
            else
            {
                const Vehicle& first = *operational_locomotives.front();
                const bool same_type = std::all_of(
                    operational_locomotives.begin(), operational_locomotives.end(),
                    [&first](const Vehicle* vehicle) { return vehicle->type_id == first.type_id; });
                const bool coupling_allowed =
                    same_type && first.multiple_coupling_capable.value_or(false);

                if (coupling_allowed)
                {
                    for (const Vehicle* vehicle : operational_locomotives)
                    {
                        add_locomotive_traction(*vehicle);
                    }
                }
                else
                {
                    // Fallback policy: additional locomotives stay as ballast load.
                    add_locomotive_traction(first);
                }
            }
        }

        if (consist.vehicle_gids.empty())
        {
            throw FleetLoadError("Train consist has no vehicles in " + path.string());
        }

        if (consist.max_speed_kmh == std::numeric_limits<float>::max())
        {
            consist.max_speed_kmh = 0.0f;
        }

        if (consist.total_mass_t > 0.0f)
        {
            consist.consist_lambda_pct = static_cast<float>(lambda_mass_sum / consist.total_mass_t);
        }
        else
        {
            consist.consist_lambda_pct = 0.0f;
        }

        const auto [it, inserted] = consists_.emplace(consist.gid, std::move(consist));
        if (!inserted)
        {
            throw FleetLoadError("Duplicate consist gID: " + it->first.value + " in " +
                                 path.string());
        }
    }
}

DavisCoefficients FleetRegistry::davis_defaults_(const std::string& vehicle_type,
                                                 const std::string& vehicle_subtype,
                                                 int max_speed_kmh)
{
    const std::string type = to_upper(vehicle_type);
    const std::string subtype = to_upper(vehicle_subtype);

    if (type == "LOCOMOTIVE" && subtype == "ELECTRIC")
    {
        if (max_speed_kmh > 130)
        {
            return DavisCoefficients{34.335f, 0.17658f, 0.0024525f};
        }
        return DavisCoefficients{39.24f, 0.1962f, 0.0017658f};
    }

    if (type == "LOCOMOTIVE" && subtype == "DIESEL")
    {
        return DavisCoefficients{44.145f, 0.21582f, 0.0019620f};
    }

    if (type == "LOCOMOTIVE" && subtype == "STEAM")
    {
        return DavisCoefficients{44.145f, 0.21582f, 0.0019620f};
    }

    if (type == "EMU_UNIT" && subtype == "MOTOR")
    {
        return DavisCoefficients{34.335f, 0.17658f, 0.0014715f};
    }

    if ((type == "EMU_UNIT" || type == "DMU_UNIT") &&
        (subtype == "TRAILER" || subtype == "CONTROL"))
    {
        return DavisCoefficients{29.43f, 0.14715f, 0.0013734f};
    }

    if (type == "DMU_UNIT" && subtype == "MOTOR")
    {
        return DavisCoefficients{39.24f, 0.1962f, 0.0017658f};
    }

    if (type == "PASSENGER_WAGON")
    {
        return DavisCoefficients{29.43f, 0.14715f, 0.0013734f};
    }

    if (type == "FREIGHT_WAGON")
    {
        return DavisCoefficients{14.715f, 0.07848f, 0.0007848f};
    }

    if (type == "SERVICE_WAGON")
    {
        return DavisCoefficients{24.525f, 0.12753f, 0.0011772f};
    }

    return DavisCoefficients{39.24f, 0.1962f, 0.0017658f};
}

}  // namespace engine::core
