#include <gtest/gtest.h>

#include <engine/core/control_system.hpp>
#include <engine/core/engine_state.hpp>
#include <engine/core/state_applier.hpp>
#include <engine/core/track_model.hpp>
#include <engine/core/types.hpp>

namespace
{

using namespace engine::core;

// UIDs for test topology elements
constexpr UID kSemA = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::SIGNAL, 1, 1);
constexpr UID kOtTor = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::TRACK_SECTION, 1, 1);
constexpr UID kZwr1 = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::SWITCH, 1, 1);
constexpr UID kWk1 = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::DERAILER, 1, 1);
constexpr UID kBl1 = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::BLOCK_SECTION, 1, 1);
constexpr UID kSta1 = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::STATION, 1, 1);
constexpr UID kRtA = make_uid(UIDDomain::OPERATIONS, UIDKind::ROUTE, 1, 1);
constexpr UID kRt2 = make_uid(UIDDomain::OPERATIONS, UIDKind::ROUTE, 1, 2);
constexpr UID kRt3 = make_uid(UIDDomain::OPERATIONS, UIDKind::ROUTE, 1, 3);
constexpr UID kRtX = make_uid(UIDDomain::OPERATIONS, UIDKind::ROUTE, 1, 10);
constexpr UID kAlm1 = make_uid(UIDDomain::OPERATIONS, UIDKind::ALARM, 1, 1);
constexpr UID kSemMl8 = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::SIGNAL, 1, 2);
constexpr UID kSemB = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::SIGNAL, 1, 3);

// Minimal topology: one signal, one switch, one derailer, one block section.
EngineState make_state()
{
    EngineState st;
    st.set_session_id("TEST");
    st.set_current_tick(1);

    Signal sig;
    sig.uid = kSemA;
    sig.pid = "Wa";
    sig.type = Signal::Type::ENTRY;
    sig.governs_section_uid = kOtTor;
    sig.current_aspect = SignalAspect::S1_STOP;
    st.insert_signal(sig);

    Switch sw;
    sw.uid = kZwr1;
    sw.pid = "zwr1";
    sw.station_uid = kSta1;
    sw.position = SwitchPosition::STRAIGHT;
    st.insert_switch(sw);

    Derailer der;
    der.uid = kWk1;
    der.pid = "wk1";
    der.station_uid = kSta1;
    der.state = DerailerState::LOCKED;
    st.insert_derailer(der);

    BlockSection bs;
    bs.uid = kBl1;
    bs.pid = "bl1";
    bs.state = BlockSectionState::OPEN;
    bs.direction = BlockDirectionState::NEUTRAL;
    bs.axle_count = 0;
    st.insert_block_section(bs);

    return st;
}

// ── SignalAspectChange ────────────────────────────────────────────────────────

TEST(StateApplier, SignalAspectChange_setsAspect)
{
    auto st = make_state();
    apply(st,
          SignalAspectChange{kSemA, SignalAspect::S2_PROCEED, ChangeCause::COMMAND, std::nullopt});
    EXPECT_EQ(st.find_signal(kSemA)->current_aspect, SignalAspect::S2_PROCEED);
}

TEST(StateApplier, SignalAspectChange_setsLockWhenRouteIdPresent)
{
    auto st = make_state();
    apply(st, SignalAspectChange{kSemA, SignalAspect::S2_PROCEED, ChangeCause::AUTO, kRtA});
    const auto* sig = st.find_signal(kSemA);
    EXPECT_EQ(sig->current_aspect, SignalAspect::S2_PROCEED);
    EXPECT_TRUE(sig->locked_by_route_uid.has_value());
    EXPECT_EQ(*sig->locked_by_route_uid, kRtA);
}

TEST(StateApplier, SignalAspectChange_clearsLockOnStop)
{
    auto st = make_state();
    // First: set proceed + lock
    apply(st, SignalAspectChange{kSemA, SignalAspect::S2_PROCEED, ChangeCause::AUTO, kRtA});
    ASSERT_TRUE(st.find_signal(kSemA)->locked_by_route_uid.has_value());
    // Then: return to STOP (no route_uid → clears lock)
    apply(st, SignalAspectChange{kSemA, SignalAspect::S1_STOP, ChangeCause::COMMAND, std::nullopt});
    EXPECT_FALSE(st.find_signal(kSemA)->locked_by_route_uid.has_value());
}

// ── SwitchPositionChange ─────────────────────────────────────────────────────

TEST(StateApplier, SwitchPositionChange_setsPosition)
{
    auto st = make_state();
    apply(st, SwitchPositionChange{kZwr1, SwitchPosition::DIVERGENT, ChangeCause::COMMAND});
    EXPECT_EQ(st.find_switch(kZwr1)->position, SwitchPosition::DIVERGENT);
}

TEST(StateApplier, SwitchPositionChange_movingWithTicks)
{
    auto st = make_state();
    apply(st, SwitchPositionChange{kZwr1, SwitchPosition::MOVING, ChangeCause::COMMAND, 10});
    const auto* sw = st.find_switch(kZwr1);
    EXPECT_EQ(sw->position, SwitchPosition::MOVING);
    EXPECT_EQ(sw->moving_ticks_remaining, 10);
}

// ── SwitchLocked / SwitchUnlocked ────────────────────────────────────────────

TEST(StateApplier, SwitchLocked_setsRouteId)
{
    auto st = make_state();
    apply(st, SwitchLocked{kZwr1, kRt2});
    const auto* sw = st.find_switch(kZwr1);
    EXPECT_TRUE(sw->locked_by_route_uid.has_value());
    EXPECT_EQ(*sw->locked_by_route_uid, kRt2);
}

TEST(StateApplier, SwitchUnlocked_clearsRouteId)
{
    auto st = make_state();
    apply(st, SwitchLocked{kZwr1, kRt2});
    apply(st, SwitchUnlocked{kZwr1, kRt2});
    EXPECT_FALSE(st.find_switch(kZwr1)->locked_by_route_uid.has_value());
}

// ── DerailerStateChange ───────────────────────────────────────────────────────

TEST(StateApplier, DerailerStateChange_setsState)
{
    auto st = make_state();
    apply(st, DerailerStateChange{kWk1, DerailerState::UNLOCKED, ChangeCause::AUTO, std::nullopt});
    EXPECT_EQ(st.find_derailer(kWk1)->state, DerailerState::UNLOCKED);
}

TEST(StateApplier, DerailerStateChange_setsLockWhenRouteIdPresent)
{
    auto st = make_state();
    apply(st, DerailerStateChange{kWk1, DerailerState::UNLOCKED, ChangeCause::AUTO, kRt3});
    const auto* der = st.find_derailer(kWk1);
    EXPECT_TRUE(der->locked_by_route_uid.has_value());
    EXPECT_EQ(*der->locked_by_route_uid, kRt3);
}

TEST(StateApplier, DerailerStateChange_clearsLockOnLocked)
{
    auto st = make_state();
    apply(st, DerailerStateChange{kWk1, DerailerState::UNLOCKED, ChangeCause::AUTO, kRt3});
    apply(st, DerailerStateChange{kWk1, DerailerState::LOCKED, ChangeCause::AUTO, std::nullopt});
    EXPECT_FALSE(st.find_derailer(kWk1)->locked_by_route_uid.has_value());
}

// ── BlockSectionStateChange ───────────────────────────────────────────────────

TEST(StateApplier, BlockSectionStateChange_setsState)
{
    auto st = make_state();
    apply(st, BlockSectionStateChange{kBl1, BlockSectionState::CLOSED});
    EXPECT_EQ(st.find_block_section(kBl1)->state, BlockSectionState::CLOSED);
}

// ── BlockDirectionChange ─────────────────────────────────────────────────────

TEST(StateApplier, BlockDirectionChange_setsDirection)
{
    auto st = make_state();
    apply(st, BlockDirectionChange{kBl1, BlockDirectionState::OUTBOUND_PENDING});
    EXPECT_EQ(st.find_block_section(kBl1)->direction, BlockDirectionState::OUTBOUND_PENDING);
}

// ── RouteAdded / RouteRemoved ─────────────────────────────────────────────────

TEST(StateApplier, OperatorCommandStateChange_setsSignalStopFlag)
{
    auto st = make_state();
    apply(st, OperatorCommandStateChange{kSemA, OperatorTargetKind::SIGNAL,
                                         OperatorCommandCode::SES, true});
    EXPECT_TRUE(st.find_signal(kSemA)->operator_state.stopped);

    apply(st, OperatorCommandStateChange{kSemA, OperatorTargetKind::SIGNAL,
                                         OperatorCommandCode::SEO, false});
    EXPECT_FALSE(st.find_signal(kSemA)->operator_state.stopped);
}

TEST(StateApplier, Ml8CommandStateChange_recordsLastMl8Command)
{
    EngineState st;
    Signal sig;
    sig.uid = kSemMl8;
    st.insert_signal(sig);

    apply(st,
          Ml8CommandStateChange{kSemMl8, OperatorTargetKind::SIGNAL, Ml8CommandCode::STOJ, true});

    const auto* stored = st.find_signal(kSemMl8);
    ASSERT_NE(stored, nullptr);
    ASSERT_TRUE(stored->operator_state.active_ml8_command.has_value());
    EXPECT_EQ(stored->operator_state.active_ml8_command.value(), Ml8CommandCode::STOJ);
}

TEST(StateApplier, AxleCounterResetChange_resetsBlockAxleCount)
{
    auto st = make_state();
    st.apply_block_section_axle_count(kBl1, 4);
    apply(st, AxleCounterResetChange{kBl1, OperatorTargetKind::BLOCK_SECTION});
    EXPECT_EQ(st.find_block_section(kBl1)->axle_count, 0);
}

TEST(StateApplier, RouteAdded_addsRoute)
{
    auto st = make_state();
    RouteState rs;
    rs.uid = kRtA;
    rs.from_signal_uid = kSemA;
    rs.to_signal_uid = kSemB;
    apply(st, RouteAdded{rs});
    EXPECT_NE(st.find_route(kRtA), nullptr);
}

TEST(StateApplier, RouteRemoved_removesRoute)
{
    auto st = make_state();
    RouteState rs;
    rs.uid = kRtA;
    rs.from_signal_uid = kSemA;
    rs.to_signal_uid = kSemB;
    apply(st, RouteAdded{rs});
    ASSERT_NE(st.find_route(kRtA), nullptr);
    apply(st, RouteRemoved{kRtA, "OPERATOR_CANCEL"});
    EXPECT_EQ(st.find_route(kRtA), nullptr);
}

// ── AlarmRaised / AlarmCleared ────────────────────────────────────────────────

TEST(StateApplier, AlarmRaised_addsAlarm)
{
    auto st = make_state();
    AlarmState al;
    al.uid = kAlm1;
    al.kind = "TRACK_OCCUPIED_UNEXPECTEDLY";
    al.object_uid = kOtTor;
    al.message = "Unexpected occupancy";
    apply(st, AlarmRaised{al});
    EXPECT_NE(st.find_alarm(kAlm1), nullptr);
}

TEST(StateApplier, AlarmCleared_removesAlarm)
{
    auto st = make_state();
    AlarmState al;
    al.uid = kAlm1;
    al.kind = "ENGINE_FAULT";
    al.message = "Fault";
    apply(st, AlarmRaised{al});
    apply(st, AlarmCleared{kAlm1});
    EXPECT_EQ(st.find_alarm(kAlm1), nullptr);
}

// ── apply_all ────────────────────────────────────────────────────────────────

TEST(StateApplier, ApplyAll_appliesChangesInOrder)
{
    auto st = make_state();

    std::vector<DeviceStateChange> changes{
        SwitchPositionChange{kZwr1, SwitchPosition::DIVERGENT, ChangeCause::AUTO},
        SwitchLocked{kZwr1, kRtX},
        SignalAspectChange{kSemA, SignalAspect::S2_PROCEED, ChangeCause::AUTO, kRtX},
    };
    apply_all(st, changes);

    EXPECT_EQ(st.find_switch(kZwr1)->position, SwitchPosition::DIVERGENT);
    EXPECT_TRUE(st.find_switch(kZwr1)->locked_by_route_uid.has_value());
    EXPECT_EQ(st.find_signal(kSemA)->current_aspect, SignalAspect::S2_PROCEED);
    EXPECT_TRUE(st.find_signal(kSemA)->locked_by_route_uid.has_value());
}

}  // namespace
