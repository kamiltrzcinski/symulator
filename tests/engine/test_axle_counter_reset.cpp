#include <gtest/gtest.h>
#include "engine/core/engine_state.hpp"
#include "srk/common/device_rules.hpp"
#include "srk/common/nak_codes.hpp"

using namespace engine::core;
using namespace srk::common;

TEST(AxleCounterReset, RequiresDelay) {
    EngineState state;
    state.set_session_id("");
    BlockSection bs;
    bs.uid = UID{2};
    state.insert_block_section(bs);
    state.set_current_tick(0);
    state.apply_block_section_direction(UID{2}, BlockDirectionState::RESET_PENDING);

    ResetAxleCounterCmd cmd{UID{2}};
    
    state.set_current_tick(10);
    auto error = check_reset_axle_counter(state, cmd);
    ASSERT_TRUE(error.has_value());
    EXPECT_EQ(error->reason_code, NAK_SAFETY_BLOCK);

    state.set_current_tick(1200);
    auto success = check_reset_axle_counter(state, cmd);
    EXPECT_FALSE(success.has_value());
}
