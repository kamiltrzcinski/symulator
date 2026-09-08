#include <srk/common/device_rules.hpp>
#include <srk/common/nak_codes.hpp>
#include <srk/ml8/ml8_system.hpp>

#include <engine/core/control_system_registry.hpp>

namespace srk::ml8
{

using namespace engine::core;
using namespace srk::common;

// ── Static registration ───────────────────────────────────────────────────────

static const bool kRegistered = ControlSystemRegistry::register_static(
    "estw_ml8", [] { return std::make_unique<Ml8System>(); });

static InterlockingViolation violation(uint8_t code, std::string text, UID uid = UID{})
{
    return InterlockingViolation{code, std::move(text), uid};
}

// ── Constructor ───────────────────────────────────────────────────────────────

Ml8System::Ml8System(int eea4_throw_ticks) : eea4_throw_ticks_{eea4_throw_ticks} {}

// ── system_id ─────────────────────────────────────────────────────────────────

std::string Ml8System::system_id() const
{
    return "estw_ml8";
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

static const srk::common::FlankProtectionPolicy kFlankPolicy{};
static const std::vector<const srk::common::IRoutePathPolicy*> kPolicies = {&kFlankPolicy};

std::optional<InterlockingViolation> Ml8System::check_command(const IStateView& state,
                                                              const Command& cmd) const
{
    return std::visit(
        [&](auto&& c) -> std::optional<InterlockingViolation> {
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
                return srk::common::check_request_route(state, c, kPolicies);
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
            else if constexpr (std::is_same_v<T, Ml8CommandCmd>)
                return check_ml8_command(state, c);
            else
                return InterlockingViolation{NAK_UNSUPPORTED, "Unrecognised command", UID{}};
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
                return srk::common::execute_request_route(state, c, state.current_tick(), kPolicies);
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
            else if constexpr (std::is_same_v<T, Ml8CommandCmd>)
                return execute_ml8_command(state, c);
            else
                return {};
        },
        cmd);
}

// ── on_tick ───────────────────────────────────────────────────────────────────

std::vector<DeviceStateChange> Ml8System::on_tick(const IStateView& state, uint64_t tick_num)
{
    auto changes = srk::common::tick_switch_machines(state, pending_targets_);
    auto routes = srk::common::tick_route_auto_release(state, tick_num);
    changes.insert(changes.end(), std::make_move_iterator(routes.begin()),
                   std::make_move_iterator(routes.end()));
    return changes;
}

// ── ML8-specific commands ─────────────────────────────────────────────────────

std::optional<InterlockingViolation> Ml8System::check_ml8_command(
    const IStateView& state, const Ml8CommandCmd& cmd) const
{
    const auto target_exists = [&]
    {
        switch (cmd.target_kind)
        {
            case OperatorTargetKind::SIGNAL:
                return state.find_signal(cmd.target_uid) != nullptr;
            case OperatorTargetKind::SWITCH:
                return state.find_switch(cmd.target_uid) != nullptr ||
                       state.find_derailer(cmd.target_uid) != nullptr;
            case OperatorTargetKind::DERAILER:
                return state.find_derailer(cmd.target_uid) != nullptr;
            case OperatorTargetKind::TRACK_SECTION:
                return state.find_track_section(cmd.target_uid) != nullptr;
            case OperatorTargetKind::BLOCK_SECTION:
                return state.find_block_section(cmd.target_uid) != nullptr;
            case OperatorTargetKind::AXLE_COUNTER_SYSTEM:
            case OperatorTargetKind::STATION:
            case OperatorTargetKind::ROUTE:
            case OperatorTargetKind::LEVEL_CROSSING:
            case OperatorTargetKind::INTERLOCKING_COMPUTER:
            case OperatorTargetKind::POWER_SUPPLY:
                return cmd.target_uid.value != 0;
        }
        return false;
    };

    if (!target_exists())
        return violation(NAK_NOT_FOUND,
                         "ML8 command target not found: " + std::to_string(cmd.target_uid.value),
                         cmd.target_uid);

    if (cmd.code == Ml8CommandCode::PZ || cmd.code == Ml8CommandCode::DPZ ||
        cmd.code == Ml8CommandCode::PPZ || cmd.code == Ml8CommandCode::WPZ)
    {
        if (const Switch* sw = state.find_switch(cmd.target_uid))
        {
            if (sw->occupancy == TrackOccupancy::OCCUPIED)
                return violation(NAK_SAFETY_BLOCK,
                                 "Switch is occupied: " + std::to_string(cmd.target_uid.value),
                                 cmd.target_uid);
            if (sw->position == SwitchPosition::MOVING)
                return violation(NAK_INVALID_STATE,
                                 "Switch is moving: " + std::to_string(cmd.target_uid.value),
                                 cmd.target_uid);
        }
    }

    if (cmd.code == Ml8CommandCode::BLZ || cmd.code == Ml8CommandCode::ZWBL)
    {
        if (const BlockSection* bs = state.find_block_section(cmd.target_uid);
            bs && bs->axle_count != 0)
            return violation(NAK_SAFETY_BLOCK, "Cannot restore block: axle count != 0",
                             cmd.target_uid);
    }

    return std::nullopt;
}

std::vector<DeviceStateChange> Ml8System::execute_ml8_command(const IStateView& state,
                                                              const Ml8CommandCmd& cmd)
{
    std::vector<DeviceStateChange> changes;
    changes.push_back(Ml8CommandStateChange{cmd.target_uid, cmd.target_kind, cmd.code, true});

    switch (cmd.code)
    {
        case Ml8CommandCode::STOJ:
        case Ml8CommandCode::STJ:
        case Ml8CommandCode::STOP:
            if (const Signal* sig = state.find_signal(cmd.target_uid))
            {
                const auto aspect = sig->type == Signal::Type::SHUNTING ? SignalAspect::MS1_STOP
                                                                         : SignalAspect::S1_STOP;
                changes.push_back(
                    SignalAspectChange{cmd.target_uid, aspect, ChangeCause::COMMAND, std::nullopt});
            }
            break;

        case Ml8CommandCode::SZ:
        case Ml8CommandCode::NSZ:
            if (state.find_signal(cmd.target_uid))
                changes.push_back(SignalAspectChange{cmd.target_uid, SignalAspect::S2_PROCEED,
                                                     ChangeCause::COMMAND, std::nullopt});
            break;

        case Ml8CommandCode::PZ:
        case Ml8CommandCode::DPZ:
            if (state.find_switch(cmd.target_uid))
            {
                auto sub = srk::common::execute_set_switch_position(
                    state, SetSwitchPositionCmd{cmd.target_uid, SwitchPosition::DIVERGENT},
                    eea4_throw_ticks_);
                changes.insert(changes.end(), sub.begin(), sub.end());
                if (eea4_throw_ticks_ > 0)
                    pending_targets_[cmd.target_uid] = SwitchPosition::DIVERGENT;
            }
            else if (state.find_derailer(cmd.target_uid))
            {
                auto sub = srk::common::execute_set_derailer_position(
                    state, SetDerailerPositionCmd{cmd.target_uid, DerailerState::UNLOCKED});
                changes.insert(changes.end(), sub.begin(), sub.end());
            }
            break;

        case Ml8CommandCode::PPZ:
            changes.push_back(
                Ml8CommandStateChange{cmd.target_uid, cmd.target_kind, cmd.code, false});
            break;

        case Ml8CommandCode::ZEROLO:
            changes.push_back(AxleCounterResetChange{cmd.target_uid, cmd.target_kind});
            break;

        case Ml8CommandCode::WBL:
            changes.push_back(
                BlockDirectionChange{cmd.target_uid, BlockDirectionState::OUTBOUND_PENDING, true});
            break;

        case Ml8CommandCode::PZK:
            if (const BlockSection* bs = state.find_block_section(cmd.target_uid))
            {
                const auto direction = bs->direction == BlockDirectionState::INBOUND_PENDING
                                           ? BlockDirectionState::INBOUND
                                           : BlockDirectionState::OUTBOUND;
                changes.push_back(BlockDirectionChange{cmd.target_uid, direction, false});
                changes.push_back(BlockSectionStateChange{cmd.target_uid, BlockSectionState::OPEN});
            }
            break;

        case Ml8CommandCode::AZK:
            changes.push_back(
                BlockDirectionChange{cmd.target_uid, BlockDirectionState::EMERGENCY, false});
            break;

        case Ml8CommandCode::BLZ:
        case Ml8CommandCode::ZWBL:
        case Ml8CommandCode::OWBL:
            changes.push_back(
                BlockDirectionChange{cmd.target_uid, BlockDirectionState::NEUTRAL, false});
            changes.push_back(BlockSectionStateChange{cmd.target_uid, BlockSectionState::CLOSED});
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
            changes.push_back(
                Ml8CommandStateChange{cmd.target_uid, cmd.target_kind, cmd.code, false});
            break;

        default:
            break;
    }

    return changes;
}

}  // namespace srk::ml8
