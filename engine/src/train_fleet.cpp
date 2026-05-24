// engine/src/train_fleet.cpp

#include "engine/core/train_fleet.hpp"

namespace engine::core
{

// ── Topology helpers ──────────────────────────────────────────────────────────

namespace
{

/// Return the "ahead" port of `section` given that the train came from `from_gid`.
/// If side_a.neighbor_gid matches from_gid the train is travelling A→B,
/// so the ahead port is side_b, and vice versa.
const TrackPort& ahead_port(const TrackSection& section, const GID& from_gid)
{
    return (section.side_a.neighbor_gid == from_gid) ? section.side_b : section.side_a;
}

}  // namespace

// ── Public API ────────────────────────────────────────────────────────────────

void TrainFleet::add_train(sim::TrainSimState initial, GID from_gid)
{
    entries_.push_back(TrainEntry{sim::TrainSim{std::move(initial)}, std::move(from_gid)});
}

// ── Static topology helpers ───────────────────────────────────────────────────

NextSectionInfo TrainFleet::resolve_next_section(const IStateView& state, const GID& current_gid,
                                                 const GID& from_gid)
{
    const TrackSection* current = state.find_track_section(current_gid);
    if (!current)
        return {};

    const GID& next_gid = ahead_port(*current, from_gid).neighbor_gid;
    if (next_gid.value.empty())
        return {};

    // ── Direct track section ────────────────────────────────────────────────
    if (state.find_track_section(next_gid))
        return {next_gid, current_gid, false};

    // ── Switch traversal ────────────────────────────────────────────────────
    if (const Switch* sw = state.find_switch(next_gid))
    {
        // MOVING: switch is not yet in a stable position — train must wait.
        if (sw->position == SwitchPosition::MOVING)
            return {};

        // Determine the exit leg based on which leg connects back to current_gid.
        GID exit_gid{};
        if (sw->trunk.neighbor_gid == current_gid)
        {
            // Entering from the trunk (pień) — exit through the selected leg.
            exit_gid = (sw->position == SwitchPosition::STRAIGHT) ? sw->straight.neighbor_gid
                                                                  : sw->divergent.neighbor_gid;
        }
        else
        {
            // Entering from either the straight or divergent leg — always exit via trunk.
            exit_gid = sw->trunk.neighbor_gid;
        }

        if (exit_gid.value.empty())
            return {};

        // from_gid for the next section is the switch GID, because the next
        // TrackSection has side_X.neighbor_gid == switch, not the previous section.
        return {exit_gid, next_gid, false};
    }

    // ── Boundary node — train is leaving the LCS area ───────────────────────
    if (state.find_boundary_node(next_gid))
        return {std::nullopt, {}, true};

    // Unknown neighbour type — treat as dead-end.
    return {};
}

SignalAspect TrainFleet::ahead_signal_aspect(const IStateView& state, const TrackSection& section,
                                             const GID& from_gid)
{
    const auto& port = ahead_port(section, from_gid);
    for (const GID& sig_gid : port.signal_gids)
    {
        if (const Signal* sig = state.find_signal(sig_gid))
            return sig->current_aspect;
    }
    return SignalAspect::S2_PROCEED;
}

// ── tick_all ──────────────────────────────────────────────────────────────────

void TrainFleet::tick_all(EngineState& state, uint64_t tick_num, const PipCallback& pip_cb)
{
    (void)tick_num;  // available for logging/debug if needed

    std::vector<PipEvent> pip_events;

    for (auto& entry : entries_)
    {
        const GID& current_gid = entry.sim.state().current_section_gid;
        const TrackSection* section = state.find_track_section(current_gid);

        if (!section)
            continue;  // section not (yet) in topology — skip

        // Build DriverInput.
        const float speed_limit_ms = static_cast<float>(section->max_speed_kmh) / 3.6f;
        const float position_m = entry.sim.state().physics_state.position_m;
        const float distance_to_signal_m = std::max(0.0f, section->length_m - position_m);

        const NextSectionInfo info = resolve_next_section(state, current_gid, entry.from_gid);

        sim::TrainSimTickInput input;
        input.driver_input.aspect = ahead_signal_aspect(state, *section, entry.from_gid);
        input.driver_input.distance_to_signal_m = distance_to_signal_m;
        input.driver_input.target_speed_ms =
            std::min(speed_limit_ms, entry.sim.state().physics_params.max_speed_ms);
        input.driver_input.max_brake_kn = entry.sim.state().max_brake_kn;
        input.section_length_m = section->length_m;
        input.next_section_gid = info.section_gid;

        // Advance physics.
        const auto output = entry.sim.tick(TICK_DT_S, input);

        // ── Boundary crossing detection ────────────────────────────────────
        // When the ahead neighbour is a BoundaryNode the dead-end path in
        // TrainSim pins the train at position_m == section_length_m.  Detect
        // this once, free the section, emit the boundary PipEvent, and mark
        // the train for removal at the end of this tick.
        if (info.is_boundary_crossing && !entry.pending_boundary_removal &&
            entry.sim.state().physics_state.position_m >= section->length_m - 0.001f)
        {
            state.apply_track_section_occupancy(current_gid, TrackOccupancy::FREE, 0);
            pip_events.push_back(PipEvent{
                .section_gid = current_gid,
                .station_sid = section->sid,
                .occupancy = TrackOccupancy::FREE,
                .slot = std::nullopt,
                .lcs_boundary_crossing = true,
            });
            entry.pending_boundary_removal = true;
        }

        // ── Section crossing ───────────────────────────────────────────────
        if (output.crossing.has_value())
        {
            const auto& crossing = *output.crossing;

            // Update occupancy in EngineState.
            state.apply_track_section_occupancy(crossing.from_section_gid, TrackOccupancy::FREE, 0);
            state.apply_track_section_occupancy(
                crossing.to_section_gid, TrackOccupancy::OCCUPIED,
                entry.sim.state().physics_params.total_mass_t > 0 ? 4 : 0);

            // Emit PipEvents: old section free, new section occupied.
            const TrackSection* from_sec = state.find_track_section(crossing.from_section_gid);
            const TrackSection* to_sec = state.find_track_section(crossing.to_section_gid);

            if (from_sec)
            {
                pip_events.push_back(PipEvent{
                    .section_gid = crossing.from_section_gid,
                    .station_sid = from_sec->sid,
                    .occupancy = TrackOccupancy::FREE,
                    .slot = std::nullopt,
                    .lcs_boundary_crossing = false,
                });
            }

            if (to_sec)
            {
                const GID& train_gid = entry.sim.state().train_gid;
                TrainSlot slot;
                slot.number = train_gid.value.substr(0, 6);
                slot.entry_side = EntrySide::LEFT;

                pip_events.push_back(PipEvent{
                    .section_gid = crossing.to_section_gid,
                    .station_sid = to_sec->sid,
                    .occupancy = TrackOccupancy::OCCUPIED,
                    .slot = slot,
                    .lcs_boundary_crossing = false,
                });
            }

            // Update the train's from_gid for next tick.
            // For switch traversal info.from_gid is the switch GID, which is
            // what the new section's TrackPort::neighbor_gid points back to.
            entry.from_gid = info.from_gid;
        }
    }

    // Remove trains that have exited through a BoundaryNode this tick.
    entries_.erase(std::remove_if(entries_.begin(), entries_.end(),
                                  [](const TrainEntry& e) { return e.pending_boundary_removal; }),
                   entries_.end());

    if (pip_cb && !pip_events.empty())
        pip_cb(pip_events);
}

}  // namespace engine::core
