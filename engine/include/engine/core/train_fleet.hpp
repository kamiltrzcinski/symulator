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
    GID from_gid;  ///< GID of the section/node the train last came from.
                   ///< Used to determine which port is "ahead" on the current section.
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
    /// @param from_gid  GID of the section or node the train is coming from.
    ///                  Determines which side of the current section is "ahead".
    void add_train(sim::TrainSimState initial, GID from_gid);

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

private:
    /// Determine the next section GID for a train currently on `current_gid`
    /// that came from `from_gid`.  Returns nullopt for switches, boundary nodes,
    /// or unknown neighbors (train stops at section end).
    static std::optional<GID> resolve_next_section(const IStateView& state, const GID& current_gid,
                                                   const GID& from_gid);

    /// Get the best signal aspect visible from the "ahead" port of `section`.
    /// Falls back to S2_PROCEED when no signals are configured.
    static SignalAspect ahead_signal_aspect(const IStateView& state, const TrackSection& section,
                                            const GID& from_gid);

    std::vector<TrainEntry> entries_;
};

}  // namespace engine::core
