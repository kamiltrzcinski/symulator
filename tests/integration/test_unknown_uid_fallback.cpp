// tests/integration/test_unknown_uid_fallback.cpp
//
// Integration test for the "graceful unknown UID" fallback in
// TrainFleet::resolve_next_section() (engine/src/train_fleet.cpp).
//
// A trunk-line section's neighbor_uid may reference a section that lives in
// a station scenario that hasn't been loaded (single-station mode, or a
// cross-referenced section from an as-yet-unloaded neighbor). That neighbor
// UID resolves to nothing in EngineState — it must be treated as a world
// edge, not a dead-end, so the train is removed cleanly instead of stalling
// on the section forever.

#include "engine/core/engine_state.hpp"
#include "engine/core/train_fleet.hpp"
#include "engine/sim/train_sim.hpp"

#include <gtest/gtest.h>

using namespace engine;
using namespace engine::core;

namespace
{

constexpr UID kBndEntry = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::BOUNDARY_NODE, 1, 1);
constexpr UID kOtA = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::TRACK_SECTION, 1, 1);
constexpr UID kStationUid = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::STATION, 1, 1);

// A UID that is never inserted into EngineState — simulates a cross-reference
// to a section in a scenario that hasn't been loaded.
constexpr UID kUnknownNeighbor =
    make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::TRACK_SECTION, 2, 999);

// OT-A: side_a → BND-ENTRY, side_b → kUnknownNeighbor (never inserted).
EngineState make_topology()
{
    EngineState s;
    s.set_session_id("test");

    s.insert_boundary_node(BoundaryNode{kBndEntry, "entry", kStationUid, "entry boundary"});

    TrackSection ota{};
    ota.uid = kOtA;
    ota.pid = "OT-A";
    ota.station_uid = kStationUid;
    ota.length_m = 100.0f;
    ota.max_speed_kmh = 120;
    ota.side_a.neighbor_uid = kBndEntry;
    ota.side_a.counter_kind = TrackPort::CounterKind::IT;
    ota.side_b.neighbor_uid = kUnknownNeighbor;
    ota.side_b.counter_kind = TrackPort::CounterKind::IZ;
    s.insert_track_section(ota);

    return s;
}

// Build a minimal TrainSimState that will cross a 100m section within a few
// ticks (mirrors tests/engine/test_train_fleet.cpp's make_fast_train).
sim::TrainSimState make_fast_train(UID section_uid)
{
    sim::TrainSimState st{};
    st.train_uid = make_uid(UIDDomain::ROLLING_STOCK, UIDKind::TRAIN_CONSIST, 0, 1);
    st.current_section_uid = section_uid;
    st.max_brake_kn = 50.0f;
    st.physics_params.total_mass_t = 100.0f;
    st.physics_params.max_traction_kn = 200.0f;
    st.physics_params.max_speed_ms = 33.3f;  // ~120 km/h
    st.physics_params.davis_A = 1.0f;
    st.physics_params.davis_B = 0.01f;
    st.physics_params.davis_C = 0.0001f;
    st.physics_state.position_m = 99.0f;  // 1 m before section end
    st.physics_state.velocity_ms = 5.0f;  // 18 km/h — will cross in one tick
    st.driver_state = physics::DriverState::CRUISING;
    return st;
}

}  // namespace

TEST(UnknownUidFallback, ResolveNextSectionTreatsUnknownNeighborAsBoundaryCrossing)
{
    EngineState s = make_topology();
    const NextSectionInfo info = TrainFleet::resolve_next_section(s, kOtA, kBndEntry);

    EXPECT_FALSE(info.section_uid.has_value());
    EXPECT_TRUE(info.is_boundary_crossing);
}

TEST(UnknownUidFallback, TrainIsRemovedWithoutCrashingInsteadOfStalling)
{
    EngineState state = make_topology();
    state.apply_track_section_occupancy(kOtA, TrackOccupancy::OCCUPIED, 4);

    TrainFleet fleet;
    fleet.add_train(make_fast_train(kOtA), kBndEntry);

    for (int i = 0; i < 10 && !fleet.empty(); ++i)
        fleet.tick_all(state, static_cast<uint64_t>(i), nullptr);

    EXPECT_TRUE(fleet.empty());

    const TrackSection* ota = state.find_track_section(kOtA);
    EXPECT_EQ(ota->occupancy, TrackOccupancy::FREE);
}
