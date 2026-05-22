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
        "ResetAxleCounterCmd",  "OperatorCommandCmd",   "Ml8CommandCmd",
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
            else if constexpr (std::is_same_v<T, OperatorCommandCmd>)
                return srk::common::check_operator_command(state, c);
            else if constexpr (std::is_same_v<T, Ml8CommandCmd>)
                return check_ml8_command(state, c);
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
            else if constexpr (std::is_same_v<T, OperatorCommandCmd>)
                return srk::common::execute_operator_command(state, c, eea4_throw_ticks_);
            else if constexpr (std::is_same_v<T, Ml8CommandCmd>)
                return execute_ml8_command(state, c);
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

std::optional<InterlockingViolation> Ml8System::check_ml8_command(
    const IStateView& state, const Ml8CommandCmd& cmd) const
{
    const auto target_exists = [&]
    {
        switch (cmd.target_kind)
        {
            case OperatorTargetKind::SIGNAL:
                return state.find_signal(cmd.target_gid) != nullptr;
            case OperatorTargetKind::SWITCH:
                return state.find_switch(cmd.target_gid) != nullptr ||
                       state.find_derailer(cmd.target_gid) != nullptr;
            case OperatorTargetKind::DERAILER:
                return state.find_derailer(cmd.target_gid) != nullptr;
            case OperatorTargetKind::TRACK_SECTION:
                return state.find_track_section(cmd.target_gid) != nullptr;
            case OperatorTargetKind::BLOCK_SECTION:
                return state.find_block_section(cmd.target_gid) != nullptr;
            case OperatorTargetKind::AXLE_COUNTER_SYSTEM:
            case OperatorTargetKind::STATION:
            case OperatorTargetKind::ROUTE:
            case OperatorTargetKind::LEVEL_CROSSING:
            case OperatorTargetKind::INTERLOCKING_COMPUTER:
            case OperatorTargetKind::POWER_SUPPLY:
                return !cmd.target_gid.value.empty();
        }
        return false;
    };

    if (!target_exists())
        return violation(NAK_NOT_FOUND, "ML8 command target not found: " + cmd.target_gid.value,
                         cmd.target_gid);

    if (cmd.code == Ml8CommandCode::PZ || cmd.code == Ml8CommandCode::DPZ ||
        cmd.code == Ml8CommandCode::PPZ || cmd.code == Ml8CommandCode::WPZ)
    {
        if (const Switch* sw = state.find_switch(cmd.target_gid))
        {
            if (sw->occupancy == TrackOccupancy::OCCUPIED)
                return violation(NAK_SAFETY_BLOCK, "Switch is occupied: " + cmd.target_gid.value,
                                 cmd.target_gid);
            if (sw->position == SwitchPosition::MOVING)
                return violation(NAK_INVALID_STATE, "Switch is moving: " + cmd.target_gid.value,
                                 cmd.target_gid);
        }
    }

    if (cmd.code == Ml8CommandCode::BLZ || cmd.code == Ml8CommandCode::ZWBL)
    {
        if (const BlockSection* bs = state.find_block_section(cmd.target_gid);
            bs && bs->axle_count != 0)
            return violation(NAK_SAFETY_BLOCK, "Cannot restore block: axle count != 0",
                             cmd.target_gid);
    }

    return std::nullopt;
}

std::vector<DeviceStateChange> Ml8System::execute_ml8_command(const IStateView& state,
                                                              const Ml8CommandCmd& cmd)
{
    std::vector<DeviceStateChange> changes;
    changes.push_back(Ml8CommandStateChange{cmd.target_gid, cmd.target_kind, cmd.code, true});

    switch (cmd.code)
    {
        case Ml8CommandCode::STOJ:
        case Ml8CommandCode::STJ:
        case Ml8CommandCode::STOP:
            if (const Signal* sig = state.find_signal(cmd.target_gid))
            {
                const auto aspect = sig->type == Signal::Type::SHUNTING ? SignalAspect::MS1_STOP
                                                                         : SignalAspect::S1_STOP;
                changes.push_back(
                    SignalAspectChange{cmd.target_gid, aspect, ChangeCause::COMMAND, std::nullopt});
            }
            break;

        case Ml8CommandCode::SZ:
        case Ml8CommandCode::NSZ:
            if (state.find_signal(cmd.target_gid))
                changes.push_back(SignalAspectChange{cmd.target_gid, SignalAspect::S2_PROCEED,
                                                     ChangeCause::COMMAND, std::nullopt});
            break;

        case Ml8CommandCode::PZ:
        case Ml8CommandCode::DPZ:
            if (state.find_switch(cmd.target_gid))
            {
                auto sub = srk::common::execute_set_switch_position(
                    state, SetSwitchPositionCmd{cmd.target_gid, SwitchPosition::DIVERGENT},
                    eea4_throw_ticks_);
                changes.insert(changes.end(), sub.begin(), sub.end());
                if (eea4_throw_ticks_ > 0)
                    pending_targets_[cmd.target_gid] = SwitchPosition::DIVERGENT;
            }
            else if (state.find_derailer(cmd.target_gid))
            {
                auto sub = srk::common::execute_set_derailer_position(
                    state, SetDerailerPositionCmd{cmd.target_gid, DerailerState::UNLOCKED});
                changes.insert(changes.end(), sub.begin(), sub.end());
            }
            break;

        case Ml8CommandCode::PPZ:
            changes.push_back(Ml8CommandStateChange{cmd.target_gid, cmd.target_kind, cmd.code,
                                                    false});
            break;

        case Ml8CommandCode::ZEROLO:
            changes.push_back(AxleCounterResetChange{cmd.target_gid, cmd.target_kind});
            break;

        case Ml8CommandCode::WBL:
            changes.push_back(BlockDirectionChange{cmd.target_gid,
                                                   BlockDirectionState::OUTBOUND_PENDING, true});
            break;

        case Ml8CommandCode::PZK:
            if (const BlockSection* bs = state.find_block_section(cmd.target_gid))
            {
                const auto direction = bs->direction == BlockDirectionState::INBOUND_PENDING
                                           ? BlockDirectionState::INBOUND
                                           : BlockDirectionState::OUTBOUND;
                changes.push_back(BlockDirectionChange{cmd.target_gid, direction, false});
                changes.push_back(BlockSectionStateChange{cmd.target_gid, BlockSectionState::OPEN});
            }
            break;

        case Ml8CommandCode::AZK:
            changes.push_back(
                BlockDirectionChange{cmd.target_gid, BlockDirectionState::EMERGENCY, false});
            break;

        case Ml8CommandCode::BLZ:
        case Ml8CommandCode::ZWBL:
        case Ml8CommandCode::OWBL:
            changes.push_back(
                BlockDirectionChange{cmd.target_gid, BlockDirectionState::NEUTRAL, false});
            changes.push_back(BlockSectionStateChange{cmd.target_gid, BlockSectionState::CLOSED});
            break;

        case Ml8CommandCode::OSTOP:
        case Ml8CommandCode::OUZ:
        case Ml8CommandCode::OUZ_DR:
        case Ml8CommandCode::OUZ_DZ:
        case Ml8CommandCode::OUZ_JN:
        case Ml8CommandCode::OUZ_PJ:
        case Ml8CommandCode::OUZ_X:
        case Ml8CommandCode::OUZ_ZN:
        case Ml8CommandCode::OPO:
        case Ml8CommandCode::WPN:
        case Ml8CommandCode::WPZ:
            changes.push_back(Ml8CommandStateChange{cmd.target_gid, cmd.target_kind, cmd.code,
                                                    false});
            break;

        default:
            break;
    }

    return changes;
}

}  // namespace srk::ml8
