#pragma once

#include <filesystem>

#include "data/i_data_source.hpp"

namespace symulator::tools
{

class DirectoryDataSource final : public IDataSource
{
public:
    explicit DirectoryDataSource(std::filesystem::path root);

    void validate() const override;
    [[nodiscard]] std::vector<VehicleType> loadVehicleTypes() const override;
    [[nodiscard]] std::vector<Vehicle> loadVehicles() const override;
    [[nodiscard]] std::vector<Train> loadTrains() const override;
    [[nodiscard]] const std::filesystem::path& root() const noexcept;

private:
    std::filesystem::path root_;
};

}  // namespace symulator::tools
