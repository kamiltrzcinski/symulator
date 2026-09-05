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
// See docs/ARCHITECTURE.md for the threading model.

namespace engine::core
{

class EngineState final : public IStateView
{
public:
    // ── IStateView ────────────────────────────────────────────────────────────
    const BoundaryNode* find_boundary_node(UID uid) const noexcept override;
    const TrackSection* find_track_section(UID uid) const noexcept override;
    const Switch* find_switch(UID uid) const noexcept override;
    const Signal* find_signal(UID uid) const noexcept override;
    const Derailer* find_derailer(UID uid) const noexcept override;
    const BlockSection* find_block_section(UID uid) const noexcept override;
    const RouteState* find_route(UID route_uid) const noexcept override;
    const AlarmState* find_alarm(UID alarm_uid) const noexcept override;

    void for_each_track_section(std::function<void(const TrackSection&)> fn) const override;
    void for_each_switch(std::function<void(const Switch&)> fn) const override;
    void for_each_signal(std::function<void(const Signal&)> fn) const override;
    void for_each_derailer(std::function<void(const Derailer&)> fn) const override;
    void for_each_block_section(std::function<void(const BlockSection&)> fn) const override;
    void for_each_route(std::function<void(const RouteState&)> fn) const override;
    void for_each_alarm(std::function<void(const AlarmState&)> fn) const override;
    void for_each_boundary_node(std::function<void(const BoundaryNode&)> fn) const override;
    void for_each_level_crossing(std::function<void(const LevelCrossing&)> fn) const override;

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
    void insert_level_crossing(LevelCrossing lx);

    // Apply runtime state changes (called by StateApplier during ENGINE tick)
    void apply_track_section_occupancy(UID uid, TrackOccupancy occ, int axle_count);
    void apply_switch_position(UID uid, SwitchPosition pos, int moving_ticks);
    void apply_switch_control(UID uid, bool control_lost);
    void apply_switch_lock(UID uid, std::optional<UID> route_uid);
    void apply_switch_occupancy(UID uid, TrackOccupancy occ, int axle_count);
    void apply_level_crossing_status(UID uid, LevelCrossingStatus status);
    void apply_signal_aspect(UID uid, SignalAspect aspect);
    void apply_signal_lock(UID uid, std::optional<UID> route_uid);
    void apply_derailer_state(UID uid, DerailerState state);
    void apply_derailer_lock(UID uid, std::optional<UID> route_uid);
    void apply_block_section_state(UID uid, BlockSectionState state);
    void apply_block_section_direction(UID uid, BlockDirectionState dir);
    void apply_block_section_axle_count(UID uid, int axle_count);
    void apply_operator_command_state(UID uid, OperatorTargetKind target_kind,
                                      OperatorCommandCode code, bool active);
    void apply_ml8_command_state(UID uid, OperatorTargetKind target_kind, Ml8CommandCode code,
                                 bool active);
    void apply_axle_counter_reset(UID uid, OperatorTargetKind target_kind);
    void add_route(RouteState route);
    void remove_route(UID route_uid);
    void add_alarm(AlarmState alarm);
    void remove_alarm(UID alarm_uid);

    // Direct mutable access for scenario loader and test harnesses
    std::unordered_map<UID, BoundaryNode, std::hash<UID>>& boundary_nodes()
    {
        return boundary_nodes_;
    }
    std::unordered_map<UID, TrackSection, std::hash<UID>>& track_sections()
    {
        return track_sections_;
    }
    std::unordered_map<UID, Switch, std::hash<UID>>& switches() { return switches_; }
    std::unordered_map<UID, Signal, std::hash<UID>>& signals() { return signals_; }
    std::unordered_map<UID, Derailer, std::hash<UID>>& derailers() { return derailers_; }
    std::unordered_map<UID, BlockSection, std::hash<UID>>& block_sections()
    {
        return block_sections_;
    }
    std::unordered_map<UID, RouteState, std::hash<UID>>& routes() { return routes_; }
    std::unordered_map<UID, AlarmState, std::hash<UID>>& alarms() { return alarms_; }

private:
    std::string session_id_;
    uint64_t current_tick_ = 0;

    std::unordered_map<UID, BoundaryNode, std::hash<UID>> boundary_nodes_;
    std::unordered_map<UID, TrackSection, std::hash<UID>> track_sections_;
    std::unordered_map<UID, Switch, std::hash<UID>> switches_;
    std::unordered_map<UID, Signal, std::hash<UID>> signals_;
    std::unordered_map<UID, Derailer, std::hash<UID>> derailers_;
    std::unordered_map<UID, BlockSection, std::hash<UID>> block_sections_;
    std::unordered_map<UID, RouteState, std::hash<UID>> routes_;
    std::unordered_map<UID, AlarmState, std::hash<UID>> alarms_;
    std::unordered_map<UID, LevelCrossing, std::hash<UID>> level_crossings_;
};

}  // namespace engine::core
