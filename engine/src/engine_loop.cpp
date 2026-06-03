// engine/src/engine_loop.cpp

#include "engine/core/engine_loop.hpp"
#include "engine/core/engine_snapshot.hpp"
#include "engine/core/state_applier.hpp"

#include <memory>

namespace engine::core
{

EngineLoop::EngineLoop(EngineState& state, IControlSystem& control,
                       PriorityCommandQueue<EnvelopedCommand>& queue, AtomicSnapshot& snapshot,
                       NakCallback nak_cb, StateChangesCallback changes_cb, PipCallback pip_cb)
    : state_(state),
      control_(control),
      queue_(queue),
      snapshot_(snapshot),
      nak_cb_(std::move(nak_cb)),
      changes_cb_(std::move(changes_cb)),
      pip_cb_(std::move(pip_cb))
{
}

void EngineLoop::add_train(sim::TrainSimState initial, UID from_uid)
{
    train_fleet_.add_train(std::move(initial), from_uid);
}

EngineLoop::~EngineLoop()
{
    stop();
}

void EngineLoop::start()
{
    if (running_.exchange(true, std::memory_order_relaxed))
        return;  // already running

    thread_ = std::jthread([this](std::stop_token st) { run(std::move(st)); });
}

void EngineLoop::stop()
{
    if (!running_.exchange(false, std::memory_order_relaxed))
        return;  // not running

    thread_.request_stop();
    cv_.notify_all();  // interrupt sleep_until
    if (thread_.joinable())
        thread_.join();
}

void EngineLoop::run(std::stop_token stoken)
{
    using clock = std::chrono::steady_clock;

    // Register a stop callback so the condition variable is notified when
    // stop is requested externally (e.g. via jthread destructor).
    std::stop_callback stop_cb(stoken, [this] { cv_.notify_all(); });

    auto next_tick = clock::now() + TICK_PERIOD;

    while (!stoken.stop_requested())
    {
        do_tick();

        // Sleep until the next scheduled tick, but wake early if stop is
        // requested via the stop_token callback above.
        std::unique_lock<std::mutex> lock(cv_mutex_);
        cv_.wait_until(lock, stoken, next_tick, [&] { return stoken.stop_requested(); });
        next_tick += TICK_PERIOD;
    }
}

void EngineLoop::do_tick()
{
    std::vector<DeviceStateChange> tick_all_changes;

    // 1. Drain up to MAX_CMDS_PER_TICK from the priority command queue.
    int processed = 0;
    while (processed < MAX_CMDS_PER_TICK)
    {
        auto opt = queue_.try_pop();
        if (!opt)
            break;
        ++processed;
        auto& cmd = *opt;
        if (auto violation = control_.check_command(state_, cmd.payload))
        {
            if (nak_cb_)
                nak_cb_(cmd, *violation);
        }
        else
        {
            auto changes = control_.execute_command(state_, cmd.payload);
            apply_all(state_, changes);
            if (changes_cb_)
            {
                tick_all_changes.insert(tick_all_changes.end(), changes.begin(), changes.end());
            }
        }
    }

    // 2. Per-tick control-system logic (timer releases, SHL-12 state machine, etc.).
    const uint64_t next_tick_num = state_.current_tick() + 1;
    auto tick_changes = control_.on_tick(state_, next_tick_num);
    apply_all(state_, tick_changes);
    if (changes_cb_)
    {
        tick_all_changes.insert(tick_all_changes.end(), tick_changes.begin(), tick_changes.end());
    }

    // 3. Tick all active trains (physics + occupancy + PipEvents).
    if (!train_fleet_.empty())
        train_fleet_.tick_all(state_, next_tick_num, pip_cb_);

    // 4. Advance the logical tick counter.
    state_.set_current_tick(next_tick_num);

    // 5. Deep-copy the world state into a new immutable snapshot and publish it.
    auto snap = std::make_shared<EngineSnapshot>();
    snap->session = state_.session_id();
    snap->tick = state_.current_tick();
    snap->boundary_nodes = state_.boundary_nodes();
    snap->track_sections = state_.track_sections();
    snap->switches = state_.switches();
    snap->signals = state_.signals();
    snap->derailers = state_.derailers();
    snap->block_sections = state_.block_sections();
    snap->routes = state_.routes();
    snap->alarms = state_.alarms();
    snapshot_.publish(std::move(snap));

    // 6. Notify subscribers of all state changes from this tick.
    if (changes_cb_ && !tick_all_changes.empty())
        changes_cb_(tick_all_changes);
}

}  // namespace engine::core
