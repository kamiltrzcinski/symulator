#include <gtest/gtest.h>

#include "server/frame.hpp"

#include <tests/common/param_test_helpers.hpp>

#include <cstdint>
#include <vector>

namespace
{

using namespace server;

// ── CRC-32/ISO-HDLC ──────────────────────────────────────────────────────────

TEST(FrameCrc, KnownVector)
{
    const uint8_t data[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
    EXPECT_EQ(crc32_iso_hdlc(data, sizeof(data)), 0xCBF43926u);
}

TEST(FrameCrc, EmptyInput)
{
    EXPECT_EQ(crc32_iso_hdlc(nullptr, 0), 0x00000000u);
}

// ── Encode / decode round-trip ───────────────────────────────────────────────

struct FrameRoundTripCase
{
    const char* name;
    uint8_t msg_type;
    uint8_t flags;
    uint32_t seq_id;
    std::vector<uint8_t> payload;
};

class FrameRoundTripTest : public ::testing::TestWithParam<FrameRoundTripCase>
{
};

TEST_P(FrameRoundTripTest, PreservesFieldsAndPayload)
{
    const auto& p = GetParam();
    const auto wire = encode_frame(p.msg_type, p.flags, p.seq_id, p.payload);

    const auto result = decode_frame(wire.data(), wire.size());
    ASSERT_EQ(result.status, FrameDecodeStatus::kOk);
    EXPECT_EQ(result.frame.msg_type, p.msg_type);
    EXPECT_EQ(result.frame.flags, p.flags);
    EXPECT_EQ(result.frame.seq_id, p.seq_id);
    EXPECT_EQ(result.frame.payload, p.payload);
    EXPECT_EQ(result.bytes_consumed, static_cast<uint32_t>(kHeaderSize + p.payload.size()));
}

INSTANTIATE_TEST_SUITE_P(
    FrameRoundTripCases, FrameRoundTripTest,
    ::testing::Values(
        FrameRoundTripCase{"HeartbeatEmpty", msg_type::kHeartbeat, 0, 42, {}},
        FrameRoundTripCase{"CommandWithPayload",
                           msg_type::kCommand,
                           kFlagIsLastChunk,
                           99,
                           {0xDE, 0xAD, 0xBE, 0xEF}},
        FrameRoundTripCase{"Handshake", msg_type::kHandshake, 0, 1, {msg_type::kHandshake, 0xAB}},
        FrameRoundTripCase{
            "HandshakeAck", msg_type::kHandshakeAck, 0, 1, {msg_type::kHandshakeAck, 0xAB}},
        FrameRoundTripCase{"Heartbeat", msg_type::kHeartbeat, 0, 1, {msg_type::kHeartbeat, 0xAB}},
        FrameRoundTripCase{
            "HeartbeatAck", msg_type::kHeartbeatAck, 0, 1, {msg_type::kHeartbeatAck, 0xAB}},
        FrameRoundTripCase{"Command", msg_type::kCommand, 0, 1, {msg_type::kCommand, 0xAB}},
        FrameRoundTripCase{
            "CommandAck", msg_type::kCommandAck, 0, 1, {msg_type::kCommandAck, 0xAB}},
        FrameRoundTripCase{
            "CommandNak", msg_type::kCommandNak, 0, 1, {msg_type::kCommandNak, 0xAB}},
        FrameRoundTripCase{
            "DomainEvent", msg_type::kDomainEvent, 0, 1, {msg_type::kDomainEvent, 0xAB}},
        FrameRoundTripCase{"SnapshotRequest",
                           msg_type::kSnapshotRequest,
                           0,
                           1,
                           {msg_type::kSnapshotRequest, 0xAB}},
        FrameRoundTripCase{
            "SnapshotChunk", msg_type::kSnapshotChunk, 0, 1, {msg_type::kSnapshotChunk, 0xAB}},
        FrameRoundTripCase{
            "SessionNotice", msg_type::kSessionNotice, 0, 1, {msg_type::kSessionNotice, 0xAB}},
        FrameRoundTripCase{"TakeoverRequest",
                           msg_type::kTakeoverRequest,
                           0,
                           1,
                           {msg_type::kTakeoverRequest, 0xAB}},
        FrameRoundTripCase{"TakeoverResponse",
                           msg_type::kTakeoverResponse,
                           0,
                           1,
                           {msg_type::kTakeoverResponse, 0xAB}},
        FrameRoundTripCase{
            "ChatMessage", msg_type::kChatMessage, 0, 1, {msg_type::kChatMessage, 0xAB}},
        FrameRoundTripCase{"DispatchChannel",
                           msg_type::kDispatchChannel,
                           0,
                           1,
                           {msg_type::kDispatchChannel, 0xAB}},
        FrameRoundTripCase{
            "VoiceJoin", msg_type::kVoiceChanJoin, 0, 1, {msg_type::kVoiceChanJoin, 0xAB}},
        FrameRoundTripCase{
            "VoiceLeave", msg_type::kVoiceChanLeave, 0, 1, {msg_type::kVoiceChanLeave, 0xAB}},
        FrameRoundTripCase{
            "VoiceState", msg_type::kVoiceChanState, 0, 1, {msg_type::kVoiceChanState, 0xAB}}),
    tests::common::param_name<FrameRoundTripCase>);

// ── Decode error paths ───────────────────────────────────────────────────────

struct NeedMoreDataCase
{
    const char* name;
    std::vector<uint8_t> wire;
    std::size_t available;
};

NeedMoreDataCase make_header_truncated_case()
{
    auto wire = encode_frame(msg_type::kHeartbeat, 0, 1, nullptr, 0);
    return NeedMoreDataCase{"HeaderTruncated", std::move(wire),
                            static_cast<std::size_t>(kHeaderSize - 1)};
}

NeedMoreDataCase make_payload_truncated_case()
{
    const std::vector<uint8_t> payload(64, 0xCC);
    auto wire = encode_frame(msg_type::kCommand, 0, 1, payload);
    return NeedMoreDataCase{"PayloadTruncated", std::move(wire),
                            static_cast<std::size_t>(kHeaderSize + 10)};
}

class FrameDecodeNeedMoreDataTest : public ::testing::TestWithParam<NeedMoreDataCase>
{
};

TEST_P(FrameDecodeNeedMoreDataTest, ReturnsNeedMoreData)
{
    const auto& p = GetParam();
    const auto result = decode_frame(p.wire.data(), p.available);
    EXPECT_EQ(result.status, FrameDecodeStatus::kNeedMoreData);
}

INSTANTIATE_TEST_SUITE_P(NeedMoreDataCases, FrameDecodeNeedMoreDataTest,
                         ::testing::Values(make_header_truncated_case(),
                                           make_payload_truncated_case()),
                         tests::common::param_name<NeedMoreDataCase>);

using WireMutator = void (*)(std::vector<uint8_t>&);

void mutate_bad_magic(std::vector<uint8_t>& wire)
{
    wire[0] = 0xFF;
}

void mutate_bad_crc(std::vector<uint8_t>& wire)
{
    wire[12] ^= 0xFF;
}

void mutate_bad_length(std::vector<uint8_t>& wire)
{
    wire[8] = 0x00;
    wire[9] = 0x00;
    wire[10] = 0x01;
    wire[11] = 0x00;

    const uint32_t crc = crc32_iso_hdlc(wire.data(), 12);
    wire[12] = static_cast<uint8_t>(crc & 0xFF);
    wire[13] = static_cast<uint8_t>((crc >> 8) & 0xFF);
    wire[14] = static_cast<uint8_t>((crc >> 16) & 0xFF);
    wire[15] = static_cast<uint8_t>((crc >> 24) & 0xFF);
}

struct InvalidWireCase
{
    const char* name;
    WireMutator mutate;
    FrameDecodeStatus expected;
};

class FrameDecodeInvalidWireTest : public ::testing::TestWithParam<InvalidWireCase>
{
};

TEST_P(FrameDecodeInvalidWireTest, ReturnsExpectedStatus)
{
    const auto& p = GetParam();
    auto wire = encode_frame(msg_type::kHeartbeat, 0, 1, nullptr, 0);
    p.mutate(wire);

    const auto result = decode_frame(wire.data(), wire.size());
    EXPECT_EQ(result.status, p.expected);
}

INSTANTIATE_TEST_SUITE_P(
    InvalidWireCases, FrameDecodeInvalidWireTest,
    ::testing::Values(InvalidWireCase{"BadMagic", &mutate_bad_magic, FrameDecodeStatus::kBadMagic},
                      InvalidWireCase{"BadCrc", &mutate_bad_crc, FrameDecodeStatus::kBadCrc},
                      InvalidWireCase{"BadLength", &mutate_bad_length,
                                      FrameDecodeStatus::kBadLength}),
    tests::common::param_name<InvalidWireCase>);

// ── Boundary behavior ────────────────────────────────────────────────────────

TEST(FrameDecode, MaxPayload)
{
    const std::vector<uint8_t> payload(kMaxPayloadLen, 0xAB);
    const auto wire = encode_frame(msg_type::kSnapshotChunk, kFlagIsLastChunk, 7, payload);

    const auto result = decode_frame(wire.data(), wire.size());
    ASSERT_EQ(result.status, FrameDecodeStatus::kOk);
    EXPECT_EQ(result.frame.payload.size(), static_cast<std::size_t>(kMaxPayloadLen));
}

TEST(FrameDecode, TrailingDataDoesNotAffectDecode)
{
    const std::vector<uint8_t> payload = {1, 2, 3};
    auto wire = encode_frame(msg_type::kHandshake, 0, 10, payload);
    wire.push_back(0xFF);
    wire.push_back(0xFF);

    const auto result = decode_frame(wire.data(), wire.size());
    ASSERT_EQ(result.status, FrameDecodeStatus::kOk);
    EXPECT_EQ(result.bytes_consumed, static_cast<uint32_t>(kHeaderSize + payload.size()));
}

}  // namespace
