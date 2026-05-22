#include <srk/common/device_rules.hpp>
#include <srk/common/route_graph.hpp>

#include <algorithm>
#include <format>

namespace srk::common
{

// ── Violation helpers ─────────────────────────────────────────────────────────

static InterlockingViolation violation(uint8_t code, std::string text, const GID& gid = GID{})
{
    return InterlockingViolation{code, std::move(text), gid};
}

// COMMAND_NAK reason codes (from docs/09-communication-contract.md §COMMAND_NAK)
static constexpr uint8_t NAK_NOT_FOUND = 0x01;
static constexpr uint8_t NAK_SAFETY_BLOCK = 0x02;
static constexpr uint8_t NAK_INVALID_STATE = 0x03;
static constexpr uint8_t NAK_ROUTE_LOCKED = 0x04;
static constexpr uint8_t NAK_NO_PATH = 0x05;
static constexpr uint8_t NAK_SWITCH_MOVING = 0x06;

// ── R1: SetSwitchPosition ─────────────────────────────────────────────────────

std::optional<InterlockingViolation> check_set_switch_position(const IStateView& state,
                                                               const SetSwitchPositionCmd& cmd)
{
    const Switch* sw = state.find_switch(cmd.gid);
    if (!sw)
        return violation(NAK_NOT_FOUND, "Switch not found: " + cmd.gid.value, cmd.gid);

    if (sw->position == SwitchPosition::MOVING)
        return violation(NAK_SWITCH_MOVING, "Switch is already moving: " + cmd.gid.value, cmd.gid);

    if (sw->occupancy == TrackOccupancy::OCCUPIED)
        return violation(NAK_SAFETY_BLOCK, "Switch is occupied: " + cmd.gid.value, cmd.gid);

    if (sw->locked_by_route.has_value())
        return violation(
            NAK_ROUTE_LOCKED,
            "Switch locked by route " + sw->locked_by_route->value + ": " + cmd.gid.value, cmd.gid);

    if (sw->position == cmd.position)
        return violation(NAK_INVALID_STATE,
                         "Switch already in requested position: " + cmd.gid.value, cmd.gid);

    return std::nullopt;
}

std::vector<DeviceStateChange> execute_set_switch_position(const IStateView& state,
                                                           const SetSwitchPositionCmd& cmd,
                                                           int throw_time_ticks)
{
    std::vector<DeviceStateChange> changes;

    if (throw_time_ticks > 0)
    {
        // Transition through MOVING state; on_tick decrements the counter.
        changes.push_back(SwitchPositionChange{cmd.gid, SwitchPosition::MOVING,
                                               ChangeCause::COMMAND, throw_time_ticks});
    }
    else
    {
        // Instantaneous switch (for testing / simplified devices).
        changes.push_back(SwitchPositionChange{cmd.gid, cmd.position, ChangeCause::COMMAND, 0});
    }
    // Store the target so on_tick knows where to land.
    // We encode the target in the sign: positive ticks → STRAIGHT, negative → DIVERGENT.
    // Actually, we handle this in on_tick by storing target alongside MOVING state.
    // For now the callers (EbiLockSystem / Ml8System) manage per-switch target tracking.
    return changes;
}

// ── R2: SetSignalAspect ───────────────────────────────────────────────────────

std::optional<InterlockingViolation> check_set_signal_aspect(const IStateView& state,
                                                             const SetSignalAspectCmd& cmd)
{
    const Signal* sig = state.find_signal(cmd.gid);
    if (!sig)
        return violation(NAK_NOT_FOUND, "Signal not found: " + cmd.gid.value, cmd.gid);

    // Refuse to show a proceed aspect on a route-locked signal via direct command;
    // proceed aspects are set through RequestRoute.  Direct commands may only set STOP
    // or shunting aspects on locked signals.
    if (sig->locked_by_route.has_value())
    {
        const bool is_stop =
            (cmd.aspect == SignalAspect::S1_STOP || cmd.aspect == SignalAspect::MS1_STOP);
        if (!is_stop)
            return violation(NAK_ROUTE_LOCKED,
                             "Cannot change locked signal to proceed aspect: " + cmd.gid.value,
                             cmd.gid);
    }

    return std::nullopt;
}

std::vector<DeviceStateChange> execute_set_signal_aspect(const IStateView& /*state*/,
                                                         const SetSignalAspectCmd& cmd)
{
    return {SignalAspectChange{cmd.gid, cmd.aspect, ChangeCause::COMMAND, std::nullopt}};
}

// ── R3: SetDerailerPosition ───────────────────────────────────────────────────

std::optional<InterlockingViolation> check_set_derailer_position(const IStateView& state,
                                                                 const SetDerailerPositionCmd& cmd)
{
    const Derailer* d = state.find_derailer(cmd.gid);
    if (!d)
        return violation(NAK_NOT_FOUND, "Derailer not found: " + cmd.gid.value, cmd.gid);

    if (d->locked_by_route.has_value())
        return violation(
            NAK_ROUTE_LOCKED,
            "Derailer locked by route " + d->locked_by_route->value + ": " + cmd.gid.value,
            cmd.gid);

    if (cmd.position == DerailerState::UNLOCKED)
    {
        const TrackSection* guarded = state.find_track_section(d->guards_track_section_gid);
        if (guarded && guarded->occupancy == TrackOccupancy::OCCUPIED)
            return violation(NAK_SAFETY_BLOCK, "Guarded section is occupied: " + cmd.gid.value,
                             cmd.gid);
    }

    return std::nullopt;
}

std::vector<DeviceStateChange> execute_set_derailer_position(const IStateView& /*state*/,
                                                             const SetDerailerPositionCmd& cmd)
{
    return {DerailerStateChange{cmd.gid, cmd.position, ChangeCause::COMMAND, std::nullopt}};
}

// ── R4: SetBlockSection (EbiLock OPEN/CLOSED) ─────────────────────────────────

std::optional<InterlockingViolation> check_set_block_section(const IStateView& state,
                                                             const SetBlockSectionCmd& cmd)
{
    const BlockSection* bs = state.find_block_section(cmd.gid);
    if (!bs)
        return violation(NAK_NOT_FOUND, "Block section not found: " + cmd.gid.value, cmd.gid);

    if (cmd.state == BlockSectionState::CLOSED && bs->axle_count != 0)
        return violation(NAK_SAFETY_BLOCK,
                         "Cannot close block section: axle count " +
                             std::to_string(bs->axle_count) + " != 0: " + cmd.gid.value,
                         cmd.gid);

    return std::nullopt;
}

std::vector<DeviceStateChange> execute_set_block_section(const IStateView& /*state*/,
                                                         const SetBlockSectionCmd& cmd)
{
    return {BlockSectionStateChange{cmd.gid, cmd.state}};
}

// ── R5: RequestRoute ─────────────────────────────────────────────────────────

std::optional<InterlockingViolation> check_request_route(const IStateView& state,
                                                         const RequestRouteCmd& cmd)
{
    const Signal* entry = state.find_signal(cmd.from_signal_gid);
    if (!entry)
        return violation(NAK_NOT_FOUND, "Entry signal not found: " + cmd.from_signal_gid.value,
                         cmd.from_signal_gid);

    const Signal* exit = state.find_signal(cmd.to_signal_gid);
    if (!exit)
        return violation(NAK_NOT_FOUND, "Exit signal not found: " + cmd.to_signal_gid.value,
                         cmd.to_signal_gid);

    // Entry signal must not already be locked by a different route.
    if (entry->locked_by_route.has_value())
        return violation(NAK_ROUTE_LOCKED,
                         "Entry signal already route-locked: " + cmd.from_signal_gid.value,
                         cmd.from_signal_gid);

    auto path = find_route_path(state, cmd.from_signal_gid, cmd.to_signal_gid);
    if (!path)
        return violation(NAK_NO_PATH, "No topology path from " + cmd.from_signal_gid.value +
                                          " to " + cmd.to_signal_gid.value);

    // Check that no switch along the path is occupied or locked by another route.
    for (const auto& node : path->nodes)
    {
        if (node.kind != RoutePathNode::Kind::SWITCH)
            continue;
        const Switch* sw = state.find_switch(node.gid);
        if (!sw)
            continue;

        if (sw->occupancy == TrackOccupancy::OCCUPIED)
            return violation(NAK_SAFETY_BLOCK, "Switch occupied on route path: " + node.gid.value,
                             node.gid);

        if (sw->locked_by_route.has_value())
            return violation(NAK_ROUTE_LOCKED, "Switch locked by another route: " + node.gid.value,
                             node.gid);

        if (sw->position == SwitchPosition::MOVING)
            return violation(NAK_SWITCH_MOVING, "Switch is moving: " + node.gid.value, node.gid);
    }

    // Check that no section along the path is occupied.
    for (const auto& sgid : path->section_gids)
    {
        const TrackSection* ts = state.find_track_section(sgid);
        if (ts && ts->occupancy == TrackOccupancy::OCCUPIED)
            return violation(NAK_SAFETY_BLOCK,
                             "Track section occupied on route path: " + sgid.value, sgid);
    }

    return std::nullopt;
}

std::vector<DeviceStateChange> execute_request_route(const IStateView& state,
                                                     const RequestRouteCmd& cmd, uint64_t tick)
{
    auto path = find_route_path(state, cmd.from_signal_gid, cmd.to_signal_gid);
    if (!path)
        return {};

    const GID route_id = make_route_id(cmd.from_signal_gid, cmd.to_signal_gid, tick);
    std::vector<DeviceStateChange> changes;

    // Position switches to required position (instantaneous for route setting;
    // individual switch machines are already confirmed STOPPED by check).
    for (const auto& node : path->nodes)
    {
        if (node.kind != RoutePathNode::Kind::SWITCH)
            continue;
        const Switch* sw = state.find_switch(node.gid);
        if (!sw)
            continue;
        if (sw->position != node.required_position)
        {
            changes.push_back(
                SwitchPositionChange{node.gid, node.required_position, ChangeCause::AUTO, 0});
        }
        changes.push_back(SwitchLocked{node.gid, route_id});
    }

    // Lock derailers (unlock them so the route path is clear, then lock to route).
    for (const GID& dgid : path->derailer_gids)
    {
        const Derailer* d = state.find_derailer(dgid);
        if (!d)
            continue;
        if (d->state == DerailerState::LOCKED)
            changes.push_back(
                DerailerStateChange{dgid, DerailerState::UNLOCKED, ChangeCause::AUTO, route_id});
    }

    // Show proceed aspect on entry signal.
    changes.push_back(SignalAspectChange{cmd.from_signal_gid, SignalAspect::S2_PROCEED,
                                         ChangeCause::AUTO, route_id});

    // Build and add the RouteState.
    RouteState rs;
    rs.route_id = route_id;
    rs.from_signal_gid = cmd.from_signal_gid;
    rs.to_signal_gid = cmd.to_signal_gid;
    rs.section_gids = path->section_gids;
    rs.switch_gids = path->switch_gids;
    rs.derailer_gids = path->derailer_gids;
    rs.created_tick = tick;
    changes.push_back(RouteAdded{std::move(rs)});

    return changes;
}

// ── R5: CancelRoute ──────────────────────────────────────────────────────────

std::optional<InterlockingViolation> check_cancel_route(const IStateView& state,
                                                        const CancelRouteCmd& cmd)
{
    const RouteState* route = state.find_route(cmd.route_id);
    if (!route)
        return violation(NAK_NOT_FOUND, "Route not found: " + cmd.route_id.value, cmd.route_id);

    if (!cmd.force && route->train_entered)
        return violation(NAK_SAFETY_BLOCK,
                         "Train has entered route; use force to cancel: " + cmd.route_id.value,
                         cmd.route_id);

    return std::nullopt;
}

std::vector<DeviceStateChange> execute_cancel_route(const IStateView& state,
                                                    const CancelRouteCmd& cmd)
{
    const RouteState* route = state.find_route(cmd.route_id);
    if (!route)
        return {};

    std::vector<DeviceStateChange> changes;

    // Reset entry signal to STOP.
    changes.push_back(SignalAspectChange{route->from_signal_gid, SignalAspect::S1_STOP,
                                         ChangeCause::COMMAND, cmd.route_id});

    // Unlock switches.
    for (const GID& swgid : route->switch_gids)
        changes.push_back(SwitchUnlocked{swgid, cmd.route_id});

    // Re-lock derailers.
    for (const GID& dgid : route->derailer_gids)
    {
        const Derailer* d = state.find_derailer(dgid);
        if (d && d->state == DerailerState::UNLOCKED)
            changes.push_back(
                DerailerStateChange{dgid, DerailerState::LOCKED, ChangeCause::AUTO, std::nullopt});
    }

    const std::string reason = cmd.force ? "FORCE" : "OPERATOR_CANCEL";
    changes.push_back(RouteRemoved{cmd.route_id, reason});

    return changes;
}

// ── R7: AcknowledgeAlarm ──────────────────────────────────────────────────────

std::optional<InterlockingViolation> check_acknowledge_alarm(const IStateView& state,
                                                             const AcknowledgeAlarmCmd& cmd)
{
    if (!state.find_alarm(cmd.alarm_id))
        return violation(NAK_NOT_FOUND, "Alarm not found: " + cmd.alarm_id.value, cmd.alarm_id);
    return std::nullopt;
}

std::vector<DeviceStateChange> execute_acknowledge_alarm(const IStateView& /*state*/,
                                                         const AcknowledgeAlarmCmd& cmd)
{
    return {AlarmCleared{cmd.alarm_id}};
}

static bool target_exists(const IStateView& state, const OperatorCommandCmd& cmd)
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
            return !cmd.target_gid.value.empty();
    }
    return false;
}

std::optional<InterlockingViolation> check_operator_command(const IStateView& state,
                                                            const OperatorCommandCmd& cmd)
{
    if (!target_exists(state, cmd))
        return violation(NAK_NOT_FOUND, "Operator command target not found: " + cmd.target_gid.value,
                         cmd.target_gid);

    if (cmd.code == OperatorCommandCode::ZWP || cmd.code == OperatorCommandCode::ZWM ||
        cmd.code == OperatorCommandCode::ZBP || cmd.code == OperatorCommandCode::ZBM)
    {
        if (const Switch* sw = state.find_switch(cmd.target_gid))
        {
            if (sw->occupancy == TrackOccupancy::OCCUPIED)
                return violation(NAK_SAFETY_BLOCK,
                                 "Switch is occupied: " + cmd.target_gid.value, cmd.target_gid);
            if (sw->locked_by_route.has_value())
                return violation(NAK_ROUTE_LOCKED,
                                 "Switch locked by route: " + cmd.target_gid.value,
                                 cmd.target_gid);
            if (sw->position == SwitchPosition::MOVING)
                return violation(NAK_SWITCH_MOVING,
                                 "Switch is moving: " + cmd.target_gid.value, cmd.target_gid);
        }
    }

    if (cmd.code == OperatorCommandCode::BLZ)
    {
        const BlockSection* bs = state.find_block_section(cmd.target_gid);
        if (bs && bs->axle_count != 0)
            return violation(NAK_SAFETY_BLOCK, "Cannot release block direction: axle count != 0",
                             cmd.target_gid);
    }

    return std::nullopt;
}

std::vector<DeviceStateChange> execute_operator_command(const IStateView& state,
                                                        const OperatorCommandCmd& cmd,
                                                        int throw_time_ticks)
{
    std::vector<DeviceStateChange> changes;
    changes.push_back(OperatorCommandStateChange{cmd.target_gid, cmd.target_kind, cmd.code, true});

    switch (cmd.code)
    {
        case OperatorCommandCode::SES:
            if (const Signal* sig = state.find_signal(cmd.target_gid))
            {
                const auto aspect = sig->type == Signal::Type::SHUNTING ? SignalAspect::MS1_STOP
                                                                         : SignalAspect::S1_STOP;
                changes.push_back(
                    SignalAspectChange{cmd.target_gid, aspect, ChangeCause::COMMAND, std::nullopt});
            }
            break;
        case OperatorCommandCode::SEO:
        case OperatorCommandCode::SZO:
        case OperatorCommandCode::SAW:
        case OperatorCommandCode::ZWO:
        case OperatorCommandCode::ITO:
        case OperatorCommandCode::OST:
        case OperatorCommandCode::OZK:
            changes.push_back(
                OperatorCommandStateChange{cmd.target_gid, cmd.target_kind, cmd.code, false});
            break;
        case OperatorCommandCode::SZW:
        case OperatorCommandCode::SZN:
            changes.push_back(SignalAspectChange{cmd.target_gid, SignalAspect::S2_PROCEED,
                                                 ChangeCause::COMMAND, std::nullopt});
            break;
        case OperatorCommandCode::ZWP:
        case OperatorCommandCode::ZBP:
            if (state.find_switch(cmd.target_gid))
            {
                auto sub = execute_set_switch_position(
                    state, SetSwitchPositionCmd{cmd.target_gid, SwitchPosition::STRAIGHT},
                    throw_time_ticks);
                changes.insert(changes.end(), sub.begin(), sub.end());
            }
            else if (state.find_derailer(cmd.target_gid))
            {
                auto sub = execute_set_derailer_position(
                    state, SetDerailerPositionCmd{cmd.target_gid, DerailerState::LOCKED});
                changes.insert(changes.end(), sub.begin(), sub.end());
            }
            break;
        case OperatorCommandCode::ZWM:
        case OperatorCommandCode::ZBM:
            if (state.find_switch(cmd.target_gid))
            {
                auto sub = execute_set_switch_position(
                    state, SetSwitchPositionCmd{cmd.target_gid, SwitchPosition::DIVERGENT},
                    throw_time_ticks);
                changes.insert(changes.end(), sub.begin(), sub.end());
            }
            else if (state.find_derailer(cmd.target_gid))
            {
                auto sub = execute_set_derailer_position(
                    state, SetDerailerPositionCmd{cmd.target_gid, DerailerState::UNLOCKED});
                changes.insert(changes.end(), sub.begin(), sub.end());
            }
            break;
        case OperatorCommandCode::SLK:
            changes.push_back(AxleCounterResetChange{cmd.target_gid, cmd.target_kind});
            changes.push_back(
                OperatorCommandStateChange{cmd.target_gid, cmd.target_kind, cmd.code, false});
            break;
        case OperatorCommandCode::BLS:
            changes.push_back(BlockSectionStateChange{cmd.target_gid, BlockSectionState::CLOSED});
            break;
        case OperatorCommandCode::BLW:
            changes.push_back(BlockDirectionChange{cmd.target_gid,
                                                   BlockDirectionState::OUTBOUND_PENDING, true});
            break;
        case OperatorCommandCode::BLP:
            if (const BlockSection* bs = state.find_block_section(cmd.target_gid))
            {
                const auto direction = bs->direction == BlockDirectionState::INBOUND_PENDING
                                           ? BlockDirectionState::INBOUND
                                           : BlockDirectionState::OUTBOUND;
                changes.push_back(BlockDirectionChange{cmd.target_gid, direction, false});
                changes.push_back(BlockSectionStateChange{cmd.target_gid, BlockSectionState::OPEN});
            }
            break;
        case OperatorCommandCode::BLO:
        case OperatorCommandCode::BLZ:
        case OperatorCommandCode::BLA:
        case OperatorCommandCode::OPS:
            changes.push_back(
                BlockDirectionChange{cmd.target_gid, BlockDirectionState::NEUTRAL, false});
            changes.push_back(BlockSectionStateChange{cmd.target_gid, BlockSectionState::CLOSED});
            break;
        case OperatorCommandCode::BLAI:
            changes.push_back(
                BlockDirectionChange{cmd.target_gid, BlockDirectionState::EMERGENCY, false});
            break;
        default:
            break;
    }

    return changes;
}

// ── Tick helpers ──────────────────────────────────────────────────────────────

std::vector<DeviceStateChange> tick_switch_machines(const IStateView& state)
{
    std::vector<DeviceStateChange> changes;

    state.for_each_switch(
        [&](const Switch& sw)
        {
            if (sw.position != SwitchPosition::MOVING)
                return;
            if (sw.moving_ticks_remaining <= 0)
                return;

            const int remaining = sw.moving_ticks_remaining - 1;
            if (remaining == 0)
            {
                // The SRK library cannot know the target position without storing it.
                // The actual landing position is stored by the SRK implementation that
                // issued the MOVING change (EbiLockSystem / Ml8System via pending_targets_).
                // tick_switch_machines() is intentionally not called here directly —
                // each system calls it from on_tick() where it also tracks targets.
                // This helper just decrements for visualization purposes.
                changes.push_back(SwitchPositionChange{sw.gid, SwitchPosition::MOVING,
                                                       ChangeCause::AUTO, remaining});
            }
            else
            {
                changes.push_back(SwitchPositionChange{sw.gid, SwitchPosition::MOVING,
                                                       ChangeCause::AUTO, remaining});
            }
        });

    return changes;
}

std::vector<DeviceStateChange> tick_route_auto_release(const IStateView& state)
{
    std::vector<DeviceStateChange> changes;

    state.for_each_route(
        [&](const RouteState& route)
        {
            if (!route.train_entered)
                return;

            // Check whether all sections on the route are now free.
            bool all_free = true;
            for (const GID& sgid : route.section_gids)
            {
                const TrackSection* ts = state.find_track_section(sgid);
                if (ts && ts->occupancy == TrackOccupancy::OCCUPIED)
                {
                    all_free = false;
                    break;
                }
            }
            if (!all_free)
                return;

            // Reset entry signal to STOP.
            changes.push_back(SignalAspectChange{route.from_signal_gid, SignalAspect::S1_STOP,
                                                 ChangeCause::AUTO, route.route_id});

            // Unlock switches.
            for (const GID& swgid : route.switch_gids)
                changes.push_back(SwitchUnlocked{swgid, route.route_id});

            // Re-lock derailers.
            // (We don't have derailer states here without the state view — handled by engine)

            changes.push_back(RouteRemoved{route.route_id, "TRAIN_CLEARED"});
        });

    return changes;
}

}  // namespace srk::common
