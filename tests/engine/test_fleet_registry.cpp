#include "engine/core/fleet_registry.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

using namespace engine::core;

namespace
{

class TempDir
{
public:
    TempDir()
    {
        const auto nonce = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        path_ = std::filesystem::temp_directory_path() /
                ("symulator_fleet_registry_test_" + std::to_string(nonce));
        std::filesystem::create_directories(path_);
    }

    ~TempDir() { std::filesystem::remove_all(path_); }

    const std::filesystem::path& path() const { return path_; }

private:
    std::filesystem::path path_;
};

void write_text(const std::filesystem::path& path, const std::string& content)
{
    std::filesystem::create_directories(path.parent_path());
    std::ofstream f(path);
    f << content;
}

void create_minimal_tree(const std::filesystem::path& root)
{
    std::filesystem::create_directories(root / "vehicle_types");
    std::filesystem::create_directories(root / "vehicles");
    std::filesystem::create_directories(root / "trains");
}

}  // namespace

TEST(FleetRegistry, LoadsRecursiveAndBuildsDerivedConsist)
{
    TempDir tmp;
    const auto root = tmp.path() / "data";

    write_text(root / "vehicle_types/locomotive/electric/201e.json", R"json({
  "typeID": "VT-GLB-201E-0000001",
  "typeName": "201E",
  "pkpSeries": "ET22",
  "vehicleType": "LOCOMOTIVE",
  "vehicleSubtype": "ELECTRIC",
  "lengthM": 19.24,
  "axleCount": 6,
  "massEmptyT": 120.0,
  "massGrossT": 120.0,
  "maxSpeedKmh": 125,
  "brakingLambdaPct": 130,
  "powerKW": 3000.0,
  "tractionForceKN": 411.0,
  "family": "et22",
  "davisA": 39.24,
  "davisB": 0.1962,
  "davisC": 0.0017658
})json");

    write_text(root / "vehicle_types/freight_wagon/hopper/452w.json", R"json({
  "typeID": "VT-GLB-452W-0000002",
  "typeName": "452W",
  "pkpSeries": null,
  "vehicleType": "FREIGHT_WAGON",
  "vehicleSubtype": "HOPPER",
  "lengthM": 14.04,
  "axleCount": 4,
  "massEmptyT": 22.0,
  "massGrossT": 90.0,
  "maxSpeedKmh": 120,
  "brakingLambdaPct": 100,
  "powerKW": null,
  "tractionForceKN": null,
  "family": "hopper",
  "davisA": 14.715,
  "davisB": 0.07848,
  "davisC": 0.0007848
})json");

    write_text(root / "vehicles/locomotive/et22_001.json", R"json({
  "gID": "VEH-TRJ-ET22-001-0000001",
  "pID": "ET22-001",
  "typeID": "VT-GLB-201E-0000001",
  "displayName": "ET22-001"
})json");

    write_text(root / "vehicles/freight_wagon/452w_1.json", R"json({
  "gID": "VEH-TRJ-452W-537-0000001",
  "pID": "452W-5375001",
  "typeID": "VT-GLB-452W-0000002",
  "displayName": "452W-5375001"
})json");

    write_text(root / "vehicles/freight_wagon/452w_2.json", R"json({
  "gID": "VEH-TRJ-452W-537-0000002",
  "pID": "452W-5375002",
  "typeID": "VT-GLB-452W-0000002",
  "displayName": "452W-5375002",
  "massGrossT": 22.0
})json");

    write_text(root / "trains/freight/tow54321.json", R"json({
  "gID": "TRN-TRJ-TOW543210-0000001",
  "pID": "Tow 543210",
  "displayName": "Tow 543210",
  "trainCategory": "FREIGHT",
  "vehicles": [
    "VEH-TRJ-ET22-001-0000001",
    "VEH-TRJ-452W-537-0000001",
    "VEH-TRJ-452W-537-0000002"
  ]
})json");

    FleetRegistry registry;
    ASSERT_NO_THROW(registry.load(root));

    const auto& type = registry.get_type(GID{"VT-GLB-201E-0000001"});
    EXPECT_EQ(type.pkp_series, "ET22");
    EXPECT_EQ(type.family, "et22");

    const auto& wagon_2 = registry.get_vehicle(GID{"VEH-TRJ-452W-537-0000002"});
    EXPECT_NEAR(wagon_2.effective_mass_t, 22.0f, 0.001f);

    const auto& consist = registry.get_consist(GID{"TRN-TRJ-TOW543210-0000001"});
    EXPECT_EQ(consist.train_category, TrainCategory::FREIGHT);
    EXPECT_EQ(consist.total_axles, 14);                  // 6 + 4 + 4
    EXPECT_NEAR(consist.total_length_m, 47.32f, 0.01f);  // 19.24 + 14.04 + 14.04
    EXPECT_NEAR(consist.total_mass_t, 232.0f, 0.01f);    // 120 + 90 + 22
    EXPECT_NEAR(consist.total_traction_kn, 411.0f, 0.01f);
    EXPECT_NEAR(consist.max_speed_kmh, 120.0f, 0.01f);
}

TEST(FleetRegistry, AppliesDavisDefaultsWhenMissing)
{
    TempDir tmp;
    const auto root = tmp.path() / "data";

    write_text(root / "vehicle_types/locomotive/electric/eu07.json", R"json({
  "typeID": "VT-GLB-EU07-0000001",
  "typeName": "EU07",
  "pkpSeries": "EU07",
  "vehicleType": "LOCOMOTIVE",
  "vehicleSubtype": "ELECTRIC",
  "lengthM": 15.9,
  "axleCount": 4,
  "massEmptyT": 80.0,
  "massGrossT": 80.0,
  "maxSpeedKmh": 125,
  "brakingLambdaPct": 100,
  "powerKW": 2000.0,
  "tractionForceKN": 280.0,
  "family": "eu07"
})json");

    write_text(root / "vehicles/locomotive/eu07_001.json", R"json({
  "gID": "VEH-TRJ-EU07-001-0000001",
  "pID": "EU07-001",
  "typeID": "VT-GLB-EU07-0000001",
  "displayName": "EU07-001"
})json");

    write_text(root / "trains/freight/test_train.json", R"json({
  "gID": "TRN-TRJ-TEST-0000001",
  "pID": "Test",
  "displayName": "Test",
  "trainCategory": "FREIGHT",
  "vehicles": ["VEH-TRJ-EU07-001-0000001"]
})json");

    FleetRegistry registry;
    ASSERT_NO_THROW(registry.load(root));

    const auto& type = registry.get_type(GID{"VT-GLB-EU07-0000001"});
    EXPECT_NEAR(type.davis.a, 39.24f, 0.001f);
    EXPECT_NEAR(type.davis.b, 0.1962f, 0.0001f);
    EXPECT_NEAR(type.davis.c, 0.0017658f, 0.000001f);
}

TEST(FleetRegistry, ThrowsOnUnknownTypeReference)
{
    TempDir tmp;
    const auto root = tmp.path() / "data";
    create_minimal_tree(root);

    write_text(root / "vehicles/locomotive/bad.json", R"json({
  "gID": "VEH-TRJ-BAD-0000001",
  "pID": "BAD",
  "typeID": "VT-GLB-DOES-NOT-EXIST-0000001",
  "displayName": "BAD"
})json");

    FleetRegistry registry;
    EXPECT_THROW(registry.load(root), FleetLoadError);
}

TEST(FleetRegistry, ThrowsOnMissingTrainCategory)
{
    TempDir tmp;
    const auto root = tmp.path() / "data";

    write_text(root / "vehicle_types/locomotive/diesel/sm42.json", R"json({
  "typeID": "VT-GLB-SM42-0000001",
  "typeName": "SM42",
  "pkpSeries": "SM42",
  "vehicleType": "LOCOMOTIVE",
  "vehicleSubtype": "DIESEL",
  "lengthM": 14.24,
  "axleCount": 4,
  "massEmptyT": 70.0,
  "massGrossT": 70.0,
  "maxSpeedKmh": 90,
  "brakingLambdaPct": 100,
  "powerKW": 588.0,
  "tractionForceKN": 196.0,
  "family": "sm42"
})json");

    write_text(root / "vehicles/locomotive/sm42_001.json", R"json({
  "gID": "VEH-TRJ-SM42-001-0000001",
  "pID": "SM42-001",
  "typeID": "VT-GLB-SM42-0000001",
  "displayName": "SM42-001"
})json");

    write_text(root / "trains/freight/missing_category.json", R"json({
  "gID": "TRN-TRJ-TEST-0000001",
  "pID": "Test",
  "displayName": "Test",
  "vehicles": ["VEH-TRJ-SM42-001-0000001"]
})json");

    FleetRegistry registry;
    EXPECT_THROW(registry.load(root), FleetLoadError);
}

TEST(FleetRegistry, ThrowsOnTrainCategoryFolderMismatch)
{
    TempDir tmp;
    const auto root = tmp.path() / "data";

    write_text(root / "vehicle_types/locomotive/diesel/sm42.json", R"json({
  "typeID": "VT-GLB-SM42-0000001",
  "typeName": "SM42",
  "pkpSeries": "SM42",
  "vehicleType": "LOCOMOTIVE",
  "vehicleSubtype": "DIESEL",
  "lengthM": 14.24,
  "axleCount": 4,
  "massEmptyT": 70.0,
  "massGrossT": 70.0,
  "maxSpeedKmh": 90,
  "brakingLambdaPct": 100,
  "powerKW": 588.0,
  "tractionForceKN": 196.0,
  "family": "sm42"
})json");

    write_text(root / "vehicles/locomotive/sm42_001.json", R"json({
  "gID": "VEH-TRJ-SM42-001-0000001",
  "pID": "SM42-001",
  "typeID": "VT-GLB-SM42-0000001",
  "displayName": "SM42-001"
})json");

    write_text(root / "trains/freight/mismatch.json", R"json({
  "gID": "TRN-TRJ-TEST-0000001",
  "pID": "Test",
  "displayName": "Test",
  "trainCategory": "PASSENGER",
  "vehicles": ["VEH-TRJ-SM42-001-0000001"]
})json");

    FleetRegistry registry;
    EXPECT_THROW(registry.load(root), FleetLoadError);
}
