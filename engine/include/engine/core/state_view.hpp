#pragma once

#include "track_model.hpp"

#include <functional>

// ── Read-only view of the engine's current world state ───────────────────────
// IStateView is the interface through which IControlSystem (interlocking logic)
// reads topology and runtime device state without holding any locks.
//
// The engine exposes its EngineState as an IStateView to IControlSystem on the
// ENGINE thread.  IControlSystem must never retain a reference to an IStateView
// beyond the scope of a single check_command / execute_command / on_tick call.
//
// All find_* methods return a raw pointer into the engine's internal storage.
// The pointer is valid only for the duration of the call.  nullptr means the
// object does not exist in this scenario.

namespace engine::core
{

class IStateView
{
public:
    virtual ~IStateView() = default;

    // ── Topology lookup by GID ────────────────────────────────────────────────
    virtual const BoundaryNode* find_boundary_node(const GID& gid) const noexcept = 0;
    virtual const TrackSection* find_track_section(const GID& gid) const noexcept = 0;
    virtual const Switch* find_switch(const GID& gid) const noexcept = 0;
    virtual const Signal* find_signal(const GID& gid) const noexcept = 0;
    virtual const Derailer* find_derailer(const GID& gid) const noexcept = 0;
    virtual const BlockSection* find_block_section(const GID& gid) const noexcept = 0;
    virtual const RouteState* find_route(const GID& route_id) const noexcept = 0;
    virtual const AlarmState* find_alarm(const GID& alarm_id) const noexcept = 0;

    // ── Iteration (needed for BFS and full-snapshot copy) ────────────────────
    virtual void for_each_track_section(std::function<void(const TrackSection&)> fn) const = 0;
    virtual void for_each_switch(std::function<void(const Switch&)> fn) const = 0;
    virtual void for_each_signal(std::function<void(const Signal&)> fn) const = 0;
    virtual void for_each_derailer(std::function<void(const Derailer&)> fn) const = 0;
    virtual void for_each_block_section(std::function<void(const BlockSection&)> fn) const = 0;
    virtual void for_each_route(std::function<void(const RouteState&)> fn) const = 0;
    virtual void for_each_alarm(std::function<void(const AlarmState&)> fn) const = 0;
    virtual void for_each_boundary_node(std::function<void(const BoundaryNode&)> fn) const = 0;

    // ── Session metadata ──────────────────────────────────────────────────────
    virtual const std::string& session_id() const noexcept = 0;
    virtual uint64_t current_tick() const noexcept = 0;
};

}  // namespace engine::core
