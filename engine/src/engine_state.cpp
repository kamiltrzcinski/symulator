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
