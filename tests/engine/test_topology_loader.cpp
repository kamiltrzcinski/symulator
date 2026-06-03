#include <gtest/gtest.h>

#include <engine/core/engine_state.hpp>
#include <engine/core/topology_loader.hpp>
#include <engine/core/types.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace
{

using namespace engine::core;

// ── Temp directory helper ─────────────────────────────────────────────────────

class TempDir
{
public:
    TempDir()
    {
        const auto nonce = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        path_ = std::filesystem::temp_directory_path() /
                ("symulator_topo_test_" + std::to_string(nonce));
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

// UIDs used in test scenario (scope=1 = station GOr)
// Boundary nodes
constexpr UID kBndN = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::BOUNDARY_NODE, 1, 1);
constexpr UID kBndS = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::BOUNDARY_NODE, 1, 2);
// Track sections
constexpr UID kOtT1a = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::TRACK_SECTION, 1, 1);
constexpr UID kOtT2a = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::TRACK_SECTION, 1, 2);
// Switches
constexpr UID kZwr1 = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::SWITCH, 1, 1);
// Signals
constexpr UID kWp1 = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::SIGNAL, 1, 1);
constexpr UID kWp2 = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::SIGNAL, 1, 2);
constexpr UID kWw1 = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::SIGNAL, 1, 3);
// Derailers
constexpr UID kWk1 = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::DERAILER, 1, 1);
// Axle counters
constexpr UID kItN = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::AXLE_COUNTER, 1, 1);
constexpr UID kItS = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::AXLE_COUNTER, 1, 2);
constexpr UID kIz1t = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::AXLE_COUNTER, 1, 3);
constexpr UID kIz1s = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::AXLE_COUNTER, 1, 4);
constexpr UID kIz1d = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::AXLE_COUNTER, 1, 5);
// Station UID (derived: scope=1, instance=1)
constexpr UID kSta = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::STATION, 1, 1);

// Create a minimal test scenario directory with meta.json + topology.json + objects.json.
std::filesystem::path make_test_scenario()
{
    static TempDir tmp;
    const auto dir = tmp.path() / "scenario";
    std::filesystem::create_directories(dir);

    // meta.json
    write_text(dir / "meta.json", R"json({
  "station_sid": "GOr",
  "control_system": "ebilock_x4",
  "schema_version": 1
})json");

    // topology.json — uses numeric UIDs
    write_text(dir / "topology.json",
               "{\n"
               "  \"boundary_nodes\": [\n"
               "    { \"uid\": " +
                   std::to_string(kBndN.value) +
                   ", \"pID\": \"granica_polnocna\" },\n"
                   "    { \"uid\": " +
                   std::to_string(kBndS.value) +
                   ", \"pID\": \"granica_poludniowa\" }\n"
                   "  ],\n"
                   "  \"track_sections\": [\n"
                   "    {\n"
                   "      \"uid\": " +
                   std::to_string(kOtT1a.value) +
                   ",\n"
                   "      \"pID\": \"t1a\",\n"
                   "      \"lengthM\": 200.0,\n"
                   "      \"electrified\": true,\n"
                   "      \"maxSpeedKmh\": 110,\n"
                   "      \"sideA\": { \"neighborUID\": " +
                   std::to_string(kBndS.value) + ", \"itUID\": " + std::to_string(kItS.value) +
                   ", \"signalUIDs\": [" + std::to_string(kWp1.value) +
                   "] },\n"
                   "      \"sideB\": { \"neighborUID\": " +
                   std::to_string(kZwr1.value) + ", \"izUID\": " + std::to_string(kIz1t.value) +
                   ", \"signalUIDs\": [] }\n"
                   "    },\n"
                   "    {\n"
                   "      \"uid\": " +
                   std::to_string(kOtT2a.value) +
                   ",\n"
                   "      \"pID\": \"t2a\",\n"
                   "      \"lengthM\": 150.0,\n"
                   "      \"electrified\": true,\n"
                   "      \"maxSpeedKmh\": 110,\n"
                   "      \"sideA\": { \"neighborUID\": " +
                   std::to_string(kBndN.value) + ", \"itUID\": " + std::to_string(kItN.value) +
                   " },\n"
                   "      \"sideB\": { \"neighborUID\": " +
                   std::to_string(kZwr1.value) + ", \"izUID\": " + std::to_string(kIz1s.value) +
                   " }\n"
                   "    }\n"
                   "  ],\n"
                   "  \"switches\": [\n"
                   "    {\n"
                   "      \"uid\": " +
                   std::to_string(kZwr1.value) +
                   ",\n"
                   "      \"pID\": \"zwr1\",\n"
                   "      \"typeID\": \"DVT-GLB-ZWR-EEA4-0000002\",\n"
                   "      \"lengthM\": 28.5,\n"
                   "      \"maxSpeedStraightKmh\": 110,\n"
                   "      \"maxSpeedDivergentKmh\": 60,\n"
                   "      \"trunk\":    { \"neighborUID\": " +
                   std::to_string(kOtT1a.value) + ", \"izUID\": " + std::to_string(kIz1t.value) +
                   " },\n"
                   "      \"straight\": { \"neighborUID\": " +
                   std::to_string(kOtT2a.value) + ", \"izUID\": " + std::to_string(kIz1s.value) +
                   " },\n"
                   "      \"divergent\": { \"neighborUID\": " +
                   std::to_string(kBndN.value) + ", \"izUID\": " + std::to_string(kIz1d.value) +
                   " }\n"
                   "    }\n"
                   "  ]\n"
                   "}\n");

    // objects.json
    write_text(dir / "objects.json",
               "{\n"
               "  \"signals\": [\n"
               "    {\n"
               "      \"uid\": " +
                   std::to_string(kWp1.value) +
                   ",\n"
                   "      \"pID\": \"Wp1\",\n"
                   "      \"type\": \"ENTRY\",\n"
                   "      \"governs_section\": " +
                   std::to_string(kOtT1a.value) +
                   ",\n"
                   "      \"initial_aspect\": \"S1_STOP\"\n"
                   "    },\n"
                   "    {\n"
                   "      \"uid\": " +
                   std::to_string(kWp2.value) +
                   ",\n"
                   "      \"pID\": \"Wp2\",\n"
                   "      \"type\": \"ENTRY\",\n"
                   "      \"governs_section\": " +
                   std::to_string(kOtT2a.value) +
                   ",\n"
                   "      \"initial_aspect\": \"S1_STOP\"\n"
                   "    },\n"
                   "    {\n"
                   "      \"uid\": " +
                   std::to_string(kWw1.value) +
                   ",\n"
                   "      \"pID\": \"Ww1\",\n"
                   "      \"type\": \"DEPARTURE\",\n"
                   "      \"governs_section\": " +
                   std::to_string(kOtT1a.value) +
                   ",\n"
                   "      \"initial_aspect\": \"S1_STOP\"\n"
                   "    }\n"
                   "  ],\n"
                   "  \"derailers\": [\n"
                   "    {\n"
                   "      \"uid\": " +
                   std::to_string(kWk1.value) +
                   ",\n"
                   "      \"pID\": \"Wk1\",\n"
                   "      \"guards_section\": " +
                   std::to_string(kOtT1a.value) +
                   "\n"
                   "    }\n"
                   "  ]\n"
                   "}\n");

    return dir;
}

}  // namespace

// ── meta.json ─────────────────────────────────────────────────────────────────

TEST(TopologyLoader, MetaFieldsParsed)
{
    EngineState st;
    const auto meta = load_scenario(st, make_test_scenario());
    EXPECT_EQ(meta.station_sid, "GOr");
    EXPECT_EQ(meta.control_system_id, "ebilock_x4");
    EXPECT_EQ(meta.schema_version, 1);
}

TEST(TopologyLoader, SessionIdSetFromStationSid)
{
    EngineState st;
    load_scenario(st, make_test_scenario());
    EXPECT_EQ(st.session_id(), "GOr");
}

// ── topology.json — boundary nodes ────────────────────────────────────────────

TEST(TopologyLoader, BoundaryNodesLoaded)
{
    EngineState st;
    load_scenario(st, make_test_scenario());
    EXPECT_NE(st.find_boundary_node(kBndN), nullptr);
    EXPECT_NE(st.find_boundary_node(kBndS), nullptr);
}

TEST(TopologyLoader, BoundaryNodeFieldsCorrect)
{
    EngineState st;
    load_scenario(st, make_test_scenario());
    const auto* bn = st.find_boundary_node(kBndN);
    ASSERT_NE(bn, nullptr);
    EXPECT_EQ(bn->pid, "granica_polnocna");
    // station_uid is derived from scope of kBndN
    EXPECT_EQ(bn->station_uid, kSta);
}

// ── topology.json — track sections ────────────────────────────────────────────

TEST(TopologyLoader, TrackSectionsLoaded)
{
    EngineState st;
    load_scenario(st, make_test_scenario());
    EXPECT_NE(st.find_track_section(kOtT1a), nullptr);
    EXPECT_NE(st.find_track_section(kOtT2a), nullptr);
}

TEST(TopologyLoader, TrackSectionFieldsCorrect)
{
    EngineState st;
    load_scenario(st, make_test_scenario());
    const auto* ts = st.find_track_section(kOtT1a);
    ASSERT_NE(ts, nullptr);
    EXPECT_EQ(ts->pid, "t1a");
    EXPECT_EQ(ts->station_uid, kSta);
    EXPECT_FLOAT_EQ(ts->length_m, 200.0f);
    EXPECT_TRUE(ts->electrified);
    EXPECT_EQ(ts->max_speed_kmh, 110);
    EXPECT_EQ(ts->occupancy, TrackOccupancy::FREE);
}

TEST(TopologyLoader, TrackSectionPortsWired)
{
    EngineState st;
    load_scenario(st, make_test_scenario());
    const auto* ts = st.find_track_section(kOtT1a);
    ASSERT_NE(ts, nullptr);
    // sideA neighbors south boundary
    EXPECT_EQ(ts->side_a.neighbor_uid, kBndS);
    EXPECT_EQ(ts->side_a.counter_kind, TrackPort::CounterKind::IT);
    // sideB neighbors switch zwr1
    EXPECT_EQ(ts->side_b.neighbor_uid, kZwr1);
    EXPECT_EQ(ts->side_b.counter_kind, TrackPort::CounterKind::IZ);
}

TEST(TopologyLoader, TrackSectionSignalRefsCorrect)
{
    EngineState st;
    load_scenario(st, make_test_scenario());
    const auto* ts = st.find_track_section(kOtT1a);
    ASSERT_NE(ts, nullptr);
    // sideA has entry signal Wp1
    ASSERT_EQ(ts->side_a.signal_uids.size(), 1u);
    EXPECT_EQ(ts->side_a.signal_uids[0], kWp1);
}

// ── topology.json — switches ──────────────────────────────────────────────────

TEST(TopologyLoader, SwitchesLoaded)
{
    EngineState st;
    load_scenario(st, make_test_scenario());
    EXPECT_NE(st.find_switch(kZwr1), nullptr);
}

TEST(TopologyLoader, SwitchFieldsCorrect)
{
    EngineState st;
    load_scenario(st, make_test_scenario());
    const auto* sw = st.find_switch(kZwr1);
    ASSERT_NE(sw, nullptr);
    EXPECT_EQ(sw->pid, "zwr1");
    EXPECT_EQ(sw->type_id, "DVT-GLB-ZWR-EEA4-0000002");
    EXPECT_FLOAT_EQ(sw->length_m, 28.5f);
    EXPECT_EQ(sw->max_speed_straight_kmh, 110);
    EXPECT_EQ(sw->max_speed_divergent_kmh, 60);
    EXPECT_EQ(sw->position, SwitchPosition::STRAIGHT);
    EXPECT_FALSE(sw->locked_by_route_uid.has_value());
}

TEST(TopologyLoader, SwitchLegsWired)
{
    EngineState st;
    load_scenario(st, make_test_scenario());
    const auto* sw = st.find_switch(kZwr1);
    ASSERT_NE(sw, nullptr);
    EXPECT_EQ(sw->trunk.neighbor_uid, kOtT1a);
    EXPECT_EQ(sw->straight.neighbor_uid, kOtT2a);
    EXPECT_EQ(sw->divergent.neighbor_uid, kBndN);
}

// ── objects.json — signals ────────────────────────────────────────────────────

TEST(TopologyLoader, SignalsLoaded)
{
    EngineState st;
    load_scenario(st, make_test_scenario());
    EXPECT_NE(st.find_signal(kWp1), nullptr);
    EXPECT_NE(st.find_signal(kWp2), nullptr);
    EXPECT_NE(st.find_signal(kWw1), nullptr);
}

TEST(TopologyLoader, EntrySignalFieldsCorrect)
{
    EngineState st;
    load_scenario(st, make_test_scenario());
    const auto* sig = st.find_signal(kWp1);
    ASSERT_NE(sig, nullptr);
    EXPECT_EQ(sig->pid, "Wp1");
    EXPECT_EQ(sig->station_uid, kSta);
    EXPECT_EQ(sig->type, Signal::Type::ENTRY);
    EXPECT_EQ(sig->governs_section_uid, kOtT1a);
    EXPECT_EQ(sig->current_aspect, SignalAspect::S1_STOP);
    EXPECT_FALSE(sig->locked_by_route_uid.has_value());
}

TEST(TopologyLoader, DepartureSignalType)
{
    EngineState st;
    load_scenario(st, make_test_scenario());
    const auto* sig = st.find_signal(kWw1);
    ASSERT_NE(sig, nullptr);
    EXPECT_EQ(sig->type, Signal::Type::DEPARTURE);
}

// ── Error handling ────────────────────────────────────────────────────────────

TEST(TopologyLoader, ThrowsOnMissingDirectory)
{
    EngineState st;
    EXPECT_THROW(load_scenario(st, "/nonexistent/path/to/scenario"), std::runtime_error);
}
