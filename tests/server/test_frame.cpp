// tests/server/test_frame.cpp
#include <gtest/gtest.h>
#include <cstdint>
#include <vector>
#include "server/frame.hpp"

using namespace server;

// ── CRC-32/ISO-HDLC ──────────────────────────────────────────────────────────

TEST(FrameCrc, KnownVector)
{
    // CRC-32/ISO-HDLC over "123456789" == 0xCBF43926
    const uint8_t data[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
    EXPECT_EQ(crc32_iso_hdlc(data, sizeof(data)), 0xCBF43926u);
}

TEST(FrameCrc, EmptyInput)
{
    // CRC-32/ISO-HDLC with no bytes == 0xFFFFFFFF ^ 0xFFFFFFFF == 0x00000000
    EXPECT_EQ(crc32_iso_hdlc(nullptr, 0), 0x00000000u);
}

// ── Encode / decode round-trip ────────────────────────────────────────────────

TEST(FrameRoundTrip, EmptyPayload)
{
    const auto wire = encode_frame(msg_type::kHeartbeat, 0, 42, nullptr, 0);
    ASSERT_EQ(wire.size(), static_cast<std::size_t>(kHeaderSize));

    const auto result = decode_frame(wire.data(), wire.size());
    EXPECT_EQ(result.status, FrameDecodeStatus::kOk);
    EXPECT_EQ(result.frame.msg_type, msg_type::kHeartbeat);
    EXPECT_EQ(result.frame.flags, 0u);
    EXPECT_EQ(result.frame.seq_id, 42u);
    EXPECT_TRUE(result.frame.payload.empty());
    EXPECT_EQ(result.bytes_consumed, static_cast<uint32_t>(kHeaderSize));
}

TEST(FrameRoundTrip, PayloadPreserved)
{
    const std::vector<uint8_t> payload = {0xDE, 0xAD, 0xBE, 0xEF};
    const auto wire = encode_frame(msg_type::kCommand, kFlagIsLastChunk, 99, payload);
    ASSERT_EQ(wire.size(), static_cast<std::size_t>(kHeaderSize) + payload.size());

    const auto result = decode_frame(wire.data(), wire.size());
    EXPECT_EQ(result.status, FrameDecodeStatus::kOk);
    EXPECT_EQ(result.frame.msg_type, msg_type::kCommand);
    EXPECT_EQ(result.frame.flags, kFlagIsLastChunk);
    EXPECT_EQ(result.frame.seq_id, 99u);
    EXPECT_EQ(result.frame.payload, payload);
}

TEST(FrameRoundTrip, AllMsgTypes)
{
    const std::vector<uint8_t> msg_types = {
        msg_type::kHandshake,     msg_type::kHandshakeAck,  msg_type::kHeartbeat,
        msg_type::kHeartbeatAck,  msg_type::kCommand,       msg_type::kCommandAck,
        msg_type::kCommandNak,    msg_type::kDomainEvent,   msg_type::kSnapshotRequest,
        msg_type::kSnapshotChunk, msg_type::kSessionNotice,
    };
    for (auto mt : msg_types)
    {
        const std::vector<uint8_t> payload = {mt, 0xAB};
        const auto wire = encode_frame(mt, 0, 1, payload);
        const auto result = decode_frame(wire.data(), wire.size());
        EXPECT_EQ(result.status, FrameDecodeStatus::kOk) << "msg_type=" << (int)mt;
        EXPECT_EQ(result.frame.msg_type, mt);
    }
}

// ── Error cases ───────────────────────────────────────────────────────────────

TEST(FrameDecode, NeedMoreData_Header)
{
    const auto wire = encode_frame(msg_type::kHeartbeat, 0, 1, nullptr, 0);
    const auto result = decode_frame(wire.data(), wire.size() - 1);
    EXPECT_EQ(result.status, FrameDecodeStatus::kNeedMoreData);
}

TEST(FrameDecode, NeedMoreData_Payload)
{
    const std::vector<uint8_t> payload(64, 0xCC);
    const auto wire = encode_frame(msg_type::kCommand, 0, 1, payload);
    // Provide full header but incomplete payload
    const auto result = decode_frame(wire.data(), kHeaderSize + 10);
    EXPECT_EQ(result.status, FrameDecodeStatus::kNeedMoreData);
}

TEST(FrameDecode, BadMagic)
{
    auto wire = encode_frame(msg_type::kHeartbeat, 0, 1, nullptr, 0);
    wire[0] = 0xFF;  // corrupt magic
    const auto result = decode_frame(wire.data(), wire.size());
    EXPECT_EQ(result.status, FrameDecodeStatus::kBadMagic);
}

TEST(FrameDecode, BadCrc)
{
    auto wire = encode_frame(msg_type::kHeartbeat, 0, 1, nullptr, 0);
    wire[12] ^= 0xFF;  // flip CRC bytes
    const auto result = decode_frame(wire.data(), wire.size());
    EXPECT_EQ(result.status, FrameDecodeStatus::kBadCrc);
}

TEST(FrameDecode, BadLength_Overflow)
{
    auto wire = encode_frame(msg_type::kHeartbeat, 0, 1, nullptr, 0);
    // Write payload_len > kMaxPayloadLen (65535) — e.g. 0x0001_0000
    wire[8] = 0x00;
    wire[9] = 0x00;
    wire[10] = 0x01;
    wire[11] = 0x00;
    // Re-encode the CRC to match the modified header (so length check fires before CRC)
    // Actually we rely on kBadLength before CRC; adjust CRC to avoid kBadCrc first
    const auto result = decode_frame(wire.data(), wire.size());
    EXPECT_EQ(result.status, FrameDecodeStatus::kBadLength);
}

TEST(FrameDecode, MaxPayload)
{
    const std::vector<uint8_t> payload(kMaxPayloadLen, 0xAB);
    const auto wire = encode_frame(msg_type::kSnapshotChunk, kFlagIsLastChunk, 7, payload);
    const auto result = decode_frame(wire.data(), wire.size());
    EXPECT_EQ(result.status, FrameDecodeStatus::kOk);
    EXPECT_EQ(result.frame.payload.size(), static_cast<std::size_t>(kMaxPayloadLen));
}

TEST(FrameDecode, TrailingDataDoesNotAffectDecode)
{
    const std::vector<uint8_t> payload = {1, 2, 3};
    auto wire = encode_frame(msg_type::kHandshake, 0, 10, payload);
    wire.push_back(0xFF);  // trailing garbage
    wire.push_back(0xFF);

    const auto result = decode_frame(wire.data(), wire.size());
    EXPECT_EQ(result.status, FrameDecodeStatus::kOk);
    EXPECT_EQ(result.bytes_consumed, static_cast<uint32_t>(kHeaderSize + payload.size()));
}
