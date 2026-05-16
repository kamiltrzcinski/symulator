// server/include/server/command_ingress.hpp
// Deserialises COMMAND (0x10) wire frames into engine EnvelopedCommand values.
//
// The COMMAND payload begins with a 1-byte cmd_type followed by a FlatBuffers
// body.  See docs/09-communication-contract.md — "Command payload format".

#pragma once

#include "engine/core/command.hpp"

#include <cstdint>
#include <optional>

namespace server
{

// ── CommandIngress ────────────────────────────────────────────────────────────
// Stateless parser.  parse_command() is safe to call from any thread.

class CommandIngress
{
public:
    CommandIngress() = default;

    // Parse a COMMAND-family payload.
    //
    // @param cmd_type    The first byte of the COMMAND payload (subtypes 0x01–0x0A).
    // @param fb_data     Pointer to the FlatBuffers body (payload[1..]).
    // @param fb_size     Length of the FlatBuffers body in bytes.
    // @returns           Parsed Command variant, or nullopt if cmd_type is
    //                    unknown or the buffer fails FlatBuffers verification.
    static std::optional<engine::core::Command> parse_command(uint8_t cmd_type,
                                                              const uint8_t* fb_data,
                                                              uint32_t fb_size);

    // Convenience overload for a full COMMAND payload (cmd_type byte + FB body).
    // Returns nullopt if payload is empty.
    static std::optional<engine::core::Command> parse_payload(const uint8_t* payload,
                                                              uint32_t payload_len);
};

}  // namespace server
