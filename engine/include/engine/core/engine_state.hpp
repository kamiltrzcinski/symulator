#pragma once

#include "command.hpp"
#include "state_view.hpp"

#include <memory>
#include <string>
#include <unordered_map>

// ── Mutable engine state (owned by ENGINE thread) ────────────────────────────
// EngineState holds the live device and topology state for one simulator
// session.  It is the only writable owner of all topology maps and is mutated
// exclusively on the ENGINE thread (no locks needed for writes).
//
// EngineState also implements IStateView so it can be passed directly to
// IControlSystem without copying.  All IStateView methods return raw pointers
// into the internal maps — valid only for the duration of the ENGINE tick.
//
// See docs/03-initial-architecture.md for the threading model.

namespace engine::core
{

class EngineState final : public IStateView
{
public:
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

    const std::string& session_id() const noexcept override { return session_id_; }
    uint64_t current_tick() const noexcept override { return current_tick_; }

    // ── Mutators (ENGINE thread only) ─────────────────────────────────────────
    // Topology is loaded once; runtime state is updated via apply_* during ticks.

    void set_session_id(std::string id) { session_id_ = std::move(id); }
    void set_current_tick(uint64_t tick) { current_tick_ = tick; }

    // Insert topology nodes (called by ScenarioLoader, not during ticks)
    void insert_boundary_node(BoundaryNode n);
    void insert_track_section(TrackSection s);
    void insert_switch(Switch sw);
    void insert_signal(Signal sig);
    void insert_derailer(Derailer d);
    void insert_block_section(BlockSection b);

    // Apply runtime state changes (called by StateApplier during ENGINE tick)
    void apply_track_section_occupancy(const GID& gid, TrackOccupancy occ, int axle_count);
    void apply_switch_position(const GID& gid, SwitchPosition pos, int moving_ticks);
    void apply_switch_lock(const GID& gid, std::optional<GID> route_id);
    void apply_switch_occupancy(const GID& gid, TrackOccupancy occ, int axle_count);
    void apply_signal_aspect(const GID& gid, SignalAspect aspect);
    void apply_signal_lock(const GID& gid, std::optional<GID> route_id);
    void apply_derailer_state(const GID& gid, DerailerState state);
    void apply_derailer_lock(const GID& gid, std::optional<GID> route_id);
    void apply_block_section_state(const GID& gid, BlockSectionState state);
    void apply_block_section_direction(const GID& gid, BlockDirectionState dir);
    void apply_block_section_axle_count(const GID& gid, int axle_count);
    void apply_operator_command_state(const GID& gid, OperatorTargetKind target_kind,
                                      OperatorCommandCode code, bool active);
    void apply_axle_counter_reset(const GID& gid, OperatorTargetKind target_kind);
    void add_route(RouteState route);
    void remove_route(const GID& route_id);
    void add_alarm(AlarmState alarm);
    void remove_alarm(const GID& alarm_id);

    // Direct mutable access for scenario loader and test harnesses
    std::unordered_map<GID, BoundaryNode, std::hash<GID>>& boundary_nodes()
    {
        return boundary_nodes_;
    }
    std::unordered_map<GID, TrackSection, std::hash<GID>>& track_sections()
    {
        return track_sections_;
    }
    std::unordered_map<GID, Switch, std::hash<GID>>& switches() { return switches_; }
    std::unordered_map<GID, Signal, std::hash<GID>>& signals() { return signals_; }
    std::unordered_map<GID, Derailer, std::hash<GID>>& derailers() { return derailers_; }
    std::unordered_map<GID, BlockSection, std::hash<GID>>& block_sections()
    {
        return block_sections_;
    }
    std::unordered_map<GID, RouteState, std::hash<GID>>& routes() { return routes_; }
    std::unordered_map<GID, AlarmState, std::hash<GID>>& alarms() { return alarms_; }

private:
    std::string session_id_;
    uint64_t current_tick_ = 0;

    std::unordered_map<GID, BoundaryNode, std::hash<GID>> boundary_nodes_;
    std::unordered_map<GID, TrackSection, std::hash<GID>> track_sections_;
    std::unordered_map<GID, Switch, std::hash<GID>> switches_;
    std::unordered_map<GID, Signal, std::hash<GID>> signals_;
    std::unordered_map<GID, Derailer, std::hash<GID>> derailers_;
    std::unordered_map<GID, BlockSection, std::hash<GID>> block_sections_;
    std::unordered_map<GID, RouteState, std::hash<GID>> routes_;
    std::unordered_map<GID, AlarmState, std::hash<GID>> alarms_;
};

}  // namespace engine::core
