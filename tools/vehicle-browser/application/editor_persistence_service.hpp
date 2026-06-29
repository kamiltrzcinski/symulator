#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "domain/fleet_types.hpp"

namespace symulator::tools::vehicle_browser
{

struct VehicleSaveRequest
{
    std::filesystem::path file;
    bool preserve_existing_fields = false;
    UID uid;
    UID type_uid;
    std::string pid;
    std::string display_name;
    std::optional<UID> carrier_id;
    std::optional<std::string> inventory_number;
    std::optional<std::string> notes;
};

struct TrainSaveRequest
{
    std::filesystem::path file;
    bool preserve_existing_fields = false;
    UID uid;
    std::string pid;
    std::string display_name;
    std::string train_category;
    std::optional<UID> carrier_id;
    std::vector<UID> vehicle_uids;
};

class EditorPersistenceService
{
public:
    [[nodiscard]] std::filesystem::path saveVehicle(const VehicleSaveRequest& request) const;
    [[nodiscard]] UID saveTrain(const TrainSaveRequest& request) const;
};

}  // namespace symulator::tools::vehicle_browser
