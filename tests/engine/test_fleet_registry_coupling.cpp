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

class FleetRegistryCouplingTest : public FleetDataFixture
{
};

TEST_F(FleetRegistryCouplingTest, AcceptsMultipleCouplingCapabilityForEmuMotorType)
{
    write_data("vehicle-types/emu_unit/motor/en57.json", R"json({
  "uid": 1103806595076,
  "typeName": "EN57",
  "pkpSeries": "EN57",
  "vehicleType": "EMU_UNIT",
  "vehicleSubtype": "MOTOR",
  "lengthM": 64.97,
  "axleCount": 12,
  "massEmptyT": 123.0,
  "massGrossT": 123.0,
  "maxSpeedKmh": 110,
  "brakingLambdaPct": 80,
  "powerKW": 580.0,
  "tractionForceKN": 80.0,
  "multipleCouplingCapable": true,
  "family": "en57"
})json");

    write_data("vehicles/emu_unit/motor/en57/en57-001/vehicle.json", R"json({
  "uid": 1108101562374,
  "pID": "EN57-001",
  "type_uid": 1103806595076,
  "displayName": "EN57-001"
})json");

    write_data("trains/passenger/test_train.json", R"json({
  "uid": 1112396529666,
  "pID": "Test",
  "displayName": "Test",
  "trainCategory": "PASSENGER",
  "vehicle_uids": [1108101562374]
})json");

    const auto registry = load_registry();
    const auto& type = registry.get_type(VT_EN57);

    ASSERT_TRUE(type.multiple_coupling_capable.has_value());
    EXPECT_TRUE(*type.multiple_coupling_capable);
}

TEST_F(FleetRegistryCouplingTest, RejectsMultipleCouplingCapabilityForNonTractionType)
{
    write_data("vehicle-types/freight_wagon/hopper/452w.json", R"json({
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
  "multipleCouplingCapable": true,
  "family": "hopper"
})json");

    FleetRegistry registry;
    EXPECT_THROW(registry.load(data_root()), FleetLoadError);
}

TEST_F(FleetRegistryCouplingTest, DefaultsTractionStatusAndAppliesDefectiveAsBallast)
{
    write_data("vehicle-types/locomotive/electric/201e.json", R"json({
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
  "multipleCouplingCapable": true,
  "family": "et22"
})json");

    write_data("vehicle-types/freight_wagon/hopper/452w.json", R"json({
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
  "family": "hopper"
})json");

    write_data("vehicles/locomotive/electric/et22/et22-001/vehicle.json", R"json({
  "uid": 1108101562369,
  "pID": "ET22-001",
  "type_uid": 1103806595073,
  "displayName": "ET22-001"
})json");

    write_data("vehicles/locomotive/electric/et22/et22-002/vehicle.json", R"json({
  "uid": 1108101562375,
  "pID": "ET22-002",
  "type_uid": 1103806595073,
  "displayName": "ET22-002",
  "tractionStatus": "DEFECTIVE"
})json");

    write_data("vehicles/freight_wagon/hopper/452w/452w-5375001/vehicle.json", R"json({
  "uid": 1108101562370,
  "pID": "452W-5375001",
  "type_uid": 1103806595074,
  "displayName": "452W-5375001"
})json");

    write_data("trains/freight/tow54321.json", R"json({
  "uid": 1112396529665,
  "pID": "Tow 543210",
  "displayName": "Tow 543210",
  "trainCategory": "FREIGHT",
  "vehicle_uids": [
    1108101562369,
    1108101562375,
    1108101562370
  ]
})json");

    const auto registry = load_registry();

    const auto& operational_loco = registry.get_vehicle(VEH_ET22_001);
    ASSERT_TRUE(operational_loco.traction_status.has_value());
    EXPECT_EQ(*operational_loco.traction_status, TractionStatus::OPERATIONAL);
    EXPECT_TRUE(operational_loco.traction_capable);

    const auto& wagon = registry.get_vehicle(VEH_452W_537_0001);
    EXPECT_FALSE(wagon.traction_capable);
    EXPECT_FALSE(wagon.traction_status.has_value());

    const auto& consist = registry.get_consist(TRN_TOW543210);
    EXPECT_NEAR(consist.total_traction_kn, 411.0f, 0.01f);
    EXPECT_NEAR(consist.total_power_kw, 3000.0f, 0.01f);
}

enum class CouplingScenario
{
    SameTypeCapable,
    CapabilityUnknown,
    DifferentTypesCapable,
};

struct CouplingCase
{
    const char* name;
    CouplingScenario scenario;
    float expected_traction_kn;
    float expected_power_kw;
};

class FleetRegistryCouplingParamTest : public FleetRegistryCouplingTest,
                                       public ::testing::WithParamInterface<CouplingCase>
{
};

TEST_P(FleetRegistryCouplingParamTest, ComputesTractionByCouplingPolicy)
{
    const auto param = GetParam();

    if (param.scenario == CouplingScenario::DifferentTypesCapable)
    {
        write_data("vehicle-types/locomotive/electric/eu07.json", R"json({
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
  "multipleCouplingCapable": true,
  "family": "eu07"
})json");

        write_data("vehicle-types/locomotive/electric/201e.json", R"json({
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
  "multipleCouplingCapable": true,
  "family": "et22"
})json");

        write_data("vehicles/locomotive/electric/eu07/eu07-001/vehicle.json", R"json({
  "uid": 1108101562372,
  "pID": "EU07-001",
  "type_uid": 1103806595075,
  "displayName": "EU07-001"
})json");

        write_data("vehicles/locomotive/electric/et22/et22-001/vehicle.json", R"json({
  "uid": 1108101562369,
  "pID": "ET22-001",
  "type_uid": 1103806595073,
  "displayName": "ET22-001"
})json");

        write_data("trains/freight/test_train.json", R"json({
  "uid": 1112396529666,
  "pID": "Test",
  "displayName": "Test",
  "trainCategory": "FREIGHT",
  "vehicle_uids": [
    1108101562372,
    1108101562369
  ]
})json");
    }
    else
    {
        const bool include_capability = param.scenario == CouplingScenario::SameTypeCapable;
        const char* capability_field =
            include_capability ? "\n  \"multipleCouplingCapable\": true," : "";

        write_data("vehicle-types/locomotive/electric/eu07.json", std::string(R"json({
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
  "tractionForceKN": 280.0,)json") + capability_field +
                                                                      R"json(
  "family": "eu07"
})json");

        write_data("vehicles/locomotive/electric/eu07/eu07-001/vehicle.json", R"json({
  "uid": 1108101562372,
  "pID": "EU07-001",
  "type_uid": 1103806595075,
  "displayName": "EU07-001"
})json");

        write_data("vehicles/locomotive/electric/eu07/eu07-002/vehicle.json", R"json({
  "uid": 1108101562373,
  "pID": "EU07-002",
  "type_uid": 1103806595075,
  "displayName": "EU07-002"
})json");

        write_data("trains/freight/test_train.json", R"json({
  "uid": 1112396529666,
  "pID": "Test",
  "displayName": "Test",
  "trainCategory": "FREIGHT",
  "vehicle_uids": [
    1108101562372,
    1108101562373
  ]
})json");
    }

    const auto registry = load_registry();
    const auto& consist = registry.get_consist(TRN_TEST);

    EXPECT_NEAR(consist.total_traction_kn, param.expected_traction_kn, 0.01f);
    EXPECT_NEAR(consist.total_power_kw, param.expected_power_kw, 0.01f);
}

INSTANTIATE_TEST_SUITE_P(
    CouplingPolicy, FleetRegistryCouplingParamTest,
    ::testing::Values(
        CouplingCase{"SameTypeCapable", CouplingScenario::SameTypeCapable, 560.0f, 4000.0f},
        CouplingCase{"CapabilityUnknown", CouplingScenario::CapabilityUnknown, 280.0f, 2000.0f},
        CouplingCase{"DifferentTypesCapable", CouplingScenario::DifferentTypesCapable, 280.0f,
                     2000.0f}),
    tests::common::param_name<CouplingCase>);

}  // namespace
