#include "engine/core/engine_snapshot.hpp"

namespace engine::core
{

// ── IStateView: find_* ────────────────────────────────────────────────────────

const BoundaryNode* EngineSnapshot::find_boundary_node(const GID& gid) const noexcept
{
    auto it = boundary_nodes.find(gid);
    return it != boundary_nodes.end() ? &it->second : nullptr;
}

const TrackSection* EngineSnapshot::find_track_section(const GID& gid) const noexcept
{
    auto it = track_sections.find(gid);
    return it != track_sections.end() ? &it->second : nullptr;
}

const Switch* EngineSnapshot::find_switch(const GID& gid) const noexcept
{
    auto it = switches.find(gid);
    return it != switches.end() ? &it->second : nullptr;
}

const Signal* EngineSnapshot::find_signal(const GID& gid) const noexcept
{
    auto it = signals.find(gid);
    return it != signals.end() ? &it->second : nullptr;
}

const Derailer* EngineSnapshot::find_derailer(const GID& gid) const noexcept
{
    auto it = derailers.find(gid);
    return it != derailers.end() ? &it->second : nullptr;
}

const BlockSection* EngineSnapshot::find_block_section(const GID& gid) const noexcept
{
    auto it = block_sections.find(gid);
    return it != block_sections.end() ? &it->second : nullptr;
}

const RouteState* EngineSnapshot::find_route(const GID& route_id) const noexcept
{
    auto it = routes.find(route_id);
    return it != routes.end() ? &it->second : nullptr;
}

const AlarmState* EngineSnapshot::find_alarm(const GID& alarm_id) const noexcept
{
    auto it = alarms.find(alarm_id);
    return it != alarms.end() ? &it->second : nullptr;
}

// ── IStateView: for_each_* ────────────────────────────────────────────────────

void EngineSnapshot::for_each_track_section(std::function<void(const TrackSection&)> fn) const
{
    for (auto& [_, v] : track_sections)
        fn(v);
}

void EngineSnapshot::for_each_switch(std::function<void(const Switch&)> fn) const
{
    for (auto& [_, v] : switches)
        fn(v);
}

void EngineSnapshot::for_each_signal(std::function<void(const Signal&)> fn) const
{
    for (auto& [_, v] : signals)
        fn(v);
}

void EngineSnapshot::for_each_derailer(std::function<void(const Derailer&)> fn) const
{
    for (auto& [_, v] : derailers)
        fn(v);
}

void EngineSnapshot::for_each_block_section(std::function<void(const BlockSection&)> fn) const
{
    for (auto& [_, v] : block_sections)
        fn(v);
}

void EngineSnapshot::for_each_route(std::function<void(const RouteState&)> fn) const
{
    for (auto& [_, v] : routes)
        fn(v);
}

void EngineSnapshot::for_each_alarm(std::function<void(const AlarmState&)> fn) const
{
    for (auto& [_, v] : alarms)
        fn(v);
}

void EngineSnapshot::for_each_boundary_node(std::function<void(const BoundaryNode&)> fn) const
{
    for (auto& [_, v] : boundary_nodes)
        fn(v);
}

}  // namespace engine::core
