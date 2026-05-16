#include <srk/common/device_rules.hpp>
#include <srk/ebilock/ebilock_system.hpp>

#include <engine/core/control_system_registry.hpp>

#include <stdexcept>

namespace srk::ebilock
{

using namespace engine::core;

// ── Static registration ───────────────────────────────────────────────────────

// Self-registers in the ControlSystemRegistry at static-init time.
// The engine simply calls ControlSystemRegistry::instance().create({"ebilock_x4"}).
static const bool kRegistered = ControlSystemRegistry::register_static(
    ControlSystemID{"ebilock_x4"}, [] { return std::make_unique<EbiLockSystem>(); });

// ── Constructor ───────────────────────────────────────────────────────────────

EbiLockSystem::EbiLockSystem(int eea4_throw_ticks) : eea4_throw_ticks_{eea4_throw_ticks} {}

// ── system_id ─────────────────────────────────────────────────────────────────

ControlSystemID EbiLockSystem::system_id() const
{
    return ControlSystemID{"ebilock_x4"};
}

// ── supported_command_types ───────────────────────────────────────────────────

std::vector<std::string> EbiLockSystem::supported_command_types() const
{
    return {
        "SetSwitchPositionCmd", "SetSignalAspectCmd", "SetDerailerPositionCmd",
        "SetBlockSectionCmd",   "RequestRouteCmd",    "CancelRouteCmd",
        "AcknowledgeAlarmCmd",
    };
}

// ── check_command ─────────────────────────────────────────────────────────────

std::optional<InterlockingViolation> EbiLockSystem::check_command(const IStateView& state,
                                                                  const Command& cmd) const
{
    return std::visit(
        [&](const auto& c) -> std::optional<InterlockingViolation>
        {
            using T = std::decay_t<decltype(c)>;

            if constexpr (std::is_same_v<T, SetSwitchPositionCmd>)
                return srk::common::check_set_switch_position(state, c);

            else if constexpr (std::is_same_v<T, SetSignalAspectCmd>)
                return srk::common::check_set_signal_aspect(state, c);

            else if constexpr (std::is_same_v<T, SetDerailerPositionCmd>)
                return srk::common::check_set_derailer_position(state, c);

            else if constexpr (std::is_same_v<T, SetBlockSectionCmd>)
                return srk::common::check_set_block_section(state, c);

            else if constexpr (std::is_same_v<T, RequestRouteCmd>)
                return srk::common::check_request_route(state, c);

            else if constexpr (std::is_same_v<T, CancelRouteCmd>)
                return srk::common::check_cancel_route(state, c);

            else if constexpr (std::is_same_v<T, AcknowledgeAlarmCmd>)
                return srk::common::check_acknowledge_alarm(state, c);

            else
                // SHL-12 commands are not handled by this system.
                return InterlockingViolation{0x07, "Command not supported by EbiLock X4", GID{}};
        },
        cmd);
}

// ── execute_command ───────────────────────────────────────────────────────────

std::vector<DeviceStateChange> EbiLockSystem::execute_command(const IStateView& state,
                                                              const Command& cmd)
{
    return std::visit(
        [&](const auto& c) -> std::vector<DeviceStateChange>
        {
            using T = std::decay_t<decltype(c)>;

            if constexpr (std::is_same_v<T, SetSwitchPositionCmd>)
            {
                auto changes =
                    srk::common::execute_set_switch_position(state, c, eea4_throw_ticks_);
                // Record the target so on_tick() knows where to land the switch.
                if (eea4_throw_ticks_ > 0)
                    pending_targets_[c.gid] = c.position;
                return changes;
            }
            else if constexpr (std::is_same_v<T, SetSignalAspectCmd>)
                return srk::common::execute_set_signal_aspect(state, c);

            else if constexpr (std::is_same_v<T, SetDerailerPositionCmd>)
                return srk::common::execute_set_derailer_position(state, c);

            else if constexpr (std::is_same_v<T, SetBlockSectionCmd>)
                return srk::common::execute_set_block_section(state, c);

            else if constexpr (std::is_same_v<T, RequestRouteCmd>)
                return srk::common::execute_request_route(state, c, state.current_tick());

            else if constexpr (std::is_same_v<T, CancelRouteCmd>)
                return srk::common::execute_cancel_route(state, c);

            else if constexpr (std::is_same_v<T, AcknowledgeAlarmCmd>)
                return srk::common::execute_acknowledge_alarm(state, c);

            else
                return {};
        },
        cmd);
}

// ── on_tick ───────────────────────────────────────────────────────────────────

std::vector<DeviceStateChange> EbiLockSystem::on_tick(const IStateView& state, uint64_t /*tick*/)
{
    std::vector<DeviceStateChange> changes;

    // Advance EEA-4 switch machine timers.
    state.for_each_switch(
        [&](const Switch& sw)
        {
            if (sw.position != SwitchPosition::MOVING)
            {
                pending_targets_.erase(sw.gid);
                return;
            }
            if (sw.moving_ticks_remaining <= 0)
            {
                // Should not happen; land the switch.
                auto it = pending_targets_.find(sw.gid);
                SwitchPosition target =
                    (it != pending_targets_.end()) ? it->second : SwitchPosition::STRAIGHT;
                pending_targets_.erase(sw.gid);
                changes.push_back(SwitchPositionChange{sw.gid, target, ChangeCause::AUTO, 0});
                return;
            }

            const int remaining = sw.moving_ticks_remaining - 1;
            if (remaining == 0)
            {
                auto it = pending_targets_.find(sw.gid);
                SwitchPosition target =
                    (it != pending_targets_.end()) ? it->second : SwitchPosition::STRAIGHT;
                pending_targets_.erase(sw.gid);
                changes.push_back(SwitchPositionChange{sw.gid, target, ChangeCause::AUTO, 0});
            }
            else
            {
                changes.push_back(SwitchPositionChange{sw.gid, SwitchPosition::MOVING,
                                                       ChangeCause::AUTO, remaining});
            }
        });

    // Auto-release routes whose trains have cleared.
    auto release_changes = srk::common::tick_route_auto_release(state);
    changes.insert(changes.end(), std::make_move_iterator(release_changes.begin()),
                   std::make_move_iterator(release_changes.end()));

    return changes;
}

}  // namespace srk::ebilock
