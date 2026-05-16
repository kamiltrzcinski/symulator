#pragma once

#include "state_view.hpp"

#include <atomic>
#include <memory>
#include <string>
#include <unordered_map>

// ── Immutable snapshot of all engine state at a given tick ───────────────────
// EngineSnapshot is a deep-copy value type.  The ENGINE thread constructs a new
// snapshot after every tick and publishes it through AtomicSnapshot.
//
// Any other thread (PIP_WRITER, DB_WRITER, EDR, server broadcast, …) can call
// AtomicSnapshot::load() at any time and get a consistent, fully-formed copy
// of the world state without blocking the ENGINE thread.
//
// EngineSnapshot also implements IStateView so tests can use it directly.

namespace engine::core
{

struct EngineSnapshot final : public IStateView
{
    std::string session;  // the session UUID
    uint64_t tick = 0;

    std::unordered_map<GID, BoundaryNode, std::hash<GID>> boundary_nodes;
    std::unordered_map<GID, TrackSection, std::hash<GID>> track_sections;
    std::unordered_map<GID, Switch, std::hash<GID>> switches;
    std::unordered_map<GID, Signal, std::hash<GID>> signals;
    std::unordered_map<GID, Derailer, std::hash<GID>> derailers;
    std::unordered_map<GID, BlockSection, std::hash<GID>> block_sections;
    std::unordered_map<GID, RouteState, std::hash<GID>> routes;
    std::unordered_map<GID, AlarmState, std::hash<GID>> alarms;

    // ── IStateView ────────────────────────────────────────────────────────────
    const BoundaryNode* find_boundary_node(const GID& gid) const noexcept override;
    const TrackSection* find_track_section(const GID& gid) const noexcept override;
    const Switch* find_switch(const GID& gid) const noexcept override;
    const Signal* find_signal(const GID& gid) const noexcept override;
    const Derailer* find_derailer(const GID& gid) const noexcept override;
    const BlockSection* find_block_section(const GID& gid) const noexcept override;
    const RouteState* find_route(const GID& route_id) const noexcept override;
    const AlarmState* find_alarm(const GID& alarm_id) const noexcept override;

    void for_each_track_section(std::function<void(const TrackSection&)> fn) const override;
    void for_each_switch(std::function<void(const Switch&)> fn) const override;
    void for_each_signal(std::function<void(const Signal&)> fn) const override;
    void for_each_derailer(std::function<void(const Derailer&)> fn) const override;
    void for_each_block_section(std::function<void(const BlockSection&)> fn) const override;
    void for_each_route(std::function<void(const RouteState&)> fn) const override;
    void for_each_alarm(std::function<void(const AlarmState&)> fn) const override;
    void for_each_boundary_node(std::function<void(const BoundaryNode&)> fn) const override;

    const std::string& session_id() const noexcept override { return session; }
    uint64_t current_tick() const noexcept override { return tick; }
};

// ── Thread-safe snapshot publisher / reader ───────────────────────────────────
// Published by ENGINE after every tick.  All non-ENGINE threads read via load().
// Uses acquire/release memory ordering — no mutex needed.
class AtomicSnapshot
{
public:
    AtomicSnapshot() = default;

    // Called by ENGINE thread only.
    void publish(std::shared_ptr<const EngineSnapshot> snap)
    {
        current_.store(std::move(snap), std::memory_order_release);
    }

    // Called by any thread.  Returns nullptr before the first publish().
    std::shared_ptr<const EngineSnapshot> load() const
    {
        return current_.load(std::memory_order_acquire);
    }

private:
    std::atomic<std::shared_ptr<const EngineSnapshot>> current_{nullptr};
};

}  // namespace engine::core
