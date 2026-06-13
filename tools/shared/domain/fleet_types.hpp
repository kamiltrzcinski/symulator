#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "domain/uid_types.hpp"

namespace symulator::tools
{

struct VehicleType
{
    UID uid;
    std::string type_name;
    std::optional<std::string> pkp_series;
    std::optional<std::string> family;
    std::string vehicle_type;
    std::optional<std::string> vehicle_subtype;
    double length_m = 0.0;
    int axle_count = 0;
    double mass_empty_t = 0.0;
    std::optional<double> mass_gross_t;
    int max_speed_kmh = 0;
    int braking_lambda_pct = 0;
    std::optional<double> power_kw;
    std::optional<double> traction_force_kn;
    std::optional<bool> multiple_coupling_capable;
    std::optional<double> davis_a;
    std::optional<double> davis_b;
    std::optional<double> davis_c;
    std::filesystem::path source_file;
};

struct Vehicle
{
    UID uid;
    UID type_uid;
    std::string pid;
    std::string display_name;
    std::optional<std::string> vehicle_type;
    std::optional<std::string> vehicle_subtype;
    std::optional<double> length_m;
    std::optional<int> axle_count;
    std::optional<double> mass_empty_t;
    std::optional<double> mass_gross_t;
    std::optional<int> max_speed_kmh;
    std::optional<int> braking_lambda_pct;
    std::optional<double> power_kw;
    std::optional<double> traction_force_kn;
    std::optional<double> davis_a;
    std::optional<double> davis_b;
    std::optional<double> davis_c;
    std::optional<std::string> traction_status;
    std::optional<UID> carrier_id;
    std::optional<std::string> inventory_number;
    std::optional<std::string> notes;
    std::filesystem::path source_file;
};

struct Train
{
    UID uid;
    std::string pid;
    std::string display_name;
    std::string train_category;
    std::optional<UID> carrier_id;
    std::vector<UID> vehicle_uids;
    std::filesystem::path source_file;
};

}  // namespace symulator::tools
