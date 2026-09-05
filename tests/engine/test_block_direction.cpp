#include <gtest/gtest.h>
#include "engine/core/engine_state.hpp"
#include "srk/common/device_rules.hpp"

using namespace engine::core;
using namespace srk::common;

TEST(BlockDirection, StateTransitions) {
    EngineState state{""};
    BlockSection bs;
    bs.uid = UID{3};
    state.insert_block_section(bs);

    SetBlockDirectionCmd cmd{UID{3}, BlockDirectionOperation::REQUEST_OUTBOUND};
    auto changes = execute_set_block_direction(state, cmd);
    ASSERT_EQ(changes.size(), 1);
    auto change = std::get<BlockDirectionChange>(changes[0]);
    EXPECT_EQ(change.uid, UID{3});
    EXPECT_EQ(change.new_direction, BlockDirectionState::OUTBOUND_PENDING);
}
