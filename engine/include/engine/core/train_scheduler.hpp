// engine/include/engine/core/train_scheduler.hpp
//
// Placeholder for timetable-driven train spawning/despawning (plan_engine.md
// E5). Blocked on the schedule data format in symulator-data/packages/schedules/
// gaining engine-ready spawn fields (boundary_node_uid, tick_time, consist_uid,
// direction) — today it only describes a human-readable timetable (station
// names, arrival/departure clock times). See docs/plan_engine.md and
// docs/plan_migracji.md.
//
// Not wired into EngineLoop or the server — this is a shape declaration only.

#pragma once

#include "engine/core/engine_state.hpp"

namespace engine::core
{

class TrainScheduler
{
public:
    explicit TrainScheduler(EngineState& state) : state_(state) {}

    /// No-op until the schedule data format supports engine-ready spawn events.
    void on_tick(std::uint64_t tick_num) { (void)tick_num; }

private:
    EngineState& state_;
};

}  // namespace engine::core
