// tests/srk/test_srk_common_route_graph.cpp
//
// Unit tests for srk::common route-graph functions:
//   find_route_path()  — BFS path search between two signals
//   make_route_uid()   — stable OPERATIONS/ROUTE UID generation

#include <gtest/gtest.h>

#include <srk/common/route_graph.hpp>

#include <engine/core/engine_state.hpp>

namespace
{

using namespace engine::core;
using namespace srk::common;

// ── UID constants for test topology ──────────────────────────────────────────
// All with station SCOPE=1 (arbitrary test station).

constexpr UID BND_N = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::BOUNDARY_NODE, 1, 1);
constexpr UID BND_S = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::BOUNDARY_NODE, 1, 2);
constexpr UID BND_S2 = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::BOUNDARY_NODE, 1, 3);
constexpr UID TOR_A = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::TRACK_SECTION, 1, 1);
constexpr UID TOR_B = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::TRACK_SECTION, 1, 2);
constexpr UID TOR_C = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::TRACK_SECTION, 1, 3);
constexpr UID ZWR1 = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::SWITCH, 1, 1);
constexpr UID SEM_W = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::SIGNAL, 1, 1);
constexpr UID SEM_W2 = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::SIGNAL, 1, 2);
constexpr UID SEM_E = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::SIGNAL, 1, 3);
constexpr UID SEM_D = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::SIGNAL, 1, 4);
constexpr UID WK1 = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::DERAILER, 1, 1);
constexpr UID STA1 = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::STATION, 1, 1);

// Axle counter UIDs (needed for TrackPort::counter_uid)
constexpr UID IT_A_N = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::AXLE_COUNTER, 1, 1);
constexpr UID IZ_A_S = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::AXLE_COUNTER, 1, 2);
constexpr UID IZ_B_N = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::AXLE_COUNTER, 1, 3);
constexpr UID IT_B_S = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::AXLE_COUNTER, 1, 4);
constexpr UID IZ_C_N = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::AXLE_COUNTER, 1, 5);
constexpr UID IT_C_S = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::AXLE_COUNTER, 1, 6);

EngineState make_state()
{
    EngineState st;
    st.set_session_id("TEST");
    st.set_current_tick(1);

    // Boundary nodes
    BoundaryNode bn_n;
    bn_n.uid = BND_N;
    bn_n.pid = "BND-N";
    bn_n.station_uid = STA1;
    BoundaryNode bn_s;
    bn_s.uid = BND_S;
    bn_s.pid = "BND-S";
    bn_s.station_uid = STA1;
    BoundaryNode bn_s2;
    bn_s2.uid = BND_S2;
    bn_s2.pid = "BND-S2";
    bn_s2.station_uid = STA1;
    st.insert_boundary_node(bn_n);
    st.insert_boundary_node(bn_s);
    st.insert_boundary_node(bn_s2);

    // Signals
    Signal sem_w;
    sem_w.uid = SEM_W;
    sem_w.pid = "Wp1";
    sem_w.station_uid = STA1;
    sem_w.type = Signal::Type::ENTRY;
    sem_w.governs_section_uid = TOR_A;
    sem_w.current_aspect = SignalAspect::S1_STOP;
    st.insert_signal(sem_w);

    Signal sem_w2;
    sem_w2.uid = SEM_W2;
    sem_w2.pid = "Wp2";
    sem_w2.station_uid = STA1;
    sem_w2.type = Signal::Type::ENTRY;
    sem_w2.governs_section_uid = TOR_A;
    sem_w2.current_aspect = SignalAspect::S1_STOP;
    st.insert_signal(sem_w2);

    Signal sem_e;
    sem_e.uid = SEM_E;
    sem_e.pid = "Wy1";
    sem_e.station_uid = STA1;
    sem_e.type = Signal::Type::DEPARTURE;
    sem_e.governs_section_uid = TOR_B;
    sem_e.current_aspect = SignalAspect::S1_STOP;
    st.insert_signal(sem_e);

    Signal sem_d;
    sem_d.uid = SEM_D;
    sem_d.pid = "Wd1";
    sem_d.station_uid = STA1;
    sem_d.type = Signal::Type::DEPARTURE;
    sem_d.governs_section_uid = TOR_C;
    sem_d.current_aspect = SignalAspect::S1_STOP;
    st.insert_signal(sem_d);

    // tor_a: BND-N ── [tor_a] ── ZWR1
    TrackSection ta;
    ta.uid = TOR_A;
    ta.pid = "tor_a";
    ta.station_uid = STA1;
    ta.side_a.neighbor_uid = BND_N;
    ta.side_a.counter_uid = IT_A_N;
    ta.side_a.counter_kind = TrackPort::CounterKind::IT;
    ta.side_a.signal_uids = {SEM_W};
    ta.side_b.neighbor_uid = ZWR1;
    ta.side_b.counter_uid = IZ_A_S;
    ta.side_b.counter_kind = TrackPort::CounterKind::IZ;
    ta.occupancy = TrackOccupancy::FREE;
    st.insert_track_section(ta);

    // tor_b: ZWR1 ── [tor_b] ── BND-S  (straight leg)
    TrackSection tb;
    tb.uid = TOR_B;
    tb.pid = "tor_b";
    tb.station_uid = STA1;
    tb.side_a.neighbor_uid = ZWR1;
    tb.side_a.counter_uid = IZ_B_N;
    tb.side_a.counter_kind = TrackPort::CounterKind::IZ;
    tb.side_b.neighbor_uid = BND_S;
    tb.side_b.counter_uid = IT_B_S;
    tb.side_b.counter_kind = TrackPort::CounterKind::IT;
    tb.side_b.signal_uids = {SEM_E};
    tb.occupancy = TrackOccupancy::FREE;
    st.insert_track_section(tb);

    // tor_c: ZWR1 ── [tor_c] ── BND-S2  (divergent leg)
    TrackSection tc;
    tc.uid = TOR_C;
    tc.pid = "tor_c";
    tc.station_uid = STA1;
    tc.side_a.neighbor_uid = ZWR1;
    tc.side_a.counter_uid = IZ_C_N;
    tc.side_a.counter_kind = TrackPort::CounterKind::IZ;
    tc.side_b.neighbor_uid = BND_S2;
    tc.side_b.counter_uid = IT_C_S;
    tc.side_b.counter_kind = TrackPort::CounterKind::IT;
    tc.side_b.signal_uids = {SEM_D};
    tc.occupancy = TrackOccupancy::FREE;
    st.insert_track_section(tc);

    // zwr1: trunk → tor_a, straight → tor_b, divergent → tor_c
    Switch sw1;
    sw1.uid = ZWR1;
    sw1.pid = "zwr1";
    sw1.station_uid = STA1;
    sw1.type_id = "DVT-GLB-ZWR-EEA4-0000002";
    sw1.trunk.neighbor_uid = TOR_A;
    sw1.trunk.iz_uid = IZ_A_S;
    sw1.straight.neighbor_uid = TOR_B;
    sw1.straight.iz_uid = IZ_B_N;
    sw1.divergent.neighbor_uid = TOR_C;
    sw1.divergent.iz_uid = IZ_C_N;
    sw1.position = SwitchPosition::STRAIGHT;
    sw1.occupancy = TrackOccupancy::FREE;
    st.insert_switch(sw1);

    // Derailer WK1 guards tor_a
    Derailer wk;
    wk.uid = WK1;
    wk.pid = "wk1";
    wk.station_uid = STA1;
    wk.type_id = "DVT-GLB-WK-0000004";
    wk.guards_section_uid = TOR_A;
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
    EXPECT_EQ(result->from_signal_uid, SEM_W);
    EXPECT_EQ(result->to_signal_uid, SEM_E);

    ASSERT_EQ(result->section_uids.size(), 2u);
    EXPECT_EQ(result->section_uids[0], TOR_A);
    EXPECT_EQ(result->section_uids[1], TOR_B);

    ASSERT_EQ(result->switch_uids.size(), 1u);
    EXPECT_EQ(result->switch_uids[0], ZWR1);

    bool found_switch_node = false;
    for (const auto& node : result->nodes)
    {
        if (node.kind == RoutePathNode::Kind::SWITCH && node.uid == ZWR1)
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
    EXPECT_EQ(result->from_signal_uid, SEM_W);
    EXPECT_EQ(result->to_signal_uid, SEM_D);

    ASSERT_EQ(result->section_uids.size(), 2u);
    EXPECT_EQ(result->section_uids[0], TOR_A);
    EXPECT_EQ(result->section_uids[1], TOR_C);

    bool found_switch_node = false;
    for (const auto& node : result->nodes)
    {
        if (node.kind == RoutePathNode::Kind::SWITCH && node.uid == ZWR1)
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
    const auto result = find_route_path(st, SEM_W, SEM_W2);

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->section_uids.size(), 1u);
    EXPECT_EQ(result->section_uids[0], TOR_A);
    EXPECT_TRUE(result->switch_uids.empty());
}

TEST(SrkCommonRouteGraph, FindRoutePath_PopulatesDerailerUids)
{
    const auto st = make_state();
    const auto result = find_route_path(st, SEM_W, SEM_E);

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->derailer_uids.size(), 1u);
    EXPECT_EQ(result->derailer_uids[0], WK1);
}

TEST(SrkCommonRouteGraph, FindRoutePath_DerailerNotIncluded_WhenNotOnPath)
{
    const auto st = make_state();
    const auto result = find_route_path(st, SEM_E, SEM_D);

    ASSERT_TRUE(result.has_value());
    for (const auto& duid : result->derailer_uids)
    {
        EXPECT_NE(duid, WK1) << "WK1 should not appear on a path that skips tor_a";
    }
}

// ── find_route_path: error cases ─────────────────────────────────────────────

TEST(SrkCommonRouteGraph, FindRoutePath_NulloptWhenFromSignalMissing)
{
    const auto st = make_state();
    const UID nonexistent = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::SIGNAL, 1, 99);
    const auto result = find_route_path(st, nonexistent, SEM_E);
    EXPECT_FALSE(result.has_value());
}

TEST(SrkCommonRouteGraph, FindRoutePath_NulloptWhenToSignalMissing)
{
    const auto st = make_state();
    const UID nonexistent = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::SIGNAL, 1, 99);
    const auto result = find_route_path(st, SEM_W, nonexistent);
    EXPECT_FALSE(result.has_value());
}

// ── make_route_uid ───────────────────────────────────────────────────────────

TEST(SrkCommonRouteGraph, MakeRouteUid_IsDeterministicForSameInputs)
{
    const UID id1 = make_route_uid(SEM_W, SEM_E, 42);
    const UID id2 = make_route_uid(SEM_W, SEM_E, 42);
    EXPECT_EQ(id1, id2);
}

TEST(SrkCommonRouteGraph, MakeRouteUid_DifferentTicksProduceDifferentIds)
{
    const UID id_tick10 = make_route_uid(SEM_W, SEM_E, 10);
    const UID id_tick99 = make_route_uid(SEM_W, SEM_E, 99);
    EXPECT_NE(id_tick10, id_tick99);
}

TEST(SrkCommonRouteGraph, MakeRouteUid_DifferentSignalsDifferentIds)
{
    const UID id_we = make_route_uid(SEM_W, SEM_E, 1);
    const UID id_wd = make_route_uid(SEM_W, SEM_D, 1);
    EXPECT_NE(id_we, id_wd);
}

TEST(SrkCommonRouteGraph, MakeRouteUid_IsOperationsRouteKind)
{
    const UID id = make_route_uid(SEM_W, SEM_E, 5);
    EXPECT_NE(id.value, 0u);
    EXPECT_EQ(uid_domain(id), UIDDomain::OPERATIONS);
    EXPECT_EQ(uid_kind(id), UIDKind::ROUTE);
    EXPECT_TRUE(uid_is_safe_json_integer(id));
}
