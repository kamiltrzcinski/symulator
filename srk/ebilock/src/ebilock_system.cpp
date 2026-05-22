#include <srk/common/device_rules.hpp>
#include <srk/ebilock/ebilock_system.hpp>

#include <engine/core/control_system_registry.hpp>

#include <stdexcept>

namespace srk::ebilock
{

using namespace engine::core;

static constexpr uint8_t NAK_NOT_FOUND = 0x01;
static constexpr uint8_t NAK_SAFETY_BLOCK = 0x02;
static constexpr uint8_t NAK_INVALID_STATE = 0x03;
static constexpr uint8_t NAK_UNSUPPORTED = 0x07;

static InterlockingViolation violation(uint8_t code, std::string text, const GID& gid = GID{})
{
    return InterlockingViolation{code, std::move(text), gid};
}

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
                return check_shl12(state, c);

            else if constexpr (std::is_same_v<T, InitAxleCounterResetCmd>)
                return check_sli(state, c);

            else if constexpr (std::is_same_v<T, ResetAxleCounterCmd>)
                return check_slk(state, c);

            else if constexpr (std::is_same_v<T, OperatorCommandCmd>)
                return srk::common::check_operator_command(state, c);

            else
                return InterlockingViolation{NAK_UNSUPPORTED, "Unrecognised command", GID{}};
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

            else if constexpr (std::is_same_v<T, SetBlockDirectionCmd>)
                return execute_shl12(state, c);

            else if constexpr (std::is_same_v<T, InitAxleCounterResetCmd>)
                return execute_sli(state, c);

            else if constexpr (std::is_same_v<T, ResetAxleCounterCmd>)
                return execute_slk(state, c);

            else if constexpr (std::is_same_v<T, OperatorCommandCmd>)
                return srk::common::execute_operator_command(state, c, eea4_throw_ticks_);

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

std::optional<InterlockingViolation> EbiLockSystem::check_shl12(
    const IStateView& state, const SetBlockDirectionCmd& cmd) const
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
            if (bs->axle_count != 0)
                return violation(NAK_SAFETY_BLOCK, "Cannot release direction: axle count != 0",
                                 cmd.block_section_gid);
            break;

        case Shl12Op::BLAI:
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

std::vector<DeviceStateChange> EbiLockSystem::execute_shl12(const IStateView& state,
                                                            const SetBlockDirectionCmd& cmd)
{
    const BlockSection* bs = state.find_block_section(cmd.block_section_gid);
    if (!bs)
        return {};

    std::vector<DeviceStateChange> changes;

    switch (cmd.operation)
    {
        case Shl12Op::BLW:
            changes.push_back(BlockDirectionChange{
                cmd.block_section_gid, BlockDirectionState::OUTBOUND_PENDING, true});
            break;

        case Shl12Op::BLP:
            if (bs->direction == BlockDirectionState::OUTBOUND_PENDING)
            {
                changes.push_back(BlockDirectionChange{cmd.block_section_gid,
                                                       BlockDirectionState::OUTBOUND, false});
                changes.push_back(
                    BlockSectionStateChange{cmd.block_section_gid, BlockSectionState::OPEN});
            }
            else
            {
                changes.push_back(BlockDirectionChange{cmd.block_section_gid,
                                                       BlockDirectionState::INBOUND, false});
                changes.push_back(
                    BlockSectionStateChange{cmd.block_section_gid, BlockSectionState::OPEN});
            }
            break;

        case Shl12Op::BLO:
            changes.push_back(
                BlockDirectionChange{cmd.block_section_gid, BlockDirectionState::NEUTRAL, false});
            break;

        case Shl12Op::BLZ:
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
            changes.push_back(
                BlockDirectionChange{cmd.block_section_gid, BlockDirectionState::NEUTRAL, false});
            changes.push_back(
                BlockSectionStateChange{cmd.block_section_gid, BlockSectionState::CLOSED});
            break;

        case Shl12Op::OPS:
            changes.push_back(
                BlockDirectionChange{cmd.block_section_gid, BlockDirectionState::NEUTRAL, false});
            changes.push_back(
                BlockSectionStateChange{cmd.block_section_gid, BlockSectionState::CLOSED});
            break;
    }

    return changes;
}

std::optional<InterlockingViolation> EbiLockSystem::check_sli(
    const IStateView& state, const InitAxleCounterResetCmd& cmd) const
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

std::vector<DeviceStateChange> EbiLockSystem::execute_sli(
    const IStateView& /*state*/, const InitAxleCounterResetCmd& cmd)
{
    return {BlockDirectionChange{cmd.block_section_gid, BlockDirectionState::RESET_PENDING, false}};
}

std::optional<InterlockingViolation> EbiLockSystem::check_slk(
    const IStateView& state, const ResetAxleCounterCmd& cmd) const
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

std::vector<DeviceStateChange> EbiLockSystem::execute_slk(
    const IStateView& /*state*/, const ResetAxleCounterCmd& cmd)
{
    return {
        BlockDirectionChange{cmd.block_section_gid, BlockDirectionState::NEUTRAL, false},
        BlockSectionStateChange{cmd.block_section_gid, BlockSectionState::CLOSED},
    };
}

}  // namespace srk::ebilock
