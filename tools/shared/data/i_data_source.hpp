#pragma once

#include <vector>

#include "domain/fleet_types.hpp"

namespace symulator::tools
{

class IDataSource
{
public:
    virtual ~IDataSource() = default;

    [[nodiscard]] virtual std::vector<VehicleType> loadVehicleTypes() const = 0;
    [[nodiscard]] virtual std::vector<Vehicle> loadVehicles() const = 0;
    [[nodiscard]] virtual std::vector<Train> loadTrains() const = 0;
};

}  // namespace symulator::tools
