#include <gtest/gtest.h>
#include "engine/core/track_model.hpp"
#include "engine/core/types.hpp"
#include "engine/core/engine_state.hpp"
#include "srk/common/device_rules.hpp"

using namespace engine::core;
using namespace srk::common;

TEST(LevelCrossing, WarningDelay) {
    EngineState state{""};
    LevelCrossing lx;
    lx.uid = UID{1};
    lx.status = LevelCrossingStatus::WARNING;
    lx.warning_duration_ticks = 160;
    state.insert_level_crossing(lx);
    state.apply_level_crossing_status(UID{1}, LevelCrossingStatus::WARNING);

    state.set_current_tick(10);
    auto changes = tick_level_crossings(state, state.current_tick());
    EXPECT_TRUE(changes.empty());

    state.set_current_tick(160);
    changes = tick_level_crossings(state, state.current_tick());
    EXPECT_EQ(changes.size(), 1);
    auto change = std::get<LevelCrossingStateChange>(changes[0]);
    EXPECT_EQ(change.crossing_uid, UID{1});
    EXPECT_EQ(change.status, LevelCrossingStatus::CLOSED);
}
