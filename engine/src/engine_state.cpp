#include "engine/core/engine_state.hpp"

namespace engine::core
{

// ── IStateView: find_* ────────────────────────────────────────────────────────

const BoundaryNode* EngineState::find_boundary_node(UID uid) const noexcept
{
    auto it = boundary_nodes_.find(uid);
    return it != boundary_nodes_.end() ? &it->second : nullptr;
}

const TrackSection* EngineState::find_track_section(UID uid) const noexcept
{
    auto it = track_sections_.find(uid);
    return it != track_sections_.end() ? &it->second : nullptr;
}

const Switch* EngineState::find_switch(UID uid) const noexcept
{
    auto it = switches_.find(uid);
    return it != switches_.end() ? &it->second : nullptr;
}

const Signal* EngineState::find_signal(UID uid) const noexcept
{
    auto it = signals_.find(uid);
    return it != signals_.end() ? &it->second : nullptr;
}

const Derailer* EngineState::find_derailer(UID uid) const noexcept
{
    auto it = derailers_.find(uid);
    return it != derailers_.end() ? &it->second : nullptr;
}

const BlockSection* EngineState::find_block_section(UID uid) const noexcept
{
    auto it = block_sections_.find(uid);
    return it != block_sections_.end() ? &it->second : nullptr;
}

const LevelCrossing* EngineState::find_level_crossing(UID uid) const noexcept
{
    auto it = level_crossings_.find(uid);
    return it != level_crossings_.end() ? &it->second : nullptr;
}

const RouteState* EngineState::find_route(UID route_uid) const noexcept
{
    auto it = routes_.find(route_uid);
    return it != routes_.end() ? &it->second : nullptr;
}

const AlarmState* EngineState::find_alarm(UID alarm_uid) const noexcept
{
    auto it = alarms_.find(alarm_uid);
    return it != alarms_.end() ? &it->second : nullptr;
}

// ── IStateView: for_each_* ────────────────────────────────────────────────────

void EngineState::for_each_track_section(std::function<void(const TrackSection&)> fn) const
{
    for (auto& [_, v] : track_sections_)
        fn(v);
}

void EngineState::for_each_switch(std::function<void(const Switch&)> fn) const
{
    for (auto& [_, v] : switches_)
        fn(v);
}

void EngineState::for_each_signal(std::function<void(const Signal&)> fn) const
{
    for (auto& [_, v] : signals_)
        fn(v);
}

void EngineState::for_each_derailer(std::function<void(const Derailer&)> fn) const
{
    for (auto& [_, v] : derailers_)
        fn(v);
}

void EngineState::for_each_block_section(std::function<void(const BlockSection&)> fn) const
{
    for (auto& [_, v] : block_sections_)
        fn(v);
}

void EngineState::for_each_route(std::function<void(const RouteState&)> fn) const
{
    for (auto& [_, v] : routes_)
        fn(v);
}

void EngineState::for_each_alarm(std::function<void(const AlarmState&)> fn) const
{
    for (auto& [_, v] : alarms_)
        fn(v);
}

void EngineState::for_each_boundary_node(std::function<void(const BoundaryNode&)> fn) const
{
    for (auto& [_, v] : boundary_nodes_)
        fn(v);
}

void EngineState::for_each_level_crossing(std::function<void(const LevelCrossing&)> fn) const
{
    for (auto& [_, v] : level_crossings_)
        fn(v);
}

// ── Topology insertion ────────────────────────────────────────────────────────

void EngineState::insert_boundary_node(BoundaryNode n)
{
    boundary_nodes_.emplace(n.uid, std::move(n));
}
void EngineState::insert_track_section(TrackSection s)
{
    track_sections_.emplace(s.uid, std::move(s));
}
void EngineState::insert_switch(Switch sw)
{
    switches_.emplace(sw.uid, std::move(sw));
}
void EngineState::insert_signal(Signal sig)
{
    signals_.emplace(sig.uid, std::move(sig));
}
void EngineState::insert_derailer(Derailer d)
{
    derailers_.emplace(d.uid, std::move(d));
}
void EngineState::insert_block_section(BlockSection b)
{
    block_sections_.emplace(b.uid, std::move(b));
}

void EngineState::insert_level_crossing(LevelCrossing lx)
{
    level_crossings_.emplace(lx.uid, std::move(lx));
}

// ── Queries ───────────────────────────────────────────────────────────────────

void EngineState::apply_track_section_occupancy(UID uid, TrackOccupancy occ, int axle_count)
{
    if (auto it = track_sections_.find(uid); it != track_sections_.end())
    {
        it->second.occupancy = occ;
        it->second.axle_count = axle_count;
    }
}

void EngineState::apply_level_crossing_status(UID uid, LevelCrossingStatus status)
{
    if (auto it = level_crossings_.find(uid); it != level_crossings_.end())
    {
        it->second.status = status;
        if (status == LevelCrossingStatus::WARNING)
            it->second.warning_start_tick = current_tick_;
        else
            it->second.warning_start_tick = std::nullopt;
    }
}

void EngineState::apply_switch_position(UID uid, SwitchPosition pos, int moving_ticks)
{
    if (auto it = switches_.find(uid); it != switches_.end())
    {
        it->second.position = pos;
        it->second.moving_ticks_remaining = moving_ticks;
    }
}

void EngineState::apply_switch_control(UID uid, bool control_lost)
{
    if (auto it = switches_.find(uid); it != switches_.end())
    {
        it->second.control_lost = control_lost;
    }
}

void EngineState::apply_switch_lock(UID uid, std::optional<UID> route_uid)
{
    if (auto it = switches_.find(uid); it != switches_.end())
        it->second.locked_by_route_uid = std::move(route_uid);
}

void EngineState::apply_switch_occupancy(UID uid, TrackOccupancy occ, int axle_count)
{
    if (auto it = switches_.find(uid); it != switches_.end())
    {
        it->second.occupancy = occ;
        it->second.axle_count = axle_count;
    }
}

void EngineState::apply_signal_aspect(UID uid, SignalAspect aspect)
{
    if (auto it = signals_.find(uid); it != signals_.end())
        it->second.current_aspect = aspect;
}

void EngineState::apply_signal_lock(UID uid, std::optional<UID> route_uid)
{
    if (auto it = signals_.find(uid); it != signals_.end())
        it->second.locked_by_route_uid = std::move(route_uid);
}

void EngineState::apply_derailer_state(UID uid, DerailerState state)
{
    if (auto it = derailers_.find(uid); it != derailers_.end())
        it->second.state = state;
}

void EngineState::apply_derailer_lock(UID uid, std::optional<UID> route_uid)
{
    if (auto it = derailers_.find(uid); it != derailers_.end())
        it->second.locked_by_route_uid = std::move(route_uid);
}

void EngineState::apply_block_section_state(UID uid, BlockSectionState state)
{
    if (auto it = block_sections_.find(uid); it != block_sections_.end())
        it->second.state = state;
}

void EngineState::apply_block_section_direction(UID uid, BlockDirectionState dir)
{
    if (auto it = block_sections_.find(uid); it != block_sections_.end())
    {
        it->second.direction = dir;
        if (dir == BlockDirectionState::RESET_PENDING)
            it->second.reset_init_tick = current_tick_;
        else
            it->second.reset_init_tick = std::nullopt;
    }
}

void EngineState::apply_block_section_axle_count(UID uid, int axle_count)
{
    if (auto it = block_sections_.find(uid); it != block_sections_.end())
        it->second.axle_count = axle_count;
}

static void apply_operator_state(OperatorCommandRuntimeState& state, OperatorCommandCode code,
                                 bool active)
{
    switch (code)
    {
        case OperatorCommandCode::SES:
            state.stopped = active;
            break;
        case OperatorCommandCode::SEO:
            state.stopped = false;
            break;
        case OperatorCommandCode::SZI:
            state.substitute_initialized = active;
            state.special_initialized = active;
            break;
        case OperatorCommandCode::SZW:
        case OperatorCommandCode::SZN:
            state.substitute_initialized = false;
            state.substitute_active = active;
            state.special_active = active;
            break;
        case OperatorCommandCode::SZO:
            state.substitute_initialized = false;
            state.substitute_active = false;
            state.special_initialized = false;
            state.special_active = false;
            break;
        case OperatorCommandCode::SAM:
            state.automatic_route_enabled = active;
            break;
        case OperatorCommandCode::SAW:
            state.automatic_route_enabled = false;
            break;
        case OperatorCommandCode::ZWS:
            state.clamped = active;
            break;
        case OperatorCommandCode::ZWO:
            state.clamped = false;
            break;
        case OperatorCommandCode::ITS:
        case OperatorCommandCode::BLS:
            state.traffic_closed = active;
            break;
        case OperatorCommandCode::ITO:
        case OperatorCommandCode::OST:
            state.traffic_closed = false;
            break;
        case OperatorCommandCode::ZWB:
            state.detection_bypassed = active;
            break;
        case OperatorCommandCode::ZBP:
        case OperatorCommandCode::ZBM:
            state.detection_bypassed = false;
            break;
        case OperatorCommandCode::ZRI:
        case OperatorCommandCode::BLAI:
        case OperatorCommandCode::ZKB:
        case OperatorCommandCode::ZESI:
        case OperatorCommandCode::PZZI:
        case OperatorCommandCode::DPOI:
        case OperatorCommandCode::DKOI:
        case OperatorCommandCode::DKPI:
        case OperatorCommandCode::DPWI:
        case OperatorCommandCode::PDII:
        case OperatorCommandCode::PZAI:
        case OperatorCommandCode::UPAI:
            state.special_initialized = active;
            break;
        case OperatorCommandCode::ZRK:
        case OperatorCommandCode::BLA:
        case OperatorCommandCode::ZES:
        case OperatorCommandCode::PZZ:
        case OperatorCommandCode::DPO:
        case OperatorCommandCode::DKO:
        case OperatorCommandCode::DKP:
        case OperatorCommandCode::DPW:
        case OperatorCommandCode::PDI:
        case OperatorCommandCode::PZA:
        case OperatorCommandCode::PZM:
            state.special_initialized = false;
            state.special_active = active;
            break;
        case OperatorCommandCode::OPS:
        case OperatorCommandCode::OZK:
        case OperatorCommandCode::BTO:
        case OperatorCommandCode::PDO:
        case OperatorCommandCode::UPAO:
        case OperatorCommandCode::ZSO:
            state.special_initialized = false;
            state.special_active = false;
            state.axle_reset_initialized = false;
            break;
        case OperatorCommandCode::SLI:
            state.axle_reset_initialized = active;
            state.special_initialized = active;
            break;
        case OperatorCommandCode::SLK:
            state.axle_reset_initialized = false;
            state.special_initialized = false;
            break;
        case OperatorCommandCode::BPO:
        case OperatorCommandCode::BKO:
        case OperatorCommandCode::BPZ:
        case OperatorCommandCode::BTW:
        case OperatorCommandCode::PDZ:
        case OperatorCommandCode::PZT:
        case OperatorCommandCode::PAZ:
        case OperatorCommandCode::PZO:
        case OperatorCommandCode::MRS:
        case OperatorCommandCode::ODS:
        case OperatorCommandCode::ZAL:
        case OperatorCommandCode::POC:
        case OperatorCommandCode::PZW:
        case OperatorCommandCode::SSO:
        case OperatorCommandCode::SSS:
        case OperatorCommandCode::UPA:
        case OperatorCommandCode::UPN:
        case OperatorCommandCode::UPO:
        case OperatorCommandCode::ZSS:
            state.special_active = active;
            break;
        default:
            state.special_active = active;
            break;
    }
}

void EngineState::apply_operator_command_state(UID uid, OperatorTargetKind target_kind,
                                               OperatorCommandCode code, bool active)
{
    auto apply_to_uid = [&](auto& map)
    {
        if (auto it = map.find(uid); it != map.end())
            apply_operator_state(it->second.operator_state, code, active);
    };

    switch (target_kind)
    {
        case OperatorTargetKind::SIGNAL:
            apply_to_uid(signals_);
            break;
        case OperatorTargetKind::SWITCH:
            apply_to_uid(switches_);
            if (switches_.find(uid) == switches_.end())
                apply_to_uid(derailers_);
            break;
        case OperatorTargetKind::DERAILER:
            apply_to_uid(derailers_);
            break;
        case OperatorTargetKind::TRACK_SECTION:
            apply_to_uid(track_sections_);
            break;
        case OperatorTargetKind::BLOCK_SECTION:
            apply_to_uid(block_sections_);
            break;
        case OperatorTargetKind::ROUTE:
        case OperatorTargetKind::LEVEL_CROSSING:
        case OperatorTargetKind::INTERLOCKING_COMPUTER:
        case OperatorTargetKind::POWER_SUPPLY:
        case OperatorTargetKind::AXLE_COUNTER_SYSTEM:
        case OperatorTargetKind::STATION:
            break;
        default:
            break;
    }
}

void EngineState::apply_ml8_command_state(UID uid, OperatorTargetKind target_kind,
                                          Ml8CommandCode code, bool active)
{
    auto apply_to_uid = [&](auto& map)
    {
        if (auto it = map.find(uid); it != map.end())
        {
            it->second.operator_state.active_ml8_command =
                active ? std::make_optional(code) : std::nullopt;
        }
    };

    switch (target_kind)
    {
        case OperatorTargetKind::SIGNAL:
            apply_to_uid(signals_);
            break;
        case OperatorTargetKind::SWITCH:
            apply_to_uid(switches_);
            if (switches_.find(uid) == switches_.end())
                apply_to_uid(derailers_);
            break;
        case OperatorTargetKind::DERAILER:
            apply_to_uid(derailers_);
            break;
        case OperatorTargetKind::TRACK_SECTION:
            apply_to_uid(track_sections_);
            break;
        case OperatorTargetKind::BLOCK_SECTION:
            apply_to_uid(block_sections_);
            break;
        default:
            break;
    }
}

void EngineState::apply_axle_counter_reset(UID uid, OperatorTargetKind target_kind)
{
    switch (target_kind)
    {
        case OperatorTargetKind::SWITCH:
            apply_switch_occupancy(uid, TrackOccupancy::FREE, 0);
            break;
        case OperatorTargetKind::TRACK_SECTION:
            apply_track_section_occupancy(uid, TrackOccupancy::FREE, 0);
            break;
        case OperatorTargetKind::BLOCK_SECTION:
            apply_block_section_axle_count(uid, 0);
            break;
        default:
            break;
    }
}

void EngineState::add_route(RouteState route)
{
    routes_.emplace(route.uid, std::move(route));
}

void EngineState::remove_route(UID route_uid)
{
    routes_.erase(route_uid);
}

void EngineState::add_alarm(AlarmState alarm)
{
    alarms_.emplace(alarm.uid, std::move(alarm));
}

void EngineState::remove_alarm(UID alarm_uid)
{
    alarms_.erase(alarm_uid);
}

}  // namespace engine::core
