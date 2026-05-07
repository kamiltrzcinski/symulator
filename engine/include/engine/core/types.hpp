#pragma once

#include <compare>
#include <cstdint>
#include <functional>
#include <string>

// ── Domain value types and enumerations ─────────────────────────────────────
// This header defines the vocabulary types shared across all engine modules.
// No external dependencies beyond the C++20 standard library.

namespace engine::core
{

// ── Strong identifier wrappers ───────────────────────────────────────────────
// Thin wrappers around std::string that prevent accidental mixing of different
// ID kinds at compile time. Comparable with <=> and hashable via std::hash
// specialisations at the bottom of this file.

struct GID
{
    std::string value;
    auto operator<=>(const GID&) const = default;
};

struct SID
{
    std::string value;
    auto operator<=>(const SID&) const = default;
};

struct DispatchAreaID
{
    std::string value;
    auto operator<=>(const DispatchAreaID&) const = default;
};

struct PlayerID
{
    std::string value;
    auto operator<=>(const PlayerID&) const = default;
};

// ── Command priority ─────────────────────────────────────────────────────────
// Determines the bucket order inside PriorityCommandQueue.
// Lower numeric value = higher priority; ordering is intentional.
// Do not change numeric values without updating PriorityCommandQueue::kBucketCount.

enum class CommandPriority : std::uint8_t
{
    EMERGENCY = 0,   // Emergency route cancellation, STP, forced stop
    SAFETY = 1,      // Signal -> STOP, track block, interlocking lock
    NORMAL = 2,      // Route setting, switch change, signal -> PROCEED
    BACKGROUND = 3,  // State queries, reconnect sync requests
};

// ── Track and device state enumerations ─────────────────────────────────────

enum class TrackOccupancy : std::uint8_t
{
    FREE,
    OCCUPIED,
};

enum class SwitchPosition : std::uint8_t
{
    STRAIGHT,
    DIVERGENT,
    MOVING,  // Transitional — switch machine is operating
};

// ── Signal aspects ────────────────────────────────────────────────────────────
// Canonical list matching data/device_types/semafor.json (DVT-GLB-SEM-0000001).
// Individual semaphore type definitions will reference a subset of these values.

enum class SignalAspect : std::uint8_t
{
    S1_STOP,
    S2_PROCEED,
    S3_PROCEED_40,
    S4_PROCEED_40_EXPECT_STOP,
    S5_EXPECT_STOP,
    S6_PROCEED_100,
    S7_PROCEED_100_EXPECT_STOP,
    S8_PROCEED_100_EXPECT_40,
    S9_PROCEED_100_EXPECT_60,
    S10_PROCEED_40,
    S11_PROCEED_40_EXPECT_40,
    S12_PROCEED_60,
    S13_PROCEED_60_EXPECT_60,
    MS2_SHUNTING_ALLOWED,
};

// ── Random event types ────────────────────────────────────────────────────────
// Used by IRandomEventSource (see docs/03-initial-architecture.md).
// The engine dispatches these as first-class commands at EMERGENCY priority.
// New values can be appended without modifying engine dispatch logic.

enum class RandomEventType : std::uint8_t
{
    TRACK_CLOSURE,        // Section must be taken out of service
    INFRASTRUCTURE_FAIL,  // Switch or signal hardware failure
    MEDICAL_EMERGENCY,    // Ambulance required at station
    POLICE_CALL,          // Police required at station
    RECOVERY_TRAIN,       // Recovery/rescue train must be dispatched
    PASSENGER_ALARM,      // Passenger emergency stop pulled
};

}  // namespace engine::core

// ── std::hash specialisations ────────────────────────────────────────────────
// Required for use as keys in std::unordered_map / std::unordered_set.

template<>
struct std::hash<engine::core::GID>
{
    std::size_t operator()(const engine::core::GID& id) const noexcept
    {
        return std::hash<std::string>{}(id.value);
    }
};

template<>
struct std::hash<engine::core::SID>
{
    std::size_t operator()(const engine::core::SID& id) const noexcept
    {
        return std::hash<std::string>{}(id.value);
    }
};

template<>
struct std::hash<engine::core::DispatchAreaID>
{
    std::size_t operator()(const engine::core::DispatchAreaID& id) const noexcept
    {
        return std::hash<std::string>{}(id.value);
    }
};

template<>
struct std::hash<engine::core::PlayerID>
{
    std::size_t operator()(const engine::core::PlayerID& id) const noexcept
    {
        return std::hash<std::string>{}(id.value);
    }
};
