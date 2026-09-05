#include <srk/common/device_rules.hpp>
#include <srk/common/nak_codes.hpp>
#include <srk/common/route_graph.hpp>

#include <algorithm>
#include <string>

namespace srk::common
{

// ── Violation helpers ─────────────────────────────────────────────────────────

static InterlockingViolation violation(uint8_t code, std::string text, UID uid = UID{})
{
    return InterlockingViolation{code, std::move(text), uid};
}

static std::string uid_str(UID uid)
{
    return std::to_string(uid.value);
}

// ── R1: SetSwitchPosition ─────────────────────────────────────────────────────

std::optional<InterlockingViolation> check_set_switch_position(const IStateView& state,
                                                               const SetSwitchPositionCmd& cmd)
{
    const Switch* sw = state.find_switch(cmd.uid);
    if (!sw)
        return violation(NAK_NOT_FOUND, "Switch not found: " + uid_str(cmd.uid), cmd.uid);

    if (sw->position == SwitchPosition::MOVING)
        return violation(NAK_SWITCH_MOVING, "Switch is already moving: " + uid_str(cmd.uid),
                         cmd.uid);

    if (sw->occupancy == TrackOccupancy::OCCUPIED)
        return violation(NAK_SAFETY_BLOCK, "Switch is occupied: " + uid_str(cmd.uid), cmd.uid);

    if (sw->locked_by_route_uid.has_value())
        return violation(
            NAK_ROUTE_LOCKED,
            "Switch locked by route " + uid_str(*sw->locked_by_route_uid) + ": " + uid_str(cmd.uid),
            cmd.uid);

    if (sw->position == cmd.position)
        return violation(NAK_INVALID_STATE,
                         "Switch already in requested position: " + uid_str(cmd.uid), cmd.uid);

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
        changes.push_back(SwitchPositionChange{cmd.uid, SwitchPosition::MOVING,
                                               ChangeCause::COMMAND, throw_time_ticks});
    }
    else
    {
        // Instantaneous switch (for testing / simplified devices).
        changes.push_back(SwitchPositionChange{cmd.uid, cmd.position, ChangeCause::COMMAND, 0});
    }
    return changes;
}

// ── R2: SetSignalAspect ───────────────────────────────────────────────────────

std::optional<InterlockingViolation> check_set_signal_aspect(const IStateView& state,
                                                             const SetSignalAspectCmd& cmd)
{
    const Signal* sig = state.find_signal(cmd.uid);
    if (!sig)
        return violation(NAK_NOT_FOUND, "Signal not found: " + uid_str(cmd.uid), cmd.uid);

    // Refuse to show a proceed aspect on a route-locked signal via direct command;
    // proceed aspects are set through RequestRoute.  Direct commands may only set STOP
    // or shunting aspects on locked signals.
    if (sig->locked_by_route_uid.has_value())
    {
        const bool is_stop =
            (cmd.aspect == SignalAspect::S1_STOP || cmd.aspect == SignalAspect::MS1_STOP);
        if (!is_stop)
            return violation(NAK_ROUTE_LOCKED,
                             "Cannot change locked signal to proceed aspect: " + uid_str(cmd.uid),
                             cmd.uid);
    }

    return std::nullopt;
}

std::vector<DeviceStateChange> execute_set_signal_aspect(const IStateView& /*state*/,
                                                         const SetSignalAspectCmd& cmd)
{
    return {SignalAspectChange{cmd.uid, cmd.aspect, ChangeCause::COMMAND, std::nullopt}};
}

// ── R3: SetDerailerPosition ───────────────────────────────────────────────────

std::optional<InterlockingViolation> check_set_derailer_position(const IStateView& state,
                                                                 const SetDerailerPositionCmd& cmd)
{
    const Derailer* d = state.find_derailer(cmd.uid);
    if (!d)
        return violation(NAK_NOT_FOUND, "Derailer not found: " + uid_str(cmd.uid), cmd.uid);

    if (d->locked_by_route_uid.has_value())
        return violation(NAK_ROUTE_LOCKED,
                         "Derailer locked by route " + uid_str(*d->locked_by_route_uid) + ": " +
                             uid_str(cmd.uid),
                         cmd.uid);

    if (cmd.position == DerailerState::UNLOCKED)
    {
        const TrackSection* guarded = state.find_track_section(d->guards_section_uid);
        if (guarded && guarded->occupancy == TrackOccupancy::OCCUPIED)
            return violation(NAK_SAFETY_BLOCK, "Guarded section is occupied: " + uid_str(cmd.uid),
                             cmd.uid);
    }

    return std::nullopt;
}

std::vector<DeviceStateChange> execute_set_derailer_position(const IStateView& /*state*/,
                                                             const SetDerailerPositionCmd& cmd)
{
    return {DerailerStateChange{cmd.uid, cmd.position, ChangeCause::COMMAND, std::nullopt}};
}

// ── R4: SetBlockSection (EbiLock OPEN/CLOSED) ─────────────────────────────────

std::optional<InterlockingViolation> check_set_block_section(const IStateView& state,
                                                             const SetBlockSectionCmd& cmd)
{
    const BlockSection* bs = state.find_block_section(cmd.uid);
    if (!bs)
        return violation(NAK_NOT_FOUND, "Block section not found: " + uid_str(cmd.uid), cmd.uid);

    if (cmd.state == BlockSectionState::CLOSED && bs->axle_count != 0)
        return violation(NAK_SAFETY_BLOCK,
                         "Cannot close block section: axle count " +
                             std::to_string(bs->axle_count) + " != 0: " + uid_str(cmd.uid),
                         cmd.uid);

    return std::nullopt;
}

std::vector<DeviceStateChange> execute_set_block_section(const IStateView& /*state*/,
                                                         const SetBlockSectionCmd& cmd)
{
    return {BlockSectionStateChange{cmd.uid, cmd.state}};
}

// ── R5: RequestRoute ─────────────────────────────────────────────────────────

bool FlankProtectionPolicy::apply(const IStateView& state, RoutePath& path) const
{
    for (std::size_t i = 0; i < path.nodes.size(); ++i)
    {
        if (path.nodes[i].kind != RoutePathNode::Kind::SWITCH) continue;
        const Switch* sw = state.find_switch(path.nodes[i].uid);
        if (!sw) continue;

        UID unused_leg = (path.nodes[i].required_position == SwitchPosition::STRAIGHT) 
            ? sw->divergent.neighbor_uid 
            : sw->straight.neighbor_uid;
        
        if (const Switch* flank_sw = state.find_switch(unused_leg))
        {
            SwitchPosition deflect_pos = SwitchPosition::STRAIGHT;
            if (flank_sw->trunk.neighbor_uid == sw->uid) {
                // TODO: Read safe deflection positions from interlocking control tables (Tablice Zależności).
                // Assuming DIVERGENT is safe is a heuristic that might lead to main tracks.
                deflect_pos = SwitchPosition::DIVERGENT; 
            } else if (flank_sw->straight.neighbor_uid == sw->uid) {
                    deflect_pos = SwitchPosition::DIVERGENT;
                } else if (flank_sw->divergent.neighbor_uid == sw->uid) {
                    deflect_pos = SwitchPosition::STRAIGHT;
                }
                path.flank_switches.push_back({RoutePathNode::Kind::SWITCH, flank_sw->uid, deflect_pos});
            }
        }
    }
    return true;
}

std::optional<InterlockingViolation> check_request_route(const IStateView& state,
                                                         const RequestRouteCmd& cmd,
                                                         const std::vector<const IRoutePathPolicy*>& policies)
{
    const Signal* entry = state.find_signal(cmd.from_signal_uid);
    if (!entry)
        return violation(NAK_NOT_FOUND, "Entry signal not found: " + uid_str(cmd.from_signal_uid),
                         cmd.from_signal_uid);

    const Signal* exit = state.find_signal(cmd.to_signal_uid);
    if (!exit)
        return violation(NAK_NOT_FOUND, "Exit signal not found: " + uid_str(cmd.to_signal_uid),
                         cmd.to_signal_uid);

    // Entry signal must not already be locked by a different route.
    if (entry->locked_by_route_uid.has_value())
        return violation(NAK_ROUTE_LOCKED,
                         "Entry signal already route-locked: " + uid_str(cmd.from_signal_uid),
                         cmd.from_signal_uid);

    auto path = find_route_path(state, cmd.from_signal_uid, cmd.to_signal_uid, policies);
    if (!path)
        return violation(NAK_NO_PATH, "No topology path from " + uid_str(cmd.from_signal_uid) +
                                          " to " + uid_str(cmd.to_signal_uid));

    // Check that no switch along the path is occupied or locked by another route.
    for (const auto& node : path->nodes)
    {
        if (node.kind != RoutePathNode::Kind::SWITCH)
            continue;
        const Switch* sw = state.find_switch(node.uid);
        if (!sw)
            continue;

        if (sw->occupancy == TrackOccupancy::OCCUPIED)
            return violation(NAK_SAFETY_BLOCK,
                             "Switch occupied on route path: " + uid_str(node.uid), node.uid);

        if (sw->locked_by_route_uid.has_value())
            return violation(NAK_ROUTE_LOCKED,
                             "Switch locked by another route: " + uid_str(node.uid), node.uid);

        if (sw->position == SwitchPosition::MOVING)
            return violation(NAK_SWITCH_MOVING, "Switch is moving: " + uid_str(node.uid), node.uid);
    }

    // Check that no section along the path is occupied.
    for (const auto& suid : path->section_uids)
    {
        const TrackSection* ts = state.find_track_section(suid);
        if (ts && ts->occupancy == TrackOccupancy::OCCUPIED)
            return violation(NAK_SAFETY_BLOCK,
                             "Track section occupied on route path: " + uid_str(suid), suid);
    }

    return std::nullopt;
}

std::vector<DeviceStateChange> execute_request_route(const IStateView& state,
                                                     const RequestRouteCmd& cmd, uint64_t tick,
                                                     const std::vector<const IRoutePathPolicy*>& policies)
{
    auto path = find_route_path(state, cmd.from_signal_uid, cmd.to_signal_uid, policies);
    if (!path)
        return {};

    const UID route_uid = make_route_uid(cmd.from_signal_uid, cmd.to_signal_uid, tick);
    std::vector<DeviceStateChange> changes;

    // Position switches to required position (instantaneous for route setting;
    // individual switch machines are already confirmed STOPPED by check).
    for (const auto& node : path->nodes)
    {
        if (node.kind != RoutePathNode::Kind::SWITCH)
            continue;
        const Switch* sw = state.find_switch(node.uid);
        if (!sw)
            continue;
        if (sw->position != node.required_position)
        {
            changes.push_back(
                SwitchPositionChange{node.uid, node.required_position, ChangeCause::AUTO, 0});
        }
        changes.push_back(SwitchLocked{node.uid, route_uid});
    }

    // Lock flank switches.
    for (const auto& fnode : path->flank_switches)
    {
        const Switch* sw = state.find_switch(fnode.uid);
        if (!sw) continue;
        if (sw->position != fnode.required_position)
        {
            changes.push_back(
                SwitchPositionChange{fnode.uid, fnode.required_position, ChangeCause::AUTO, 0});
        }
        changes.push_back(SwitchLocked{fnode.uid, route_uid});
        path->switch_uids.push_back(fnode.uid); // Add to RouteState for later unlocking
    }

    // Lock derailers (unlock them so the route path is clear, then lock to route).
    for (const UID& duid : path->derailer_uids)
    {
        const Derailer* d = state.find_derailer(duid);
        if (!d)
            continue;
        if (d->state == DerailerState::LOCKED)
            changes.push_back(
                DerailerStateChange{duid, DerailerState::UNLOCKED, ChangeCause::AUTO, route_uid});
    }

    // Show proceed aspect on entry signal.
    changes.push_back(SignalAspectChange{cmd.from_signal_uid, SignalAspect::S2_PROCEED,
                                         ChangeCause::AUTO, route_uid});

    // Build and add the RouteState.
    RouteState rs;
    rs.uid = route_uid;
    rs.from_signal_uid = cmd.from_signal_uid;
    rs.to_signal_uid = cmd.to_signal_uid;
    rs.section_uids = path->section_uids;
    rs.switch_uids = path->switch_uids;
    rs.derailer_uids = path->derailer_uids;
    rs.created_tick = tick;
    changes.push_back(RouteAdded{std::move(rs)});

    return changes;
}

// ── R5: CancelRoute ──────────────────────────────────────────────────────────

std::optional<InterlockingViolation> check_cancel_route(const IStateView& state,
                                                        const CancelRouteCmd& cmd)
{
    const RouteState* route = state.find_route(cmd.route_uid);
    if (!route)
        return violation(NAK_NOT_FOUND, "Route not found: " + uid_str(cmd.route_uid),
                         cmd.route_uid);

    if (!cmd.force && route->train_entered)
        return violation(NAK_SAFETY_BLOCK,
                         "Train has entered route; use force to cancel: " + uid_str(cmd.route_uid),
                         cmd.route_uid);

    return std::nullopt;
}

std::vector<DeviceStateChange> execute_cancel_route(const IStateView& state,
                                                    const CancelRouteCmd& cmd)
{
    const RouteState* route = state.find_route(cmd.route_uid);
    if (!route)
        return {};

    std::vector<DeviceStateChange> changes;

    // Reset entry signal to STOP.
    changes.push_back(SignalAspectChange{route->from_signal_uid, SignalAspect::S1_STOP,
                                         ChangeCause::COMMAND, cmd.route_uid});

    // Unlock switches.
    for (const UID& swuid : route->switch_uids)
        changes.push_back(SwitchUnlocked{swuid, cmd.route_uid});

    // Re-lock derailers.
    for (const UID& duid : route->derailer_uids)
    {
        const Derailer* d = state.find_derailer(duid);
        if (d && d->state == DerailerState::UNLOCKED)
            changes.push_back(
                DerailerStateChange{duid, DerailerState::LOCKED, ChangeCause::AUTO, std::nullopt});
    }

    const std::string reason = cmd.force ? "FORCE" : "OPERATOR_CANCEL";
    if (cmd.force)
    {
        changes.push_back(EmergencyRouteReleaseExecuted{cmd.route_uid});
    }
    changes.push_back(RouteRemoved{cmd.route_uid, reason});

    return changes;
}

// ── R7: AcknowledgeAlarm ──────────────────────────────────────────────────────

std::optional<InterlockingViolation> check_acknowledge_alarm(const IStateView& state,
                                                             const AcknowledgeAlarmCmd& cmd)
{
    if (!state.find_alarm(cmd.alarm_uid))
        return violation(NAK_NOT_FOUND, "Alarm not found: " + uid_str(cmd.alarm_uid),
                         cmd.alarm_uid);
    return std::nullopt;
}

std::vector<DeviceStateChange> execute_acknowledge_alarm(const IStateView& /*state*/,
                                                         const AcknowledgeAlarmCmd& cmd)
{
    return {AlarmCleared{cmd.alarm_uid}};
}

static bool target_exists(const IStateView& state, const OperatorCommandCmd& cmd)
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
}

std::optional<InterlockingViolation> check_operator_command(const IStateView& state,
                                                            const OperatorCommandCmd& cmd)
{
    if (!target_exists(state, cmd))
        return violation(NAK_NOT_FOUND,
                         "Operator command target not found: " + uid_str(cmd.target_uid),
                         cmd.target_uid);

    if (cmd.code == OperatorCommandCode::ZWP || cmd.code == OperatorCommandCode::ZWM ||
        cmd.code == OperatorCommandCode::ZBP || cmd.code == OperatorCommandCode::ZBM)
    {
        if (const Switch* sw = state.find_switch(cmd.target_uid))
        {
            if (sw->occupancy == TrackOccupancy::OCCUPIED)
                return violation(NAK_SAFETY_BLOCK, "Switch is occupied: " + uid_str(cmd.target_uid),
                                 cmd.target_uid);
            if (sw->locked_by_route_uid.has_value())
                return violation(NAK_ROUTE_LOCKED,
                                 "Switch locked by route: " + uid_str(cmd.target_uid),
                                 cmd.target_uid);
            if (sw->position == SwitchPosition::MOVING)
                return violation(NAK_SWITCH_MOVING, "Switch is moving: " + uid_str(cmd.target_uid),
                                 cmd.target_uid);
        }
    }

    if (cmd.code == OperatorCommandCode::BLZ)
    {
        const BlockSection* bs = state.find_block_section(cmd.target_uid);
        if (bs && bs->axle_count != 0)
            return violation(NAK_SAFETY_BLOCK, "Cannot release block direction: axle count != 0",
                             cmd.target_uid);
    }

    return std::nullopt;
}

std::vector<DeviceStateChange> execute_operator_command(const IStateView& state,
                                                        const OperatorCommandCmd& cmd,
                                                        int throw_time_ticks)
{
    std::vector<DeviceStateChange> changes;
    changes.push_back(OperatorCommandStateChange{cmd.target_uid, cmd.target_kind, cmd.code, true});

    switch (cmd.code)
    {
        case OperatorCommandCode::SES:
            if (const Signal* sig = state.find_signal(cmd.target_uid))
            {
                const auto aspect = sig->type == Signal::Type::SHUNTING ? SignalAspect::MS1_STOP
                                                                         : SignalAspect::S1_STOP;
                changes.push_back(
                    SignalAspectChange{cmd.target_uid, aspect, ChangeCause::COMMAND, std::nullopt});
            }
            break;
        case OperatorCommandCode::SEO:
        case OperatorCommandCode::SZO:
        case OperatorCommandCode::SAW:
        case OperatorCommandCode::ZWO:
        case OperatorCommandCode::ITO:
        case OperatorCommandCode::OST:
        case OperatorCommandCode::OZK:
        case OperatorCommandCode::BTO:
        case OperatorCommandCode::PDO:
        case OperatorCommandCode::UPAO:
        case OperatorCommandCode::ZSO:
            changes.push_back(
                OperatorCommandStateChange{cmd.target_uid, cmd.target_kind, cmd.code, false});
            break;
        case OperatorCommandCode::SZW:
        case OperatorCommandCode::SZN:
            changes.push_back(SignalAspectChange{cmd.target_uid, SignalAspect::S2_PROCEED,
                                                 ChangeCause::COMMAND, std::nullopt});
            break;
        case OperatorCommandCode::ZWP:
        case OperatorCommandCode::ZBP:
            if (state.find_switch(cmd.target_uid))
            {
                auto sub = execute_set_switch_position(
                    state, SetSwitchPositionCmd{cmd.target_uid, SwitchPosition::STRAIGHT},
                    throw_time_ticks);
                changes.insert(changes.end(), sub.begin(), sub.end());
            }
            else if (state.find_derailer(cmd.target_uid))
            {
                auto sub = execute_set_derailer_position(
                    state, SetDerailerPositionCmd{cmd.target_uid, DerailerState::LOCKED});
                changes.insert(changes.end(), sub.begin(), sub.end());
            }
            break;
        case OperatorCommandCode::ZWM:
        case OperatorCommandCode::ZBM:
            if (state.find_switch(cmd.target_uid))
            {
                auto sub = execute_set_switch_position(
                    state, SetSwitchPositionCmd{cmd.target_uid, SwitchPosition::DIVERGENT},
                    throw_time_ticks);
                changes.insert(changes.end(), sub.begin(), sub.end());
            }
            else if (state.find_derailer(cmd.target_uid))
            {
                auto sub = execute_set_derailer_position(
                    state, SetDerailerPositionCmd{cmd.target_uid, DerailerState::UNLOCKED});
                changes.insert(changes.end(), sub.begin(), sub.end());
            }
            break;
        case OperatorCommandCode::SLK:
            changes.push_back(AxleCounterResetChange{cmd.target_uid, cmd.target_kind});
            changes.push_back(
                OperatorCommandStateChange{cmd.target_uid, cmd.target_kind, cmd.code, false});
            break;
        case OperatorCommandCode::BLS:
            changes.push_back(BlockSectionStateChange{cmd.target_uid, BlockSectionState::CLOSED});
            break;
        case OperatorCommandCode::BLW:
        case OperatorCommandCode::BPZ:
            changes.push_back(
                BlockDirectionChange{cmd.target_uid, BlockDirectionState::OUTBOUND_PENDING, true});
            break;
        case OperatorCommandCode::BPO:
        case OperatorCommandCode::BKO:
        case OperatorCommandCode::BTW:
        case OperatorCommandCode::POC:
        case OperatorCommandCode::PZW:
            changes.push_back(BlockSectionStateChange{cmd.target_uid, BlockSectionState::CLOSED});
            break;
        case OperatorCommandCode::BLP:
        case OperatorCommandCode::POZ:
            if (const BlockSection* bs = state.find_block_section(cmd.target_uid))
            {
                const auto direction = bs->direction == BlockDirectionState::INBOUND_PENDING
                                           ? BlockDirectionState::INBOUND
                                           : BlockDirectionState::OUTBOUND;
                changes.push_back(BlockDirectionChange{cmd.target_uid, direction, false});
                changes.push_back(BlockSectionStateChange{cmd.target_uid, BlockSectionState::OPEN});
            }
            break;
        case OperatorCommandCode::BLO:
        case OperatorCommandCode::BLZ:
        case OperatorCommandCode::BLA:
        case OperatorCommandCode::OPS:
        case OperatorCommandCode::DPW:
            changes.push_back(
                BlockDirectionChange{cmd.target_uid, BlockDirectionState::NEUTRAL, false});
            changes.push_back(BlockSectionStateChange{cmd.target_uid, BlockSectionState::CLOSED});
            break;
        case OperatorCommandCode::BLAI:
        case OperatorCommandCode::DPWI:
            changes.push_back(
                BlockDirectionChange{cmd.target_uid, BlockDirectionState::EMERGENCY, false});
            break;
        default:
            break;
    }

    return changes;
}

// ── R8: SetBlockDirection (SHL-12) ────────────────────────────────────────────

std::optional<InterlockingViolation> check_set_block_direction(const IStateView& state,
                                                               const SetBlockDirectionCmd& cmd)
{
    const BlockSection* bs = state.find_block_section(cmd.block_section_uid);
    if (!bs)
        return violation(NAK_NOT_FOUND,
                         "Block section not found: " + uid_str(cmd.block_section_uid),
                         cmd.block_section_uid);

    const BlockDirectionState dir = bs->direction;

    switch (cmd.operation)
    {
        case Shl12Op::BLW:
            if (dir != BlockDirectionState::NEUTRAL)
                return violation(NAK_INVALID_STATE,
                                 "BLW requires NEUTRAL direction, current: " +
                                     std::to_string(static_cast<int>(dir)),
                                 cmd.block_section_uid);
            break;

        case Shl12Op::BLP:
            if (dir != BlockDirectionState::OUTBOUND_PENDING &&
                dir != BlockDirectionState::INBOUND_PENDING)
                return violation(NAK_INVALID_STATE,
                                 "BLP requires OUTBOUND_PENDING or INBOUND_PENDING",
                                 cmd.block_section_uid);
            break;

        case Shl12Op::BLO:
            if (dir != BlockDirectionState::OUTBOUND_PENDING)
                return violation(NAK_INVALID_STATE, "BLO requires OUTBOUND_PENDING",
                                 cmd.block_section_uid);
            break;

        case Shl12Op::BLZ:
            if (dir != BlockDirectionState::OUTBOUND && dir != BlockDirectionState::INBOUND)
                return violation(NAK_INVALID_STATE, "BLZ requires OUTBOUND or INBOUND",
                                 cmd.block_section_uid);
            if (bs->axle_count != 0)
                return violation(NAK_SAFETY_BLOCK, "Cannot release direction: axle count != 0",
                                 cmd.block_section_uid);
            break;

        case Shl12Op::BLAI:
            if (dir == BlockDirectionState::RESET_PENDING)
                return violation(NAK_INVALID_STATE, "BLAI not allowed in RESET_PENDING state",
                                 cmd.block_section_uid);
            break;

        case Shl12Op::BLA:
            if (dir != BlockDirectionState::EMERGENCY)
                return violation(NAK_INVALID_STATE, "BLA requires EMERGENCY state",
                                 cmd.block_section_uid);
            break;

        case Shl12Op::OPS:
            if (dir != BlockDirectionState::EMERGENCY && dir != BlockDirectionState::RESET_PENDING)
                return violation(NAK_INVALID_STATE, "OPS requires EMERGENCY or RESET_PENDING state",
                                 cmd.block_section_uid);
            break;
    }

    return std::nullopt;
}

std::vector<DeviceStateChange> execute_set_block_direction(const IStateView& state,
                                                           const SetBlockDirectionCmd& cmd)
{
    const BlockSection* bs = state.find_block_section(cmd.block_section_uid);
    if (!bs)
        return {};

    std::vector<DeviceStateChange> changes;

    switch (cmd.operation)
    {
        case Shl12Op::BLW:
            changes.push_back(BlockDirectionChange{
                cmd.block_section_uid, BlockDirectionState::OUTBOUND_PENDING,
                true  // requires_neighbor_confirmation
            });
            break;

        case Shl12Op::BLP:
            if (bs->direction == BlockDirectionState::OUTBOUND_PENDING)
            {
                changes.push_back(BlockDirectionChange{cmd.block_section_uid,
                                                       BlockDirectionState::OUTBOUND, false});
                changes.push_back(
                    BlockSectionStateChange{cmd.block_section_uid, BlockSectionState::OPEN});
            }
            else
            {
                changes.push_back(BlockDirectionChange{cmd.block_section_uid,
                                                       BlockDirectionState::INBOUND, false});
                changes.push_back(
                    BlockSectionStateChange{cmd.block_section_uid, BlockSectionState::OPEN});
            }
            break;

        case Shl12Op::BLO:
            changes.push_back(
                BlockDirectionChange{cmd.block_section_uid, BlockDirectionState::NEUTRAL, false});
            break;

        case Shl12Op::BLZ:
            changes.push_back(
                BlockDirectionChange{cmd.block_section_uid, BlockDirectionState::NEUTRAL, false});
            changes.push_back(
                BlockSectionStateChange{cmd.block_section_uid, BlockSectionState::CLOSED});
            break;

        case Shl12Op::BLAI:
            changes.push_back(
                BlockDirectionChange{cmd.block_section_uid, BlockDirectionState::EMERGENCY, false});
            break;

        case Shl12Op::BLA:
            changes.push_back(
                BlockDirectionChange{cmd.block_section_uid, BlockDirectionState::NEUTRAL, false});
            changes.push_back(
                BlockSectionStateChange{cmd.block_section_uid, BlockSectionState::CLOSED});
            break;

        case Shl12Op::OPS:
            changes.push_back(
                BlockDirectionChange{cmd.block_section_uid, BlockDirectionState::NEUTRAL, false});
            changes.push_back(
                BlockSectionStateChange{cmd.block_section_uid, BlockSectionState::CLOSED});
            break;
    }

    return changes;
}

// ── R9: InitAxleCounterReset (SLI) ────────────────────────────────────────────

std::optional<InterlockingViolation> check_init_axle_counter_reset(
    const IStateView& state, const InitAxleCounterResetCmd& cmd)
{
    const BlockSection* bs = state.find_block_section(cmd.block_section_uid);
    if (!bs)
        return violation(NAK_NOT_FOUND,
                         "Block section not found: " + uid_str(cmd.block_section_uid),
                         cmd.block_section_uid);

    if (bs->direction != BlockDirectionState::NEUTRAL)
        return violation(NAK_INVALID_STATE, "SLI requires NEUTRAL direction",
                         cmd.block_section_uid);
    return std::nullopt;
}

std::vector<DeviceStateChange> execute_init_axle_counter_reset(const IStateView& /*state*/,
                                                               const InitAxleCounterResetCmd& cmd)
{
    return {BlockDirectionChange{cmd.block_section_uid, BlockDirectionState::RESET_PENDING, false}};
}

// ── R10: ResetAxleCounter (SLK) ───────────────────────────────────────────────

std::optional<InterlockingViolation> check_reset_axle_counter(const IStateView& state,
                                                              const ResetAxleCounterCmd& cmd)
{
    const BlockSection* bs = state.find_block_section(cmd.block_section_uid);
    if (!bs)
        return violation(NAK_NOT_FOUND,
                         "Block section not found: " + uid_str(cmd.block_section_uid),
                         cmd.block_section_uid);

    if (bs->direction != BlockDirectionState::RESET_PENDING)
        return violation(NAK_INVALID_STATE, "SLK requires RESET_PENDING state",
                         cmd.block_section_uid);
                         
    if (!bs->reset_init_tick.has_value() || (state.current_tick() < *bs->reset_init_tick + 60 * engine::core::ENGINE_TICKS_PER_SECOND))
        return violation(NAK_SAFETY_BLOCK, "SLK requires 60s delay after SLI",
                         cmd.block_section_uid);
                         
    return std::nullopt;
}

std::vector<DeviceStateChange> execute_reset_axle_counter(const IStateView& /*state*/,
                                                          const ResetAxleCounterCmd& cmd)
{
    return {
        BlockDirectionChange{cmd.block_section_uid, BlockDirectionState::NEUTRAL, false},
        BlockSectionStateChange{cmd.block_section_uid, BlockSectionState::CLOSED},
    };
}

// ── Tick helpers ──────────────────────────────────────────────────────────────

std::vector<DeviceStateChange> tick_switch_machines(
    const IStateView& state,
    std::unordered_map<UID, SwitchPosition, std::hash<UID>>& pending_targets)
{
    std::vector<DeviceStateChange> changes;

    state.for_each_switch(
        [&](const Switch& sw)
        {
            if (sw.position != SwitchPosition::MOVING)
            {
                pending_targets.erase(sw.uid);
                return;
            }
            if (sw.moving_ticks_remaining <= 0)
            {
                // Should not happen; land the switch as a safety fallback.
                auto it = pending_targets.find(sw.uid);
                SwitchPosition target =
                    (it != pending_targets.end()) ? it->second : SwitchPosition::STRAIGHT;
                pending_targets.erase(sw.uid);
                changes.push_back(SwitchPositionChange{sw.uid, target, ChangeCause::AUTO, 0});
                return;
            }

            const int remaining = sw.moving_ticks_remaining - 1;
            if (remaining == 0)
            {
                auto it = pending_targets.find(sw.uid);
                SwitchPosition target =
                    (it != pending_targets.end()) ? it->second : SwitchPosition::STRAIGHT;
                pending_targets.erase(sw.uid);
                changes.push_back(SwitchPositionChange{sw.uid, target, ChangeCause::AUTO, 0});
            }
            else
            {
                changes.push_back(SwitchPositionChange{sw.uid, SwitchPosition::MOVING,
                                                       ChangeCause::AUTO, remaining});
            }
        });

    return changes;
}

std::vector<DeviceStateChange> tick_route_auto_release(const IStateView& state, uint64_t current_tick)
{
    std::vector<DeviceStateChange> changes;

    state.for_each_route(
        [&](const RouteState& route)
        {
            if (!route.train_entered)
                return;

            // Check whether all sections on the route are now free.
            bool all_free = true;
            for (const UID& suid : route.section_uids)
            {
                const TrackSection* ts = state.find_track_section(suid);
                if (ts && ts->occupancy == TrackOccupancy::OCCUPIED)
                {
                    all_free = false;
                    break;
                }
            }
            if (!all_free)
                return;

            if (!route.overlap_release_tick.has_value())
            {
                // Start overlap timer for 60s
                changes.push_back(RouteOverlapTimerStarted{route.uid, current_tick + 60 * engine::core::ENGINE_TICKS_PER_SECOND});
                return;
            }

            if (current_tick < *route.overlap_release_tick)
                return; // Still waiting for overlap release

            // Reset entry signal to STOP.
            changes.push_back(SignalAspectChange{route.from_signal_uid, SignalAspect::S1_STOP,
                                                 ChangeCause::AUTO, route.uid});

            // Unlock switches.
            for (const UID& swuid : route.switch_uids)
                changes.push_back(SwitchUnlocked{swuid, route.uid});

            changes.push_back(RouteRemoved{route.uid, "TRAIN_CLEARED"});
        });

    return changes;
}

std::vector<DeviceStateChange> tick_level_crossings(const IStateView& state, uint64_t current_tick)
{
    std::vector<DeviceStateChange> changes;

    state.for_each_level_crossing(
        [&](const LevelCrossing& lx)
        {
            if (lx.status == LevelCrossingStatus::WARNING && lx.warning_start_tick.has_value())
            {
                if (current_tick >= *lx.warning_start_tick + lx.warning_duration_ticks)
                {
                    // Warning time elapsed, close the crossing
                    changes.push_back(LevelCrossingStateChange{lx.uid, LevelCrossingStatus::CLOSED});
                }
            }
        });

    return changes;
}

}  // namespace srk::common
