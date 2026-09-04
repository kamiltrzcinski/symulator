#include <srk/common/device_rules.hpp>
#include <srk/common/nak_codes.hpp>
#include <srk/ebilock/ebilock_system.hpp>

#include <engine/core/control_system_registry.hpp>

#include <stdexcept>

namespace srk::ebilock
{

using namespace engine::core;
using namespace srk::common;

// ── Static registration ───────────────────────────────────────────────────────

// Self-registers in the ControlSystemRegistry at static-init time.
// The engine simply calls ControlSystemRegistry::instance().create("ebilock_x4").
static const bool kRegistered = ControlSystemRegistry::register_static(
    "ebilock_x4", [] { return std::make_unique<EbiLockSystem>(); });

// ── Constructor ───────────────────────────────────────────────────────────────

EbiLockSystem::EbiLockSystem(int eea4_throw_ticks) : eea4_throw_ticks_{eea4_throw_ticks} {}

// ── system_id ─────────────────────────────────────────────────────────────────

std::string EbiLockSystem::system_id() const
{
    return "ebilock_x4";
}

// ── supported_command_types ───────────────────────────────────────────────────

std::vector<std::string> EbiLockSystem::supported_command_types() const
{
    return {
        "SetSwitchPositionCmd", "SetSignalAspectCmd",   "SetDerailerPositionCmd",
        "SetBlockSectionCmd",   "RequestRouteCmd",      "CancelRouteCmd",
        "AcknowledgeAlarmCmd",  "SetBlockDirectionCmd", "InitAxleCounterResetCmd",
        "ResetAxleCounterCmd",  "OperatorCommandCmd",
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

            else if constexpr (std::is_same_v<T, SetBlockDirectionCmd>)
                return srk::common::check_set_block_direction(state, c);

            else if constexpr (std::is_same_v<T, InitAxleCounterResetCmd>)
                return srk::common::check_init_axle_counter_reset(state, c);

            else if constexpr (std::is_same_v<T, ResetAxleCounterCmd>)
                return srk::common::check_reset_axle_counter(state, c);

            else if constexpr (std::is_same_v<T, OperatorCommandCmd>)
                return srk::common::check_operator_command(state, c);

            else
                return InterlockingViolation{NAK_UNSUPPORTED, "Unrecognised command", UID{}};
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
                    pending_targets_[c.uid] = c.position;
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

            else if constexpr (std::is_same_v<T, SetBlockDirectionCmd>)
                return srk::common::execute_set_block_direction(state, c);

            else if constexpr (std::is_same_v<T, InitAxleCounterResetCmd>)
                return srk::common::execute_init_axle_counter_reset(state, c);

            else if constexpr (std::is_same_v<T, ResetAxleCounterCmd>)
                return srk::common::execute_reset_axle_counter(state, c);

            else if constexpr (std::is_same_v<T, OperatorCommandCmd>)
                return srk::common::execute_operator_command(state, c, eea4_throw_ticks_);

            else
                return {};
        },
        cmd);
}

// ── on_tick ───────────────────────────────────────────────────────────────────

std::vector<DeviceStateChange> EbiLockSystem::on_tick(const IStateView& state, uint64_t tick_num)
{
    auto changes = srk::common::tick_switch_machines(state, pending_targets_);
    auto routes = srk::common::tick_route_auto_release(state, tick_num);
    changes.insert(changes.end(), std::make_move_iterator(routes.begin()),
                   std::make_move_iterator(routes.end()));
    return changes;
}

}  // namespace srk::ebilock
