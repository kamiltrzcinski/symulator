// tests/srk/test_srk_common_device_rules.cpp
//
// Unit tests for srk::common device rules (R1–R7).
// These exercise the check_* and execute_* helpers directly,
// independently of any concrete IControlSystem implementation.
//
// Layout:
//   BND-N ── [tor_a] ── [zwr1(STRAIGHT)] ── [tor_b] ── BND-S
// Signals: SEM-W (governs tor_a), SEM-E (governs tor_b)
// Derailer: WK1 (guards tor_a)
// Block section: BL1

#include <gtest/gtest.h>

#include <tests/common/srk_state_builders.hpp>

#include <srk/common/device_rules.hpp>
#include <srk/common/nak_codes.hpp>

#include <engine/core/engine_state.hpp>

#include <variant>

namespace
{

using namespace engine::core;
using namespace srk::common;
namespace srk_test = tests::common::srk;

using srk_test::BL1;
using srk_test::SEM_W;
using srk_test::TOR_A;
using srk_test::WK1;
using srk_test::ZWR1;

constexpr UID ALARM1 = make_uid(UIDDomain::OPERATIONS, UIDKind::ALARM, 1, 1);
constexpr UID ROUTE1 = make_uid(UIDDomain::OPERATIONS, UIDKind::ROUTE, 1, 1);

// Non-existent UIDs for "not found" test cases
constexpr UID ZWR_MISSING = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::SWITCH, 1, 99);
constexpr UID SEM_MISSING = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::SIGNAL, 1, 99);
constexpr UID WK_MISSING = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::DERAILER, 1, 99);
constexpr UID BL_MISSING = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::BLOCK_SECTION, 1, 99);
constexpr UID ALM_MISSING = make_uid(UIDDomain::OPERATIONS, UIDKind::ALARM, 1, 99);

EngineState make_state()
{
    return srk_test::make_linear_infra_state_with_block();
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
    EXPECT_EQ(v->offending_uid, ZWR1);
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
    const auto st = make_state();
    const SetSwitchPositionCmd cmd{ZWR1, SwitchPosition::STRAIGHT};
    const auto v = check_set_switch_position(st, cmd);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->reason_code, NAK_INVALID_STATE);
}

TEST(SrkCommonDeviceRules, R1_Check_SwitchNotFound_ReturnsNotFound)
{
    const auto st = make_state();
    const SetSwitchPositionCmd cmd{ZWR_MISSING, SwitchPosition::DIVERGENT};
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
    EXPECT_EQ(chg->uid, ZWR1);
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
    const SetSignalAspectCmd cmd{SEM_MISSING, SignalAspect::S2_PROCEED};
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
    EXPECT_EQ(chg->uid, SEM_W);
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
    const SetDerailerPositionCmd cmd{WK_MISSING, DerailerState::UNLOCKED};
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
    EXPECT_EQ(chg->uid, WK1);
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
    const SetBlockSectionCmd cmd{BL_MISSING, BlockSectionState::OPEN};
    const auto v = check_set_block_section(st, cmd);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->reason_code, NAK_NOT_FOUND);
}

// ── R7: AcknowledgeAlarm ──────────────────────────────────────────────────────

TEST(SrkCommonDeviceRules, R7_Check_AlarmExists_ReturnsNullopt)
{
    auto st = make_state();
    AlarmState alarm;
    alarm.uid = ALARM1;
    alarm.object_uid = ZWR1;
    st.add_alarm(alarm);

    const AcknowledgeAlarmCmd cmd{ALARM1};
    EXPECT_FALSE(check_acknowledge_alarm(st, cmd).has_value());
}

TEST(SrkCommonDeviceRules, R7_Check_AlarmNotFound_ReturnsNotFound)
{
    const auto st = make_state();
    const AcknowledgeAlarmCmd cmd{ALM_MISSING};
    const auto v = check_acknowledge_alarm(st, cmd);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->reason_code, NAK_NOT_FOUND);
}

TEST(SrkCommonDeviceRules, R7_Execute_ReturnsAlarmCleared)
{
    auto st = make_state();
    AlarmState alarm;
    alarm.uid = ALARM1;
    alarm.object_uid = ZWR1;
    st.add_alarm(alarm);

    const AcknowledgeAlarmCmd cmd{ALARM1};
    const auto changes = execute_acknowledge_alarm(st, cmd);

    ASSERT_EQ(changes.size(), 1u);
    const auto* chg = std::get_if<AlarmCleared>(&changes[0]);
    ASSERT_NE(chg, nullptr);
    EXPECT_EQ(chg->alarm_uid, ALARM1);
}
