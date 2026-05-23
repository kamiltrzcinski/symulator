#include <gtest/gtest.h>

#include <server/frame.hpp>

using namespace server;

// ── encode / decode round-trip ───────────────────────────────────────────────

TEST(Frame, RoundTripWithPayload)
{
    const std::vector<uint8_t> payload = {0xDE, 0xAD, 0xBE, 0xEF};
    auto wire = encode_frame(msg_type::kCommand, kFlagIsLastChunk, 42, payload);

    ASSERT_GE(wire.size(), kHeaderSize + payload.size());

    auto result = decode_frame(wire.data(), wire.size());
    EXPECT_EQ(result.status, FrameDecodeStatus::kOk);
    EXPECT_EQ(result.frame.msg_type, msg_type::kCommand);
    EXPECT_EQ(result.frame.flags, kFlagIsLastChunk);
    EXPECT_EQ(result.frame.seq_id, 42u);
    EXPECT_EQ(result.frame.payload, payload);
    EXPECT_EQ(result.bytes_consumed, wire.size());
}

TEST(Frame, RoundTripEmptyPayload)
{
    auto wire = encode_frame(msg_type::kHeartbeat, 0x00, 1, std::vector<uint8_t>{});

    auto result = decode_frame(wire.data(), wire.size());
    EXPECT_EQ(result.status, FrameDecodeStatus::kOk);
    EXPECT_EQ(result.frame.msg_type, msg_type::kHeartbeat);
    EXPECT_TRUE(result.frame.payload.empty());
}

// ── CRC corruption detected ──────────────────────────────────────────────────

TEST(Frame, CorruptCrcRejected)
{
    const std::vector<uint8_t> payload = {0x01};
    auto wire = encode_frame(msg_type::kDomainEvent, 0x00, 7, payload);

    // Flip a bit in the CRC field (bytes 12-15).
    wire[12] ^= 0xFF;

    auto result = decode_frame(wire.data(), wire.size());
    EXPECT_EQ(result.status, FrameDecodeStatus::kBadCrc);
}

// ── Partial data: NeedMoreData ───────────────────────────────────────────────

TEST(Frame, TruncatedBufferNeedsMoreData)
{
    auto wire = encode_frame(msg_type::kHeartbeat, 0x00, 0, std::vector<uint8_t>{});

    // Provide only 8 bytes — not enough for the full 16-byte header.
    auto result = decode_frame(wire.data(), 8);
    EXPECT_EQ(result.status, FrameDecodeStatus::kNeedMoreData);
}

// ── CRC-32 idempotency ───────────────────────────────────────────────────────

TEST(Frame, Crc32IsDeterministic)
{
    const std::vector<uint8_t> data = {0x53, 0x52, 0x01, 0x00, 0x00};
    const uint32_t a = crc32_iso_hdlc(data.data(), data.size());
    const uint32_t b = crc32_iso_hdlc(data.data(), data.size());
    EXPECT_EQ(a, b);
    // CRC-32 of any non-empty data must be non-zero (statistically certain).
    EXPECT_NE(a, 0u);
}
