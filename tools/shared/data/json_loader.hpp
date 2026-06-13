#pragma once

#include <filesystem>
#include <stdexcept>

#include "domain/fleet_types.hpp"

namespace symulator::tools
{

enum class JsonDocumentType
{
    UNKNOWN,
    VEHICLE_TYPE,
    VEHICLE,
    TRAIN,
};

class JsonLoadError : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

class JsonLoader
{
public:
    [[nodiscard]] static JsonDocumentType detectDocumentType(const std::filesystem::path& path);
    [[nodiscard]] static VehicleType loadVehicleType(const std::filesystem::path& path);
    [[nodiscard]] static Vehicle loadVehicle(const std::filesystem::path& path);
    [[nodiscard]] static Train loadTrain(const std::filesystem::path& path);
};

}  // namespace symulator::tools
