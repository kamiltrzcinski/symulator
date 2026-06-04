#pragma once

#include <filesystem>
#include <string_view>

#include <gtest/gtest.h>

#include <engine/core/fleet_registry.hpp>

#include <tests/common/file_test_helpers.hpp>

namespace tests::common::fleet
{

class FleetDataFixture : public ::testing::Test
{
protected:
    std::filesystem::path data_root() const { return temp_dir_.path() / "data"; }

    void write_data(std::string_view relative_path, std::string_view content)
    {
        write_text(data_root() / std::filesystem::path(relative_path), content);
    }

    void create_minimal_tree() { create_minimal_fleet_tree(data_root()); }

    engine::core::FleetRegistry load_registry() const
    {
        engine::core::FleetRegistry registry;
        registry.load(data_root());
        return registry;
    }

private:
    TemporaryDirectory temp_dir_{"symulator_fleet_registry_test_"};
};

}  // namespace tests::common::fleet
