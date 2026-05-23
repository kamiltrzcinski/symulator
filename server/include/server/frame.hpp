// server/include/server/frame.hpp
// Wire frame encode / decode for the binary transport layer.
//
// Frame layout (16-byte header):
//   Offset  Size  Field
//    0       2    magic  = { 0x53, 0x52 } ('S','R')
//    2       1    msg_type
//    3       1    flags  (bit 0 = IS_LAST_CHUNK)
//    4       4    seq_id  (uint32 LE, per-sender monotonic)
//    8       4    payload_len  (uint32 LE, ≤ 65535)
//   12       4    crc32  (CRC-32/ISO-HDLC over header bytes [0..11] + payload)
//   16       N    payload
//
// See docs/09-communication-contract.md — "Frame format".

#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace server
{

// ── Constants ────────────────────────────────────────────────────────────────

constexpr uint8_t kMagicByte0 = 0x53;  // 'S'
constexpr uint8_t kMagicByte1 = 0x52;  // 'R'
constexpr uint32_t kHeaderSize = 16;
constexpr uint32_t kMaxPayloadLen = 65535;

// Flags byte — bit 0
constexpr uint8_t kFlagIsLastChunk = 0x01;

// Message types (msg_type byte)
namespace msg_type
{
constexpr uint8_t kHandshake = 0x01;
constexpr uint8_t kHandshakeAck = 0x02;
constexpr uint8_t kHeartbeat = 0x03;
constexpr uint8_t kHeartbeatAck = 0x04;
constexpr uint8_t kCommand = 0x10;
constexpr uint8_t kCommandAck = 0x11;
constexpr uint8_t kCommandNak = 0x12;
constexpr uint8_t kDomainEvent = 0x20;
constexpr uint8_t kSnapshotRequest = 0x30;
constexpr uint8_t kSnapshotChunk = 0x31;
constexpr uint8_t kSessionNotice = 0x40;
constexpr uint8_t kTakeoverRequest = 0x50;
constexpr uint8_t kTakeoverResponse = 0x51;
constexpr uint8_t kChatMessage = 0x60;
constexpr uint8_t kBilateral = 0x61;
constexpr uint8_t kVoiceChanJoin = 0x70;
constexpr uint8_t kVoiceChanLeave = 0x71;
constexpr uint8_t kVoiceChanState = 0x72;
}  // namespace msg_type

// ── CRC-32/ISO-HDLC (= standard Ethernet CRC32) ──────────────────────────────
// Poly 0x04C11DB7, reflected, init 0xFFFFFFFF, final XOR 0xFFFFFFFF.
// Same algorithm as zlib crc32 / PNG / Ethernet.

uint32_t crc32_iso_hdlc(const uint8_t* data, std::size_t len) noexcept;

// ── Encoding ─────────────────────────────────────────────────────────────────

/// Build a complete framed message.
/// @param msg_type  Message type byte.
/// @param flags     Flags byte (0 or kFlagIsLastChunk).
/// @param seq_id    Per-sender monotonic sequence number.
/// @param payload   FlatBuffers payload bytes (may be empty).
/// @returns         Complete wire bytes including 16-byte header.
std::vector<uint8_t> encode_frame(uint8_t msg_type, uint8_t flags, uint32_t seq_id,
                                  const uint8_t* payload, uint32_t payload_len);

inline std::vector<uint8_t> encode_frame(uint8_t msg_type, uint8_t flags, uint32_t seq_id,
                                         const std::vector<uint8_t>& payload)
{
    return encode_frame(msg_type, flags, seq_id, payload.empty() ? nullptr : payload.data(),
                        static_cast<uint32_t>(payload.size()));
}

// ── Decoding ─────────────────────────────────────────────────────────────────

enum class FrameDecodeStatus
{
    kOk,            // Full frame parsed successfully.
    kNeedMoreData,  // Not enough bytes yet; caller should buffer and retry.
    kBadMagic,      // First two bytes do not match magic.
    kBadLength,     // payload_len exceeds kMaxPayloadLen.
    kBadCrc,        // CRC mismatch.
};

struct DecodedFrame
{
    uint8_t msg_type = 0;
    uint8_t flags = 0;
    uint32_t seq_id = 0;
    std::vector<uint8_t> payload;  // FlatBuffers bytes only (no header)
};

struct FrameDecodeResult
{
    FrameDecodeStatus status = FrameDecodeStatus::kNeedMoreData;
    DecodedFrame frame;       // valid only when status == kOk
    uint32_t bytes_consumed;  // bytes to advance the read cursor
};

/// Attempt to parse one frame from the front of the buffer.
/// Does NOT advance any pointer — returns bytes_consumed to let the caller do it.
FrameDecodeResult decode_frame(const uint8_t* data, std::size_t available) noexcept;

}  // namespace server
