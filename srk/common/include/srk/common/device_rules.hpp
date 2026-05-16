#pragma once

#include <engine/core/command.hpp>
#include <engine/core/control_system.hpp>
#include <engine/core/state_view.hpp>

#include <optional>
#include <string>
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
// See docs/14-interlocking-model.md for full rule tables.

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
// Rejects if: alarm_id does not exist in active alarms.

std::optional<InterlockingViolation> check_acknowledge_alarm(const IStateView& state,
                                                             const AcknowledgeAlarmCmd& cmd);

std::vector<DeviceStateChange> execute_acknowledge_alarm(const IStateView& state,
                                                         const AcknowledgeAlarmCmd& cmd);

// ── Tick helpers ──────────────────────────────────────────────────────────────
// Called from IControlSystem::on_tick.

// Advance EEA-4 switch machine timers; emit SwitchPositionChange when done.
std::vector<DeviceStateChange> tick_switch_machines(const IStateView& state);

// Auto-release routes whose trains have fully cleared.
std::vector<DeviceStateChange> tick_route_auto_release(const IStateView& state);

}  // namespace srk::common
