// tests/engine/test_train_fleet.cpp
//
// Unit tests for TrainFleet::resolve_next_section and tick_all traversal.
//
// Topology used in most tests:
//
//   BND-ENTRY  ←[side_a]  OT-A [side_b]→  ZWR-1 (trunk)
//                                            ├─ straight → OT-B [side_a]→ BND-EXIT-B
//                                            └─ divergent → OT-C [side_a]→ BND-EXIT-C

#include "engine/core/engine_state.hpp"
#include "engine/core/train_fleet.hpp"
#include "engine/sim/train_sim.hpp"

#include <gtest/gtest.h>

using namespace engine;
using namespace engine::core;

namespace
{

// ── Topology UID constants ────────────────────────────────────────────────────
// Use INFRASTRUCTURE domain UIDs for topology objects.
// Scope = 1 (station 1), different instances.

constexpr UID kBndEntry = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::BOUNDARY_NODE, 1, 1);
constexpr UID kBndExitB = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::BOUNDARY_NODE, 1, 2);
constexpr UID kBndExitC = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::BOUNDARY_NODE, 1, 3);
constexpr UID kOtA = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::TRACK_SECTION, 1, 1);
constexpr UID kOtB = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::TRACK_SECTION, 1, 2);
constexpr UID kOtC = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::TRACK_SECTION, 1, 3);
constexpr UID kZwr1 = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::SWITCH, 1, 1);
constexpr UID kStationUid = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::STATION, 1, 1);

// Build a minimal EngineState with the topology described in the file header.
// Switch position is configurable.
EngineState make_topology(SwitchPosition sw_pos)
{
    EngineState s;
    s.set_session_id("test");

    // Boundary nodes
    s.insert_boundary_node(BoundaryNode{kBndEntry, "entry", kStationUid, "entry boundary"});
    s.insert_boundary_node(BoundaryNode{kBndExitB, "exit-b", kStationUid, "exit B"});
    s.insert_boundary_node(BoundaryNode{kBndExitC, "exit-c", kStationUid, "exit C"});

    // OT-A: side_a → BND-ENTRY, side_b → ZWR-1
    TrackSection ota{};
    ota.uid = kOtA;
    ota.pid = "OT-A";
    ota.station_uid = kStationUid;
    ota.length_m = 100.0f;
    ota.max_speed_kmh = 120;
    ota.side_a.neighbor_uid = kBndEntry;
    ota.side_a.counter_kind = TrackPort::CounterKind::IT;
    ota.side_b.neighbor_uid = kZwr1;
    ota.side_b.counter_kind = TrackPort::CounterKind::IZ;
    s.insert_track_section(ota);

    // OT-B: side_a → ZWR-1, side_b → BND-EXIT-B
    TrackSection otb{};
    otb.uid = kOtB;
    otb.pid = "OT-B";
    otb.station_uid = kStationUid;
    otb.length_m = 100.0f;
    otb.max_speed_kmh = 120;
    otb.side_a.neighbor_uid = kZwr1;
    otb.side_a.counter_kind = TrackPort::CounterKind::IZ;
    otb.side_b.neighbor_uid = kBndExitB;
    otb.side_b.counter_kind = TrackPort::CounterKind::IT;
    s.insert_track_section(otb);

    // OT-C: side_a → ZWR-1, side_b → BND-EXIT-C
    TrackSection otc{};
    otc.uid = kOtC;
    otc.pid = "OT-C";
    otc.station_uid = kStationUid;
    otc.length_m = 100.0f;
    otc.max_speed_kmh = 120;
    otc.side_a.neighbor_uid = kZwr1;
    otc.side_a.counter_kind = TrackPort::CounterKind::IZ;
    otc.side_b.neighbor_uid = kBndExitC;
    otc.side_b.counter_kind = TrackPort::CounterKind::IT;
    s.insert_track_section(otc);

    // ZWR-1: trunk → OT-A, straight → OT-B, divergent → OT-C
    Switch zwr{};
    zwr.uid = kZwr1;
    zwr.pid = "zwr1";
    zwr.station_uid = kStationUid;
    zwr.position = sw_pos;
    zwr.trunk.neighbor_uid = kOtA;
    zwr.trunk.iz_uid = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::AXLE_COUNTER, 1, 1);
    zwr.straight.neighbor_uid = kOtB;
    zwr.straight.iz_uid = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::AXLE_COUNTER, 1, 2);
    zwr.divergent.neighbor_uid = kOtC;
    zwr.divergent.iz_uid = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::AXLE_COUNTER, 1, 3);
    s.insert_switch(zwr);

    return s;
}

// Build a minimal TrainSimState that will cross a 100m section within a few
// ticks.  The train starts at position_m = 99.0 m with v = 5 m/s.
sim::TrainSimState make_fast_train(UID section_uid)
{
    sim::TrainSimState st{};
    st.train_uid = make_uid(UIDDomain::ROLLING_STOCK, UIDKind::TRAIN_CONSIST, 0, 1);
    st.current_section_uid = section_uid;
    st.max_brake_kn = 50.0f;
    st.physics_params.total_mass_t = 100.0f;
    st.physics_params.max_traction_kn = 200.0f;
    st.physics_params.max_speed_ms = 33.3f;  // ~120 km/h
    // Davis coefficients — small values so resistance doesn't stop the train.
    st.physics_params.davis_A = 1.0f;
    st.physics_params.davis_B = 0.01f;
    st.physics_params.davis_C = 0.0001f;
    st.physics_state.position_m = 99.0f;  // 1 m before section end
    st.physics_state.velocity_ms = 5.0f;  // 18 km/h — will cross in one tick
    st.driver_state = physics::DriverState::CRUISING;
    return st;
}

}  // namespace

// ── resolve_next_section tests ────────────────────────────────────────────────

TEST(TrainFleet_ResolveNextSection, DirectSection_ReturnsSectionUid)
{
    // Build a simple two-section topology.
    EngineState s;
    s.set_session_id("test");

    constexpr UID sec1Uid = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::TRACK_SECTION, 2, 1);
    constexpr UID sec2Uid = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::TRACK_SECTION, 2, 2);
    constexpr UID bndA = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::BOUNDARY_NODE, 2, 1);
    constexpr UID bndB = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::BOUNDARY_NODE, 2, 2);
    constexpr UID sta = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::STATION, 2, 1);

    TrackSection sec1{};
    sec1.uid = sec1Uid;
    sec1.station_uid = sta;
    sec1.length_m = 50.0f;
    sec1.max_speed_kmh = 80;
    sec1.side_a.neighbor_uid = bndA;
    sec1.side_a.counter_kind = TrackPort::CounterKind::IT;
    sec1.side_b.neighbor_uid = sec2Uid;
    sec1.side_b.counter_kind = TrackPort::CounterKind::IT;
    s.insert_track_section(sec1);

    TrackSection sec2{};
    sec2.uid = sec2Uid;
    sec2.station_uid = sta;
    sec2.length_m = 50.0f;
    sec2.max_speed_kmh = 80;
    sec2.side_a.neighbor_uid = sec1Uid;
    sec2.side_a.counter_kind = TrackPort::CounterKind::IT;
    sec2.side_b.neighbor_uid = bndB;
    sec2.side_b.counter_kind = TrackPort::CounterKind::IT;
    s.insert_track_section(sec2);

    const NextSectionInfo info = TrainFleet::resolve_next_section(s, sec1Uid, bndA);
    ASSERT_TRUE(info.section_uid.has_value());
    EXPECT_EQ(info.section_uid->value, sec2Uid.value);
    EXPECT_EQ(info.from_uid.value, sec1Uid.value);
    EXPECT_FALSE(info.is_boundary_crossing);
}

TEST(TrainFleet_ResolveNextSection, TrunkToStraight_WhenStraight)
{
    EngineState s = make_topology(SwitchPosition::STRAIGHT);
    // OT-A, came from BND-ENTRY (side_a), ahead = ZWR-1 (trunk side) → STRAIGHT → OT-B
    const NextSectionInfo info = TrainFleet::resolve_next_section(s, kOtA, kBndEntry);

    ASSERT_TRUE(info.section_uid.has_value());
    EXPECT_EQ(info.section_uid->value, kOtB.value);
    EXPECT_EQ(info.from_uid.value, kZwr1.value);  // from_uid is the switch UID
    EXPECT_FALSE(info.is_boundary_crossing);
}

TEST(TrainFleet_ResolveNextSection, TrunkToDivergent_WhenDivergent)
{
    EngineState s = make_topology(SwitchPosition::DIVERGENT);
    const NextSectionInfo info = TrainFleet::resolve_next_section(s, kOtA, kBndEntry);

    ASSERT_TRUE(info.section_uid.has_value());
    EXPECT_EQ(info.section_uid->value, kOtC.value);
    EXPECT_EQ(info.from_uid.value, kZwr1.value);
    EXPECT_FALSE(info.is_boundary_crossing);
}

TEST(TrainFleet_ResolveNextSection, MovingSwitch_ReturnsNullopt)
{
    EngineState s = make_topology(SwitchPosition::MOVING);
    const NextSectionInfo info = TrainFleet::resolve_next_section(s, kOtA, kBndEntry);

    EXPECT_FALSE(info.section_uid.has_value());
    EXPECT_FALSE(info.is_boundary_crossing);
}

TEST(TrainFleet_ResolveNextSection, LegToTrunk_StraightLeg)
{
    EngineState s = make_topology(SwitchPosition::STRAIGHT);
    // OT-B side_a = ZWR-1, side_b = BND-EXIT-B.
    // Train came from BND-EXIT-B → ahead = side_a = ZWR-1 (straight leg).
    const NextSectionInfo info = TrainFleet::resolve_next_section(s, kOtB, kBndExitB);

    ASSERT_TRUE(info.section_uid.has_value());
    EXPECT_EQ(info.section_uid->value, kOtA.value);  // trunk leads to OT-A
    EXPECT_EQ(info.from_uid.value, kZwr1.value);
    EXPECT_FALSE(info.is_boundary_crossing);
}

TEST(TrainFleet_ResolveNextSection, BoundaryNode_IsBoundaryCrossing)
{
    EngineState s = make_topology(SwitchPosition::STRAIGHT);
    // OT-B, came from ZWR-1 (side_a) → ahead = BND-EXIT-B
    const NextSectionInfo info = TrainFleet::resolve_next_section(s, kOtB, kZwr1);

    EXPECT_FALSE(info.section_uid.has_value());
    EXPECT_TRUE(info.is_boundary_crossing);
}

// ── tick_all integration tests ────────────────────────────────────────────────

TEST(TrainFleet_TickAll, TraversesThroughStraightSwitch)
{
    EngineState state = make_topology(SwitchPosition::STRAIGHT);
    state.apply_track_section_occupancy(kOtA, TrackOccupancy::OCCUPIED, 4);

    TrainFleet fleet;
    fleet.add_train(make_fast_train(kOtA), kBndEntry);

    // Tick until the train crosses into OT-B (or give up after 10 ticks).
    for (int i = 0; i < 10; ++i)
    {
        fleet.tick_all(state, static_cast<uint64_t>(i), nullptr);
        if (state.find_track_section(kOtB)->occupancy == TrackOccupancy::OCCUPIED)
            break;
    }

    const TrackSection* ota = state.find_track_section(kOtA);
    const TrackSection* otb = state.find_track_section(kOtB);

    EXPECT_EQ(ota->occupancy, TrackOccupancy::FREE);
    EXPECT_EQ(otb->occupancy, TrackOccupancy::OCCUPIED);
    EXPECT_EQ(fleet.size(), 1u);
}

TEST(TrainFleet_TickAll, BoundaryNodeRemovesTrain_AndEmitsPipEvent)
{
    EngineState state = make_topology(SwitchPosition::STRAIGHT);
    state.apply_track_section_occupancy(kOtB, TrackOccupancy::OCCUPIED, 4);

    TrainFleet fleet;
    // Train on OT-B, came from ZWR-1 → ahead = BND-EXIT-B
    fleet.add_train(make_fast_train(kOtB), kZwr1);

    std::vector<PipEvent> captured;
    const TrainFleet::PipCallback cb = [&](const std::vector<PipEvent>& evs)
    { captured.insert(captured.end(), evs.begin(), evs.end()); };

    // Tick until train is gone (boundary removal) or 10 ticks max.
    for (int i = 0; i < 10 && !fleet.empty(); ++i)
        fleet.tick_all(state, static_cast<uint64_t>(i), cb);

    EXPECT_TRUE(fleet.empty());

    // OT-B must be free after the boundary crossing.
    const TrackSection* otb = state.find_track_section(kOtB);
    EXPECT_EQ(otb->occupancy, TrackOccupancy::FREE);

    // At least one boundary PipEvent for OT-B must have been emitted.
    const bool has_boundary_event = std::any_of(captured.begin(), captured.end(),
                                                [](const PipEvent& e)
                                                {
                                                    return e.lcs_boundary_crossing &&
                                                           e.section_uid == kOtB &&
                                                           e.occupancy == TrackOccupancy::FREE;
                                                });
    EXPECT_TRUE(has_boundary_event);
}
