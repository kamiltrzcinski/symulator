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
#include "engine/timetable/timetable_catalog.hpp"

namespace engine::core
{

struct ScheduleStop {
    UID point_uid;
    // arrival/departure times, etc. can be added later
};

struct Schedule {
    UID schedule_uid;
    std::vector<ScheduleStop> route;
};

class TrainScheduler
{
public:
    explicit TrainScheduler(EngineState& state, const timetable::TimetableCatalog& catalog) 
        : state_(state), catalog_(catalog) {}

    /// Validates a schedule macro-route using the timetable catalog
    bool validate_schedule_route(const Schedule& schedule) const {
        if (schedule.route.empty()) return false;
        
        for (size_t i = 0; i < schedule.route.size() - 1; ++i) {
            UID current_uid = schedule.route[i].point_uid;
            UID next_uid = schedule.route[i+1].point_uid;
            
            if (!catalog_.contains_point(current_uid) || !catalog_.contains_point(next_uid)) {
                return false;
            }
            if (catalog_.has_connections()) {
                if (!catalog_.are_connected(current_uid, next_uid)) return false;
            }
        }
        return true;
    }

    /// No-op until the schedule data format supports engine-ready spawn events.
    void on_tick(std::uint64_t tick_num) { (void)tick_num; }

private:
    EngineState& state_;
    const timetable::TimetableCatalog& catalog_;
};

}  // namespace engine::core
