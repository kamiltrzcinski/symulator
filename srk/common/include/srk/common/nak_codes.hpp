#pragma once

#include <cstdint>

// ── COMMAND_NAK reason codes ──────────────────────────────────────────────────
// Wire-protocol reason codes sent in the COMMAND_NAK frame (0x12).
// Source of truth: docs/ARCHITECTURE.md §COMMAND_NAK
//
// Used by: srk::common (device_rules), srk::ebilock, srk::ml8.
// Must not be defined locally in each translation unit.

namespace srk::common
{

constexpr uint8_t NAK_UNSPECIFIED = 0x00;     // Generic / unclassified failure
constexpr uint8_t NAK_NOT_FOUND = 0x01;       // Requested device does not exist
constexpr uint8_t NAK_SAFETY_BLOCK = 0x02;    // Safety interlock prevents action
constexpr uint8_t NAK_INVALID_STATE = 0x03;   // Device in wrong state for command
constexpr uint8_t NAK_ROUTE_LOCKED = 0x04;    // Device route-locked
constexpr uint8_t NAK_NO_PATH = 0x05;         // No valid route path found
constexpr uint8_t NAK_SWITCH_MOVING = 0x06;   // Switch machine currently in motion
constexpr uint8_t NAK_UNSUPPORTED = 0x07;     // Command not supported by this system
constexpr uint8_t NAK_UNAUTHORIZED = 0x08;    // Operator does not hold ownership
constexpr uint8_t NAK_SESSION_PAUSED = 0x09;  // Session is paused; commands blocked

}  // namespace srk::common
