#pragma once

#include <engine/core/command.hpp>
#include <engine/core/control_system.hpp>
#include <engine/core/state_view.hpp>

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

// ── Shared interlocking rules (R1–R7) ────────────────────────────────────────
// Both EbiLock X4 and ESTW ML8 share the same physical device rules for
// station equipment (switches, signals, derailers, routes, alarms).
// This header exposes stateless helpers that implement each rule.
//
// Naming: check_* returns a violation on failure, nullopt on success.
//         execute_* returns the list of state changes to apply.
//
// All functions operate on the ENGINE thread and must not block.
// See docs/ARCHITECTURE.md for full rule tables.

namespace srk::common
{

using namespace engine::core;

// ── R1: SetSwitchPosition ────────────────────────────────────────────────────
// Rejects if: switch occupied | switch in MOVING | switch route-locked |
//             target position == current position.

std::optional<InterlockingViolation> check_set_switch_position(const IStateView& state,
                                                               const SetSwitchPositionCmd& cmd);

std::vector<DeviceStateChange> execute_set_switch_position(const IStateView& state,
                                                           const SetSwitchPositionCmd& cmd,
                                                           int throw_time_ticks);

// ── R2: SetSignalAspect ──────────────────────────────────────────────────────
// Rejects if: signal does not exist | aspect not in signal type's allowed set |
//             signal route-locked and new aspect is not STOP.

std::optional<InterlockingViolation> check_set_signal_aspect(const IStateView& state,
                                                             const SetSignalAspectCmd& cmd);

std::vector<DeviceStateChange> execute_set_signal_aspect(const IStateView& state,
                                                         const SetSignalAspectCmd& cmd);

// ── R3: SetDerailerPosition ──────────────────────────────────────────────────
// Rejects if: guarded section is OCCUPIED | derailer route-locked.

std::optional<InterlockingViolation> check_set_derailer_position(const IStateView& state,
                                                                 const SetDerailerPositionCmd& cmd);

std::vector<DeviceStateChange> execute_set_derailer_position(const IStateView& state,
                                                             const SetDerailerPositionCmd& cmd);

// ── R4: SetBlockSection (EbiLock OPEN/CLOSED) ────────────────────────────────
// Rejects if: section occupied and command is CLOSE | axle_count != 0 and CLOSE.

std::optional<InterlockingViolation> check_set_block_section(const IStateView& state,
                                                             const SetBlockSectionCmd& cmd);

std::vector<DeviceStateChange> execute_set_block_section(const IStateView& state,
                                                         const SetBlockSectionCmd& cmd);

// ── R5: RequestRoute / CancelRoute ───────────────────────────────────────────
// RequestRoute: BFS path from entry to exit signal; locks all switches,
//               derailers, and sections along the path; shows proceed aspect.
// CancelRoute: unlocks all devices; resets entry signal to STOP.

std::optional<InterlockingViolation> check_request_route(const IStateView& state,
                                                         const RequestRouteCmd& cmd);

std::vector<DeviceStateChange> execute_request_route(const IStateView& state,
                                                     const RequestRouteCmd& cmd, uint64_t tick);

std::optional<InterlockingViolation> check_cancel_route(const IStateView& state,
                                                        const CancelRouteCmd& cmd);

std::vector<DeviceStateChange> execute_cancel_route(const IStateView& state,
                                                    const CancelRouteCmd& cmd);

// ── R7: AcknowledgeAlarm ─────────────────────────────────────────────────────
// Rejects if: alarm_uid does not exist in active alarms.

std::optional<InterlockingViolation> check_acknowledge_alarm(const IStateView& state,
                                                             const AcknowledgeAlarmCmd& cmd);

std::vector<DeviceStateChange> execute_acknowledge_alarm(const IStateView& state,
                                                         const AcknowledgeAlarmCmd& cmd);

std::optional<InterlockingViolation> check_operator_command(const IStateView& state,
                                                            const OperatorCommandCmd& cmd);

std::vector<DeviceStateChange> execute_operator_command(const IStateView& state,
                                                        const OperatorCommandCmd& cmd,
                                                        int throw_time_ticks);

// ── R8: SetBlockDirection (SHL-12) ───────────────────────────────────────────
// Block-direction state machine shared by EbiLock X4 and ESTW ML8.
// BLW → OUTBOUND_PENDING; BLP confirms; BLO cancels; BLZ releases;
// BLAI initiates emergency; BLA/OPS execute emergency reset.

std::optional<InterlockingViolation> check_set_block_direction(const IStateView& state,
                                                               const SetBlockDirectionCmd& cmd);

std::vector<DeviceStateChange> execute_set_block_direction(const IStateView& state,
                                                           const SetBlockDirectionCmd& cmd);

// ── R9: InitAxleCounterReset (SLI) ───────────────────────────────────────────
// Requires block section in NEUTRAL state → transitions to RESET_PENDING.

std::optional<InterlockingViolation> check_init_axle_counter_reset(
    const IStateView& state, const InitAxleCounterResetCmd& cmd);

std::vector<DeviceStateChange> execute_init_axle_counter_reset(const IStateView& state,
                                                               const InitAxleCounterResetCmd& cmd);

// ── R10: ResetAxleCounter (SLK) ──────────────────────────────────────────────
// Requires RESET_PENDING → transitions to NEUTRAL and closes block section.

std::optional<InterlockingViolation> check_reset_axle_counter(const IStateView& state,
                                                              const ResetAxleCounterCmd& cmd);

std::vector<DeviceStateChange> execute_reset_axle_counter(const IStateView& state,
                                                          const ResetAxleCounterCmd& cmd);

// ── Tick helpers ──────────────────────────────────────────────────────────────
// Called from IControlSystem::on_tick.

// Advance EEA-4 switch machine timers; land switches when the throw-time
// expires.  pending_targets maps switch UID → intended final position and is
// maintained by the calling system (populated in execute_command, erased here
// when the switch lands).
std::vector<DeviceStateChange> tick_switch_machines(
    const IStateView& state,
    std::unordered_map<UID, SwitchPosition, std::hash<UID>>& pending_targets);

// Auto-release routes whose trains have fully cleared.
std::vector<DeviceStateChange> tick_route_auto_release(const IStateView& state, uint64_t current_tick);

}  // namespace srk::common
