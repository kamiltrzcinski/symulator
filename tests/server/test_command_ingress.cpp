// tests/server/test_command_ingress.cpp
#include "commands_generated.h"
#include "server/command_ingress.hpp"

#include <flatbuffers/flatbuffers.h>
#include <gtest/gtest.h>
#include <cstdint>
#include <vector>

using namespace server;
using namespace engine::core;

namespace
{

// Build a COMMAND payload: [cmd_type:1, fb_body:N]
std::vector<uint8_t> make_payload(uint8_t cmd_type, const flatbuffers::FlatBufferBuilder& fbb)
{
    std::vector<uint8_t> out;
    out.push_back(cmd_type);
    out.insert(out.end(), fbb.GetBufferPointer(), fbb.GetBufferPointer() + fbb.GetSize());
    return out;
}

}  // namespace

// ── SetSwitchPosition (0x01) ──────────────────────────────────────────────────
TEST(CommandIngress, SetSwitchPosition)
{
    flatbuffers::FlatBufferBuilder fbb;
    auto gid = fbb.CreateString("SW-01");
    fbb.Finish(proto::CreateSetSwitchPosition(fbb, gid, proto::SwitchPosition_DIVERGENT));

    const auto payload = make_payload(0x01, fbb);
    const auto result =
        CommandIngress::parse_payload(payload.data(), static_cast<uint32_t>(payload.size()));
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(std::holds_alternative<SetSwitchPositionCmd>(*result));
    const auto& cmd = std::get<SetSwitchPositionCmd>(*result);
    EXPECT_EQ(cmd.gid.value, "SW-01");
    EXPECT_EQ(cmd.position, SwitchPosition::DIVERGENT);
}

// ── SetSignalAspect (0x02) ────────────────────────────────────────────────────
TEST(CommandIngress, SetSignalAspect)
{
    flatbuffers::FlatBufferBuilder fbb;
    auto gid = fbb.CreateString("SIG-12");
    fbb.Finish(proto::CreateSetSignalAspect(fbb, gid, proto::Aspect_S2_PROCEED));

    const auto payload = make_payload(0x02, fbb);
    const auto result =
        CommandIngress::parse_payload(payload.data(), static_cast<uint32_t>(payload.size()));
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(std::holds_alternative<SetSignalAspectCmd>(*result));
    const auto& cmd = std::get<SetSignalAspectCmd>(*result);
    EXPECT_EQ(cmd.gid.value, "SIG-12");
    EXPECT_EQ(cmd.aspect, SignalAspect::S2_PROCEED);
}

// ── SetBlockSection (0x04) ────────────────────────────────────────────────────
TEST(CommandIngress, SetBlockSection_Inverted)
{
    flatbuffers::FlatBufferBuilder fbb;
    auto gid = fbb.CreateString("BLK-03");
    // proto OPEN=0 → engine OPEN
    fbb.Finish(proto::CreateSetBlockSection(fbb, gid, proto::BlockSectionState_OPEN));

    const auto payload = make_payload(0x04, fbb);
    const auto result =
        CommandIngress::parse_payload(payload.data(), static_cast<uint32_t>(payload.size()));
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(std::holds_alternative<SetBlockSectionCmd>(*result));
    EXPECT_EQ(std::get<SetBlockSectionCmd>(*result).state, BlockSectionState::OPEN);

    // proto CLOSED=1 → engine CLOSED
    flatbuffers::FlatBufferBuilder fbb2;
    auto gid2 = fbb2.CreateString("BLK-03");
    fbb2.Finish(proto::CreateSetBlockSection(fbb2, gid2, proto::BlockSectionState_CLOSED));
    const auto payload2 = make_payload(0x04, fbb2);
    const auto result2 =
        CommandIngress::parse_payload(payload2.data(), static_cast<uint32_t>(payload2.size()));
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(std::get<SetBlockSectionCmd>(*result2).state, BlockSectionState::CLOSED);
}

// ── Unknown cmd_type returns nullopt ─────────────────────────────────────────
TEST(CommandIngress, OperatorCommand)
{
    flatbuffers::FlatBufferBuilder fbb;
    auto gid = fbb.CreateString("ZBG_2P");
    fbb.Finish(proto::CreateOperatorCommand(fbb, gid, proto::OperatorTargetKind_BLOCK_SECTION,
                                            proto::OperatorCommandCode_BLW));

    const auto payload = make_payload(0x20, fbb);
    const auto result =
        CommandIngress::parse_payload(payload.data(), static_cast<uint32_t>(payload.size()));
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(std::holds_alternative<OperatorCommandCmd>(*result));
    const auto& cmd = std::get<OperatorCommandCmd>(*result);
    EXPECT_EQ(cmd.target_gid.value, "ZBG_2P");
    EXPECT_EQ(cmd.target_kind, OperatorTargetKind::BLOCK_SECTION);
    EXPECT_EQ(cmd.code, OperatorCommandCode::BLW);
}

TEST(CommandIngress, OperatorCommand_LevelCrossing)
{
    flatbuffers::FlatBufferBuilder fbb;
    auto gid = fbb.CreateString("MMZ_2148");
    fbb.Finish(proto::CreateOperatorCommand(fbb, gid, proto::OperatorTargetKind_LEVEL_CROSSING,
                                            proto::OperatorCommandCode_PDZ));

    const auto payload = make_payload(0x20, fbb);
    const auto result =
        CommandIngress::parse_payload(payload.data(), static_cast<uint32_t>(payload.size()));
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(std::holds_alternative<OperatorCommandCmd>(*result));
    const auto& cmd = std::get<OperatorCommandCmd>(*result);
    EXPECT_EQ(cmd.target_gid.value, "MMZ_2148");
    EXPECT_EQ(cmd.target_kind, OperatorTargetKind::LEVEL_CROSSING);
    EXPECT_EQ(cmd.code, OperatorCommandCode::PDZ);
}

TEST(CommandIngress, Ml8Command)
{
    flatbuffers::FlatBufferBuilder fbb;
    auto gid = fbb.CreateString("ML8_SIG_A");
    fbb.Finish(proto::CreateMl8Command(fbb, gid, proto::OperatorTargetKind_SIGNAL,
                                       proto::Ml8CommandCode_STOJ));

    const auto payload = make_payload(0x21, fbb);
    const auto result =
        CommandIngress::parse_payload(payload.data(), static_cast<uint32_t>(payload.size()));
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(std::holds_alternative<Ml8CommandCmd>(*result));
    const auto& cmd = std::get<Ml8CommandCmd>(*result);
    EXPECT_EQ(cmd.target_gid.value, "ML8_SIG_A");
    EXPECT_EQ(cmd.target_kind, OperatorTargetKind::SIGNAL);
    EXPECT_EQ(cmd.code, Ml8CommandCode::STOJ);
}

TEST(CommandIngress, UnknownCmdType)
{
    const std::vector<uint8_t> payload = {0xFF, 0x01, 0x02};
    EXPECT_FALSE(
        CommandIngress::parse_payload(payload.data(), static_cast<uint32_t>(payload.size()))
            .has_value());
}

// ── Empty payload returns nullopt ────────────────────────────────────────────
TEST(CommandIngress, EmptyPayload)
{
    EXPECT_FALSE(CommandIngress::parse_payload(nullptr, 0).has_value());
}

// ── Truncated FlatBuffers body returns nullopt ────────────────────────────────
TEST(CommandIngress, TruncatedBody)
{
    // Only the cmd_type byte, no FB body
    const std::vector<uint8_t> payload = {0x01};
    EXPECT_FALSE(
        CommandIngress::parse_payload(payload.data(), static_cast<uint32_t>(payload.size()))
            .has_value());
}
