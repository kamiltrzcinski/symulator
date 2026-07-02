// engine/include/engine/core/train_fleet.hpp
// Manages all active TrainSim instances for one session.
//
// TrainFleet is owned by EngineLoop and ticked exclusively on the ENGINE
// thread, immediately after IControlSystem::on_tick() and before the
// AtomicSnapshot is published.
//
// Per tick it:
//   1. Resolves DriverInput from IStateView (current signal, section speed).
//   2. Advances each TrainSim by TICK_DT_S seconds.
//   3. Applies occupancy mutations to EngineState on section crossings.
//   4. Collects PipEvents and fires the PipCallback once per tick.

#pragma once

#include "engine/core/engine_state.hpp"
#include "engine/core/types.hpp"
#include "engine/sim/train_sim.hpp"

#include <functional>
#include <optional>
#include <vector>

namespace engine::core
{

// ── TrainEntry ────────────────────────────────────────────────────────────────
// Internal per-train record.
struct TrainEntry
{
    sim::TrainSim sim;
    UID from_uid;                           ///< UID of the section/node the train last came from.
                                            ///< Used to determine which port is "ahead" on the
                                            ///< current section.  After a switch crossing this is
                                            ///< the switch UID, not the previous track section.
    bool pending_boundary_removal = false;  ///< Set when the train has reached a BoundaryNode
                                            ///< and will be erased at the end of the current tick.
};

// ── NextSectionInfo ───────────────────────────────────────────────────────────
// Result of resolve_next_section — describes what lies ahead and what the new
// from_uid should be if a crossing occurs.
struct NextSectionInfo
{
    std::optional<UID> section_uid;     ///< UID of the next TrackSection, or nullopt when
                                        ///< the train is blocked (MOVING switch) or exiting
                                        ///< via a BoundaryNode / unknown neighbor (world edge).
    UID from_uid;                       ///< Value to store in TrainEntry::from_uid after a
                                        ///< crossing.  For switch traversal this is the switch
                                        ///< UID so that ahead_port() works on the next section.
    bool is_boundary_crossing = false;  ///< True when the ahead neighbor is a BoundaryNode;
                                        ///< the train will be removed after this tick.
};

// ── TrainFleet ────────────────────────────────────────────────────────────────

class TrainFleet
{
public:
    /// Emitted once per tick with all PipEvents generated during that tick.
    using PipCallback = std::function<void(const std::vector<PipEvent>&)>;

    /// Tick duration in seconds (= 1 / 20 Hz).
    static constexpr float TICK_DT_S = 0.05f;

    // ── Mutation (call before EngineLoop::start()) ────────────────────────────

    /// Add a train to the fleet.  Must be called before EngineLoop::start().
    ///
    /// @param initial   Initial physics + position state (use make_train_sim_state).
    /// @param from_uid  UID of the section or node the train is coming from.
    ///                  Determines which side of the current section is "ahead".
    void add_train(sim::TrainSimState initial, UID from_uid);

    bool empty() const noexcept { return entries_.empty(); }
    std::size_t size() const noexcept { return entries_.size(); }

    // ── Tick (ENGINE thread only) ─────────────────────────────────────────────

    /// Advance all trains by one tick.
    ///
    /// @param state    Mutable world state — occupancy is updated on crossings.
    /// @param tick_num Current tick counter (for logging).
    /// @param pip_cb   Callback invoked once with all PipEvents for this tick.
    ///                 May be nullptr (no events emitted).
    void tick_all(EngineState& state, uint64_t tick_num, const PipCallback& pip_cb);

    /// Determine the next section and traversal metadata for a train on `current_uid`
    /// that came from `from_uid`.
    ///
    /// Handles:
    ///   - Direct TrackSection neighbour (unchanged behaviour)
    ///   - Switch traversal: trunk→straight/divergent and leg→trunk based on position
    ///   - BoundaryNode, or an unknown neighbour (e.g. a cross-referenced section from
    ///     an unloaded scenario): marks is_boundary_crossing = true so tick_all removes
    ///     the train, instead of leaving it stalled indefinitely
    ///   - MOVING switch: returns section_uid = nullopt (dead-end, train waits)
    static NextSectionInfo resolve_next_section(const IStateView& state, UID current_uid,
                                                UID from_uid);

private:
    /// Get the best signal aspect visible from the "ahead" port of `section`.
    /// Falls back to S2_PROCEED when no signals are configured.
    static SignalAspect ahead_signal_aspect(const IStateView& state, const TrackSection& section,
                                            UID from_uid);

    std::vector<TrainEntry> entries_;
};

}  // namespace engine::core
