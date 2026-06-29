#pragma once

#include <stdexcept>
#include <vector>

#include "domain/fleet_types.hpp"

namespace symulator::tools
{

class DataSourceError : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

class IDataSource
{
public:
    virtual ~IDataSource() = default;

    // Throws DataSourceError when the source root is missing or has an invalid structure.
    virtual void validate() const = 0;

    [[nodiscard]] virtual std::vector<VehicleType> loadVehicleTypes() const = 0;
    [[nodiscard]] virtual std::vector<Vehicle> loadVehicles() const = 0;
    [[nodiscard]] virtual std::vector<Train> loadTrains() const = 0;
};

}  // namespace symulator::tools
