// engine/src/train_fleet.cpp

#include "engine/core/train_fleet.hpp"

namespace engine::core
{

// ── Topology helpers ──────────────────────────────────────────────────────────

namespace
{

/// Return the "ahead" port of `section` given that the train came from `from_uid`.
/// If side_a.neighbor_uid matches from_uid the train is travelling A→B,
/// so the ahead port is side_b, and vice versa.
const TrackPort& ahead_port(const TrackSection& section, UID from_uid)
{
    return (section.side_a.neighbor_uid == from_uid) ? section.side_b : section.side_a;
}

}  // namespace

// ── Public API ────────────────────────────────────────────────────────────────

void TrainFleet::add_train(sim::TrainSimState initial, UID from_uid)
{
    entries_.push_back(TrainEntry{sim::TrainSim{std::move(initial)}, from_uid});
}

// ── Static topology helpers ───────────────────────────────────────────────────

NextSectionInfo TrainFleet::resolve_next_section(const IStateView& state, UID current_uid,
                                                 UID from_uid)
{
    const TrackSection* current = state.find_track_section(current_uid);
    if (!current)
        return {};

    const UID& next_uid = ahead_port(*current, from_uid).neighbor_uid;
    if (next_uid.value == 0)
        return {};

    // ── Direct track section ────────────────────────────────────────────────
    if (state.find_track_section(next_uid))
        return {next_uid, current_uid, false};

    // ── Switch traversal ────────────────────────────────────────────────────
    if (const Switch* sw = state.find_switch(next_uid))
    {
        // MOVING: switch is not yet in a stable position — train must wait.
        if (sw->position == SwitchPosition::MOVING)
            return {};

        // Determine the exit leg based on which leg connects back to current_uid.
        UID exit_uid{};
        if (sw->trunk.neighbor_uid == current_uid)
        {
            // Entering from the trunk (pień) — exit through the selected leg.
            exit_uid = (sw->position == SwitchPosition::STRAIGHT) ? sw->straight.neighbor_uid
                                                                  : sw->divergent.neighbor_uid;
        }
        else
        {
            // Entering from either the straight or divergent leg — always exit via trunk.
            exit_uid = sw->trunk.neighbor_uid;
        }

        if (exit_uid.value == 0)
            return {};

        // from_uid for the next section is the switch UID, because the next
        // TrackSection has side_X.neighbor_uid == switch, not the previous section.
        return {exit_uid, next_uid, false};
    }

    // ── Boundary node — train is leaving the LCS area ───────────────────────
    if (state.find_boundary_node(next_uid))
        return {std::nullopt, {}, true};

    // Unknown neighbour type — treat as dead-end.
    return {};
}

SignalAspect TrainFleet::ahead_signal_aspect(const IStateView& state, const TrackSection& section,
                                             UID from_uid)
{
    const auto& port = ahead_port(section, from_uid);
    for (const UID& sig_uid : port.signal_uids)
    {
        if (const Signal* sig = state.find_signal(sig_uid))
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
        const UID& current_uid = entry.sim.state().current_section_uid;
        const TrackSection* section = state.find_track_section(current_uid);

        if (!section)
            continue;  // section not (yet) in topology — skip

        // Build DriverInput.
        const float speed_limit_ms = static_cast<float>(section->max_speed_kmh) / 3.6f;
        const float position_m = entry.sim.state().physics_state.position_m;
        const float distance_to_signal_m = std::max(0.0f, section->length_m - position_m);

        const NextSectionInfo info = resolve_next_section(state, current_uid, entry.from_uid);

        sim::TrainSimTickInput input;
        input.driver_input.aspect = ahead_signal_aspect(state, *section, entry.from_uid);
        input.driver_input.distance_to_signal_m = distance_to_signal_m;
        input.driver_input.target_speed_ms =
            std::min(speed_limit_ms, entry.sim.state().physics_params.max_speed_ms);
        input.driver_input.max_brake_kn = entry.sim.state().max_brake_kn;
        input.section_length_m = section->length_m;
        input.next_section_uid = info.section_uid;

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
            state.apply_track_section_occupancy(current_uid, TrackOccupancy::FREE, 0);
            pip_events.push_back(PipEvent{
                .section_uid = current_uid,
                .station_uid = section->station_uid,
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
            state.apply_track_section_occupancy(crossing.from_section_uid, TrackOccupancy::FREE, 0);
            state.apply_track_section_occupancy(
                crossing.to_section_uid, TrackOccupancy::OCCUPIED,
                entry.sim.state().physics_params.total_mass_t > 0 ? 4 : 0);

            // Emit PipEvents: old section free, new section occupied.
            const TrackSection* from_sec = state.find_track_section(crossing.from_section_uid);
            const TrackSection* to_sec = state.find_track_section(crossing.to_section_uid);

            if (from_sec)
            {
                pip_events.push_back(PipEvent{
                    .section_uid = crossing.from_section_uid,
                    .station_uid = from_sec->station_uid,
                    .occupancy = TrackOccupancy::FREE,
                    .slot = std::nullopt,
                    .lcs_boundary_crossing = false,
                });
            }

            if (to_sec)
            {
                const UID& train_uid = entry.sim.state().train_uid;
                TrainSlot slot;
                // Use the lower 16 bits of the uid value as a short display number
                slot.number = std::to_string(uid_instance(train_uid));
                slot.entry_side = EntrySide::LEFT;

                pip_events.push_back(PipEvent{
                    .section_uid = crossing.to_section_uid,
                    .station_uid = to_sec->station_uid,
                    .occupancy = TrackOccupancy::OCCUPIED,
                    .slot = slot,
                    .lcs_boundary_crossing = false,
                });
            }

            // Update the train's from_uid for next tick.
            // For switch traversal info.from_uid is the switch UID, which is
            // what the new section's TrackPort::neighbor_uid points back to.
            entry.from_uid = info.from_uid;
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
