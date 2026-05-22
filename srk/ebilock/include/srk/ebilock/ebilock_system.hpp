#pragma once

#include <engine/core/control_system.hpp>

#include <memory>
#include <unordered_map>

// ── EbiLockSystem — EbiLock X4 interlocking ──────────────────────────────────
// Implements IControlSystem for the EbiLock X4 / EbiScreen X4 station
// interlocking system as deployed by PKP PLK.
//
// Supported commands (docs/14-interlocking-model.md):
//   R1  SetSwitchPositionCmd
//   R2  SetSignalAspectCmd
//   R3  SetDerailerPositionCmd
//   R4  SetBlockSectionCmd     (OPEN / CLOSED)
//   R5  RequestRouteCmd / CancelRouteCmd
//   R7  AcknowledgeAlarmCmd
//   SHL-12 block-direction and axle-counter reset commands
//
// Per-switch target tracking: when an EEA-4 switch machine transitions through
// the MOVING state, EbiLockSystem stores the intended target position
// (STRAIGHT/DIVERGENT) in pending_targets_ so that on_tick() can land the
// switch in the correct position after the throw-time has elapsed.

namespace srk::ebilock
{

class EbiLockSystem final : public engine::core::IControlSystem
{
public:
    // EEA-4 throw time in engine ticks (1 tick = 50 ms at 20 Hz).
    // Default: 4.5 s → 90 ticks.
    explicit EbiLockSystem(int eea4_throw_ticks = 90);

    engine::core::ControlSystemID system_id() const override;

    std::optional<engine::core::InterlockingViolation> check_command(
        const engine::core::IStateView& state, const engine::core::Command& cmd) const override;

    std::vector<engine::core::DeviceStateChange> execute_command(
        const engine::core::IStateView& state, const engine::core::Command& cmd) override;

    std::vector<engine::core::DeviceStateChange> on_tick(const engine::core::IStateView& state,
                                                         uint64_t tick) override;

    std::vector<std::string> supported_command_types() const override;

private:
    int eea4_throw_ticks_;

    // Maps switch GID → intended target position when a switch is MOVING.
    std::unordered_map<engine::core::GID, engine::core::SwitchPosition,
                       std::hash<engine::core::GID>>
        pending_targets_;

    std::optional<engine::core::InterlockingViolation> check_shl12(
        const engine::core::IStateView& state, const engine::core::SetBlockDirectionCmd& cmd) const;

    std::vector<engine::core::DeviceStateChange> execute_shl12(
        const engine::core::IStateView& state, const engine::core::SetBlockDirectionCmd& cmd);

    std::optional<engine::core::InterlockingViolation> check_sli(
        const engine::core::IStateView& state,
        const engine::core::InitAxleCounterResetCmd& cmd) const;

    std::vector<engine::core::DeviceStateChange> execute_sli(
        const engine::core::IStateView& state, const engine::core::InitAxleCounterResetCmd& cmd);

    std::optional<engine::core::InterlockingViolation> check_slk(
        const engine::core::IStateView& state, const engine::core::ResetAxleCounterCmd& cmd) const;

    std::vector<engine::core::DeviceStateChange> execute_slk(
        const engine::core::IStateView& state, const engine::core::ResetAxleCounterCmd& cmd);
};

}  // namespace srk::ebilock
