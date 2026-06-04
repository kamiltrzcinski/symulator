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

class FleetRegistryCarrierTest : public FleetDataFixture
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

    void write_known_carrier_file()
    {
        write_data("carriers.json", R"json({
  "carriers": [
    {
      "id": 1116691496961,
      "name": "KNOWN-CARRIER",
      "type": ["freight"],
      "logo": null
    }
  ]
})json");
    }
};

TEST_F(FleetRegistryCarrierTest, LoadsCarriersFromFile)
{
    create_minimal_tree();
    write_data("carriers.json", R"json({
  "carriers": [
    {
      "id": 1116691496961,
      "name": "CARRIER-A",
      "type": ["freight"],
      "logo": null
    },
    {
      "id": 1116691496962,
      "name": "CARRIER-B",
      "type": ["passenger"],
      "logo": null
    },
    {
      "id": 1116691496963,
      "name": "CARRIER-C",
      "type": ["passenger", "freight"],
      "logo": "logos/carrier-c.svg"
    }
  ]
})json");

    const auto registry = load_registry();
    const auto& carriers = registry.all_carriers();

    ASSERT_EQ(carriers.size(), 3u);
    ASSERT_TRUE(registry.has_carrier(UID{1116691496961}));
    ASSERT_TRUE(registry.has_carrier(UID{1116691496962}));
    ASSERT_TRUE(registry.has_carrier(UID{1116691496963}));
    EXPECT_EQ(carriers.at(UID{1116691496961}).name, "CARRIER-A");
    EXPECT_EQ(carriers.at(UID{1116691496962}).service_types[0], "passenger");
    ASSERT_TRUE(carriers.at(UID{1116691496963}).logo.has_value());
    EXPECT_EQ(*carriers.at(UID{1116691496963}).logo, "logos/carrier-c.svg");
}

enum class CarrierAssignmentScenario
{
    ValidCarrier,
    UnknownCarrier,
};

struct CarrierAssignmentCase
{
    const char* name;
    CarrierAssignmentScenario scenario;
    bool should_throw;
};

class FleetRegistryCarrierAssignmentParamTest
    : public FleetRegistryCarrierTest,
      public ::testing::WithParamInterface<CarrierAssignmentCase>
{
};

TEST_P(FleetRegistryCarrierAssignmentParamTest, ValidatesCarrierReferences)
{
    const auto param = GetParam();

    if (param.scenario == CarrierAssignmentScenario::ValidCarrier)
    {
        write_data("carriers.json", R"json({
  "carriers": [
    {
      "id": 1116691496961,
      "name": "PKP Cargo",
      "type": ["freight"],
      "logo": null
    },
    {
      "id": 1116691496962,
      "name": "CARGO Master",
      "type": ["passenger", "freight"],
      "logo": null
    }
  ]
})json");

        write_sm42_type_and_vehicle();
        write_data("trains/freight/test_carrier.json", R"json({
  "uid": 1112396529667,
  "pID": "TestCarrier",
  "displayName": "Test Carrier Train",
  "trainCategory": "FREIGHT",
  "carrierId": 1116691496961,
  "vehicle_uids": [1108101562376]
})json");
    }
    else
    {
        write_known_carrier_file();
        write_sm42_type_and_vehicle();
        write_data("trains/freight/bad_carrier.json", R"json({
  "uid": 1112396529668,
  "pID": "BadCarrier",
  "displayName": "Bad Carrier Train",
  "trainCategory": "FREIGHT",
  "carrierId": 1116691496962,
  "vehicle_uids": [1108101562376]
})json");
    }

    FleetRegistry registry;
    if (param.should_throw)
    {
        EXPECT_THROW(registry.load(data_root()), FleetLoadError);
        return;
    }

    ASSERT_NO_THROW(registry.load(data_root()));
    const auto& consist = registry.get_consist(TRN_TEST_CARRIER);
    ASSERT_TRUE(consist.carrier_id.has_value());
    EXPECT_EQ(consist.carrier_id->value, 1116691496961ULL);
}

INSTANTIATE_TEST_SUITE_P(
    CarrierAssignment, FleetRegistryCarrierAssignmentParamTest,
    ::testing::Values(
        CarrierAssignmentCase{"ValidCarrier", CarrierAssignmentScenario::ValidCarrier, false},
        CarrierAssignmentCase{"UnknownCarrier", CarrierAssignmentScenario::UnknownCarrier, true}),
    tests::common::param_name<CarrierAssignmentCase>);

TEST_F(FleetRegistryCarrierTest, AllowsNullOrMissingCarrierField)
{
    write_known_carrier_file();
    write_sm42_type_and_vehicle();

    write_data("trains/freight/null_carrier.json", R"json({
  "uid": 1112396529668,
  "pID": "NullCarrier",
  "displayName": "Null Carrier Train",
  "trainCategory": "FREIGHT",
  "carrierId": null,
  "vehicle_uids": [1108101562376]
})json");

    write_data("trains/freight/missing_carrier.json", R"json({
  "uid": 1112396529669,
  "pID": "MissingCarrier",
  "displayName": "Missing Carrier Train",
  "trainCategory": "FREIGHT",
  "vehicle_uids": [1108101562376]
})json");

    const auto registry = load_registry();

    const auto& null_consist = registry.get_consist(TRN_NULL_CARRIER);
    EXPECT_FALSE(null_consist.carrier_id.has_value());

    const auto& missing_consist = registry.get_consist(TRN_MISS_CARRIER);
    EXPECT_FALSE(missing_consist.carrier_id.has_value());
}

enum class InvalidCarrierFileScenario
{
    RootNotObject,
    ContainsNonObjectEntry,
};

struct InvalidCarrierFileCase
{
    const char* name;
    InvalidCarrierFileScenario scenario;
};

class FleetRegistryInvalidCarrierFileTest
    : public FleetRegistryCarrierTest,
      public ::testing::WithParamInterface<InvalidCarrierFileCase>
{
};

TEST_P(FleetRegistryInvalidCarrierFileTest, ThrowsFleetLoadError)
{
    create_minimal_tree();

    if (GetParam().scenario == InvalidCarrierFileScenario::RootNotObject)
    {
        write_data("carriers.json", R"json([
  "CARRIER-A"
])json");
    }
    else
    {
        write_data("carriers.json", R"json({
  "carriers": [
    {
      "id": 1116691496961,
      "name": "CARRIER-A",
      "type": ["freight"],
      "logo": null
    },
    12345
  ]
})json");
    }

    FleetRegistry registry;
    EXPECT_THROW(registry.load(data_root()), FleetLoadError);
}

INSTANTIATE_TEST_SUITE_P(
    InvalidCarrierFiles, FleetRegistryInvalidCarrierFileTest,
    ::testing::Values(InvalidCarrierFileCase{"RootNotObject",
                                             InvalidCarrierFileScenario::RootNotObject},
                      InvalidCarrierFileCase{"ContainsNonObjectEntry",
                                             InvalidCarrierFileScenario::ContainsNonObjectEntry}),
    tests::common::param_name<InvalidCarrierFileCase>);

}  // namespace
