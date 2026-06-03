#include <gtest/gtest.h>

#include <engine/core/command.hpp>
#include <engine/core/control_system.hpp>
#include <engine/core/control_system_registry.hpp>
#include <engine/core/engine_state.hpp>
#include <srk/ebilock/ebilock_system.hpp>

#include <algorithm>

// ── Test helpers ──────────────────────────────────────────────────────────────
// Build a minimal EngineState topology for testing interlocking rules.
//
// Layout (linear):
//   BND-N  ──  [tor_a]  ──  [zwr1]  ──  [tor_b]  ──  BND-S
//
// Signals:
//   SEM-W : entry signal on tor_a, facing south (from BND-N)
//   SEM-E : departure signal on tor_b, facing north (toward BND-S)

namespace
{

using namespace engine::core;

// IDs used throughout the tests
constexpr UID BND_N = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::BOUNDARY_NODE, 1, 1);
constexpr UID BND_S = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::BOUNDARY_NODE, 1, 2);
constexpr UID TOR_A = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::TRACK_SECTION, 1, 1);
constexpr UID TOR_B = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::TRACK_SECTION, 1, 2);
constexpr UID ZWR1 = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::SWITCH, 1, 1);
constexpr UID SEM_W = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::SIGNAL, 1, 1);
constexpr UID SEM_E = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::SIGNAL, 1, 2);
constexpr UID WK1 = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::DERAILER, 1, 1);
constexpr UID BL1 = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::BLOCK_SECTION, 1, 1);

// Counter UIDs (axle counters)
constexpr UID IT_A_N = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::AXLE_COUNTER, 1, 1);
constexpr UID IZ_A_S = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::AXLE_COUNTER, 1, 2);
constexpr UID IZ_B_N = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::AXLE_COUNTER, 1, 3);
constexpr UID IT_B_S = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::AXLE_COUNTER, 1, 4);
constexpr UID IZ_DIV = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::AXLE_COUNTER, 1, 5);

// Station UIDs
constexpr UID STA1 = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::STATION, 1, 1);
constexpr UID STA_NGR = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::STATION, 2, 1);

// Route/lock UIDs for test state setup
constexpr UID RTE_001 = make_uid(UIDDomain::OPERATIONS, UIDKind::ROUTE, 1, 1);
constexpr UID RTE_EXISTING = make_uid(UIDDomain::OPERATIONS, UIDKind::ROUTE, 1, 2);

// Alarm UID
constexpr UID ALM_001 = make_uid(UIDDomain::OPERATIONS, UIDKind::ALARM, 1, 1);
constexpr UID ALM_GHOST = make_uid(UIDDomain::OPERATIONS, UIDKind::ALARM, 1, 99);

// Level crossing UID
constexpr UID MMZ_2148 = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::LEVEL_CROSSING, 1, 1);

EngineState make_state()
{
    EngineState st;
    st.set_session_id("TEST");
    st.set_current_tick(1);

    // Boundary nodes
    BoundaryNode bn_n;
    bn_n.uid = BND_N;
    bn_n.pid = "BND-N";
    BoundaryNode bn_s;
    bn_s.uid = BND_S;
    bn_s.pid = "BND-S";
    st.insert_boundary_node(bn_n);
    st.insert_boundary_node(bn_s);

    // Signals
    Signal sw;
    sw.uid = SEM_W;
    sw.pid = "Wp1";
    sw.type = Signal::Type::ENTRY;
    sw.governs_section_uid = TOR_A;
    sw.current_aspect = SignalAspect::S1_STOP;
    st.insert_signal(sw);

    Signal se;
    se.uid = SEM_E;
    se.pid = "Wy1";
    se.type = Signal::Type::DEPARTURE;
    se.governs_section_uid = TOR_B;
    se.current_aspect = SignalAspect::S1_STOP;
    st.insert_signal(se);

    // tor_a:  BND-N ── (IT-a-N) ──[tor_a]── (IZ-a-S, connects to zwr1) ── ZWR1
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

    // tor_b:  ZWR1 ── (IZ-b-N) ──[tor_b]── (IT-b-S) ── BND-S
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

    // zwr1 (switch): trunk→tor_a, straight→tor_b, divergent→BND-S (simplified)
    Switch sw1;
    sw1.uid = ZWR1;
    sw1.pid = "zwr1";
    sw1.station_uid = STA1;
    sw1.type_id = "DVT-GLB-ZWR-EEA4-0000002";
    sw1.trunk.neighbor_uid = TOR_A;
    sw1.trunk.iz_uid = IZ_A_S;
    sw1.straight.neighbor_uid = TOR_B;
    sw1.straight.iz_uid = IZ_B_N;
    sw1.divergent.neighbor_uid = BND_S;
    sw1.divergent.iz_uid = IZ_DIV;
    sw1.position = SwitchPosition::STRAIGHT;
    sw1.occupancy = TrackOccupancy::FREE;
    st.insert_switch(sw1);

    // Derailer on tor_a
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

EngineState make_state_with_block(BlockDirectionState dir = BlockDirectionState::NEUTRAL,
                                  BlockSectionState state = BlockSectionState::CLOSED,
                                  int axle_count = 0)
{
    EngineState st;
    st.set_session_id("TEST");
    st.set_current_tick(1);

    BlockSection bs;
    bs.uid = BL1;
    bs.pid = "bl1";
    bs.station_uid = STA1;
    bs.neighbor_station_uid = STA_NGR;
    bs.direction = dir;
    bs.state = state;
    bs.axle_count = axle_count;
    st.insert_block_section(bs);

    return st;
}

}  // namespace

// ── R1: SetSwitchPosition ─────────────────────────────────────────────────────

TEST(EbiLockR1, AcceptsValidPositionChange)
{
    srk::ebilock::EbiLockSystem sys{0};  // throw_ticks=0 for instant switch
    auto st = make_state();

    Command cmd = SetSwitchPositionCmd{ZWR1, SwitchPosition::DIVERGENT};
    EXPECT_FALSE(sys.check_command(st, cmd).has_value());

    auto changes = sys.execute_command(st, cmd);
    ASSERT_EQ(changes.size(), 1u);
    auto* chg = std::get_if<SwitchPositionChange>(&changes[0]);
    ASSERT_NE(chg, nullptr);
    EXPECT_EQ(chg->new_position, SwitchPosition::DIVERGENT);
}

TEST(EbiLockR1, RejectsOccupiedSwitch)
{
    srk::ebilock::EbiLockSystem sys{0};
    auto st = make_state();
    st.apply_switch_occupancy(ZWR1, TrackOccupancy::OCCUPIED, 4);

    Command cmd = SetSwitchPositionCmd{ZWR1, SwitchPosition::DIVERGENT};
    auto v = sys.check_command(st, cmd);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->reason_code, 0x02);  // SAFETY_BLOCK
}

TEST(EbiLockR1, RejectsMovingSwitch)
{
    srk::ebilock::EbiLockSystem sys{90};
    auto st = make_state();
    // Manually put switch in MOVING state
    st.apply_switch_position(ZWR1, SwitchPosition::MOVING, 45);

    Command cmd = SetSwitchPositionCmd{ZWR1, SwitchPosition::DIVERGENT};
    auto v = sys.check_command(st, cmd);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->reason_code, 0x06);  // SWITCH_MOVING
}

TEST(EbiLockR1, RejectsAlreadyInPosition)
{
    srk::ebilock::EbiLockSystem sys{0};
    auto st = make_state();  // switch already STRAIGHT

    Command cmd = SetSwitchPositionCmd{ZWR1, SwitchPosition::STRAIGHT};
    auto v = sys.check_command(st, cmd);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->reason_code, 0x03);  // INVALID_STATE
}

TEST(EbiLockR1, RejectsRouteLocked)
{
    srk::ebilock::EbiLockSystem sys{0};
    auto st = make_state();
    st.apply_switch_lock(ZWR1, RTE_001);

    Command cmd = SetSwitchPositionCmd{ZWR1, SwitchPosition::DIVERGENT};
    auto v = sys.check_command(st, cmd);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->reason_code, 0x04);  // ROUTE_LOCKED
}

// ── R1: EEA-4 throw timer ─────────────────────────────────────────────────────

TEST(EbiLockR1, ThrowTimerCounts)
{
    srk::ebilock::EbiLockSystem sys{3};  // 3 ticks
    auto st = make_state();

    Command cmd = SetSwitchPositionCmd{ZWR1, SwitchPosition::DIVERGENT};
    auto changes = sys.execute_command(st, cmd);

    // First change should be MOVING with 3 ticks remaining
    ASSERT_EQ(changes.size(), 1u);
    auto* chg = std::get_if<SwitchPositionChange>(&changes[0]);
    ASSERT_NE(chg, nullptr);
    EXPECT_EQ(chg->new_position, SwitchPosition::MOVING);
    EXPECT_EQ(chg->moving_ticks_remaining, 3);

    // Apply MOVING state
    st.apply_switch_position(ZWR1, SwitchPosition::MOVING, 3);

    // on_tick should count down and eventually land on DIVERGENT
    for (int i = 3; i > 1; --i)
    {
        auto tc = sys.on_tick(st, 0);
        ASSERT_EQ(tc.size(), 1u);
        auto* tc_chg = std::get_if<SwitchPositionChange>(&tc[0]);
        ASSERT_NE(tc_chg, nullptr);
        EXPECT_EQ(tc_chg->new_position, SwitchPosition::MOVING);
        st.apply_switch_position(ZWR1, SwitchPosition::MOVING, tc_chg->moving_ticks_remaining);
    }

    auto final_changes = sys.on_tick(st, 0);
    ASSERT_FALSE(final_changes.empty());
    auto* fc = std::get_if<SwitchPositionChange>(&final_changes[0]);
    ASSERT_NE(fc, nullptr);
    EXPECT_EQ(fc->new_position, SwitchPosition::DIVERGENT);
    EXPECT_EQ(fc->moving_ticks_remaining, 0);
}

// ── R2: SetSignalAspect ───────────────────────────────────────────────────────

TEST(EbiLockR2, AcceptsStopOnUnlockedSignal)
{
    srk::ebilock::EbiLockSystem sys{0};
    auto st = make_state();
    st.apply_signal_aspect(SEM_W, SignalAspect::S2_PROCEED);

    Command cmd = SetSignalAspectCmd{SEM_W, SignalAspect::S1_STOP};
    EXPECT_FALSE(sys.check_command(st, cmd).has_value());
}

TEST(EbiLockR2, RejectsProceedOnRouteLocked)
{
    srk::ebilock::EbiLockSystem sys{0};
    auto st = make_state();
    st.apply_signal_lock(SEM_W, RTE_001);

    Command cmd = SetSignalAspectCmd{SEM_W, SignalAspect::S2_PROCEED};
    auto v = sys.check_command(st, cmd);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->reason_code, 0x04);  // ROUTE_LOCKED
}

TEST(EbiLockR2, AcceptsStopOnRouteLocked)
{
    srk::ebilock::EbiLockSystem sys{0};
    auto st = make_state();
    st.apply_signal_lock(SEM_W, RTE_001);
    st.apply_signal_aspect(SEM_W, SignalAspect::S2_PROCEED);

    Command cmd = SetSignalAspectCmd{SEM_W, SignalAspect::S1_STOP};
    EXPECT_FALSE(sys.check_command(st, cmd).has_value());
}

// ── R3: SetDerailerPosition ───────────────────────────────────────────────────

TEST(EbiLockR3, AcceptsUnlockOnFreeSection)
{
    srk::ebilock::EbiLockSystem sys{0};
    auto st = make_state();

    Command cmd = SetDerailerPositionCmd{WK1, DerailerState::UNLOCKED};
    EXPECT_FALSE(sys.check_command(st, cmd).has_value());
}

TEST(EbiLockR3, RejectsUnlockOnOccupiedSection)
{
    srk::ebilock::EbiLockSystem sys{0};
    auto st = make_state();
    st.apply_track_section_occupancy(TOR_A, TrackOccupancy::OCCUPIED, 2);

    Command cmd = SetDerailerPositionCmd{WK1, DerailerState::UNLOCKED};
    auto v = sys.check_command(st, cmd);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->reason_code, 0x02);  // SAFETY_BLOCK
}

TEST(EbiLockR3, RejectsWhenRouteLocked)
{
    srk::ebilock::EbiLockSystem sys{0};
    auto st = make_state();
    st.apply_derailer_lock(WK1, RTE_001);

    Command cmd = SetDerailerPositionCmd{WK1, DerailerState::UNLOCKED};
    auto v = sys.check_command(st, cmd);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->reason_code, 0x04);  // ROUTE_LOCKED
}

// ── R5: RequestRoute ──────────────────────────────────────────────────────────

TEST(EbiLockR5, AcceptsValidRoute)
{
    srk::ebilock::EbiLockSystem sys{0};
    auto st = make_state();

    Command cmd = RequestRouteCmd{SEM_W, SEM_E};
    EXPECT_FALSE(sys.check_command(st, cmd).has_value());

    auto changes = sys.execute_command(st, cmd);

    // Expect: SwitchPositionChange (already straight, so may be skipped),
    //         SwitchLocked, DerailerStateChange (unlock wk1), SignalAspectChange, RouteAdded
    bool route_added = false;
    bool signal_proceed = false;
    for (auto& c : changes)
    {
        if (auto* ra = std::get_if<RouteAdded>(&c))
        {
            route_added = true;
            EXPECT_EQ(ra->route.from_signal_uid, SEM_W);
            EXPECT_EQ(ra->route.to_signal_uid, SEM_E);
            EXPECT_NE(ra->route.uid.value, 0u);
        }
        if (auto* sa = std::get_if<SignalAspectChange>(&c))
        {
            if (sa->uid == SEM_W && sa->new_aspect == SignalAspect::S2_PROCEED)
                signal_proceed = true;
        }
    }
    EXPECT_TRUE(route_added);
    EXPECT_TRUE(signal_proceed);
}

TEST(EbiLockR5, RejectsWhenEntrySignalAlreadyLocked)
{
    srk::ebilock::EbiLockSystem sys{0};
    auto st = make_state();
    st.apply_signal_lock(SEM_W, RTE_EXISTING);

    Command cmd = RequestRouteCmd{SEM_W, SEM_E};
    auto v = sys.check_command(st, cmd);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->reason_code, 0x04);  // ROUTE_LOCKED
}

TEST(EbiLockR5, RejectsWhenSectionOccupied)
{
    srk::ebilock::EbiLockSystem sys{0};
    auto st = make_state();
    st.apply_track_section_occupancy(TOR_A, TrackOccupancy::OCCUPIED, 4);

    Command cmd = RequestRouteCmd{SEM_W, SEM_E};
    auto v = sys.check_command(st, cmd);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->reason_code, 0x02);  // SAFETY_BLOCK
}

// ── R7: AcknowledgeAlarm ──────────────────────────────────────────────────────

TEST(EbiLockR7, AcceptsExistingAlarm)
{
    srk::ebilock::EbiLockSystem sys{0};
    auto st = make_state();
    AlarmState alarm;
    alarm.uid = ALM_001;
    alarm.kind = "SWITCH_FAILURE";
    alarm.object_uid = ZWR1;
    st.add_alarm(alarm);

    Command cmd = AcknowledgeAlarmCmd{ALM_001};
    EXPECT_FALSE(sys.check_command(st, cmd).has_value());

    auto changes = sys.execute_command(st, cmd);
    ASSERT_EQ(changes.size(), 1u);
    auto* ac = std::get_if<AlarmCleared>(&changes[0]);
    ASSERT_NE(ac, nullptr);
    EXPECT_EQ(ac->alarm_uid, ALM_001);
}

TEST(EbiLockR7, RejectsNonExistentAlarm)
{
    srk::ebilock::EbiLockSystem sys{0};
    auto st = make_state();

    Command cmd = AcknowledgeAlarmCmd{ALM_GHOST};
    auto v = sys.check_command(st, cmd);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->reason_code, 0x01);  // NOT_FOUND
}

// ── SHL-12 block commands ─────────────────────────────────────────────────────

TEST(EbiLockShl12, SupportedCommandTypesIncludeBlockCommands)
{
    srk::ebilock::EbiLockSystem sys{0};

    const auto types = sys.supported_command_types();
    EXPECT_NE(std::find(types.begin(), types.end(), "SetBlockDirectionCmd"), types.end());
    EXPECT_NE(std::find(types.begin(), types.end(), "InitAxleCounterResetCmd"), types.end());
    EXPECT_NE(std::find(types.begin(), types.end(), "ResetAxleCounterCmd"), types.end());
}

TEST(EbiLockShl12, BLWAcceptsFromNeutral)
{
    srk::ebilock::EbiLockSystem sys{0};
    auto st = make_state_with_block(BlockDirectionState::NEUTRAL);

    Command cmd = SetBlockDirectionCmd{BL1, Shl12Op::BLW};
    EXPECT_FALSE(sys.check_command(st, cmd).has_value());

    auto changes = sys.execute_command(st, cmd);
    ASSERT_EQ(changes.size(), 1u);
    auto* bdc = std::get_if<BlockDirectionChange>(&changes[0]);
    ASSERT_NE(bdc, nullptr);
    EXPECT_EQ(bdc->new_direction, BlockDirectionState::OUTBOUND_PENDING);
    EXPECT_TRUE(bdc->requires_neighbor_confirmation);
}

TEST(EbiLockShl12, BLPConfirmsOutboundAndOpensBlock)
{
    srk::ebilock::EbiLockSystem sys{0};
    auto st = make_state_with_block(BlockDirectionState::OUTBOUND_PENDING);

    Command cmd = SetBlockDirectionCmd{BL1, Shl12Op::BLP};
    EXPECT_FALSE(sys.check_command(st, cmd).has_value());

    auto changes = sys.execute_command(st, cmd);
    ASSERT_EQ(changes.size(), 2u);
    auto* bdc = std::get_if<BlockDirectionChange>(&changes[0]);
    ASSERT_NE(bdc, nullptr);
    EXPECT_EQ(bdc->new_direction, BlockDirectionState::OUTBOUND);
    auto* bsc = std::get_if<BlockSectionStateChange>(&changes[1]);
    ASSERT_NE(bsc, nullptr);
    EXPECT_EQ(bsc->new_state, BlockSectionState::OPEN);
}

TEST(EbiLockShl12, BLZRejectsOccupiedBlock)
{
    srk::ebilock::EbiLockSystem sys{0};
    auto st = make_state_with_block(BlockDirectionState::OUTBOUND, BlockSectionState::OPEN, 2);

    Command cmd = SetBlockDirectionCmd{BL1, Shl12Op::BLZ};
    auto v = sys.check_command(st, cmd);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->reason_code, 0x02);  // SAFETY_BLOCK
}

TEST(EbiLockShl12, SLIAndSLKResetAxleCounterProcedure)
{
    srk::ebilock::EbiLockSystem sys{0};
    auto st = make_state_with_block(BlockDirectionState::NEUTRAL);

    Command init = InitAxleCounterResetCmd{BL1};
    EXPECT_FALSE(sys.check_command(st, init).has_value());
    auto init_changes = sys.execute_command(st, init);
    ASSERT_EQ(init_changes.size(), 1u);
    auto* init_dir = std::get_if<BlockDirectionChange>(&init_changes[0]);
    ASSERT_NE(init_dir, nullptr);
    EXPECT_EQ(init_dir->new_direction, BlockDirectionState::RESET_PENDING);

    st.apply_block_section_direction(BL1, BlockDirectionState::RESET_PENDING);

    Command reset = ResetAxleCounterCmd{BL1};
    EXPECT_FALSE(sys.check_command(st, reset).has_value());
    auto reset_changes = sys.execute_command(st, reset);
    ASSERT_EQ(reset_changes.size(), 2u);
    auto* reset_dir = std::get_if<BlockDirectionChange>(&reset_changes[0]);
    ASSERT_NE(reset_dir, nullptr);
    EXPECT_EQ(reset_dir->new_direction, BlockDirectionState::NEUTRAL);
    auto* close = std::get_if<BlockSectionStateChange>(&reset_changes[1]);
    ASSERT_NE(close, nullptr);
    EXPECT_EQ(close->new_state, BlockSectionState::CLOSED);
}

// ── ControlSystemRegistry integration ────────────────────────────────────────

TEST(EbiLockOperatorCommands, SESStopsSignal)
{
    srk::ebilock::EbiLockSystem sys{0};
    auto st = make_state();
    st.apply_signal_aspect(SEM_W, SignalAspect::S2_PROCEED);

    Command cmd = OperatorCommandCmd{SEM_W, OperatorTargetKind::SIGNAL, OperatorCommandCode::SES};
    EXPECT_FALSE(sys.check_command(st, cmd).has_value());
    auto changes = sys.execute_command(st, cmd);

    bool stopped = false;
    for (const auto& change : changes)
    {
        if (const auto* sig = std::get_if<SignalAspectChange>(&change))
            stopped = sig->uid == SEM_W && sig->new_aspect == SignalAspect::S1_STOP;
    }
    EXPECT_TRUE(stopped);
}

TEST(EbiLockOperatorCommands, BLWUsesGenericOperatorCommand)
{
    srk::ebilock::EbiLockSystem sys{0};
    auto st = make_state_with_block(BlockDirectionState::NEUTRAL);

    Command cmd =
        OperatorCommandCmd{BL1, OperatorTargetKind::BLOCK_SECTION, OperatorCommandCode::BLW};
    EXPECT_FALSE(sys.check_command(st, cmd).has_value());
    auto changes = sys.execute_command(st, cmd);

    ASSERT_GE(changes.size(), 2u);
    auto* bdc = std::get_if<BlockDirectionChange>(&changes[1]);
    ASSERT_NE(bdc, nullptr);
    EXPECT_EQ(bdc->new_direction, BlockDirectionState::OUTBOUND_PENDING);
}

TEST(EbiLockOperatorCommands, BPZUsesGenericOperatorCommand)
{
    srk::ebilock::EbiLockSystem sys{0};
    auto st = make_state_with_block(BlockDirectionState::NEUTRAL);

    Command cmd =
        OperatorCommandCmd{BL1, OperatorTargetKind::BLOCK_SECTION, OperatorCommandCode::BPZ};
    EXPECT_FALSE(sys.check_command(st, cmd).has_value());
    auto changes = sys.execute_command(st, cmd);

    ASSERT_GE(changes.size(), 2u);
    auto* bdc = std::get_if<BlockDirectionChange>(&changes[1]);
    ASSERT_NE(bdc, nullptr);
    EXPECT_EQ(bdc->new_direction, BlockDirectionState::OUTBOUND_PENDING);
}

TEST(EbiLockOperatorCommands, LevelCrossingCommandAcceptedAsOperatorState)
{
    srk::ebilock::EbiLockSystem sys{0};
    auto st = make_state();

    Command cmd =
        OperatorCommandCmd{MMZ_2148, OperatorTargetKind::LEVEL_CROSSING, OperatorCommandCode::PDZ};
    EXPECT_FALSE(sys.check_command(st, cmd).has_value());
    auto changes = sys.execute_command(st, cmd);

    ASSERT_EQ(changes.size(), 1u);
    auto* op = std::get_if<OperatorCommandStateChange>(&changes[0]);
    ASSERT_NE(op, nullptr);
    EXPECT_EQ(op->uid, MMZ_2148);
    EXPECT_EQ(op->target_kind, OperatorTargetKind::LEVEL_CROSSING);
    EXPECT_EQ(op->code, OperatorCommandCode::PDZ);
}

TEST(ControlSystemRegistry, EbiLockRegistered)
{
    EXPECT_TRUE(engine::core::ControlSystemRegistry::instance().has("ebilock_x4"));
}
