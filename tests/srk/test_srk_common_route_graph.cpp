// tests/srk/test_srk_common_route_graph.cpp
//
// Unit tests for srk::common route-graph functions:
//   find_route_path()  — BFS path search between two signals
//   make_route_id()    — deterministic route GID generation

#include <gtest/gtest.h>

#include <srk/common/route_graph.hpp>

#include <engine/core/engine_state.hpp>

namespace
{

using namespace engine::core;
using namespace srk::common;

// ── Shared topology IDs ───────────────────────────────────────────────────────
// Layout A (linear, STRAIGHT route):
//   BND-N ── [tor_a] ── [zwr1] ── [tor_b] ── BND-S
//                                  (divergent → [tor_c] ── BND-S2)
//
// Signals:
//   SEM-W  : governs tor_a  (entry from BND-N)
//   SEM-E  : governs tor_b  (departure toward BND-S, via STRAIGHT leg of zwr1)
//   SEM-D  : governs tor_c  (departure toward BND-S2, via DIVERGENT leg of zwr1)
//   SEM-W2 : also governs tor_a (for same-section test)
//
// Derailer WK1 guards tor_a.

static const GID BND_N = GID{"BND-N"};
static const GID BND_S = GID{"BND-S"};
static const GID BND_S2 = GID{"BND-S2"};
static const GID TOR_A = GID{"OT-tor_a"};
static const GID TOR_B = GID{"OT-tor_b"};
static const GID TOR_C = GID{"OT-tor_c"};
static const GID ZWR1 = GID{"ZWR-zwr1"};
static const GID SEM_W = GID{"SEM-W"};
static const GID SEM_W2 = GID{"SEM-W2"};
static const GID SEM_E = GID{"SEM-E"};
static const GID SEM_D = GID{"SEM-D"};
static const GID WK1 = GID{"WK-wk1"};

EngineState make_state()
{
    EngineState st;
    st.set_session_id("TEST");
    st.set_current_tick(1);

    // Boundary nodes
    BoundaryNode bn_n;
    bn_n.gid = BND_N;
    bn_n.pid = "BND-N";
    BoundaryNode bn_s;
    bn_s.gid = BND_S;
    bn_s.pid = "BND-S";
    BoundaryNode bn_s2;
    bn_s2.gid = BND_S2;
    bn_s2.pid = "BND-S2";
    st.insert_boundary_node(bn_n);
    st.insert_boundary_node(bn_s);
    st.insert_boundary_node(bn_s2);

    // Signals
    Signal sw;
    sw.gid = SEM_W;
    sw.pid = "Wp1";
    sw.type = Signal::Type::ENTRY;
    sw.governs_track_section_gid = TOR_A;
    sw.current_aspect = SignalAspect::S1_STOP;
    st.insert_signal(sw);

    Signal sw2;
    sw2.gid = SEM_W2;
    sw2.pid = "Wp2";
    sw2.type = Signal::Type::ENTRY;
    sw2.governs_track_section_gid = TOR_A;  // same section as SEM_W
    sw2.current_aspect = SignalAspect::S1_STOP;
    st.insert_signal(sw2);

    Signal se;
    se.gid = SEM_E;
    se.pid = "Wy1";
    se.type = Signal::Type::DEPARTURE;
    se.governs_track_section_gid = TOR_B;
    se.current_aspect = SignalAspect::S1_STOP;
    st.insert_signal(se);

    Signal sd;
    sd.gid = SEM_D;
    sd.pid = "Wd1";
    sd.type = Signal::Type::DEPARTURE;
    sd.governs_track_section_gid = TOR_C;
    sd.current_aspect = SignalAspect::S1_STOP;
    st.insert_signal(sd);

    // tor_a: BND-N ── [tor_a] ── ZWR1
    TrackSection ta;
    ta.gid = TOR_A;
    ta.pid = "tor_a";
    ta.sid = SID{"TST"};
    ta.side_a.neighbor_gid = BND_N;
    ta.side_a.counter_gid = GID{"IT-a-N"};
    ta.side_a.counter_kind = TrackPort::CounterKind::IT;
    ta.side_a.signal_gids = {SEM_W};
    ta.side_b.neighbor_gid = ZWR1;
    ta.side_b.counter_gid = GID{"IZ-a-S"};
    ta.side_b.counter_kind = TrackPort::CounterKind::IZ;
    ta.occupancy = TrackOccupancy::FREE;
    st.insert_track_section(ta);

    // tor_b: ZWR1 ── [tor_b] ── BND-S  (straight leg)
    TrackSection tb;
    tb.gid = TOR_B;
    tb.pid = "tor_b";
    tb.sid = SID{"TST"};
    tb.side_a.neighbor_gid = ZWR1;
    tb.side_a.counter_gid = GID{"IZ-b-N"};
    tb.side_a.counter_kind = TrackPort::CounterKind::IZ;
    tb.side_b.neighbor_gid = BND_S;
    tb.side_b.counter_gid = GID{"IT-b-S"};
    tb.side_b.counter_kind = TrackPort::CounterKind::IT;
    tb.side_b.signal_gids = {SEM_E};
    tb.occupancy = TrackOccupancy::FREE;
    st.insert_track_section(tb);

    // tor_c: ZWR1 ── [tor_c] ── BND-S2  (divergent leg)
    TrackSection tc;
    tc.gid = TOR_C;
    tc.pid = "tor_c";
    tc.sid = SID{"TST"};
    tc.side_a.neighbor_gid = ZWR1;
    tc.side_a.counter_gid = GID{"IZ-c-N"};
    tc.side_a.counter_kind = TrackPort::CounterKind::IZ;
    tc.side_b.neighbor_gid = BND_S2;
    tc.side_b.counter_gid = GID{"IT-c-S"};
    tc.side_b.counter_kind = TrackPort::CounterKind::IT;
    tc.side_b.signal_gids = {SEM_D};
    tc.occupancy = TrackOccupancy::FREE;
    st.insert_track_section(tc);

    // zwr1: trunk → tor_a, straight → tor_b, divergent → tor_c
    Switch sw1;
    sw1.gid = ZWR1;
    sw1.pid = "zwr1";
    sw1.sid = SID{"TST"};
    sw1.type_id = "DVT-GLB-ZWR-EEA4-0000002";
    sw1.trunk.neighbor_gid = TOR_A;
    sw1.trunk.iz_gid = GID{"IZ-a-S"};
    sw1.straight.neighbor_gid = TOR_B;
    sw1.straight.iz_gid = GID{"IZ-b-N"};
    sw1.divergent.neighbor_gid = TOR_C;
    sw1.divergent.iz_gid = GID{"IZ-c-N"};
    sw1.position = SwitchPosition::STRAIGHT;
    sw1.occupancy = TrackOccupancy::FREE;
    st.insert_switch(sw1);

    // Derailer WK1 guards tor_a
    Derailer wk;
    wk.gid = WK1;
    wk.pid = "wk1";
    wk.sid = SID{"TST"};
    wk.type_id = "DVT-GLB-WK-0000004";
    wk.guards_track_section_gid = TOR_A;
    wk.state = DerailerState::LOCKED;
    st.insert_derailer(wk);

    return st;
}

}  // namespace

// ── find_route_path: happy paths ─────────────────────────────────────────────

TEST(SrkCommonRouteGraph, FindRoutePath_LinearPath_StraightSwitch)
{
    const auto st = make_state();
    const auto result = find_route_path(st, SEM_W, SEM_E);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->from_signal_gid, SEM_W);
    EXPECT_EQ(result->to_signal_gid, SEM_E);

    // Sections: tor_a, tor_b
    ASSERT_EQ(result->section_gids.size(), 2u);
    EXPECT_EQ(result->section_gids[0], TOR_A);
    EXPECT_EQ(result->section_gids[1], TOR_B);

    // Switch: zwr1
    ASSERT_EQ(result->switch_gids.size(), 1u);
    EXPECT_EQ(result->switch_gids[0], ZWR1);

    // The switch node must require STRAIGHT (trunk→TOR_A, going to TOR_B via straight)
    bool found_switch_node = false;
    for (const auto& node : result->nodes)
    {
        if (node.kind == RoutePathNode::Kind::SWITCH && node.gid == ZWR1)
        {
            EXPECT_EQ(node.required_position, SwitchPosition::STRAIGHT);
            found_switch_node = true;
        }
    }
    EXPECT_TRUE(found_switch_node);
}

TEST(SrkCommonRouteGraph, FindRoutePath_LinearPath_DivergentSwitch)
{
    const auto st = make_state();
    const auto result = find_route_path(st, SEM_W, SEM_D);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->from_signal_gid, SEM_W);
    EXPECT_EQ(result->to_signal_gid, SEM_D);

    // Sections: tor_a, tor_c
    ASSERT_EQ(result->section_gids.size(), 2u);
    EXPECT_EQ(result->section_gids[0], TOR_A);
    EXPECT_EQ(result->section_gids[1], TOR_C);

    // Switch node must require DIVERGENT (trunk→TOR_A, going to TOR_C via divergent)
    bool found_switch_node = false;
    for (const auto& node : result->nodes)
    {
        if (node.kind == RoutePathNode::Kind::SWITCH && node.gid == ZWR1)
        {
            EXPECT_EQ(node.required_position, SwitchPosition::DIVERGENT);
            found_switch_node = true;
        }
    }
    EXPECT_TRUE(found_switch_node);
}

TEST(SrkCommonRouteGraph, FindRoutePath_SameSection_ReturnsSingleSectionPath)
{
    const auto st = make_state();
    // SEM_W and SEM_W2 both govern TOR_A
    const auto result = find_route_path(st, SEM_W, SEM_W2);

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->section_gids.size(), 1u);
    EXPECT_EQ(result->section_gids[0], TOR_A);
    EXPECT_TRUE(result->switch_gids.empty());
}

TEST(SrkCommonRouteGraph, FindRoutePath_PopulatesDerailerGids)
{
    const auto st = make_state();
    const auto result = find_route_path(st, SEM_W, SEM_E);

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->derailer_gids.size(), 1u);
    EXPECT_EQ(result->derailer_gids[0], WK1);
}

TEST(SrkCommonRouteGraph, FindRoutePath_DerailerNotIncluded_WhenNotOnPath)
{
    const auto st = make_state();
    // Route SEM_E → SEM_D does not pass through tor_a (where WK1 is).
    // BFS from tor_b via zwr1 to tor_c.
    const auto result = find_route_path(st, SEM_E, SEM_D);

    ASSERT_TRUE(result.has_value());
    // WK1 guards tor_a which is NOT on this path
    for (const auto& dgid : result->derailer_gids)
    {
        EXPECT_NE(dgid, WK1) << "WK1 should not appear on a path that skips tor_a";
    }
}

// ── find_route_path: error cases ─────────────────────────────────────────────

TEST(SrkCommonRouteGraph, FindRoutePath_NulloptWhenFromSignalMissing)
{
    const auto st = make_state();
    const auto result = find_route_path(st, GID{"SEM-NONEXISTENT"}, SEM_E);
    EXPECT_FALSE(result.has_value());
}

TEST(SrkCommonRouteGraph, FindRoutePath_NulloptWhenToSignalMissing)
{
    const auto st = make_state();
    const auto result = find_route_path(st, SEM_W, GID{"SEM-NONEXISTENT"});
    EXPECT_FALSE(result.has_value());
}

// ── make_route_id ─────────────────────────────────────────────────────────────

TEST(SrkCommonRouteGraph, MakeRouteId_IsDeterministicForSameInputs)
{
    const GID id1 = make_route_id(SEM_W, SEM_E, 42);
    const GID id2 = make_route_id(SEM_W, SEM_E, 42);
    EXPECT_EQ(id1, id2);
}

TEST(SrkCommonRouteGraph, MakeRouteId_DifferentTicksProduceDifferentIds)
{
    const GID id_tick10 = make_route_id(SEM_W, SEM_E, 10);
    const GID id_tick99 = make_route_id(SEM_W, SEM_E, 99);
    EXPECT_NE(id_tick10, id_tick99);
}

TEST(SrkCommonRouteGraph, MakeRouteId_DifferentSignalsDifferentIds)
{
    const GID id_we = make_route_id(SEM_W, SEM_E, 1);
    const GID id_wd = make_route_id(SEM_W, SEM_D, 1);
    EXPECT_NE(id_we, id_wd);
}

TEST(SrkCommonRouteGraph, MakeRouteId_ContainsSignalGidValues)
{
    const GID id = make_route_id(SEM_W, SEM_E, 5);
    EXPECT_NE(id.value.find("SEM-W"), std::string::npos);
    EXPECT_NE(id.value.find("SEM-E"), std::string::npos);
}
