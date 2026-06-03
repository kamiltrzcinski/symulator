// engine/include/engine/core/engine_loop.hpp
// 20 Hz ENGINE loop — command drain → interlocking → StateApplier →
//                     TrainFleet tick → snapshot publish.
//
// Usage:
//   EngineLoop loop(state, control_system, command_queue, atomic_snapshot);
//   loop.add_train(train_state, from_uid);   // before start()
//   loop.start();
//   // ... run scenario ...
//   loop.stop();  // blocks until the ENGINE thread has exited

#pragma once

#include "engine/core/command.hpp"
#include "engine/core/control_system.hpp"
#include "engine/core/engine_snapshot.hpp"
#include "engine/core/engine_state.hpp"
#include "engine/core/priority_command_queue.hpp"
#include "engine/core/train_fleet.hpp"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

namespace engine::core
{

class EngineLoop
{
public:
    /// Called on the ENGINE thread when a command fails an interlocking check.
    using NakCallback = std::function<void(const EnvelopedCommand&, const InterlockingViolation&)>;

    /// Called on the ENGINE thread once per tick with all DeviceStateChange
    /// values produced by that tick (command results + on_tick results).
    /// The callback receives the complete list for the tick in call order.
    using StateChangesCallback = std::function<void(const std::vector<DeviceStateChange>&)>;

    /// Called on the ENGINE thread once per tick with all PipEvents produced by
    /// TrainFleet (section crossings, occupancy changes).  nullptr = disabled.
    using PipCallback = TrainFleet::PipCallback;

    /// @param state       Mutable world state (must outlive this object).
    /// @param control     Control-system implementation for this scenario.
    /// @param queue       Thread-safe command queue fed by CommandIngress.
    /// @param snapshot    AtomicSnapshot published after each tick.
    /// @param nak_cb      Optional callback invoked on rejected commands.
    /// @param changes_cb  Optional callback invoked once per tick with all
    ///                    state changes produced during that tick.
    /// @param pip_cb      Optional callback for PipEvents from TrainFleet.
    EngineLoop(EngineState& state, IControlSystem& control,
               PriorityCommandQueue<EnvelopedCommand>& queue, AtomicSnapshot& snapshot,
               NakCallback nak_cb = nullptr, StateChangesCallback changes_cb = nullptr,
               PipCallback pip_cb = nullptr);

    ~EngineLoop();

    // Non-copyable, non-movable (owns a thread).
    EngineLoop(const EngineLoop&) = delete;
    EngineLoop& operator=(const EngineLoop&) = delete;

    /// Add a train to the fleet.  Must be called before start().
    /// @param initial  Initial TrainSimState (use make_train_sim_state()).
    /// @param from_uid UID of the section/node behind the train's current
    ///                 section — determines which direction is "ahead".
    void add_train(sim::TrainSimState initial, UID from_uid);

    /// Spawn the ENGINE thread and begin ticking.  No-op if already running.
    void start();

    /// Request shutdown, interrupt the current sleep, and join.  Safe to call
    /// multiple times or without a matching start().
    void stop();

    bool is_running() const noexcept { return running_.load(std::memory_order_relaxed); }

    /// Maximum commands drained from the queue in a single tick.
    static constexpr int MAX_CMDS_PER_TICK = 64;

    /// Target period between ticks (50 ms ≡ 20 Hz).
    static constexpr std::chrono::milliseconds TICK_PERIOD{50};

private:
    void run(std::stop_token stoken);
    void do_tick();

    EngineState& state_;
    IControlSystem& control_;
    PriorityCommandQueue<EnvelopedCommand>& queue_;
    AtomicSnapshot& snapshot_;
    NakCallback nak_cb_;
    StateChangesCallback changes_cb_;
    PipCallback pip_cb_;

    TrainFleet train_fleet_;

    std::atomic<bool> running_{false};
    std::condition_variable_any cv_;
    std::mutex cv_mutex_;
    std::jthread thread_;
};

}  // namespace engine::core
