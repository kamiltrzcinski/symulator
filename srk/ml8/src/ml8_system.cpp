#include <srk/common/device_rules.hpp>
#include <srk/ml8/ml8_system.hpp>

#include <engine/core/control_system_registry.hpp>

namespace srk::ml8
{

using namespace engine::core;

// ── Static registration ───────────────────────────────────────────────────────

static const bool kRegistered = ControlSystemRegistry::register_static(
    ControlSystemID{"estw_ml8"}, [] { return std::make_unique<Ml8System>(); });

// ── NAK codes (shared with device_rules.cpp) ─────────────────────────────────

static constexpr uint8_t NAK_NOT_FOUND = 0x01;
static constexpr uint8_t NAK_SAFETY_BLOCK = 0x02;
static constexpr uint8_t NAK_INVALID_STATE = 0x03;
static constexpr uint8_t NAK_UNSUPPORTED = 0x07;

static InterlockingViolation violation(uint8_t code, std::string text, const GID& gid = GID{})
{
    return InterlockingViolation{code, std::move(text), gid};
}

// ── Constructor ───────────────────────────────────────────────────────────────

Ml8System::Ml8System(int eea4_throw_ticks) : eea4_throw_ticks_{eea4_throw_ticks} {}

// ── system_id ─────────────────────────────────────────────────────────────────

ControlSystemID Ml8System::system_id() const
{
    return ControlSystemID{"estw_ml8"};
}

// ── supported_command_types ───────────────────────────────────────────────────

std::vector<std::string> Ml8System::supported_command_types() const
{
    return {
        "SetSwitchPositionCmd", "SetSignalAspectCmd",   "SetDerailerPositionCmd",
        "SetBlockSectionCmd",   "RequestRouteCmd",      "CancelRouteCmd",
        "AcknowledgeAlarmCmd",  "SetBlockDirectionCmd", "InitAxleCounterResetCmd",
        "ResetAxleCounterCmd",
    };
}

// ── check_command ─────────────────────────────────────────────────────────────

std::optional<InterlockingViolation> Ml8System::check_command(const IStateView& state,
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
                return check_shl12(state, c);
            else if constexpr (std::is_same_v<T, InitAxleCounterResetCmd>)
                return check_sli(state, c);
            else if constexpr (std::is_same_v<T, ResetAxleCounterCmd>)
                return check_slk(state, c);
            else
                return InterlockingViolation{NAK_UNSUPPORTED, "Unrecognised command", GID{}};
        },
        cmd);
}

// ── execute_command ───────────────────────────────────────────────────────────

std::vector<DeviceStateChange> Ml8System::execute_command(const IStateView& state,
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
            else if constexpr (std::is_same_v<T, SetBlockDirectionCmd>)
                return execute_shl12(state, c);
            else if constexpr (std::is_same_v<T, InitAxleCounterResetCmd>)
                return execute_sli(state, c);
            else if constexpr (std::is_same_v<T, ResetAxleCounterCmd>)
                return execute_slk(state, c);
            else
                return {};
        },
        cmd);
}

// ── on_tick ───────────────────────────────────────────────────────────────────

std::vector<DeviceStateChange> Ml8System::on_tick(const IStateView& state, uint64_t /*tick*/)
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

    auto release = srk::common::tick_route_auto_release(state);
    changes.insert(changes.end(), std::make_move_iterator(release.begin()),
                   std::make_move_iterator(release.end()));

    return changes;
}

// ── SHL-12: check_shl12 ───────────────────────────────────────────────────────

std::optional<InterlockingViolation> Ml8System::check_shl12(const IStateView& state,
                                                            const SetBlockDirectionCmd& cmd) const
{
    const BlockSection* bs = state.find_block_section(cmd.block_section_gid);
    if (!bs)
        return violation(NAK_NOT_FOUND, "Block section not found: " + cmd.block_section_gid.value,
                         cmd.block_section_gid);

    const BlockDirectionState dir = bs->direction;

    switch (cmd.operation)
    {
        case Shl12Op::BLW:
            if (dir != BlockDirectionState::NEUTRAL)
                return violation(NAK_INVALID_STATE,
                                 "BLW requires NEUTRAL direction, current: " +
                                     std::to_string(static_cast<int>(dir)),
                                 cmd.block_section_gid);
            break;

        case Shl12Op::BLP:
            if (dir != BlockDirectionState::OUTBOUND_PENDING &&
                dir != BlockDirectionState::INBOUND_PENDING)
                return violation(NAK_INVALID_STATE,
                                 "BLP requires OUTBOUND_PENDING or INBOUND_PENDING",
                                 cmd.block_section_gid);
            break;

        case Shl12Op::BLO:
            if (dir != BlockDirectionState::OUTBOUND_PENDING)
                return violation(NAK_INVALID_STATE, "BLO requires OUTBOUND_PENDING",
                                 cmd.block_section_gid);
            break;

        case Shl12Op::BLZ:
            if (dir != BlockDirectionState::OUTBOUND && dir != BlockDirectionState::INBOUND)
                return violation(NAK_INVALID_STATE, "BLZ requires OUTBOUND or INBOUND",
                                 cmd.block_section_gid);
            // Cannot release direction while block is occupied.
            if (bs->axle_count != 0)
                return violation(NAK_SAFETY_BLOCK, "Cannot release direction: axle count != 0",
                                 cmd.block_section_gid);
            break;

        case Shl12Op::BLAI:
            // BLAI can be issued in most states except RESET_PENDING.
            if (dir == BlockDirectionState::RESET_PENDING)
                return violation(NAK_INVALID_STATE, "BLAI not allowed in RESET_PENDING state",
                                 cmd.block_section_gid);
            break;

        case Shl12Op::BLA:
            if (dir != BlockDirectionState::EMERGENCY)
                return violation(NAK_INVALID_STATE, "BLA requires EMERGENCY state",
                                 cmd.block_section_gid);
            break;

        case Shl12Op::OPS:
            if (dir != BlockDirectionState::EMERGENCY && dir != BlockDirectionState::RESET_PENDING)
                return violation(NAK_INVALID_STATE, "OPS requires EMERGENCY or RESET_PENDING state",
                                 cmd.block_section_gid);
            break;
    }

    return std::nullopt;
}

// ── SHL-12: execute_shl12 ────────────────────────────────────────────────────

std::vector<DeviceStateChange> Ml8System::execute_shl12(const IStateView& state,
                                                        const SetBlockDirectionCmd& cmd)
{
    const BlockSection* bs = state.find_block_section(cmd.block_section_gid);
    if (!bs)
        return {};

    std::vector<DeviceStateChange> changes;

    switch (cmd.operation)
    {
        case Shl12Op::BLW:
            // Request outbound direction — transition to OUTBOUND_PENDING.
            // The neighbour must respond with BLP to confirm.
            changes.push_back(BlockDirectionChange{
                cmd.block_section_gid, BlockDirectionState::OUTBOUND_PENDING,
                true  // requires_neighbor_confirmation
            });
            break;

        case Shl12Op::BLP:
            // Confirm a pending direction.
            if (bs->direction == BlockDirectionState::OUTBOUND_PENDING)
            {
                // Our own BLW was acknowledged by the neighbour.
                changes.push_back(BlockDirectionChange{cmd.block_section_gid,
                                                       BlockDirectionState::OUTBOUND, false});
                // Open the block section for departure.
                changes.push_back(
                    BlockSectionStateChange{cmd.block_section_gid, BlockSectionState::OPEN});
            }
            else
            {
                // Neighbour's BLW — we confirm INBOUND.
                changes.push_back(BlockDirectionChange{cmd.block_section_gid,
                                                       BlockDirectionState::INBOUND, false});
                changes.push_back(
                    BlockSectionStateChange{cmd.block_section_gid, BlockSectionState::OPEN});
            }
            break;

        case Shl12Op::BLO:
            // Cancel pending outbound request.
            changes.push_back(
                BlockDirectionChange{cmd.block_section_gid, BlockDirectionState::NEUTRAL, false});
            break;

        case Shl12Op::BLZ:
            // Release established direction.
            changes.push_back(
                BlockDirectionChange{cmd.block_section_gid, BlockDirectionState::NEUTRAL, false});
            changes.push_back(
                BlockSectionStateChange{cmd.block_section_gid, BlockSectionState::CLOSED});
            break;

        case Shl12Op::BLAI:
            changes.push_back(
                BlockDirectionChange{cmd.block_section_gid, BlockDirectionState::EMERGENCY, false});
            break;

        case Shl12Op::BLA:
            // Execute emergency direction change — resets to NEUTRAL.
            changes.push_back(
                BlockDirectionChange{cmd.block_section_gid, BlockDirectionState::NEUTRAL, false});
            changes.push_back(
                BlockSectionStateChange{cmd.block_section_gid, BlockSectionState::CLOSED});
            break;

        case Shl12Op::OPS:
            // Cancel special procedure — reset to NEUTRAL.
            changes.push_back(
                BlockDirectionChange{cmd.block_section_gid, BlockDirectionState::NEUTRAL, false});
            changes.push_back(
                BlockSectionStateChange{cmd.block_section_gid, BlockSectionState::CLOSED});
            break;
    }

    return changes;
}

// ── SLI / SLK ────────────────────────────────────────────────────────────────

std::optional<InterlockingViolation> Ml8System::check_sli(const IStateView& state,
                                                          const InitAxleCounterResetCmd& cmd) const
{
    const BlockSection* bs = state.find_block_section(cmd.block_section_gid);
    if (!bs)
        return violation(NAK_NOT_FOUND, "Block section not found: " + cmd.block_section_gid.value,
                         cmd.block_section_gid);

    if (bs->direction != BlockDirectionState::NEUTRAL)
        return violation(NAK_INVALID_STATE, "SLI requires NEUTRAL direction",
                         cmd.block_section_gid);
    return std::nullopt;
}

std::vector<DeviceStateChange> Ml8System::execute_sli(const IStateView& /*state*/,
                                                      const InitAxleCounterResetCmd& cmd)
{
    return {BlockDirectionChange{cmd.block_section_gid, BlockDirectionState::RESET_PENDING, false}};
}

std::optional<InterlockingViolation> Ml8System::check_slk(const IStateView& state,
                                                          const ResetAxleCounterCmd& cmd) const
{
    const BlockSection* bs = state.find_block_section(cmd.block_section_gid);
    if (!bs)
        return violation(NAK_NOT_FOUND, "Block section not found: " + cmd.block_section_gid.value,
                         cmd.block_section_gid);

    if (bs->direction != BlockDirectionState::RESET_PENDING)
        return violation(NAK_INVALID_STATE, "SLK requires RESET_PENDING state",
                         cmd.block_section_gid);
    return std::nullopt;
}

std::vector<DeviceStateChange> Ml8System::execute_slk(const IStateView& /*state*/,
                                                      const ResetAxleCounterCmd& cmd)
{
    return {
        BlockDirectionChange{cmd.block_section_gid, BlockDirectionState::NEUTRAL, false},
        BlockSectionStateChange{cmd.block_section_gid, BlockSectionState::CLOSED},
    };
}

}  // namespace srk::ml8
