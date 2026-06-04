#include <engine/core/fleet_registry.hpp>

#include <gtest/gtest.h>

#include <tests/common/fleet_test_fixture.hpp>
#include <tests/common/fleet_test_uids.hpp>
#include <tests/common/param_test_helpers.hpp>

namespace
{

using namespace engine::core;
using tests::common::fleet::FleetDataFixture;
using namespace tests::common::fleet;

TEST(UID, EncodesDomainKindTypeAndItem)
{
    constexpr UID en57_1120 = make_uid(UIDDomain::ROLLING_STOCK, UIDKind::VEHICLE, 0x0001, 1120);

    EXPECT_EQ(en57_1120.value, 0x010200010460ULL);
    EXPECT_EQ(uid_domain(en57_1120), UIDDomain::ROLLING_STOCK);
    EXPECT_EQ(uid_kind(en57_1120), UIDKind::VEHICLE);
    EXPECT_EQ(uid_scope(en57_1120), 0x0001);
    EXPECT_EQ(uid_instance(en57_1120), 1120);
    EXPECT_TRUE(uid_is_safe_json_integer(en57_1120));
}

class FleetRegistryLoadingTest : public FleetDataFixture
{
protected:
    void write_sm42_type_and_vehicle()
    {
        write_data("vehicle_types/locomotive/diesel/sm42.json", R"json({
  "uid": 1103806595077,
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

        write_data("vehicles/locomotive/diesel/sm42/sm42-001/vehicle.json", R"json({
  "uid": 1108101562376,
  "pID": "SM42-001",
  "type_uid": 1103806595077,
  "displayName": "SM42-001"
})json");
    }
};

TEST_F(FleetRegistryLoadingTest, LoadsRecursiveAndBuildsDerivedConsist)
{
    write_data("vehicle_types/locomotive/electric/201e.json", R"json({
  "uid": 1103806595073,
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

    write_data("vehicle_types/freight_wagon/hopper/452w.json", R"json({
  "uid": 1103806595074,
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

    write_data("vehicles/locomotive/electric/et22/et22-001/vehicle.json", R"json({
  "uid": 1108101562369,
  "pID": "ET22-001",
  "type_uid": 1103806595073,
  "displayName": "ET22-001"
})json");

    write_data("vehicles/freight_wagon/hopper/452w/452w-5375001/vehicle.json", R"json({
  "uid": 1108101562370,
  "pID": "452W-5375001",
  "type_uid": 1103806595074,
  "displayName": "452W-5375001"
})json");

    write_data("vehicles/freight_wagon/hopper/452w/452w-5375002/vehicle.json", R"json({
  "uid": 1108101562371,
  "pID": "452W-5375002",
  "type_uid": 1103806595074,
  "displayName": "452W-5375002",
  "massGrossT": 22.0
})json");

    write_data("trains/freight/tow54321.json", R"json({
  "uid": 1112396529665,
  "pID": "Tow 543210",
  "displayName": "Tow 543210",
  "trainCategory": "FREIGHT",
  "vehicle_uids": [
    1108101562369,
    1108101562370,
    1108101562371
  ]
})json");

    const auto registry = load_registry();

    const auto& type = registry.get_type(VT_201E);
    EXPECT_EQ(type.pkp_series, "ET22");
    EXPECT_EQ(type.family, "et22");

    const auto& wagon_2 = registry.get_vehicle(VEH_452W_537_0002);
    EXPECT_NEAR(wagon_2.effective_mass_t, 22.0f, 0.001f);

    const auto& consist = registry.get_consist(TRN_TOW543210);
    EXPECT_EQ(consist.train_category, TrainCategory::FREIGHT);
    EXPECT_EQ(consist.total_axles, 14);
    EXPECT_NEAR(consist.total_length_m, 47.32f, 0.01f);
    EXPECT_NEAR(consist.total_mass_t, 232.0f, 0.01f);
    EXPECT_NEAR(consist.total_traction_kn, 411.0f, 0.01f);
    EXPECT_NEAR(consist.max_speed_kmh, 120.0f, 0.01f);
}

TEST_F(FleetRegistryLoadingTest, AppliesDavisDefaultsWhenMissing)
{
    write_data("vehicle_types/locomotive/electric/eu07.json", R"json({
  "uid": 1103806595075,
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

    write_data("vehicles/locomotive/electric/eu07/eu07-001/vehicle.json", R"json({
  "uid": 1108101562372,
  "pID": "EU07-001",
  "type_uid": 1103806595075,
  "displayName": "EU07-001"
})json");

    write_data("trains/freight/test_train.json", R"json({
  "uid": 1112396529666,
  "pID": "Test",
  "displayName": "Test",
  "trainCategory": "FREIGHT",
  "vehicle_uids": [1108101562372]
})json");

    const auto registry = load_registry();
    const auto& type = registry.get_type(VT_EU07);

    EXPECT_NEAR(type.davis.a, 39.24f, 0.001f);
    EXPECT_NEAR(type.davis.b, 0.1962f, 0.0001f);
    EXPECT_NEAR(type.davis.c, 0.0017658f, 0.000001f);
}

TEST_F(FleetRegistryLoadingTest, IgnoresVehicleSidecarJsonFiles)
{
    write_data("vehicle_types/locomotive/electric/eu07.json", R"json({
  "uid": 1103806595075,
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

    write_data("vehicles/locomotive/electric/eu07/eu07-001/vehicle.json", R"json({
  "uid": 1108101562372,
  "pID": "EU07-001",
  "type_uid": 1103806595075,
  "displayName": "EU07-001"
})json");

    write_data("vehicles/locomotive/electric/eu07/eu07-001/photos/metadata.json", R"json({
  "caption": "Front view"
})json");

    write_data("trains/freight/test_train.json", R"json({
  "uid": 1112396529666,
  "pID": "Test",
  "displayName": "Test",
  "trainCategory": "FREIGHT",
  "vehicle_uids": [1108101562372]
})json");

    const auto registry = load_registry();
    EXPECT_TRUE(registry.has_vehicle(VEH_EU07_001));
}

enum class InvalidLoadScenario
{
    UnknownTypeReference,
    MissingTrainCategory,
    TrainCategoryFolderMismatch,
};

struct InvalidLoadCase
{
    const char* name;
    InvalidLoadScenario scenario;
};

class FleetRegistryInvalidLoadingTest : public FleetRegistryLoadingTest,
                                        public ::testing::WithParamInterface<InvalidLoadCase>
{
};

TEST_P(FleetRegistryInvalidLoadingTest, ThrowsFleetLoadError)
{
    const auto param = GetParam();

    if (param.scenario == InvalidLoadScenario::UnknownTypeReference)
    {
        create_minimal_tree();
        write_data("vehicles/locomotive/electric/bad/bad-001/vehicle.json", R"json({
  "uid": 1108101562376,
  "pID": "BAD",
  "type_uid": 99999999999999,
  "displayName": "BAD"
})json");
    }
    else
    {
        write_sm42_type_and_vehicle();

        if (param.scenario == InvalidLoadScenario::MissingTrainCategory)
        {
            write_data("trains/freight/missing_category.json", R"json({
  "uid": 1112396529666,
  "pID": "Test",
  "displayName": "Test",
  "vehicle_uids": [1108101562376]
})json");
        }
        else
        {
            write_data("trains/freight/mismatch.json", R"json({
  "uid": 1112396529666,
  "pID": "Test",
  "displayName": "Test",
  "trainCategory": "PASSENGER",
  "vehicle_uids": [1108101562376]
})json");
        }
    }

    FleetRegistry registry;
    EXPECT_THROW(registry.load(data_root()), FleetLoadError);
}

INSTANTIATE_TEST_SUITE_P(
    InvalidLoadingScenarios, FleetRegistryInvalidLoadingTest,
    ::testing::Values(
        InvalidLoadCase{"UnknownTypeReference", InvalidLoadScenario::UnknownTypeReference},
        InvalidLoadCase{"MissingTrainCategory", InvalidLoadScenario::MissingTrainCategory},
        InvalidLoadCase{"TrainCategoryFolderMismatch",
                        InvalidLoadScenario::TrainCategoryFolderMismatch}),
    tests::common::param_name<InvalidLoadCase>);

}  // namespace
