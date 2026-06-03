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

// UID constants for test data (as specified in the task description).
// VehicleType UIDs: make_uid(RS, VEHICLE_TYPE, 0, N)
constexpr UID VT_201E = UID{0x010100000001ULL};
constexpr UID VT_452W = UID{0x010100000002ULL};
constexpr UID VT_EU07 = UID{0x010100000003ULL};
constexpr UID VT_EN57 = UID{0x010100000004ULL};
constexpr UID VT_SM42 = UID{0x010100000005ULL};

// Vehicle UIDs: make_uid(RS, VEHICLE, 0, N)
constexpr UID VEH_ET22_001 = UID{0x010200000001ULL};
constexpr UID VEH_452W_537_0001 = UID{0x010200000002ULL};
constexpr UID VEH_452W_537_0002 = UID{0x010200000003ULL};
constexpr UID VEH_EU07_001 = UID{0x010200000004ULL};
constexpr UID VEH_EU07_002 = UID{0x010200000005ULL};
constexpr UID VEH_EN57_001 = UID{0x010200000006ULL};
constexpr UID VEH_ET22_002 = UID{0x010200000007ULL};
constexpr UID VEH_SM42_001 = UID{0x010200000008ULL};

// TrainConsist UIDs: make_uid(RS, TRAIN_CONSIST, 0, N)
constexpr UID TRN_TOW543210 = UID{0x010300000001ULL};
constexpr UID TRN_TEST = UID{0x010300000002ULL};
constexpr UID TRN_TEST_CARRIER = UID{0x010300000003ULL};
constexpr UID TRN_NULL_CARRIER = UID{0x010300000004ULL};
constexpr UID TRN_MISS_CARRIER = UID{0x010300000005ULL};
constexpr UID TRN_BAD_CARRIER = UID{0x010300000006ULL};

}  // namespace

TEST(UID, EncodesDomainKindTypeAndItem)
{
    constexpr UID en57_1120 =
        make_uid(UIDDomain::ROLLING_STOCK, UIDKind::VEHICLE, 0x0001, 1120);

    EXPECT_EQ(en57_1120.value, 0x010200010460ULL);
    EXPECT_EQ(uid_domain(en57_1120), UIDDomain::ROLLING_STOCK);
    EXPECT_EQ(uid_kind(en57_1120), UIDKind::VEHICLE);
    EXPECT_EQ(uid_scope(en57_1120), 0x0001);
    EXPECT_EQ(uid_instance(en57_1120), 1120);
    EXPECT_TRUE(uid_is_safe_json_integer(en57_1120));
}

TEST(FleetRegistry, LoadsRecursiveAndBuildsDerivedConsist)
{
    TempDir tmp;
    const auto root = tmp.path() / "data";

    write_text(root / "vehicle_types/locomotive/electric/201e.json", R"json({
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

    write_text(root / "vehicle_types/freight_wagon/hopper/452w.json", R"json({
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

    write_text(root / "vehicles/locomotive/electric/et22/et22-001/vehicle.json", R"json({
  "uid": 1108101562369,
  "pID": "ET22-001",
  "type_uid": 1103806595073,
  "displayName": "ET22-001"
})json");

    write_text(root / "vehicles/freight_wagon/hopper/452w/452w-5375001/vehicle.json", R"json({
  "uid": 1108101562370,
  "pID": "452W-5375001",
  "type_uid": 1103806595074,
  "displayName": "452W-5375001"
})json");

    write_text(root / "vehicles/freight_wagon/hopper/452w/452w-5375002/vehicle.json", R"json({
  "uid": 1108101562371,
  "pID": "452W-5375002",
  "type_uid": 1103806595074,
  "displayName": "452W-5375002",
  "massGrossT": 22.0
})json");

    write_text(root / "trains/freight/tow54321.json", R"json({
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

    FleetRegistry registry;
    ASSERT_NO_THROW(registry.load(root));

    const auto& type = registry.get_type(VT_201E);
    EXPECT_EQ(type.pkp_series, "ET22");
    EXPECT_EQ(type.family, "et22");

    const auto& wagon_2 = registry.get_vehicle(VEH_452W_537_0002);
    EXPECT_NEAR(wagon_2.effective_mass_t, 22.0f, 0.001f);

    const auto& consist = registry.get_consist(TRN_TOW543210);
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

    write_text(root / "vehicles/locomotive/electric/eu07/eu07-001/vehicle.json", R"json({
  "uid": 1108101562372,
  "pID": "EU07-001",
  "type_uid": 1103806595075,
  "displayName": "EU07-001"
})json");

    write_text(root / "trains/freight/test_train.json", R"json({
  "uid": 1112396529666,
  "pID": "Test",
  "displayName": "Test",
  "trainCategory": "FREIGHT",
  "vehicle_uids": [1108101562372]
})json");

    FleetRegistry registry;
    ASSERT_NO_THROW(registry.load(root));

    const auto& type = registry.get_type(VT_EU07);
    EXPECT_NEAR(type.davis.a, 39.24f, 0.001f);
    EXPECT_NEAR(type.davis.b, 0.1962f, 0.0001f);
    EXPECT_NEAR(type.davis.c, 0.0017658f, 0.000001f);
}

TEST(FleetRegistry, IgnoresVehicleSidecarJsonFiles)
{
    TempDir tmp;
    const auto root = tmp.path() / "data";

    write_text(root / "vehicle_types/locomotive/electric/eu07.json", R"json({
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

    write_text(root / "vehicles/locomotive/electric/eu07/eu07-001/vehicle.json", R"json({
  "uid": 1108101562372,
  "pID": "EU07-001",
  "type_uid": 1103806595075,
  "displayName": "EU07-001"
})json");

    write_text(root / "vehicles/locomotive/electric/eu07/eu07-001/photos/metadata.json", R"json({
  "caption": "Front view"
})json");

    write_text(root / "trains/freight/test_train.json", R"json({
  "uid": 1112396529666,
  "pID": "Test",
  "displayName": "Test",
  "trainCategory": "FREIGHT",
  "vehicle_uids": [1108101562372]
})json");

    FleetRegistry registry;
    ASSERT_NO_THROW(registry.load(root));
    // The vehicle uid is VEH_EU07_001 = 0x010200000004ULL = 1108101562372
    EXPECT_TRUE(registry.has_vehicle(VEH_EU07_001));
}

TEST(FleetRegistry, AcceptsMultipleCouplingCapabilityForEmuMotorType)
{
    TempDir tmp;
    const auto root = tmp.path() / "data";

    write_text(root / "vehicle_types/emu_unit/motor/en57.json", R"json({
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

    write_text(root / "vehicles/emu_unit/motor/en57/en57-001/vehicle.json", R"json({
  "uid": 1108101562374,
  "pID": "EN57-001",
  "type_uid": 1103806595076,
  "displayName": "EN57-001"
})json");

    write_text(root / "trains/passenger/test_train.json", R"json({
  "uid": 1112396529666,
  "pID": "Test",
  "displayName": "Test",
  "trainCategory": "PASSENGER",
  "vehicle_uids": [1108101562374]
})json");

    FleetRegistry registry;
    ASSERT_NO_THROW(registry.load(root));

    const auto& type = registry.get_type(VT_EN57);
    ASSERT_TRUE(type.multiple_coupling_capable.has_value());
    EXPECT_TRUE(*type.multiple_coupling_capable);
}

TEST(FleetRegistry, RejectsMultipleCouplingCapabilityForNonTractionType)
{
    TempDir tmp;
    const auto root = tmp.path() / "data";

    write_text(root / "vehicle_types/freight_wagon/hopper/452w.json", R"json({
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
    EXPECT_THROW(registry.load(root), FleetLoadError);
}

TEST(FleetRegistry, DefaultsTractionStatusAndAppliesDefectiveAsBallast)
{
    TempDir tmp;
    const auto root = tmp.path() / "data";

    write_text(root / "vehicle_types/locomotive/electric/201e.json", R"json({
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

    write_text(root / "vehicle_types/freight_wagon/hopper/452w.json", R"json({
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

    write_text(root / "vehicles/locomotive/electric/et22/et22-001/vehicle.json", R"json({
  "uid": 1108101562369,
  "pID": "ET22-001",
  "type_uid": 1103806595073,
  "displayName": "ET22-001"
})json");

    write_text(root / "vehicles/locomotive/electric/et22/et22-002/vehicle.json", R"json({
  "uid": 1108101562375,
  "pID": "ET22-002",
  "type_uid": 1103806595073,
  "displayName": "ET22-002",
  "tractionStatus": "DEFECTIVE"
})json");

    write_text(root / "vehicles/freight_wagon/hopper/452w/452w-5375001/vehicle.json", R"json({
  "uid": 1108101562370,
  "pID": "452W-5375001",
  "type_uid": 1103806595074,
  "displayName": "452W-5375001"
})json");

    write_text(root / "trains/freight/tow54321.json", R"json({
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

    FleetRegistry registry;
    ASSERT_NO_THROW(registry.load(root));

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

TEST(FleetRegistry, CouplesOperationalLocomotivesWhenSameTypeAndCapable)
{
    TempDir tmp;
    const auto root = tmp.path() / "data";

    write_text(root / "vehicle_types/locomotive/electric/eu07.json", R"json({
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

    write_text(root / "vehicles/locomotive/electric/eu07/eu07-001/vehicle.json", R"json({
  "uid": 1108101562372,
  "pID": "EU07-001",
  "type_uid": 1103806595075,
  "displayName": "EU07-001"
})json");

    write_text(root / "vehicles/locomotive/electric/eu07/eu07-002/vehicle.json", R"json({
  "uid": 1108101562373,
  "pID": "EU07-002",
  "type_uid": 1103806595075,
  "displayName": "EU07-002"
})json");

    write_text(root / "trains/freight/test_train.json", R"json({
  "uid": 1112396529666,
  "pID": "Test",
  "displayName": "Test",
  "trainCategory": "FREIGHT",
  "vehicle_uids": [
    1108101562372,
    1108101562373
  ]
})json");

    FleetRegistry registry;
    ASSERT_NO_THROW(registry.load(root));

    const auto& consist = registry.get_consist(TRN_TEST);
    EXPECT_NEAR(consist.total_traction_kn, 560.0f, 0.01f);
    EXPECT_NEAR(consist.total_power_kw, 4000.0f, 0.01f);
}

TEST(FleetRegistry, DoesNotCoupleLocomotivesWhenCapabilityUnknown)
{
    TempDir tmp;
    const auto root = tmp.path() / "data";

    write_text(root / "vehicle_types/locomotive/electric/eu07.json", R"json({
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

    write_text(root / "vehicles/locomotive/electric/eu07/eu07-001/vehicle.json", R"json({
  "uid": 1108101562372,
  "pID": "EU07-001",
  "type_uid": 1103806595075,
  "displayName": "EU07-001"
})json");

    write_text(root / "vehicles/locomotive/electric/eu07/eu07-002/vehicle.json", R"json({
  "uid": 1108101562373,
  "pID": "EU07-002",
  "type_uid": 1103806595075,
  "displayName": "EU07-002"
})json");

    write_text(root / "trains/freight/test_train.json", R"json({
  "uid": 1112396529666,
  "pID": "Test",
  "displayName": "Test",
  "trainCategory": "FREIGHT",
  "vehicle_uids": [
    1108101562372,
    1108101562373
  ]
})json");

    FleetRegistry registry;
    ASSERT_NO_THROW(registry.load(root));

    const auto& consist = registry.get_consist(TRN_TEST);
    EXPECT_NEAR(consist.total_traction_kn, 280.0f, 0.01f);
    EXPECT_NEAR(consist.total_power_kw, 2000.0f, 0.01f);
}

TEST(FleetRegistry, DoesNotCoupleLocomotivesAcrossDifferentTypes)
{
    TempDir tmp;
    const auto root = tmp.path() / "data";

    write_text(root / "vehicle_types/locomotive/electric/eu07.json", R"json({
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

    write_text(root / "vehicle_types/locomotive/electric/201e.json", R"json({
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

    write_text(root / "vehicles/locomotive/electric/eu07/eu07-001/vehicle.json", R"json({
  "uid": 1108101562372,
  "pID": "EU07-001",
  "type_uid": 1103806595075,
  "displayName": "EU07-001"
})json");

    write_text(root / "vehicles/locomotive/electric/et22/et22-001/vehicle.json", R"json({
  "uid": 1108101562369,
  "pID": "ET22-001",
  "type_uid": 1103806595073,
  "displayName": "ET22-001"
})json");

    write_text(root / "trains/freight/test_train.json", R"json({
  "uid": 1112396529666,
  "pID": "Test",
  "displayName": "Test",
  "trainCategory": "FREIGHT",
  "vehicle_uids": [
    1108101562372,
    1108101562369
  ]
})json");

    FleetRegistry registry;
    ASSERT_NO_THROW(registry.load(root));

    const auto& consist = registry.get_consist(TRN_TEST);
    EXPECT_NEAR(consist.total_traction_kn, 280.0f, 0.01f);
    EXPECT_NEAR(consist.total_power_kw, 2000.0f, 0.01f);
}

TEST(FleetRegistry, ThrowsOnUnknownTypeReference)
{
    TempDir tmp;
    const auto root = tmp.path() / "data";
    create_minimal_tree(root);

    write_text(root / "vehicles/locomotive/electric/bad/bad-001/vehicle.json", R"json({
  "uid": 1108101562376,
  "pID": "BAD",
  "type_uid": 99999999999999,
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

    write_text(root / "vehicles/locomotive/diesel/sm42/sm42-001/vehicle.json", R"json({
  "uid": 1108101562376,
  "pID": "SM42-001",
  "type_uid": 1103806595077,
  "displayName": "SM42-001"
})json");

    write_text(root / "trains/freight/missing_category.json", R"json({
  "uid": 1112396529666,
  "pID": "Test",
  "displayName": "Test",
  "vehicle_uids": [1108101562376]
})json");

    FleetRegistry registry;
    EXPECT_THROW(registry.load(root), FleetLoadError);
}

TEST(FleetRegistry, ThrowsOnTrainCategoryFolderMismatch)
{
    TempDir tmp;
    const auto root = tmp.path() / "data";

    write_text(root / "vehicle_types/locomotive/diesel/sm42.json", R"json({
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

    write_text(root / "vehicles/locomotive/diesel/sm42/sm42-001/vehicle.json", R"json({
  "uid": 1108101562376,
  "pID": "SM42-001",
  "type_uid": 1103806595077,
  "displayName": "SM42-001"
})json");

    write_text(root / "trains/freight/mismatch.json", R"json({
  "uid": 1112396529666,
  "pID": "Test",
  "displayName": "Test",
  "trainCategory": "PASSENGER",
  "vehicle_uids": [1108101562376]
})json");

    FleetRegistry registry;
    EXPECT_THROW(registry.load(root), FleetLoadError);
}

TEST(FleetRegistry, LoadsCarriersFromFile)
{
    TempDir tmp;
    const auto root = tmp.path() / "data";
    create_minimal_tree(root);

    write_text(root / "carriers.json", R"json({
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

    FleetRegistry registry;
    ASSERT_NO_THROW(registry.load(root));

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

TEST(FleetRegistry, AcceptsValidCarrierInTrain)
{
    TempDir tmp;
    const auto root = tmp.path() / "data";

    write_text(root / "carriers.json", R"json({
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

    write_text(root / "vehicle_types/locomotive/diesel/sm42.json", R"json({
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

    write_text(root / "vehicles/locomotive/diesel/sm42/sm42-001/vehicle.json", R"json({
  "uid": 1108101562376,
  "pID": "SM42-001",
  "type_uid": 1103806595077,
  "displayName": "SM42-001"
})json");

    write_text(root / "trains/freight/test_carrier.json", R"json({
  "uid": 1112396529667,
  "pID": "TestCarrier",
  "displayName": "Test Carrier Train",
  "trainCategory": "FREIGHT",
  "carrierId": 1116691496961,
  "vehicle_uids": [1108101562376]
})json");

    FleetRegistry registry;
    ASSERT_NO_THROW(registry.load(root));

    const auto& consist = registry.get_consist(TRN_TEST_CARRIER);
    ASSERT_TRUE(consist.carrier_id.has_value());
    EXPECT_EQ(consist.carrier_id->value, 1116691496961ULL);
}

TEST(FleetRegistry, ThrowsOnUnknownCarrier)
{
    TempDir tmp;
    const auto root = tmp.path() / "data";

    write_text(root / "carriers.json", R"json({
  "carriers": [
    {
      "id": 1116691496961,
      "name": "KNOWN-CARRIER",
      "type": ["freight"],
      "logo": null
    }
  ]
})json");

    write_text(root / "vehicle_types/locomotive/diesel/sm42.json", R"json({
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

    write_text(root / "vehicles/locomotive/diesel/sm42/sm42-001/vehicle.json", R"json({
  "uid": 1108101562376,
  "pID": "SM42-001",
  "type_uid": 1103806595077,
  "displayName": "SM42-001"
})json");

    write_text(root / "trains/freight/bad_carrier.json", R"json({
  "uid": 1112396529668,
  "pID": "BadCarrier",
  "displayName": "Bad Carrier Train",
  "trainCategory": "FREIGHT",
  "carrierId": 1116691496962,
  "vehicle_uids": [1108101562376]
})json");

    FleetRegistry registry;
    EXPECT_THROW(registry.load(root), FleetLoadError);
}

TEST(FleetRegistry, AllowsNullOrMissingCarrierField)
{
    TempDir tmp;
    const auto root = tmp.path() / "data";

    write_text(root / "carriers.json", R"json({
  "carriers": [
    {
      "id": 1116691496961,
      "name": "KNOWN-CARRIER",
      "type": ["freight"],
      "logo": null
    }
  ]
})json");

    write_text(root / "vehicle_types/locomotive/diesel/sm42.json", R"json({
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

    write_text(root / "vehicles/locomotive/diesel/sm42/sm42-001/vehicle.json", R"json({
  "uid": 1108101562376,
  "pID": "SM42-001",
  "type_uid": 1103806595077,
  "displayName": "SM42-001"
})json");

    write_text(root / "trains/freight/null_carrier.json", R"json({
  "uid": 1112396529668,
  "pID": "NullCarrier",
  "displayName": "Null Carrier Train",
  "trainCategory": "FREIGHT",
  "carrierId": null,
  "vehicle_uids": [1108101562376]
})json");

    write_text(root / "trains/freight/missing_carrier.json", R"json({
  "uid": 1112396529669,
  "pID": "MissingCarrier",
  "displayName": "Missing Carrier Train",
  "trainCategory": "FREIGHT",
  "vehicle_uids": [1108101562376]
})json");

    FleetRegistry registry;
    ASSERT_NO_THROW(registry.load(root));

    const auto& null_consist = registry.get_consist(TRN_NULL_CARRIER);
    EXPECT_FALSE(null_consist.carrier_id.has_value());

    const auto& missing_consist = registry.get_consist(TRN_MISS_CARRIER);
    EXPECT_FALSE(missing_consist.carrier_id.has_value());
}

TEST(FleetRegistry, ThrowsOnInvalidCarriersJsonFormat)
{
    TempDir tmp;
    const auto root = tmp.path() / "data";
    create_minimal_tree(root);

    write_text(root / "carriers.json", R"json([
  "CARRIER-A"
])json");

    FleetRegistry registry;
    EXPECT_THROW(registry.load(root), FleetLoadError);
}

TEST(FleetRegistry, ThrowsOnNonStringCarrierEntry)
{
    TempDir tmp;
    const auto root = tmp.path() / "data";
    create_minimal_tree(root);

    write_text(root / "carriers.json", R"json({
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

    FleetRegistry registry;
    EXPECT_THROW(registry.load(root), FleetLoadError);
}
