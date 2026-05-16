#include <gtest/gtest.h>

#include <engine/core/command.hpp>
#include <engine/core/control_system.hpp>
#include <engine/core/control_system_registry.hpp>
#include <engine/core/engine_state.hpp>
#include <srk/ml8/ml8_system.hpp>

// ── Test helpers ──────────────────────────────────────────────────────────────

namespace
{

using namespace engine::core;

static const GID BL1 = GID{"BL-SHL12-001"};

EngineState make_state_with_block(BlockDirectionState dir = BlockDirectionState::NEUTRAL,
                                  BlockSectionState state = BlockSectionState::CLOSED,
                                  int axle_count = 0)
{
    EngineState st;
    st.set_session_id("TEST");
    st.set_current_tick(1);

    BlockSection bs;
    bs.gid = BL1;
    bs.pid = "bl1";
    bs.sid = SID{"TST"};
    bs.neighbor_sid = SID{"NGR"};
    bs.direction = dir;
    bs.state = state;
    bs.axle_count = axle_count;
    st.insert_block_section(bs);

    return st;
}

}  // namespace

// ── BLW: NEUTRAL → OUTBOUND_PENDING ──────────────────────────────────────────

TEST(Ml8Shl12_BLW, AcceptsFromNeutral)
{
    srk::ml8::Ml8System sys;
    auto st = make_state_with_block(BlockDirectionState::NEUTRAL);

    Command cmd = SetBlockDirectionCmd{BL1, Shl12Op::BLW};
    EXPECT_FALSE(sys.check_command(st, cmd).has_value());

    auto changes = sys.execute_command(st, cmd);
    ASSERT_EQ(changes.size(), 1u);
    auto* bdc = std::get_if<BlockDirectionChange>(&changes[0]);
    ASSERT_NE(bdc, nullptr);
    EXPECT_EQ(bdc->new_direction, BlockDirectionState::OUTBOUND_PENDING);
    EXPECT_TRUE(bdc->requires_neighbor_confirmation);
}

TEST(Ml8Shl12_BLW, RejectsFromNonNeutral)
{
    srk::ml8::Ml8System sys;
    auto st = make_state_with_block(BlockDirectionState::OUTBOUND);

    Command cmd = SetBlockDirectionCmd{BL1, Shl12Op::BLW};
    auto v = sys.check_command(st, cmd);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->reason_code, 0x03);  // INVALID_STATE
}

// ── BLP: OUTBOUND_PENDING → OUTBOUND ─────────────────────────────────────────

TEST(Ml8Shl12_BLP, AcceptsOutboundPending)
{
    srk::ml8::Ml8System sys;
    auto st = make_state_with_block(BlockDirectionState::OUTBOUND_PENDING);

    Command cmd = SetBlockDirectionCmd{BL1, Shl12Op::BLP};
    EXPECT_FALSE(sys.check_command(st, cmd).has_value());

    auto changes = sys.execute_command(st, cmd);
    ASSERT_EQ(changes.size(), 2u);

    bool dir_outbound = false;
    bool state_open = false;
    for (auto& c : changes)
    {
        if (auto* bdc = std::get_if<BlockDirectionChange>(&c))
        {
            EXPECT_EQ(bdc->new_direction, BlockDirectionState::OUTBOUND);
            dir_outbound = true;
        }
        if (auto* bsc = std::get_if<BlockSectionStateChange>(&c))
        {
            EXPECT_EQ(bsc->new_state, BlockSectionState::OPEN);
            state_open = true;
        }
    }
    EXPECT_TRUE(dir_outbound);
    EXPECT_TRUE(state_open);
}

TEST(Ml8Shl12_BLP, AcceptsInboundPending)
{
    srk::ml8::Ml8System sys;
    auto st = make_state_with_block(BlockDirectionState::INBOUND_PENDING);

    Command cmd = SetBlockDirectionCmd{BL1, Shl12Op::BLP};
    EXPECT_FALSE(sys.check_command(st, cmd).has_value());

    auto changes = sys.execute_command(st, cmd);
    bool dir_inbound = false;
    for (auto& c : changes)
    {
        if (auto* bdc = std::get_if<BlockDirectionChange>(&c))
        {
            if (bdc->new_direction == BlockDirectionState::INBOUND)
                dir_inbound = true;
        }
    }
    EXPECT_TRUE(dir_inbound);
}

TEST(Ml8Shl12_BLP, RejectsFromNeutral)
{
    srk::ml8::Ml8System sys;
    auto st = make_state_with_block(BlockDirectionState::NEUTRAL);

    Command cmd = SetBlockDirectionCmd{BL1, Shl12Op::BLP};
    auto v = sys.check_command(st, cmd);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->reason_code, 0x03);
}

// ── BLO: cancel pending ───────────────────────────────────────────────────────

TEST(Ml8Shl12_BLO, AcceptsOutboundPending)
{
    srk::ml8::Ml8System sys;
    auto st = make_state_with_block(BlockDirectionState::OUTBOUND_PENDING);

    Command cmd = SetBlockDirectionCmd{BL1, Shl12Op::BLO};
    EXPECT_FALSE(sys.check_command(st, cmd).has_value());

    auto changes = sys.execute_command(st, cmd);
    ASSERT_EQ(changes.size(), 1u);
    auto* bdc = std::get_if<BlockDirectionChange>(&changes[0]);
    ASSERT_NE(bdc, nullptr);
    EXPECT_EQ(bdc->new_direction, BlockDirectionState::NEUTRAL);
}

TEST(Ml8Shl12_BLO, RejectsFromOutbound)
{
    srk::ml8::Ml8System sys;
    auto st = make_state_with_block(BlockDirectionState::OUTBOUND);

    Command cmd = SetBlockDirectionCmd{BL1, Shl12Op::BLO};
    auto v = sys.check_command(st, cmd);
    ASSERT_TRUE(v.has_value());
}

// ── BLZ: release direction ────────────────────────────────────────────────────

TEST(Ml8Shl12_BLZ, AcceptsOutboundWithNoAxles)
{
    srk::ml8::Ml8System sys;
    auto st = make_state_with_block(BlockDirectionState::OUTBOUND, BlockSectionState::OPEN, 0);

    Command cmd = SetBlockDirectionCmd{BL1, Shl12Op::BLZ};
    EXPECT_FALSE(sys.check_command(st, cmd).has_value());

    auto changes = sys.execute_command(st, cmd);
    bool neutral = false;
    bool closed = false;
    for (auto& c : changes)
    {
        if (auto* bdc = std::get_if<BlockDirectionChange>(&c))
            if (bdc->new_direction == BlockDirectionState::NEUTRAL)
                neutral = true;
        if (auto* bsc = std::get_if<BlockSectionStateChange>(&c))
            if (bsc->new_state == BlockSectionState::CLOSED)
                closed = true;
    }
    EXPECT_TRUE(neutral);
    EXPECT_TRUE(closed);
}

TEST(Ml8Shl12_BLZ, RejectsWhenAxleCountNonZero)
{
    srk::ml8::Ml8System sys;
    auto st = make_state_with_block(BlockDirectionState::OUTBOUND, BlockSectionState::OPEN, 4);

    Command cmd = SetBlockDirectionCmd{BL1, Shl12Op::BLZ};
    auto v = sys.check_command(st, cmd);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->reason_code, 0x02);  // SAFETY_BLOCK
}

TEST(Ml8Shl12_BLZ, RejectsFromNeutral)
{
    srk::ml8::Ml8System sys;
    auto st = make_state_with_block(BlockDirectionState::NEUTRAL);

    Command cmd = SetBlockDirectionCmd{BL1, Shl12Op::BLZ};
    auto v = sys.check_command(st, cmd);
    ASSERT_TRUE(v.has_value());
}

// ── BLAI / BLA: emergency ────────────────────────────────────────────────────

TEST(Ml8Shl12_BLAI, TransitionsToEmergency)
{
    srk::ml8::Ml8System sys;
    auto st = make_state_with_block(BlockDirectionState::OUTBOUND);

    Command cmd = SetBlockDirectionCmd{BL1, Shl12Op::BLAI};
    EXPECT_FALSE(sys.check_command(st, cmd).has_value());

    auto changes = sys.execute_command(st, cmd);
    ASSERT_EQ(changes.size(), 1u);
    auto* bdc = std::get_if<BlockDirectionChange>(&changes[0]);
    ASSERT_NE(bdc, nullptr);
    EXPECT_EQ(bdc->new_direction, BlockDirectionState::EMERGENCY);
}

TEST(Ml8Shl12_BLA, ExecutesEmergencyChange)
{
    srk::ml8::Ml8System sys;
    auto st = make_state_with_block(BlockDirectionState::EMERGENCY);

    Command cmd = SetBlockDirectionCmd{BL1, Shl12Op::BLA};
    EXPECT_FALSE(sys.check_command(st, cmd).has_value());

    auto changes = sys.execute_command(st, cmd);
    bool neutral = false;
    bool closed = false;
    for (auto& c : changes)
    {
        if (auto* bdc = std::get_if<BlockDirectionChange>(&c))
            if (bdc->new_direction == BlockDirectionState::NEUTRAL)
                neutral = true;
        if (auto* bsc = std::get_if<BlockSectionStateChange>(&c))
            if (bsc->new_state == BlockSectionState::CLOSED)
                closed = true;
    }
    EXPECT_TRUE(neutral);
    EXPECT_TRUE(closed);
}

TEST(Ml8Shl12_BLA, RejectsFromNonEmergency)
{
    srk::ml8::Ml8System sys;
    auto st = make_state_with_block(BlockDirectionState::OUTBOUND);

    Command cmd = SetBlockDirectionCmd{BL1, Shl12Op::BLA};
    auto v = sys.check_command(st, cmd);
    ASSERT_TRUE(v.has_value());
}

// ── OPS: cancel special procedure ────────────────────────────────────────────

TEST(Ml8Shl12_OPS, CancelsEmergencyState)
{
    srk::ml8::Ml8System sys;
    auto st = make_state_with_block(BlockDirectionState::EMERGENCY);

    Command cmd = SetBlockDirectionCmd{BL1, Shl12Op::OPS};
    EXPECT_FALSE(sys.check_command(st, cmd).has_value());

    auto changes = sys.execute_command(st, cmd);
    bool neutral = false;
    for (auto& c : changes)
        if (auto* bdc = std::get_if<BlockDirectionChange>(&c))
            if (bdc->new_direction == BlockDirectionState::NEUTRAL)
                neutral = true;
    EXPECT_TRUE(neutral);
}

TEST(Ml8Shl12_OPS, CancelsResetPendingState)
{
    srk::ml8::Ml8System sys;
    auto st = make_state_with_block(BlockDirectionState::RESET_PENDING);

    Command cmd = SetBlockDirectionCmd{BL1, Shl12Op::OPS};
    EXPECT_FALSE(sys.check_command(st, cmd).has_value());
}

// ── SLI / SLK: axle counter reset ────────────────────────────────────────────

TEST(Ml8Shl12_SLI, InitiatesResetFromNeutral)
{
    srk::ml8::Ml8System sys;
    auto st = make_state_with_block(BlockDirectionState::NEUTRAL);

    Command cmd = InitAxleCounterResetCmd{BL1};
    EXPECT_FALSE(sys.check_command(st, cmd).has_value());

    auto changes = sys.execute_command(st, cmd);
    ASSERT_EQ(changes.size(), 1u);
    auto* bdc = std::get_if<BlockDirectionChange>(&changes[0]);
    ASSERT_NE(bdc, nullptr);
    EXPECT_EQ(bdc->new_direction, BlockDirectionState::RESET_PENDING);
}

TEST(Ml8Shl12_SLI, RejectsFromNonNeutral)
{
    srk::ml8::Ml8System sys;
    auto st = make_state_with_block(BlockDirectionState::OUTBOUND);

    Command cmd = InitAxleCounterResetCmd{BL1};
    auto v = sys.check_command(st, cmd);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->reason_code, 0x03);
}

TEST(Ml8Shl12_SLK, ResetsCounter)
{
    srk::ml8::Ml8System sys;
    auto st = make_state_with_block(BlockDirectionState::RESET_PENDING);

    Command cmd = ResetAxleCounterCmd{BL1};
    EXPECT_FALSE(sys.check_command(st, cmd).has_value());

    auto changes = sys.execute_command(st, cmd);
    ASSERT_EQ(changes.size(), 2u);

    bool neutral = false;
    bool closed = false;
    for (auto& c : changes)
    {
        if (auto* bdc = std::get_if<BlockDirectionChange>(&c))
            if (bdc->new_direction == BlockDirectionState::NEUTRAL)
                neutral = true;
        if (auto* bsc = std::get_if<BlockSectionStateChange>(&c))
            if (bsc->new_state == BlockSectionState::CLOSED)
                closed = true;
    }
    EXPECT_TRUE(neutral);
    EXPECT_TRUE(closed);
}

TEST(Ml8Shl12_SLK, RejectsFromNonResetPending)
{
    srk::ml8::Ml8System sys;
    auto st = make_state_with_block(BlockDirectionState::NEUTRAL);

    Command cmd = ResetAxleCounterCmd{BL1};
    auto v = sys.check_command(st, cmd);
    ASSERT_TRUE(v.has_value());
}

// ── Registry ──────────────────────────────────────────────────────────────────

TEST(ControlSystemRegistry, Ml8Registered)
{
    EXPECT_TRUE(engine::core::ControlSystemRegistry::instance().has(
        engine::core::ControlSystemID{"estw_ml8"}));
}
