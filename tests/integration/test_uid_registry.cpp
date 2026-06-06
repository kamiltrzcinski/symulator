// tests/integration/test_uid_registry.cpp
//
// Integration tests for the UID registry — verifies that data files and
// topology use consistent, valid, non-duplicate UIDs.
//
// These tests load files from the repository tree (data/ and scenarios/).
// They do not require a PostgreSQL instance.
//
// Labelled "integration" so they run in the same CTest pass as DB tests,
// but they only need the source tree to be present.

#include <gtest/gtest.h>

#include <engine/core/types.hpp>

#include <filesystem>
#include <fstream>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

using namespace engine::core;
namespace fs = std::filesystem;

// Path to repository root — injected by CMake.
#ifndef REPO_ROOT
#error "REPO_ROOT must be defined by CMake"
#endif

namespace
{

using json = nlohmann::json;

json read_json(const fs::path& p)
{
    std::ifstream f(p);
    if (!f)
        throw std::runtime_error("Cannot open: " + p.string());
    json j;
    f >> j;
    return j;
}

struct StationEntry
{
    int instance;
    std::string code;
    std::string name;
};

std::vector<StationEntry> load_stations()
{
    const auto path = fs::path(REPO_ROOT) / "scenarios" / "stations.json";
    const json j = read_json(path);
    std::vector<StationEntry> result;
    for (const auto& entry : j)
        result.push_back({entry.at("instance").get<int>(), entry.at("code").get<std::string>(),
                          entry.at("name").get<std::string>()});
    return result;
}

constexpr std::uint64_t MAX_SAFE = (1ULL << 53) - 1ULL;

bool is_valid_uid(std::uint64_t v)
{
    if (v == 0 || v > MAX_SAFE)
        return false;
    const auto domain = (v >> 40) & 0xFF;
    const auto kind = (v >> 32) & 0xFF;
    const auto instance = v & 0xFFFF;
    if (domain == 0 || domain > 0x03)
        return false;
    static const std::set<uint8_t> valid_kinds = {
        0x01, 0x02, 0x03, 0x04, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
        0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x21, 0x22, 0x23,
    };
    if (!valid_kinds.count(static_cast<uint8_t>(kind)))
        return false;
    if (instance == 0)
        return false;
    return true;
}

}  // namespace

// ── stations.json ─────────────────────────────────────────────────────────────

TEST(UidRegistry, StationsJsonExists)
{
    const auto path = fs::path(REPO_ROOT) / "scenarios" / "stations.json";
    EXPECT_TRUE(fs::exists(path)) << "scenarios/stations.json not found";
}

TEST(UidRegistry, StationsHaveUniqueInstances)
{
    const auto stations = load_stations();
    ASSERT_FALSE(stations.empty());

    std::set<int> seen_instances;
    std::set<std::string> seen_codes;
    for (const auto& s : stations)
    {
        EXPECT_TRUE(seen_instances.insert(s.instance).second)
            << "Duplicate station instance: " << s.instance;
        EXPECT_TRUE(seen_codes.insert(s.code).second) << "Duplicate station code: " << s.code;
        EXPECT_GT(s.instance, 0) << "Station instance must be positive";
    }
}

// ── vehicle_types ─────────────────────────────────────────────────────────────

TEST(UidRegistry, VehicleTypesHaveValidUids)
{
    const auto types_dir = fs::path(REPO_ROOT) / "packages" / "vehicle-types";
    ASSERT_TRUE(fs::exists(types_dir))
        << "packages/vehicle-types not found — run scripts/fetch_packages.py";

    std::set<std::uint64_t> seen;
    int count = 0;
    for (const auto& entry : fs::recursive_directory_iterator(types_dir))
    {
        if (entry.path().extension() != ".json")
            continue;
        const json j = read_json(entry.path());
        if (!j.contains("uid"))
            continue;

        const auto uid_val = j.at("uid").get<std::uint64_t>();
        EXPECT_TRUE(is_valid_uid(uid_val)) << "Invalid UID " << uid_val << " in " << entry.path();
        EXPECT_TRUE(seen.insert(uid_val).second)
            << "Duplicate UID " << uid_val << " in " << entry.path();
        EXPECT_EQ(uid_kind(UID{uid_val}), UIDKind::VEHICLE_TYPE)
            << "Wrong KIND for vehicle_type in " << entry.path();
        ++count;
    }
    EXPECT_GT(count, 0) << "No vehicle type files found";
}

// ── vehicles ──────────────────────────────────────────────────────────────────

TEST(UidRegistry, VehiclesHaveValidUids)
{
    const auto vehicles_dir = fs::path(REPO_ROOT) / "packages" / "vehicles";
    ASSERT_TRUE(fs::exists(vehicles_dir))
        << "packages/vehicles not found — run scripts/fetch_packages.py";

    std::set<std::uint64_t> seen;
    int count = 0;
    for (const auto& entry : fs::recursive_directory_iterator(vehicles_dir))
    {
        if (entry.path().filename() != "vehicle.json")
            continue;
        const json j = read_json(entry.path());
        ASSERT_TRUE(j.contains("uid")) << "Missing uid in " << entry.path();

        const auto uid_val = j.at("uid").get<std::uint64_t>();
        EXPECT_TRUE(is_valid_uid(uid_val)) << "Invalid UID " << uid_val << " in " << entry.path();
        EXPECT_TRUE(seen.insert(uid_val).second)
            << "Duplicate UID " << uid_val << " in " << entry.path();
        EXPECT_EQ(uid_kind(UID{uid_val}), UIDKind::VEHICLE)
            << "Wrong KIND for vehicle in " << entry.path();
        ++count;
    }
    EXPECT_GT(count, 0) << "No vehicle files found";
}

TEST(UidRegistry, VehicleTypeUidsReferencedInVehicles)
{
    const auto types_dir = fs::path(REPO_ROOT) / "packages" / "vehicle-types";
    const auto vehicles_dir = fs::path(REPO_ROOT) / "packages" / "vehicles";
    ASSERT_TRUE(fs::exists(types_dir) && fs::exists(vehicles_dir));

    std::set<std::uint64_t> known_types;
    for (const auto& entry : fs::recursive_directory_iterator(types_dir))
    {
        if (entry.path().extension() != ".json")
            continue;
        const json j = read_json(entry.path());
        if (j.contains("uid"))
            known_types.insert(j.at("uid").get<std::uint64_t>());
    }

    for (const auto& entry : fs::recursive_directory_iterator(vehicles_dir))
    {
        if (entry.path().filename() != "vehicle.json")
            continue;
        const json j = read_json(entry.path());
        if (!j.contains("type_uid"))
            continue;
        const auto type_uid = j.at("type_uid").get<std::uint64_t>();
        EXPECT_TRUE(known_types.count(type_uid))
            << "Unknown type_uid " << type_uid << " in " << entry.path();
    }
}

// ── topology ──────────────────────────────────────────────────────────────────

TEST(UidRegistry, TopologyUidsValidAndUnique)
{
    const auto scenarios_dir = fs::path(REPO_ROOT) / "scenarios";
    ASSERT_TRUE(fs::exists(scenarios_dir));

    for (const auto& scenario : fs::directory_iterator(scenarios_dir))
    {
        if (!scenario.is_directory())
            continue;

        const auto topo_path = scenario.path() / "topology.json";
        if (!fs::exists(topo_path))
            continue;

        const json topo = read_json(topo_path);
        std::set<std::uint64_t> seen_in_file;

        auto check_items =
            [&](const json& items, UIDKind expected_kind, const std::string& kind_name)
        {
            for (const auto& item : items)
            {
                if (!item.contains("uid"))
                    continue;
                const auto uid_val = item.at("uid").get<std::uint64_t>();
                EXPECT_TRUE(is_valid_uid(uid_val))
                    << "Invalid UID " << uid_val << " (" << kind_name << ") in " << topo_path;
                EXPECT_EQ(uid_kind(UID{uid_val}), expected_kind)
                    << "Wrong KIND for " << kind_name << " uid=" << uid_val << " in " << topo_path;
                EXPECT_TRUE(seen_in_file.insert(uid_val).second)
                    << "Duplicate UID " << uid_val << " in " << topo_path;
            }
        };

        check_items(topo.value("boundary_nodes", json::array()), UIDKind::BOUNDARY_NODE,
                    "boundary_node");
        check_items(topo.value("track_sections", json::array()), UIDKind::TRACK_SECTION,
                    "track_section");
        check_items(topo.value("switches", json::array()), UIDKind::SWITCH, "switch");

        const auto objects_path = scenario.path() / "objects.json";
        if (fs::exists(objects_path))
        {
            const json objs = read_json(objects_path);
            check_items(objs.value("signals", json::array()), UIDKind::SIGNAL, "signal");
            check_items(objs.value("derailers", json::array()), UIDKind::DERAILER, "derailer");
        }
    }
}

TEST(UidRegistry, TopologyScopesMatchStations)
{
    const auto stations = load_stations();
    std::set<int> known_instances;
    for (const auto& s : stations)
        known_instances.insert(s.instance);

    const auto scenarios_dir = fs::path(REPO_ROOT) / "scenarios";
    for (const auto& scenario : fs::directory_iterator(scenarios_dir))
    {
        if (!scenario.is_directory())
            continue;
        const auto topo_path = scenario.path() / "topology.json";
        if (!fs::exists(topo_path))
            continue;

        const json topo = read_json(topo_path);
        auto check_scope = [&](const json& items)
        {
            for (const auto& item : items)
            {
                if (!item.contains("uid"))
                    continue;
                const auto uid_val = item.at("uid").get<std::uint64_t>();
                if (!is_valid_uid(uid_val))
                    continue;
                const int scope = static_cast<int>((uid_val >> 16) & 0xFFFF);
                EXPECT_TRUE(known_instances.count(scope))
                    << "INFRA UID " << uid_val << " has unknown scope (station instance) " << scope
                    << " in " << topo_path;
            }
        };

        check_scope(topo.value("boundary_nodes", json::array()));
        check_scope(topo.value("track_sections", json::array()));
        check_scope(topo.value("switches", json::array()));
    }
}
