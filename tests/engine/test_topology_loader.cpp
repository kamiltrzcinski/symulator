#include <gtest/gtest.h>

#include <engine/core/engine_state.hpp>
#include <engine/core/topology_loader.hpp>
#include <engine/core/types.hpp>

#include <filesystem>
#include <stdexcept>

namespace
{

using namespace engine::core;

// Path to the reference scenario directory (resolved relative to the repo root).
// CTest is launched from the build directory, and the repo root is two levels up.
static std::filesystem::path scenario_dir()
{
    // CMake sets SCENARIO_DIR via target_compile_definitions — fall back to relative path.
#ifdef SCENARIO_DIR
    return std::filesystem::path(SCENARIO_DIR);
#else
    // Assume build dir is <repo>/build/ — go two levels up.
    return std::filesystem::path(__FILE__).parent_path().parent_path().parent_path() / "scenarios" /
           "reference" / "gdynia_orlowo";
#endif
}

// ── meta.json ─────────────────────────────────────────────────────────────────

TEST(TopologyLoader, MetaFieldsParsed)
{
    EngineState st;
    const auto meta = load_scenario(st, scenario_dir());
    EXPECT_EQ(meta.station_sid, "GOr");
    EXPECT_EQ(meta.control_system_id, "ebilock_x4");
    EXPECT_EQ(meta.schema_version, 1);
}

TEST(TopologyLoader, SessionIdSetFromStationSid)
{
    EngineState st;
    load_scenario(st, scenario_dir());
    EXPECT_EQ(st.session_id(), "GOr");
}

// ── topology.json — boundary nodes ────────────────────────────────────────────

TEST(TopologyLoader, BoundaryNodesLoaded)
{
    EngineState st;
    load_scenario(st, scenario_dir());
    EXPECT_NE(st.find_boundary_node(GID{"BND-TRJ-GOr-N"}), nullptr);
    EXPECT_NE(st.find_boundary_node(GID{"BND-TRJ-GOr-S"}), nullptr);
}

TEST(TopologyLoader, BoundaryNodeFieldsCorrect)
{
    EngineState st;
    load_scenario(st, scenario_dir());
    const auto* bn = st.find_boundary_node(GID{"BND-TRJ-GOr-N"});
    ASSERT_NE(bn, nullptr);
    EXPECT_EQ(bn->pid, "granica_polnocna");
    EXPECT_EQ(bn->sid.value, "GOr");
}

// ── topology.json — track sections ────────────────────────────────────────────

TEST(TopologyLoader, TrackSectionsLoaded)
{
    EngineState st;
    load_scenario(st, scenario_dir());
    // 5 sections: tor1a, tor2a, tor1b, tor2b, peron1
    EXPECT_NE(st.find_track_section(GID{"OT-TRJ-GOr-tor1a"}), nullptr);
    EXPECT_NE(st.find_track_section(GID{"OT-TRJ-GOr-tor2a"}), nullptr);
    EXPECT_NE(st.find_track_section(GID{"OT-TRJ-GOr-tor1b"}), nullptr);
    EXPECT_NE(st.find_track_section(GID{"OT-TRJ-GOr-tor2b"}), nullptr);
    EXPECT_NE(st.find_track_section(GID{"OT-TRJ-GOr-peron1"}), nullptr);
}

TEST(TopologyLoader, TrackSectionFieldsCorrect)
{
    EngineState st;
    load_scenario(st, scenario_dir());
    const auto* ts = st.find_track_section(GID{"OT-TRJ-GOr-tor1a"});
    ASSERT_NE(ts, nullptr);
    EXPECT_EQ(ts->pid, "tor_1a");
    EXPECT_EQ(ts->sid.value, "GOr");
    EXPECT_FLOAT_EQ(ts->length_m, 320.0f);
    EXPECT_TRUE(ts->electrified);
    EXPECT_EQ(ts->max_speed_kmh, 120);
    EXPECT_EQ(ts->occupancy, TrackOccupancy::FREE);
}

TEST(TopologyLoader, TrackSectionPortsWired)
{
    EngineState st;
    load_scenario(st, scenario_dir());
    const auto* ts = st.find_track_section(GID{"OT-TRJ-GOr-tor1a"});
    ASSERT_NE(ts, nullptr);
    // sideA neighbors boundary node N
    EXPECT_EQ(ts->side_a.neighbor_gid.value, "BND-TRJ-GOr-N");
    EXPECT_EQ(ts->side_a.counter_kind, TrackPort::CounterKind::IT);
    // sideB neighbors switch zwr1
    EXPECT_EQ(ts->side_b.neighbor_gid.value, "ZWR-TRJ-GOr-zwr1");
    EXPECT_EQ(ts->side_b.counter_kind, TrackPort::CounterKind::IZ);
}

TEST(TopologyLoader, TrackSectionSignalRefsCorrect)
{
    EngineState st;
    load_scenario(st, scenario_dir());
    const auto* ts = st.find_track_section(GID{"OT-TRJ-GOr-tor1a"});
    ASSERT_NE(ts, nullptr);
    // sideA has entry signal Wp1; sideB has departure signal Ww1
    ASSERT_EQ(ts->side_a.signal_gids.size(), 1u);
    EXPECT_EQ(ts->side_a.signal_gids[0].value, "SEM-TRJ-GOr-Wp1");
    ASSERT_EQ(ts->side_b.signal_gids.size(), 1u);
    EXPECT_EQ(ts->side_b.signal_gids[0].value, "SEM-TRJ-GOr-Ww1");
}

// ── topology.json — switches ──────────────────────────────────────────────────

TEST(TopologyLoader, SwitchesLoaded)
{
    EngineState st;
    load_scenario(st, scenario_dir());
    EXPECT_NE(st.find_switch(GID{"ZWR-TRJ-GOr-zwr1"}), nullptr);
    EXPECT_NE(st.find_switch(GID{"ZWR-TRJ-GOr-zwr2"}), nullptr);
}

TEST(TopologyLoader, SwitchFieldsCorrect)
{
    EngineState st;
    load_scenario(st, scenario_dir());
    const auto* sw = st.find_switch(GID{"ZWR-TRJ-GOr-zwr1"});
    ASSERT_NE(sw, nullptr);
    EXPECT_EQ(sw->pid, "zwr_1");
    EXPECT_EQ(sw->type_id, "DVT-GLB-ZWR-EEA4-0000002");
    EXPECT_FLOAT_EQ(sw->length_m, 28.5f);
    EXPECT_EQ(sw->max_speed_straight_kmh, 120);
    EXPECT_EQ(sw->max_speed_divergent_kmh, 40);
    // Default position is STRAIGHT
    EXPECT_EQ(sw->position, SwitchPosition::STRAIGHT);
    EXPECT_FALSE(sw->locked_by_route.has_value());
}

TEST(TopologyLoader, SwitchLegsWired)
{
    EngineState st;
    load_scenario(st, scenario_dir());
    const auto* sw = st.find_switch(GID{"ZWR-TRJ-GOr-zwr1"});
    ASSERT_NE(sw, nullptr);
    EXPECT_EQ(sw->trunk.neighbor_gid.value, "OT-TRJ-GOr-tor1a");
    EXPECT_EQ(sw->straight.neighbor_gid.value, "OT-TRJ-GOr-tor1b");
    EXPECT_EQ(sw->divergent.neighbor_gid.value, "OT-TRJ-GOr-peron1");
}

// ── objects.json — signals ────────────────────────────────────────────────────

TEST(TopologyLoader, SignalsLoaded)
{
    EngineState st;
    load_scenario(st, scenario_dir());
    EXPECT_NE(st.find_signal(GID{"SEM-TRJ-GOr-Wp1"}), nullptr);
    EXPECT_NE(st.find_signal(GID{"SEM-TRJ-GOr-Wp2"}), nullptr);
    EXPECT_NE(st.find_signal(GID{"SEM-TRJ-GOr-Ww1"}), nullptr);
    EXPECT_NE(st.find_signal(GID{"SEM-TRJ-GOr-Ww2"}), nullptr);
    EXPECT_NE(st.find_signal(GID{"SEM-TRJ-GOr-Sp1"}), nullptr);
    EXPECT_NE(st.find_signal(GID{"SEM-TRJ-GOr-Sp2"}), nullptr);
}

TEST(TopologyLoader, EntrySignalFieldsCorrect)
{
    EngineState st;
    load_scenario(st, scenario_dir());
    const auto* sig = st.find_signal(GID{"SEM-TRJ-GOr-Wp1"});
    ASSERT_NE(sig, nullptr);
    EXPECT_EQ(sig->pid, "Wp1");
    EXPECT_EQ(sig->sid.value, "GOr");
    EXPECT_EQ(sig->type, Signal::Type::ENTRY);
    EXPECT_EQ(sig->governs_track_section_gid.value, "OT-TRJ-GOr-tor1a");
    EXPECT_EQ(sig->current_aspect, SignalAspect::S1_STOP);
    EXPECT_FALSE(sig->locked_by_route.has_value());
}

TEST(TopologyLoader, DepartureSignalType)
{
    EngineState st;
    load_scenario(st, scenario_dir());
    const auto* sig = st.find_signal(GID{"SEM-TRJ-GOr-Ww1"});
    ASSERT_NE(sig, nullptr);
    EXPECT_EQ(sig->type, Signal::Type::DEPARTURE);
}

// ── Error handling ────────────────────────────────────────────────────────────

TEST(TopologyLoader, ThrowsOnMissingDirectory)
{
    EngineState st;
    EXPECT_THROW(load_scenario(st, "/nonexistent/path/to/scenario"), std::runtime_error);
}

}  // namespace
