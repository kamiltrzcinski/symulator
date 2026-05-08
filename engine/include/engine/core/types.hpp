#pragma once

#include <compare>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

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

// ── Train category ────────────────────────────────────────────────────────────
// Top-level classification of a train definition.
// Determines which S-form dispatch path applies and the default icon.
// Matches fleet.train_definitions.train_category in the database.
// See docs/15-dispatch-forms.md.

enum class TrainCategory : std::uint8_t
{
    PASSENGER,    // Passenger service (IC, TLK, regional, …)
    FREIGHT,      // Freight / goods train
    MAINTENANCE,  // Infrastructure maintenance or recovery train
};

// ── Dispatch form types (S-forms / Zapowiedniowiec) ───────────────────────────
// Formal bilateral exchange forms between neighbouring LCS.
// Full state machine documented in docs/15-dispatch-forms.md.

enum class DispatchFormType : std::uint8_t
{
    S2,   // Dispatch request (A → B): "Is the line clear?"
    S24,  // Line-clear reply (B → A): "Line clear, train may depart"
    S25,  // Departure notification (A → B): "Train N has departed"
    S26,  // Arrival confirmation (B → A): "Train N has arrived"
    S35,  // Cancellation request (A → B): withdraw previous S2
    S51,  // Level-crossing notification (A → B): km markers + estimated time
    S52,  // Acknowledgement of S51 (B → A)
    S55,  // Dispatch request for dangerous-goods trains (replaces S2)
    S56,  // Line-clear reply for dangerous-goods trains (replaces S24)
    S76,  // Free-form bilateral message / remarks
};

enum class TelegramDirection : std::uint8_t
{
    SENT,
    RECEIVED,
};

enum class TelegramStatus : std::uint8_t
{
    PENDING,     // Sent, awaiting acknowledgement
    CONFIRMED,   // Acknowledged by the receiving LCS
    REJECTED,    // Rejected (e.g. duplicate droga_wolna, wrong state)
    SUPERSEDED,  // Cancelled by a subsequent S35 or new exchange
};

// State of one bilateral S-form exchange.
// Transitions documented in docs/15-dispatch-forms.md.
enum class ExchangeStatus : std::uint8_t
{
    IDLE,          // No active exchange; initial state
    S2_SENT,       // S2 (or S55) sent; awaiting S24 (or S56)
    S24_RECEIVED,  // Line clear granted; train may depart
    S25_SENT,      // Departure notification sent; awaiting S26
    S26_RECEIVED,  // Arrival confirmed; exchange complete
    CLOSED,        // Exchange successfully concluded
    CANCELLED,     // Exchange withdrawn via S35
};

// ── PIP (Train Identification Panel) types ───────────────────────────────────
// These types are shared between the ENGINE (producer) and PIP_WRITER (consumer).
// See docs/03-initial-architecture.md and docs/11-database-model.md for the
// full PIP threading model and pip.track_state schema.

// Direction from which a train entered a track section.
// Used by TrainSlot to determine placement of the '*' suffix in the alternating
// display mode (two trains present on one station track).
enum class EntrySide : std::uint8_t
{
    LEFT,
    RIGHT,
};

// One train occupying (or awaiting on) a track section.
// Stored as a vector inside pip.track_state; order is insertion order.
// Invariant: number is at most 6 characters (digits/letters); the optional 7th
// character '*' is a client-side display artefact in alternating mode.
struct TrainSlot
{
    std::string number;            // ≤6 chars
    bool has_extra_info = false;   // show '!' badge (black on yellow)
    bool manually_placed = false;  // client blinks; awaiting train arrival
    EntrySide entry_side = EntrySide::LEFT;

    bool operator==(const TrainSlot&) const = default;
};

// Event pushed by ENGINE into EventQueue<PipEvent> on every track-occupancy or
// train-number change.  PIP_WRITER is the sole consumer; no other thread reads
// this queue.
struct PipEvent
{
    GID section_gid;
    SID station_sid;  // LCS that owns this section
    TrackOccupancy occupancy;
    // Present when a train enters a section or a number is assigned/removed.
    // nullopt when the section becomes free without a known train number.
    std::optional<TrainSlot> slot;
    // True when this event represents a train crossing an LCS boundary.
    // PIP_WRITER must auto-create session.edr_entry if the train number is
    // not yet known in the target station_sid.
    bool lcs_boundary_crossing = false;
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
