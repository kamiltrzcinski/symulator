// server/src/frame.cpp

#include "server/frame.hpp"

#include <array>
#include <cstring>

namespace server
{

// ── CRC-32/ISO-HDLC ──────────────────────────────────────────────────────────
// Reflected algorithm, poly 0xEDB88320 (bit-reversal of 0x04C11DB7).
// init = 0xFFFFFFFF, final XOR = 0xFFFFFFFF.

namespace
{

// Process bytes into a running CRC without applying the final XOR.
// Call with init_crc = 0xFFFFFFFF to start; chain calls by passing the
// previous return value.
uint32_t crc32_update(uint32_t crc, const uint8_t* data, std::size_t len) noexcept
{
    // Lookup table built once via IIFE; C++11 function-local static
    // guarantees thread-safe one-time initialisation — no ready flag needed.
    static const auto table = []() noexcept {
        std::array<uint32_t, 256> t{};
        for (uint32_t i = 0; i < 256; ++i)
        {
            uint32_t c = i;
            for (int k = 0; k < 8; ++k)
                c = (c & 1u) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
            t[i] = c;
        }
        return t;
    }();
    for (std::size_t i = 0; i < len; ++i)
        crc = table[(crc ^ data[i]) & 0xFFu] ^ (crc >> 8);
    return crc;
}

}  // anonymous namespace

uint32_t crc32_iso_hdlc(const uint8_t* data, std::size_t len) noexcept
{
    return crc32_update(0xFFFFFFFFu, data, len) ^ 0xFFFFFFFFu;
}

// ── encode_frame ─────────────────────────────────────────────────────────────
// CRC is computed over header bytes [0..11] + payload; not over CRC field itself.

std::vector<uint8_t> encode_frame(uint8_t msg_type_byte, uint8_t flags, uint32_t seq_id,
                                  const uint8_t* payload, uint32_t payload_len)
{
    std::vector<uint8_t> out(kHeaderSize + payload_len);

    // ── Header ──────────────────────────────────────────────────────────────
    out[0] = kMagicByte0;
    out[1] = kMagicByte1;
    out[2] = msg_type_byte;
    out[3] = flags;
    // seq_id (LE)
    out[4] = static_cast<uint8_t>(seq_id & 0xFFu);
    out[5] = static_cast<uint8_t>((seq_id >> 8) & 0xFFu);
    out[6] = static_cast<uint8_t>((seq_id >> 16) & 0xFFu);
    out[7] = static_cast<uint8_t>((seq_id >> 24) & 0xFFu);
    // payload_len (LE)
    out[8] = static_cast<uint8_t>(payload_len & 0xFFu);
    out[9] = static_cast<uint8_t>((payload_len >> 8) & 0xFFu);
    out[10] = static_cast<uint8_t>((payload_len >> 16) & 0xFFu);
    out[11] = static_cast<uint8_t>((payload_len >> 24) & 0xFFu);

    // ── Payload copy ─────────────────────────────────────────────────────────
    if (payload && payload_len > 0)
        std::memcpy(out.data() + kHeaderSize, payload, payload_len);

    // ── CRC over header[0..11] + payload ─────────────────────────────────────
    uint32_t crc = crc32_update(0xFFFFFFFFu, out.data(), 12);
    if (payload_len > 0)
        crc = crc32_update(crc, out.data() + kHeaderSize, payload_len);
    crc ^= 0xFFFFFFFFu;

    // CRC (LE)
    out[12] = static_cast<uint8_t>(crc & 0xFFu);
    out[13] = static_cast<uint8_t>((crc >> 8) & 0xFFu);
    out[14] = static_cast<uint8_t>((crc >> 16) & 0xFFu);
    out[15] = static_cast<uint8_t>((crc >> 24) & 0xFFu);

    return out;
}

// ── decode_frame ─────────────────────────────────────────────────────────────

FrameDecodeResult decode_frame(const uint8_t* data, std::size_t available) noexcept
{
    FrameDecodeResult result{};

    if (available < kHeaderSize)
    {
        result.status = FrameDecodeStatus::kNeedMoreData;
        return result;
    }

    // Magic check
    if (data[0] != kMagicByte0 || data[1] != kMagicByte1)
    {
        result.status = FrameDecodeStatus::kBadMagic;
        result.bytes_consumed = 0;
        return result;
    }

    // payload_len (LE)
    const uint32_t payload_len =
        static_cast<uint32_t>(data[8]) | (static_cast<uint32_t>(data[9]) << 8) |
        (static_cast<uint32_t>(data[10]) << 16) | (static_cast<uint32_t>(data[11]) << 24);

    if (payload_len > kMaxPayloadLen)
    {
        result.status = FrameDecodeStatus::kBadLength;
        result.bytes_consumed = 0;
        return result;
    }

    const uint32_t total = kHeaderSize + payload_len;
    if (available < total)
    {
        result.status = FrameDecodeStatus::kNeedMoreData;
        return result;
    }

    // CRC verification: header[0..11] + payload (NOT including CRC bytes [12..15])
    const uint32_t expected_crc =
        static_cast<uint32_t>(data[12]) | (static_cast<uint32_t>(data[13]) << 8) |
        (static_cast<uint32_t>(data[14]) << 16) | (static_cast<uint32_t>(data[15]) << 24);

    uint32_t actual_crc = crc32_update(0xFFFFFFFFu, data, 12);
    if (payload_len > 0)
        actual_crc = crc32_update(actual_crc, data + kHeaderSize, payload_len);
    actual_crc ^= 0xFFFFFFFFu;

    if (actual_crc != expected_crc)
    {
        result.status = FrameDecodeStatus::kBadCrc;
        result.bytes_consumed = 0;
        return result;
    }

    // seq_id (LE)
    const uint32_t seq_id = static_cast<uint32_t>(data[4]) | (static_cast<uint32_t>(data[5]) << 8) |
                            (static_cast<uint32_t>(data[6]) << 16) |
                            (static_cast<uint32_t>(data[7]) << 24);

    result.status = FrameDecodeStatus::kOk;
    result.frame.msg_type = data[2];
    result.frame.flags = data[3];
    result.frame.seq_id = seq_id;
    result.frame.payload.assign(data + kHeaderSize, data + kHeaderSize + payload_len);
    result.bytes_consumed = total;
    return result;
}

}  // namespace server
