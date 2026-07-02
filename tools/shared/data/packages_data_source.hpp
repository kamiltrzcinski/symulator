#pragma once

#include <filesystem>

#include "data/i_data_source.hpp"

namespace symulator::tools
{

class PackagesDataSource final : public IDataSource
{
public:
    explicit PackagesDataSource(
        std::filesystem::path packages_root = std::filesystem::current_path() / "packages");

    void validate() const override;
    [[nodiscard]] std::vector<VehicleType> loadVehicleTypes() const override;
    [[nodiscard]] std::vector<Vehicle> loadVehicles() const override;
    [[nodiscard]] std::vector<Train> loadTrains() const override;
    [[nodiscard]] const std::filesystem::path& root() const noexcept;

private:
    std::filesystem::path root_;
};

}  // namespace symulator::tools
