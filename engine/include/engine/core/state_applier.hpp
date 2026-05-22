// engine/include/engine/core/state_applier.hpp
// Visitor that applies a DeviceStateChange to a mutable EngineState.
//
// Usage:
//   std::visit(engine::core::StateApplier{state}, change);
//   engine::core::apply(state, change);
//   engine::core::apply_all(state, changes);

#pragma once

#include "engine/core/control_system.hpp"
#include "engine/core/engine_state.hpp"
#include "engine/core/types.hpp"

namespace engine::core
{

struct StateApplier
{
    EngineState& state;

    void operator()(const SignalAspectChange& c)
    {
        state.apply_signal_aspect(c.gid, c.new_aspect);
        if (c.route_id.has_value())
        {
            // Signal is being locked by a route (proceed aspect set).
            state.apply_signal_lock(c.gid, c.route_id);
        }
        else if (c.new_aspect == SignalAspect::S1_STOP || c.new_aspect == SignalAspect::MS1_STOP)
        {
            // Signal returning to stop — release any existing lock.
            state.apply_signal_lock(c.gid, std::nullopt);
        }
    }

    void operator()(const SwitchPositionChange& c)
    {
        state.apply_switch_position(c.gid, c.new_position, c.moving_ticks_remaining);
    }

    void operator()(const SwitchLocked& c) { state.apply_switch_lock(c.switch_gid, c.route_id); }

    void operator()(const SwitchUnlocked& c)
    {
        state.apply_switch_lock(c.switch_gid, std::nullopt);
    }

    void operator()(const DerailerStateChange& c)
    {
        state.apply_derailer_state(c.gid, c.new_state);
        if (c.route_id.has_value())
        {
            // Derailer unlocked for a route — record ownership.
            state.apply_derailer_lock(c.gid, c.route_id);
        }
        else if (c.new_state == DerailerState::LOCKED)
        {
            // Derailer returning to safe position — release lock.
            state.apply_derailer_lock(c.gid, std::nullopt);
        }
    }

    void operator()(const BlockSectionStateChange& c)
    {
        state.apply_block_section_state(c.gid, c.new_state);
    }

    void operator()(const BlockDirectionChange& c)
    {
        state.apply_block_section_direction(c.gid, c.new_direction);
    }

    void operator()(const AxleCounterResetChange& c)
    {
        state.apply_axle_counter_reset(c.gid, c.target_kind);
    }

    void operator()(const OperatorCommandStateChange& c)
    {
        state.apply_operator_command_state(c.gid, c.target_kind, c.code, c.active);
    }

    void operator()(const RouteAdded& c) { state.add_route(c.route); }

    void operator()(const RouteRemoved& c) { state.remove_route(c.route_id); }

    void operator()(const AlarmRaised& c) { state.add_alarm(c.alarm); }

    void operator()(const AlarmCleared& c) { state.remove_alarm(c.alarm_id); }
};

/// Apply a single DeviceStateChange to the given EngineState.
inline void apply(EngineState& state, const DeviceStateChange& change)
{
    std::visit(StateApplier{state}, change);
}

/// Apply a sequence of DeviceStateChanges in order.
inline void apply_all(EngineState& state, const std::vector<DeviceStateChange>& changes)
{
    for (const auto& c : changes)
        std::visit(StateApplier{state}, c);
}

}  // namespace engine::core
