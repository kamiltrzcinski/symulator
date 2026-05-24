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

// ── Topology helpers ──────────────────────────────────────────────────────────

constexpr const char* kBndEntry = "BND-TRJ-TST-entry";
constexpr const char* kBndExitB = "BND-TRJ-TST-exit-b";
constexpr const char* kBndExitC = "BND-TRJ-TST-exit-c";
constexpr const char* kOtA = "OT-TRJ-TST-A";
constexpr const char* kOtB = "OT-TRJ-TST-B";
constexpr const char* kOtC = "OT-TRJ-TST-C";
constexpr const char* kZwr1 = "ZWR-TRJ-TST-zwr1";

// Build a minimal EngineState with the topology described in the file header.
// Switch position is configurable.
EngineState make_topology(SwitchPosition sw_pos)
{
    EngineState s;
    s.set_session_id("test");

    // Boundary nodes
    s.insert_boundary_node(BoundaryNode{GID{kBndEntry}, "entry", SID{"TST"}, "entry boundary"});
    s.insert_boundary_node(BoundaryNode{GID{kBndExitB}, "exit-b", SID{"TST"}, "exit B"});
    s.insert_boundary_node(BoundaryNode{GID{kBndExitC}, "exit-c", SID{"TST"}, "exit C"});

    // OT-A: side_a → BND-ENTRY, side_b → ZWR-1
    TrackSection ota{};
    ota.gid = GID{kOtA};
    ota.pid = "OT-A";
    ota.sid = SID{"TST"};
    ota.length_m = 100.0f;
    ota.max_speed_kmh = 120;
    ota.side_a.neighbor_gid = GID{kBndEntry};
    ota.side_a.counter_kind = TrackPort::CounterKind::IT;
    ota.side_b.neighbor_gid = GID{kZwr1};
    ota.side_b.counter_kind = TrackPort::CounterKind::IZ;
    s.insert_track_section(ota);

    // OT-B: side_a → ZWR-1, side_b → BND-EXIT-B
    TrackSection otb{};
    otb.gid = GID{kOtB};
    otb.pid = "OT-B";
    otb.sid = SID{"TST"};
    otb.length_m = 100.0f;
    otb.max_speed_kmh = 120;
    otb.side_a.neighbor_gid = GID{kZwr1};
    otb.side_a.counter_kind = TrackPort::CounterKind::IZ;
    otb.side_b.neighbor_gid = GID{kBndExitB};
    otb.side_b.counter_kind = TrackPort::CounterKind::IT;
    s.insert_track_section(otb);

    // OT-C: side_a → ZWR-1, side_b → BND-EXIT-C
    TrackSection otc{};
    otc.gid = GID{kOtC};
    otc.pid = "OT-C";
    otc.sid = SID{"TST"};
    otc.length_m = 100.0f;
    otc.max_speed_kmh = 120;
    otc.side_a.neighbor_gid = GID{kZwr1};
    otc.side_a.counter_kind = TrackPort::CounterKind::IZ;
    otc.side_b.neighbor_gid = GID{kBndExitC};
    otc.side_b.counter_kind = TrackPort::CounterKind::IT;
    s.insert_track_section(otc);

    // ZWR-1: trunk → OT-A, straight → OT-B, divergent → OT-C
    Switch zwr{};
    zwr.gid = GID{kZwr1};
    zwr.pid = "zwr1";
    zwr.sid = SID{"TST"};
    zwr.position = sw_pos;
    zwr.trunk.neighbor_gid = GID{kOtA};
    zwr.trunk.iz_gid = GID{"IZ-trunk"};
    zwr.straight.neighbor_gid = GID{kOtB};
    zwr.straight.iz_gid = GID{"IZ-straight"};
    zwr.divergent.neighbor_gid = GID{kOtC};
    zwr.divergent.iz_gid = GID{"IZ-divergent"};
    s.insert_switch(zwr);

    return s;
}

// Build a minimal TrainSimState that will cross a 100m section within a few
// ticks.  The train starts at position_m = 99.0 m with v = 5 m/s.
sim::TrainSimState make_fast_train(const GID& section_gid)
{
    sim::TrainSimState st{};
    st.train_gid = GID{"TRN-TRJ-TST-000001"};
    st.current_section_gid = section_gid;
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

TEST(TrainFleet_ResolveNextSection, DirectSection_ReturnsSectionGid)
{
    // OT-A ahead → ZWR-1 (not a section) ... but if we come from the switch side
    // and the neighbor is OT-B, it should be a direct section.
    // Use OT-B: side_a → ZWR-1 (from_gid), side_b → BND-EXIT-B.
    // Travelling B→ boundary, next is BND-EXIT-B → boundary, not direct section.
    //
    // So instead test: OT-A coming from BND-ENTRY (side_a), ahead = ZWR-1 = switch.
    // To get a direct-section test, build a simple two-section topology.

    EngineState s;
    s.set_session_id("test");

    TrackSection sec1{};
    sec1.gid = GID{"OT-1"};
    sec1.sid = SID{"TST"};
    sec1.length_m = 50.0f;
    sec1.max_speed_kmh = 80;
    sec1.side_a.neighbor_gid = GID{"BND-A"};
    sec1.side_a.counter_kind = TrackPort::CounterKind::IT;
    sec1.side_b.neighbor_gid = GID{"OT-2"};
    sec1.side_b.counter_kind = TrackPort::CounterKind::IT;
    s.insert_track_section(sec1);

    TrackSection sec2{};
    sec2.gid = GID{"OT-2"};
    sec2.sid = SID{"TST"};
    sec2.length_m = 50.0f;
    sec2.max_speed_kmh = 80;
    sec2.side_a.neighbor_gid = GID{"OT-1"};
    sec2.side_a.counter_kind = TrackPort::CounterKind::IT;
    sec2.side_b.neighbor_gid = GID{"BND-B"};
    sec2.side_b.counter_kind = TrackPort::CounterKind::IT;
    s.insert_track_section(sec2);

    const NextSectionInfo info = TrainFleet::resolve_next_section(s, GID{"OT-1"}, GID{"BND-A"});
    ASSERT_TRUE(info.section_gid.has_value());
    EXPECT_EQ(info.section_gid->value, "OT-2");
    EXPECT_EQ(info.from_gid.value, "OT-1");
    EXPECT_FALSE(info.is_boundary_crossing);
}

TEST(TrainFleet_ResolveNextSection, TrunkToStraight_WhenStraight)
{
    EngineState s = make_topology(SwitchPosition::STRAIGHT);
    // OT-A, came from BND-ENTRY → ahead = ZWR-1 (trunk side) → STRAIGHT → OT-B
    const NextSectionInfo info = TrainFleet::resolve_next_section(s, GID{kOtA}, GID{kBndEntry});

    ASSERT_TRUE(info.section_gid.has_value());
    EXPECT_EQ(info.section_gid->value, kOtB);
    EXPECT_EQ(info.from_gid.value, kZwr1);  // from_gid is the switch GID
    EXPECT_FALSE(info.is_boundary_crossing);
}

TEST(TrainFleet_ResolveNextSection, TrunkToDivergent_WhenDivergent)
{
    EngineState s = make_topology(SwitchPosition::DIVERGENT);
    const NextSectionInfo info = TrainFleet::resolve_next_section(s, GID{kOtA}, GID{kBndEntry});

    ASSERT_TRUE(info.section_gid.has_value());
    EXPECT_EQ(info.section_gid->value, kOtC);
    EXPECT_EQ(info.from_gid.value, kZwr1);
    EXPECT_FALSE(info.is_boundary_crossing);
}

TEST(TrainFleet_ResolveNextSection, MovingSwitch_ReturnsNullopt)
{
    EngineState s = make_topology(SwitchPosition::MOVING);
    const NextSectionInfo info = TrainFleet::resolve_next_section(s, GID{kOtA}, GID{kBndEntry});

    EXPECT_FALSE(info.section_gid.has_value());
    EXPECT_FALSE(info.is_boundary_crossing);
}

TEST(TrainFleet_ResolveNextSection, LegToTrunk_StraightLeg)
{
    // Train on OT-B, came from ZWR-1 (side_a), ahead = BND-EXIT-B → boundary.
    // Use OT-B coming from ZWR-1 with side_a as entry... actually side_a = ZWR-1 here.
    // "came from ZWR-1" means side_a.neighbor_gid matches from_gid → ahead = side_b = BND-EXIT-B.
    // That's a boundary.
    //
    // For leg→trunk, we need a section whose side connects to the switch from the leg side,
    // and whose other side connects to another section via the trunk.
    // Let's use OT-B coming from BND-EXIT-B → ahead = ZWR-1 (via side_a, entered from side_b).
    // i.e. train is travelling right-to-left on OT-B: from_gid = BND-EXIT-B.

    EngineState s = make_topology(SwitchPosition::STRAIGHT);
    // OT-B side_a = ZWR-1, side_b = BND-EXIT-B.
    // Train came from BND-EXIT-B → ahead = side_a = ZWR-1 (straight leg).
    const NextSectionInfo info = TrainFleet::resolve_next_section(s, GID{kOtB}, GID{kBndExitB});

    ASSERT_TRUE(info.section_gid.has_value());
    EXPECT_EQ(info.section_gid->value, kOtA);  // trunk leads to OT-A
    EXPECT_EQ(info.from_gid.value, kZwr1);
    EXPECT_FALSE(info.is_boundary_crossing);
}

TEST(TrainFleet_ResolveNextSection, BoundaryNode_IsBoundaryCrossing)
{
    EngineState s = make_topology(SwitchPosition::STRAIGHT);
    // OT-B, came from ZWR-1 (side_a) → ahead = BND-EXIT-B
    const NextSectionInfo info = TrainFleet::resolve_next_section(s, GID{kOtB}, GID{kZwr1});

    EXPECT_FALSE(info.section_gid.has_value());
    EXPECT_TRUE(info.is_boundary_crossing);
}

// ── tick_all integration tests ────────────────────────────────────────────────

TEST(TrainFleet_TickAll, TraversesThroughStraightSwitch)
{
    EngineState state = make_topology(SwitchPosition::STRAIGHT);
    state.apply_track_section_occupancy(GID{kOtA}, TrackOccupancy::OCCUPIED, 4);

    TrainFleet fleet;
    fleet.add_train(make_fast_train(GID{kOtA}), GID{kBndEntry});

    // Tick until the train crosses into OT-B (or give up after 10 ticks).
    for (int i = 0; i < 10; ++i)
    {
        fleet.tick_all(state, static_cast<uint64_t>(i), nullptr);
        if (state.find_track_section(GID{kOtB})->occupancy == TrackOccupancy::OCCUPIED)
            break;
    }

    const TrackSection* ota = state.find_track_section(GID{kOtA});
    const TrackSection* otb = state.find_track_section(GID{kOtB});

    EXPECT_EQ(ota->occupancy, TrackOccupancy::FREE);
    EXPECT_EQ(otb->occupancy, TrackOccupancy::OCCUPIED);
    EXPECT_EQ(fleet.size(), 1u);
}

TEST(TrainFleet_TickAll, BoundaryNodeRemovesTrain_AndEmitsPipEvent)
{
    EngineState state = make_topology(SwitchPosition::STRAIGHT);
    state.apply_track_section_occupancy(GID{kOtB}, TrackOccupancy::OCCUPIED, 4);

    TrainFleet fleet;
    // Train on OT-B, came from ZWR-1 → ahead = BND-EXIT-B
    fleet.add_train(make_fast_train(GID{kOtB}), GID{kZwr1});

    std::vector<PipEvent> captured;
    const TrainFleet::PipCallback cb = [&](const std::vector<PipEvent>& evs)
    { captured.insert(captured.end(), evs.begin(), evs.end()); };

    // Tick until train is gone (boundary removal) or 10 ticks max.
    for (int i = 0; i < 10 && !fleet.empty(); ++i)
        fleet.tick_all(state, static_cast<uint64_t>(i), cb);

    EXPECT_TRUE(fleet.empty());

    // OT-B must be free after the boundary crossing.
    const TrackSection* otb = state.find_track_section(GID{kOtB});
    EXPECT_EQ(otb->occupancy, TrackOccupancy::FREE);

    // At least one boundary PipEvent for OT-B must have been emitted.
    const bool has_boundary_event = std::any_of(captured.begin(), captured.end(),
                                                [](const PipEvent& e)
                                                {
                                                    return e.lcs_boundary_crossing &&
                                                           e.section_gid.value == kOtB &&
                                                           e.occupancy == TrackOccupancy::FREE;
                                                });
    EXPECT_TRUE(has_boundary_event);
}
