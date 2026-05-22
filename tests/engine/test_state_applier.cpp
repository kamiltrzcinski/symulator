#include <gtest/gtest.h>

#include <engine/core/control_system.hpp>
#include <engine/core/engine_state.hpp>
#include <engine/core/state_applier.hpp>
#include <engine/core/track_model.hpp>
#include <engine/core/types.hpp>

namespace
{

using namespace engine::core;

// Minimal topology: one signal, one switch, one derailer, one block section.
EngineState make_state()
{
    EngineState st;
    st.set_session_id("TEST");
    st.set_current_tick(1);

    Signal sig;
    sig.gid = GID{"SEM-A"};
    sig.pid = "Wa";
    sig.type = Signal::Type::ENTRY;
    sig.governs_track_section_gid = GID{"OT-tor"};
    sig.current_aspect = SignalAspect::S1_STOP;
    st.insert_signal(sig);

    Switch sw;
    sw.gid = GID{"ZWR-1"};
    sw.pid = "zwr1";
    sw.sid = SID{"TST"};
    sw.position = SwitchPosition::STRAIGHT;
    st.insert_switch(sw);

    Derailer der;
    der.gid = GID{"WK-1"};
    der.pid = "wk1";
    der.sid = SID{"TST"};
    der.state = DerailerState::LOCKED;
    st.insert_derailer(der);

    BlockSection bs;
    bs.gid = GID{"BL-1"};
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
    apply(st, SignalAspectChange{GID{"SEM-A"}, SignalAspect::S2_PROCEED, ChangeCause::COMMAND,
                                 std::nullopt});
    EXPECT_EQ(st.find_signal(GID{"SEM-A"})->current_aspect, SignalAspect::S2_PROCEED);
}

TEST(StateApplier, SignalAspectChange_setsLockWhenRouteIdPresent)
{
    auto st = make_state();
    GID route{"RT-1"};
    apply(st, SignalAspectChange{GID{"SEM-A"}, SignalAspect::S2_PROCEED, ChangeCause::AUTO, route});
    const auto* sig = st.find_signal(GID{"SEM-A"});
    EXPECT_EQ(sig->current_aspect, SignalAspect::S2_PROCEED);
    EXPECT_TRUE(sig->locked_by_route.has_value());
    EXPECT_EQ(*sig->locked_by_route, route);
}

TEST(StateApplier, SignalAspectChange_clearsLockOnStop)
{
    auto st = make_state();
    GID route{"RT-1"};
    // First: set proceed + lock
    apply(st, SignalAspectChange{GID{"SEM-A"}, SignalAspect::S2_PROCEED, ChangeCause::AUTO, route});
    ASSERT_TRUE(st.find_signal(GID{"SEM-A"})->locked_by_route.has_value());
    // Then: return to STOP (no route_id → clears lock)
    apply(st, SignalAspectChange{GID{"SEM-A"}, SignalAspect::S1_STOP, ChangeCause::COMMAND,
                                 std::nullopt});
    EXPECT_FALSE(st.find_signal(GID{"SEM-A"})->locked_by_route.has_value());
}

// ── SwitchPositionChange ─────────────────────────────────────────────────────

TEST(StateApplier, SwitchPositionChange_setsPosition)
{
    auto st = make_state();
    apply(st, SwitchPositionChange{GID{"ZWR-1"}, SwitchPosition::DIVERGENT, ChangeCause::COMMAND});
    EXPECT_EQ(st.find_switch(GID{"ZWR-1"})->position, SwitchPosition::DIVERGENT);
}

TEST(StateApplier, SwitchPositionChange_movingWithTicks)
{
    auto st = make_state();
    apply(st, SwitchPositionChange{GID{"ZWR-1"}, SwitchPosition::MOVING, ChangeCause::COMMAND, 10});
    const auto* sw = st.find_switch(GID{"ZWR-1"});
    EXPECT_EQ(sw->position, SwitchPosition::MOVING);
    EXPECT_EQ(sw->moving_ticks_remaining, 10);
}

// ── SwitchLocked / SwitchUnlocked ────────────────────────────────────────────

TEST(StateApplier, SwitchLocked_setsRouteId)
{
    auto st = make_state();
    GID route{"RT-2"};
    apply(st, SwitchLocked{GID{"ZWR-1"}, route});
    const auto* sw = st.find_switch(GID{"ZWR-1"});
    EXPECT_TRUE(sw->locked_by_route.has_value());
    EXPECT_EQ(*sw->locked_by_route, route);
}

TEST(StateApplier, SwitchUnlocked_clearsRouteId)
{
    auto st = make_state();
    GID route{"RT-2"};
    apply(st, SwitchLocked{GID{"ZWR-1"}, route});
    apply(st, SwitchUnlocked{GID{"ZWR-1"}, route});
    EXPECT_FALSE(st.find_switch(GID{"ZWR-1"})->locked_by_route.has_value());
}

// ── DerailerStateChange ───────────────────────────────────────────────────────

TEST(StateApplier, DerailerStateChange_setsState)
{
    auto st = make_state();
    apply(st, DerailerStateChange{GID{"WK-1"}, DerailerState::UNLOCKED, ChangeCause::AUTO,
                                  std::nullopt});
    EXPECT_EQ(st.find_derailer(GID{"WK-1"})->state, DerailerState::UNLOCKED);
}

TEST(StateApplier, DerailerStateChange_setsLockWhenRouteIdPresent)
{
    auto st = make_state();
    GID route{"RT-3"};
    apply(st, DerailerStateChange{GID{"WK-1"}, DerailerState::UNLOCKED, ChangeCause::AUTO, route});
    const auto* der = st.find_derailer(GID{"WK-1"});
    EXPECT_TRUE(der->locked_by_route.has_value());
    EXPECT_EQ(*der->locked_by_route, route);
}

TEST(StateApplier, DerailerStateChange_clearsLockOnLocked)
{
    auto st = make_state();
    GID route{"RT-3"};
    apply(st, DerailerStateChange{GID{"WK-1"}, DerailerState::UNLOCKED, ChangeCause::AUTO, route});
    apply(st,
          DerailerStateChange{GID{"WK-1"}, DerailerState::LOCKED, ChangeCause::AUTO, std::nullopt});
    EXPECT_FALSE(st.find_derailer(GID{"WK-1"})->locked_by_route.has_value());
}

// ── BlockSectionStateChange ───────────────────────────────────────────────────

TEST(StateApplier, BlockSectionStateChange_setsState)
{
    auto st = make_state();
    apply(st, BlockSectionStateChange{GID{"BL-1"}, BlockSectionState::CLOSED});
    EXPECT_EQ(st.find_block_section(GID{"BL-1"})->state, BlockSectionState::CLOSED);
}

// ── BlockDirectionChange ─────────────────────────────────────────────────────

TEST(StateApplier, BlockDirectionChange_setsDirection)
{
    auto st = make_state();
    apply(st, BlockDirectionChange{GID{"BL-1"}, BlockDirectionState::OUTBOUND_PENDING});
    EXPECT_EQ(st.find_block_section(GID{"BL-1"})->direction, BlockDirectionState::OUTBOUND_PENDING);
}

// ── RouteAdded / RouteRemoved ─────────────────────────────────────────────────

TEST(StateApplier, OperatorCommandStateChange_setsSignalStopFlag)
{
    auto st = make_state();
    apply(st, OperatorCommandStateChange{GID{"SEM-A"}, OperatorTargetKind::SIGNAL,
                                         OperatorCommandCode::SES, true});
    EXPECT_TRUE(st.find_signal(GID{"SEM-A"})->operator_state.stopped);

    apply(st, OperatorCommandStateChange{GID{"SEM-A"}, OperatorTargetKind::SIGNAL,
                                         OperatorCommandCode::SEO, false});
    EXPECT_FALSE(st.find_signal(GID{"SEM-A"})->operator_state.stopped);
}

TEST(StateApplier, AxleCounterResetChange_resetsBlockAxleCount)
{
    auto st = make_state();
    st.apply_block_section_axle_count(GID{"BL-1"}, 4);
    apply(st, AxleCounterResetChange{GID{"BL-1"}, OperatorTargetKind::BLOCK_SECTION});
    EXPECT_EQ(st.find_block_section(GID{"BL-1"})->axle_count, 0);
}

TEST(StateApplier, RouteAdded_addsRoute)
{
    auto st = make_state();
    RouteState rs;
    rs.route_id = GID{"RT-A"};
    rs.from_signal_gid = GID{"SEM-A"};
    rs.to_signal_gid = GID{"SEM-B"};
    apply(st, RouteAdded{rs});
    EXPECT_NE(st.find_route(GID{"RT-A"}), nullptr);
}

TEST(StateApplier, RouteRemoved_removesRoute)
{
    auto st = make_state();
    RouteState rs;
    rs.route_id = GID{"RT-A"};
    rs.from_signal_gid = GID{"SEM-A"};
    rs.to_signal_gid = GID{"SEM-B"};
    apply(st, RouteAdded{rs});
    ASSERT_NE(st.find_route(GID{"RT-A"}), nullptr);
    apply(st, RouteRemoved{GID{"RT-A"}, "OPERATOR_CANCEL"});
    EXPECT_EQ(st.find_route(GID{"RT-A"}), nullptr);
}

// ── AlarmRaised / AlarmCleared ────────────────────────────────────────────────

TEST(StateApplier, AlarmRaised_addsAlarm)
{
    auto st = make_state();
    AlarmState al;
    al.alarm_id = GID{"ALM-1"};
    al.kind = "TRACK_OCCUPIED_UNEXPECTEDLY";
    al.object_gid = GID{"OT-tor"};
    al.message = "Unexpected occupancy";
    apply(st, AlarmRaised{al});
    EXPECT_NE(st.find_alarm(GID{"ALM-1"}), nullptr);
}

TEST(StateApplier, AlarmCleared_removesAlarm)
{
    auto st = make_state();
    AlarmState al;
    al.alarm_id = GID{"ALM-1"};
    al.kind = "ENGINE_FAULT";
    al.message = "Fault";
    apply(st, AlarmRaised{al});
    apply(st, AlarmCleared{GID{"ALM-1"}});
    EXPECT_EQ(st.find_alarm(GID{"ALM-1"}), nullptr);
}

// ── apply_all ────────────────────────────────────────────────────────────────

TEST(StateApplier, ApplyAll_appliesChangesInOrder)
{
    auto st = make_state();
    GID route{"RT-X"};

    std::vector<DeviceStateChange> changes{
        SwitchPositionChange{GID{"ZWR-1"}, SwitchPosition::DIVERGENT, ChangeCause::AUTO},
        SwitchLocked{GID{"ZWR-1"}, route},
        SignalAspectChange{GID{"SEM-A"}, SignalAspect::S2_PROCEED, ChangeCause::AUTO, route},
    };
    apply_all(st, changes);

    EXPECT_EQ(st.find_switch(GID{"ZWR-1"})->position, SwitchPosition::DIVERGENT);
    EXPECT_TRUE(st.find_switch(GID{"ZWR-1"})->locked_by_route.has_value());
    EXPECT_EQ(st.find_signal(GID{"SEM-A"})->current_aspect, SignalAspect::S2_PROCEED);
    EXPECT_TRUE(st.find_signal(GID{"SEM-A"})->locked_by_route.has_value());
}

}  // namespace
