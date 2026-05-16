#pragma once

#include <engine/core/control_system.hpp>

#include <memory>
#include <unordered_map>

// ── Ml8System — ESTW ML8 interlocking + SHL-12 block direction ───────────────
// Implements IControlSystem for the ESTW ML8 station interlocking system.
//
// ML8 handles the same station-device commands as EbiLock X4 (R1–R5, R7) and
// additionally manages the SHL-12 automatic block direction system for line
// block sections between neighbouring stations.
//
// SHL-12 operations (SetBlockDirectionCmd):
//   BLW   NEUTRAL → OUTBOUND_PENDING  (request permission for outbound direction)
//   BLP   OUTBOUND_PENDING → OUTBOUND (our direction confirmed by neighbour)
//         INBOUND_PENDING  → INBOUND  (we confirm neighbour's direction request)
//   BLO   OUTBOUND_PENDING → NEUTRAL  (cancel our own pending request)
//   BLZ   OUTBOUND/INBOUND → NEUTRAL  (release established direction)
//   BLAI  any → EMERGENCY             (initialise emergency procedure)
//   BLA   EMERGENCY → NEUTRAL         (execute emergency direction change)
//   OPS   cancel special procedure    (EMERGENCY → NEUTRAL)
//
// Axle-counter reset (InitAxleCounterResetCmd / ResetAxleCounterCmd):
//   SLI   direction=NEUTRAL, axle_count=0 required → RESET_PENDING
//   SLK   RESET_PENDING → NEUTRAL, opens block section
//
// See docs/14-interlocking-model.md §SHL-12 and the ML8 instruction manual.

namespace srk::ml8
{

class Ml8System final : public engine::core::IControlSystem
{
public:
    explicit Ml8System(int eea4_throw_ticks = 90);

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

    std::unordered_map<engine::core::GID, engine::core::SwitchPosition,
                       std::hash<engine::core::GID>>
        pending_targets_;

    // ── SHL-12 helpers ────────────────────────────────────────────────────────
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

}  // namespace srk::ml8
