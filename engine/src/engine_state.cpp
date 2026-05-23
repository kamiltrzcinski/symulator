#include "engine/core/engine_state.hpp"

namespace engine::core
{

// ── IStateView: find_* ────────────────────────────────────────────────────────

const BoundaryNode* EngineState::find_boundary_node(const GID& gid) const noexcept
{
    auto it = boundary_nodes_.find(gid);
    return it != boundary_nodes_.end() ? &it->second : nullptr;
}

const TrackSection* EngineState::find_track_section(const GID& gid) const noexcept
{
    auto it = track_sections_.find(gid);
    return it != track_sections_.end() ? &it->second : nullptr;
}

const Switch* EngineState::find_switch(const GID& gid) const noexcept
{
    auto it = switches_.find(gid);
    return it != switches_.end() ? &it->second : nullptr;
}

const Signal* EngineState::find_signal(const GID& gid) const noexcept
{
    auto it = signals_.find(gid);
    return it != signals_.end() ? &it->second : nullptr;
}

const Derailer* EngineState::find_derailer(const GID& gid) const noexcept
{
    auto it = derailers_.find(gid);
    return it != derailers_.end() ? &it->second : nullptr;
}

const BlockSection* EngineState::find_block_section(const GID& gid) const noexcept
{
    auto it = block_sections_.find(gid);
    return it != block_sections_.end() ? &it->second : nullptr;
}

const RouteState* EngineState::find_route(const GID& route_id) const noexcept
{
    auto it = routes_.find(route_id);
    return it != routes_.end() ? &it->second : nullptr;
}

const AlarmState* EngineState::find_alarm(const GID& alarm_id) const noexcept
{
    auto it = alarms_.find(alarm_id);
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

// ── Topology insertion ────────────────────────────────────────────────────────

void EngineState::insert_boundary_node(BoundaryNode n)
{
    boundary_nodes_.emplace(n.gid, std::move(n));
}
void EngineState::insert_track_section(TrackSection s)
{
    track_sections_.emplace(s.gid, std::move(s));
}
void EngineState::insert_switch(Switch sw)
{
    switches_.emplace(sw.gid, std::move(sw));
}
void EngineState::insert_signal(Signal sig)
{
    signals_.emplace(sig.gid, std::move(sig));
}
void EngineState::insert_derailer(Derailer d)
{
    derailers_.emplace(d.gid, std::move(d));
}
void EngineState::insert_block_section(BlockSection b)
{
    block_sections_.emplace(b.gid, std::move(b));
}

// ── Runtime state mutators ────────────────────────────────────────────────────

void EngineState::apply_track_section_occupancy(const GID& gid, TrackOccupancy occ, int axle_count)
{
    if (auto it = track_sections_.find(gid); it != track_sections_.end())
    {
        it->second.occupancy = occ;
        it->second.axle_count = axle_count;
    }
}

void EngineState::apply_switch_position(const GID& gid, SwitchPosition pos, int moving_ticks)
{
    if (auto it = switches_.find(gid); it != switches_.end())
    {
        it->second.position = pos;
        it->second.moving_ticks_remaining = moving_ticks;
    }
}

void EngineState::apply_switch_lock(const GID& gid, std::optional<GID> route_id)
{
    if (auto it = switches_.find(gid); it != switches_.end())
        it->second.locked_by_route = std::move(route_id);
}

void EngineState::apply_switch_occupancy(const GID& gid, TrackOccupancy occ, int axle_count)
{
    if (auto it = switches_.find(gid); it != switches_.end())
    {
        it->second.occupancy = occ;
        it->second.axle_count = axle_count;
    }
}

void EngineState::apply_signal_aspect(const GID& gid, SignalAspect aspect)
{
    if (auto it = signals_.find(gid); it != signals_.end())
        it->second.current_aspect = aspect;
}

void EngineState::apply_signal_lock(const GID& gid, std::optional<GID> route_id)
{
    if (auto it = signals_.find(gid); it != signals_.end())
        it->second.locked_by_route = std::move(route_id);
}

void EngineState::apply_derailer_state(const GID& gid, DerailerState state)
{
    if (auto it = derailers_.find(gid); it != derailers_.end())
        it->second.state = state;
}

void EngineState::apply_derailer_lock(const GID& gid, std::optional<GID> route_id)
{
    if (auto it = derailers_.find(gid); it != derailers_.end())
        it->second.locked_by_route = std::move(route_id);
}

void EngineState::apply_block_section_state(const GID& gid, BlockSectionState state)
{
    if (auto it = block_sections_.find(gid); it != block_sections_.end())
        it->second.state = state;
}

void EngineState::apply_block_section_direction(const GID& gid, BlockDirectionState dir)
{
    if (auto it = block_sections_.find(gid); it != block_sections_.end())
        it->second.direction = dir;
}

void EngineState::apply_block_section_axle_count(const GID& gid, int axle_count)
{
    if (auto it = block_sections_.find(gid); it != block_sections_.end())
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

void EngineState::apply_operator_command_state(const GID& gid, OperatorTargetKind target_kind,
                                               OperatorCommandCode code, bool active)
{
    auto apply_to_gid = [&](auto& map)
    {
        if (auto it = map.find(gid); it != map.end())
            apply_operator_state(it->second.operator_state, code, active);
    };

    switch (target_kind)
    {
        case OperatorTargetKind::SIGNAL:
            apply_to_gid(signals_);
            break;
        case OperatorTargetKind::SWITCH:
            apply_to_gid(switches_);
            if (switches_.find(gid) == switches_.end())
                apply_to_gid(derailers_);
            break;
        case OperatorTargetKind::DERAILER:
            apply_to_gid(derailers_);
            break;
        case OperatorTargetKind::TRACK_SECTION:
            apply_to_gid(track_sections_);
            break;
        case OperatorTargetKind::BLOCK_SECTION:
            apply_to_gid(block_sections_);
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

void EngineState::apply_ml8_command_state(const GID& gid, OperatorTargetKind target_kind,
                                          Ml8CommandCode code, bool active)
{
    auto apply_to_gid = [&](auto& map)
    {
        if (auto it = map.find(gid); it != map.end())
        {
            it->second.operator_state.active_ml8_command =
                active ? std::make_optional(code) : std::nullopt;
        }
    };

    switch (target_kind)
    {
        case OperatorTargetKind::SIGNAL:
            apply_to_gid(signals_);
            break;
        case OperatorTargetKind::SWITCH:
            apply_to_gid(switches_);
            if (switches_.find(gid) == switches_.end())
                apply_to_gid(derailers_);
            break;
        case OperatorTargetKind::DERAILER:
            apply_to_gid(derailers_);
            break;
        case OperatorTargetKind::TRACK_SECTION:
            apply_to_gid(track_sections_);
            break;
        case OperatorTargetKind::BLOCK_SECTION:
            apply_to_gid(block_sections_);
            break;
        default:
            break;
    }
}

void EngineState::apply_axle_counter_reset(const GID& gid, OperatorTargetKind target_kind)
{
    switch (target_kind)
    {
        case OperatorTargetKind::SWITCH:
            apply_switch_occupancy(gid, TrackOccupancy::FREE, 0);
            break;
        case OperatorTargetKind::TRACK_SECTION:
            apply_track_section_occupancy(gid, TrackOccupancy::FREE, 0);
            break;
        case OperatorTargetKind::BLOCK_SECTION:
            apply_block_section_axle_count(gid, 0);
            break;
        default:
            break;
    }
}

void EngineState::add_route(RouteState route)
{
    routes_.emplace(route.route_id, std::move(route));
}

void EngineState::remove_route(const GID& route_id)
{
    routes_.erase(route_id);
}

void EngineState::add_alarm(AlarmState alarm)
{
    alarms_.emplace(alarm.alarm_id, std::move(alarm));
}

void EngineState::remove_alarm(const GID& alarm_id)
{
    alarms_.erase(alarm_id);
}

}  // namespace engine::core
