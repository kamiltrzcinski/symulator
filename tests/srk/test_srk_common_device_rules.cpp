// tests/srk/test_srk_common_device_rules.cpp
//
// Unit tests for srk::common device rules (R1–R7).
// These exercise the check_* and execute_* helpers directly,
// independently of any concrete IControlSystem implementation.
//
// Layout (same as test_srk_common_route_graph.cpp):
//   BND-N ── [tor_a] ── [zwr1(STRAIGHT)] ── [tor_b] ── BND-S
// Signals: SEM-W (governs tor_a), SEM-E (governs tor_b)
// Derailer: WK1 (guards tor_a)
// Block section: BL1

#include <gtest/gtest.h>

#include <srk/common/device_rules.hpp>
#include <srk/common/nak_codes.hpp>

#include <engine/core/engine_state.hpp>

#include <variant>

namespace
{

using namespace engine::core;
using namespace srk::common;

static const GID BND_N = GID{"BND-N"};
static const GID BND_S = GID{"BND-S"};
static const GID TOR_A = GID{"OT-tor_a"};
static const GID TOR_B = GID{"OT-tor_b"};
static const GID ZWR1 = GID{"ZWR-zwr1"};
static const GID SEM_W = GID{"SEM-W"};
static const GID SEM_E = GID{"SEM-E"};
static const GID WK1 = GID{"WK-wk1"};
static const GID BL1 = GID{"BL-001"};
static const GID ALARM1 = GID{"ALM-001"};
static const GID ROUTE1 = GID{"RTE-001"};

EngineState make_state()
{
    EngineState st;
    st.set_session_id("TEST");
    st.set_current_tick(1);

    BoundaryNode bn_n;
    bn_n.gid = BND_N;
    bn_n.pid = "BND-N";
    BoundaryNode bn_s;
    bn_s.gid = BND_S;
    bn_s.pid = "BND-S";
    st.insert_boundary_node(bn_n);
    st.insert_boundary_node(bn_s);

    Signal sw;
    sw.gid = SEM_W;
    sw.pid = "Wp1";
    sw.type = Signal::Type::ENTRY;
    sw.governs_track_section_gid = TOR_A;
    sw.current_aspect = SignalAspect::S1_STOP;
    st.insert_signal(sw);

    Signal se;
    se.gid = SEM_E;
    se.pid = "Wy1";
    se.type = Signal::Type::DEPARTURE;
    se.governs_track_section_gid = TOR_B;
    se.current_aspect = SignalAspect::S1_STOP;
    st.insert_signal(se);

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

    Switch sw1;
    sw1.gid = ZWR1;
    sw1.pid = "zwr1";
    sw1.sid = SID{"TST"};
    sw1.type_id = "DVT-GLB-ZWR-EEA4-0000002";
    sw1.trunk.neighbor_gid = TOR_A;
    sw1.trunk.iz_gid = GID{"IZ-a-S"};
    sw1.straight.neighbor_gid = TOR_B;
    sw1.straight.iz_gid = GID{"IZ-b-N"};
    sw1.divergent.neighbor_gid = BND_S;
    sw1.divergent.iz_gid = GID{"IZ-div"};
    sw1.position = SwitchPosition::STRAIGHT;
    sw1.occupancy = TrackOccupancy::FREE;
    st.insert_switch(sw1);

    Derailer wk;
    wk.gid = WK1;
    wk.pid = "wk1";
    wk.sid = SID{"TST"};
    wk.type_id = "DVT-GLB-WK-0000004";
    wk.guards_track_section_gid = TOR_A;
    wk.state = DerailerState::LOCKED;
    st.insert_derailer(wk);

    BlockSection bs;
    bs.gid = BL1;
    bs.pid = "bl1";
    bs.sid = SID{"TST"};
    bs.neighbor_sid = SID{"NGR"};
    bs.direction = BlockDirectionState::NEUTRAL;
    bs.state = BlockSectionState::CLOSED;
    bs.axle_count = 0;
    st.insert_block_section(bs);

    return st;
}

}  // namespace

// ── R1: SetSwitchPosition ─────────────────────────────────────────────────────

TEST(SrkCommonDeviceRules, R1_Check_Valid_ReturnsNullopt)
{
    const auto st = make_state();
    const SetSwitchPositionCmd cmd{ZWR1, SwitchPosition::DIVERGENT};
    EXPECT_FALSE(check_set_switch_position(st, cmd).has_value());
}

TEST(SrkCommonDeviceRules, R1_Check_OccupiedSwitch_ReturnsSafetyBlock)
{
    auto st = make_state();
    st.apply_switch_occupancy(ZWR1, TrackOccupancy::OCCUPIED, 4);

    const SetSwitchPositionCmd cmd{ZWR1, SwitchPosition::DIVERGENT};
    const auto v = check_set_switch_position(st, cmd);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->reason_code, NAK_SAFETY_BLOCK);
    EXPECT_EQ(v->offending_gid, ZWR1);
}

TEST(SrkCommonDeviceRules, R1_Check_MovingSwitch_ReturnsSwitchMoving)
{
    auto st = make_state();
    st.apply_switch_position(ZWR1, SwitchPosition::MOVING, 5);

    const SetSwitchPositionCmd cmd{ZWR1, SwitchPosition::DIVERGENT};
    const auto v = check_set_switch_position(st, cmd);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->reason_code, NAK_SWITCH_MOVING);
}

TEST(SrkCommonDeviceRules, R1_Check_RouteLocked_ReturnsRouteLocked)
{
    auto st = make_state();
    st.apply_switch_lock(ZWR1, ROUTE1);

    const SetSwitchPositionCmd cmd{ZWR1, SwitchPosition::DIVERGENT};
    const auto v = check_set_switch_position(st, cmd);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->reason_code, NAK_ROUTE_LOCKED);
}

TEST(SrkCommonDeviceRules, R1_Check_AlreadyInPosition_ReturnsInvalidState)
{
    const auto st = make_state();  // switch already STRAIGHT
    const SetSwitchPositionCmd cmd{ZWR1, SwitchPosition::STRAIGHT};
    const auto v = check_set_switch_position(st, cmd);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->reason_code, NAK_INVALID_STATE);
}

TEST(SrkCommonDeviceRules, R1_Check_SwitchNotFound_ReturnsNotFound)
{
    const auto st = make_state();
    const SetSwitchPositionCmd cmd{GID{"ZWR-MISSING"}, SwitchPosition::DIVERGENT};
    const auto v = check_set_switch_position(st, cmd);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->reason_code, NAK_NOT_FOUND);
}

TEST(SrkCommonDeviceRules, R1_Execute_InstantThrow_ReturnsSinglePositionChange)
{
    const auto st = make_state();
    const SetSwitchPositionCmd cmd{ZWR1, SwitchPosition::DIVERGENT};
    const auto changes = execute_set_switch_position(st, cmd, /*throw_time_ticks=*/0);

    ASSERT_EQ(changes.size(), 1u);
    const auto* chg = std::get_if<SwitchPositionChange>(&changes[0]);
    ASSERT_NE(chg, nullptr);
    EXPECT_EQ(chg->gid, ZWR1);
    EXPECT_EQ(chg->new_position, SwitchPosition::DIVERGENT);
}

TEST(SrkCommonDeviceRules, R1_Execute_DelayedThrow_ReturnsMovingState)
{
    const auto st = make_state();
    const SetSwitchPositionCmd cmd{ZWR1, SwitchPosition::DIVERGENT};
    const auto changes = execute_set_switch_position(st, cmd, /*throw_time_ticks=*/3);

    ASSERT_EQ(changes.size(), 1u);
    const auto* chg = std::get_if<SwitchPositionChange>(&changes[0]);
    ASSERT_NE(chg, nullptr);
    EXPECT_EQ(chg->new_position, SwitchPosition::MOVING);
    EXPECT_EQ(chg->moving_ticks_remaining, 3);
}

// ── R2: SetSignalAspect ───────────────────────────────────────────────────────

TEST(SrkCommonDeviceRules, R2_Check_Valid_ReturnsNullopt)
{
    const auto st = make_state();
    const SetSignalAspectCmd cmd{SEM_W, SignalAspect::S2_PROCEED};
    EXPECT_FALSE(check_set_signal_aspect(st, cmd).has_value());
}

TEST(SrkCommonDeviceRules, R2_Check_SignalNotFound_ReturnsNotFound)
{
    const auto st = make_state();
    const SetSignalAspectCmd cmd{GID{"SEM-MISSING"}, SignalAspect::S2_PROCEED};
    const auto v = check_set_signal_aspect(st, cmd);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->reason_code, NAK_NOT_FOUND);
}

TEST(SrkCommonDeviceRules, R2_Check_RouteLocked_ProceedAspect_ReturnsRouteLocked)
{
    auto st = make_state();
    st.apply_signal_lock(SEM_W, ROUTE1);

    const SetSignalAspectCmd cmd{SEM_W, SignalAspect::S2_PROCEED};
    const auto v = check_set_signal_aspect(st, cmd);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->reason_code, NAK_ROUTE_LOCKED);
}

TEST(SrkCommonDeviceRules, R2_Check_RouteLocked_StopAspect_ReturnsNullopt)
{
    auto st = make_state();
    st.apply_signal_lock(SEM_W, ROUTE1);

    // STOP is allowed even on a route-locked signal.
    const SetSignalAspectCmd cmd{SEM_W, SignalAspect::S1_STOP};
    EXPECT_FALSE(check_set_signal_aspect(st, cmd).has_value());
}

TEST(SrkCommonDeviceRules, R2_Execute_ReturnsAspectChange)
{
    const auto st = make_state();
    const SetSignalAspectCmd cmd{SEM_W, SignalAspect::S2_PROCEED};
    const auto changes = execute_set_signal_aspect(st, cmd);

    ASSERT_EQ(changes.size(), 1u);
    const auto* chg = std::get_if<SignalAspectChange>(&changes[0]);
    ASSERT_NE(chg, nullptr);
    EXPECT_EQ(chg->gid, SEM_W);
    EXPECT_EQ(chg->new_aspect, SignalAspect::S2_PROCEED);
}

// ── R3: SetDerailerPosition ───────────────────────────────────────────────────

TEST(SrkCommonDeviceRules, R3_Check_Valid_Unlock_ReturnsNullopt)
{
    const auto st = make_state();
    const SetDerailerPositionCmd cmd{WK1, DerailerState::UNLOCKED};
    EXPECT_FALSE(check_set_derailer_position(st, cmd).has_value());
}

TEST(SrkCommonDeviceRules, R3_Check_DerailerNotFound_ReturnsNotFound)
{
    const auto st = make_state();
    const SetDerailerPositionCmd cmd{GID{"WK-MISSING"}, DerailerState::UNLOCKED};
    const auto v = check_set_derailer_position(st, cmd);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->reason_code, NAK_NOT_FOUND);
}

TEST(SrkCommonDeviceRules, R3_Check_RouteLocked_ReturnsRouteLocked)
{
    auto st = make_state();
    st.apply_derailer_lock(WK1, ROUTE1);

    const SetDerailerPositionCmd cmd{WK1, DerailerState::UNLOCKED};
    const auto v = check_set_derailer_position(st, cmd);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->reason_code, NAK_ROUTE_LOCKED);
}

TEST(SrkCommonDeviceRules, R3_Check_GuardedSectionOccupied_ReturnsSafetyBlock)
{
    auto st = make_state();
    st.apply_track_section_occupancy(TOR_A, TrackOccupancy::OCCUPIED, 4);

    const SetDerailerPositionCmd cmd{WK1, DerailerState::UNLOCKED};
    const auto v = check_set_derailer_position(st, cmd);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->reason_code, NAK_SAFETY_BLOCK);
}

TEST(SrkCommonDeviceRules, R3_Execute_ReturnsDerailerStateChange)
{
    const auto st = make_state();
    const SetDerailerPositionCmd cmd{WK1, DerailerState::UNLOCKED};
    const auto changes = execute_set_derailer_position(st, cmd);

    ASSERT_EQ(changes.size(), 1u);
    const auto* chg = std::get_if<DerailerStateChange>(&changes[0]);
    ASSERT_NE(chg, nullptr);
    EXPECT_EQ(chg->gid, WK1);
    EXPECT_EQ(chg->new_state, DerailerState::UNLOCKED);
}

// ── R4: SetBlockSection ───────────────────────────────────────────────────────

TEST(SrkCommonDeviceRules, R4_Check_OpenBlockSection_Valid_ReturnsNullopt)
{
    const auto st = make_state();
    const SetBlockSectionCmd cmd{BL1, BlockSectionState::OPEN};
    EXPECT_FALSE(check_set_block_section(st, cmd).has_value());
}

TEST(SrkCommonDeviceRules, R4_Check_CloseBlockSection_WithAxles_ReturnsSafetyBlock)
{
    auto st = make_state();
    st.apply_block_section_axle_count(BL1, 2);

    const SetBlockSectionCmd cmd{BL1, BlockSectionState::CLOSED};
    const auto v = check_set_block_section(st, cmd);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->reason_code, NAK_SAFETY_BLOCK);
}

TEST(SrkCommonDeviceRules, R4_Check_BlockSectionNotFound_ReturnsNotFound)
{
    const auto st = make_state();
    const SetBlockSectionCmd cmd{GID{"BL-MISSING"}, BlockSectionState::OPEN};
    const auto v = check_set_block_section(st, cmd);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->reason_code, NAK_NOT_FOUND);
}

// ── R7: AcknowledgeAlarm ──────────────────────────────────────────────────────

TEST(SrkCommonDeviceRules, R7_Check_AlarmExists_ReturnsNullopt)
{
    auto st = make_state();
    AlarmState alarm;
    alarm.alarm_id = ALARM1;
    alarm.object_gid = ZWR1;
    st.add_alarm(alarm);

    const AcknowledgeAlarmCmd cmd{ALARM1};
    EXPECT_FALSE(check_acknowledge_alarm(st, cmd).has_value());
}

TEST(SrkCommonDeviceRules, R7_Check_AlarmNotFound_ReturnsNotFound)
{
    const auto st = make_state();
    const AcknowledgeAlarmCmd cmd{GID{"ALM-MISSING"}};
    const auto v = check_acknowledge_alarm(st, cmd);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->reason_code, NAK_NOT_FOUND);
}

TEST(SrkCommonDeviceRules, R7_Execute_ReturnsAlarmCleared)
{
    auto st = make_state();
    AlarmState alarm;
    alarm.alarm_id = ALARM1;
    alarm.object_gid = ZWR1;
    st.add_alarm(alarm);

    const AcknowledgeAlarmCmd cmd{ALARM1};
    const auto changes = execute_acknowledge_alarm(st, cmd);

    ASSERT_EQ(changes.size(), 1u);
    const auto* chg = std::get_if<AlarmCleared>(&changes[0]);
    ASSERT_NE(chg, nullptr);
    EXPECT_EQ(chg->alarm_id, ALARM1);
}
