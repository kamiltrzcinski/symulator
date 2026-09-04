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

// ── Universal UID ────────────────────────────────────────────────────────────
// 48-bit structured integer; bits 63-48 reserved (must be 0).
// Layout: DOMAIN[47:40] | KIND[39:32] | SCOPE[31:16] | INSTANCE[15:0]
// Full specification: docs/uid_legend.md

struct UID
{
    std::uint64_t value = 0;
    auto operator<=>(const UID&) const = default;
};

// ── PlayerID — kept as string for Steam integration ──────────────────────────

struct PlayerID
{
    std::string value;
    auto operator<=>(const PlayerID&) const = default;
};

enum class UIDDomain : std::uint8_t
{
    ROLLING_STOCK = 0x01,
    INFRASTRUCTURE = 0x02,
    OPERATIONS = 0x03,
};

enum class UIDKind : std::uint8_t
{
    // ROLLING_STOCK (0x01–0x0F)
    VEHICLE_TYPE = 0x01,
    VEHICLE = 0x02,
    TRAIN_CONSIST = 0x03,
    CARRIER = 0x04,

    // INFRASTRUCTURE (0x11–0x1F)
    STATION = 0x11,
    DISPATCH_AREA = 0x12,
    TRACK_SECTION = 0x13,
    SWITCH = 0x14,
    SIGNAL = 0x15,
    DERAILER = 0x16,
    BLOCK_SECTION = 0x17,
    BOUNDARY_NODE = 0x18,
    LEVEL_CROSSING = 0x19,
    AXLE_COUNTER = 0x1A,
    INTERLOCKING = 0x1B,
    POWER_SUPPLY = 0x1C,

    // OPERATIONS (0x21–0x2F)
    ROUTE = 0x21,
    ALARM = 0x22,
    DISPATCH_EXCHANGE = 0x23,
    TIMETABLE_POINT = 0x24,
    TIMETABLE_CONNECTION = 0x25,
};

constexpr std::uint64_t UID_MAX_SAFE_JSON_INTEGER = (1ULL << 53) - 1ULL;

[[nodiscard]] constexpr UID make_uid(UIDDomain domain, UIDKind kind, std::uint16_t scope,
                                     std::uint16_t instance) noexcept
{
    return UID{(static_cast<std::uint64_t>(domain) << 40) |
               (static_cast<std::uint64_t>(kind) << 32) |
               (static_cast<std::uint64_t>(scope) << 16) | static_cast<std::uint64_t>(instance)};
}

[[nodiscard]] constexpr UIDDomain uid_domain(UID uid) noexcept
{
    return static_cast<UIDDomain>((uid.value >> 40) & 0xFFU);
}

[[nodiscard]] constexpr UIDKind uid_kind(UID uid) noexcept
{
    return static_cast<UIDKind>((uid.value >> 32) & 0xFFU);
}

[[nodiscard]] constexpr std::uint16_t uid_scope(UID uid) noexcept
{
    return static_cast<std::uint16_t>((uid.value >> 16) & 0xFFFFU);
}

[[nodiscard]] constexpr std::uint16_t uid_instance(UID uid) noexcept
{
    return static_cast<std::uint16_t>(uid.value & 0xFFFFU);
}

[[nodiscard]] constexpr bool uid_is_safe_json_integer(UID uid) noexcept
{
    return uid.value <= UID_MAX_SAFE_JSON_INTEGER;
}

[[nodiscard]] constexpr bool uid_has_kind(UID uid, UIDDomain domain, UIDKind kind) noexcept
{
    return uid_is_safe_json_integer(uid) && uid_domain(uid) == domain && uid_kind(uid) == kind;
}

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
    TRAILED_DAMAGED, // Hardware damaged due to run-through
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
    MS1_STOP,              // Shunting signal — stop (Ms1)
    MS2_SHUNTING_ALLOWED,  // Shunting signal — manoeuvre allowed (Ms2)
};

// ── Random event types ────────────────────────────────────────────────────────
// Used by IRandomEventSource (see docs/ARCHITECTURE.md).
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
// See docs/ARCHITECTURE.md.

enum class TrainCategory : std::uint8_t
{
    PASSENGER,    // Passenger service (IC, TLK, regional, …)
    FREIGHT,      // Freight / goods train
    MAINTENANCE,  // Infrastructure maintenance or recovery train
};

// Operational condition of a traction-capable vehicle.
// Applied to LOCOMOTIVE and EMU/DMU MOTOR instances.
enum class TractionStatus : std::uint8_t
{
    OPERATIONAL,
    DEFECTIVE,
};

// ── Dispatch form types (S-forms) ────────────────────────────────────────────
// Formal dispatch exchange forms between neighbouring LCS.
// Full state machine documented in docs/ARCHITECTURE.md.

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
    S76,  // Free-form dispatch message / remarks
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

// State of one dispatch S-form exchange.
// Transitions documented in docs/ARCHITECTURE.md.
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

// ── Cause of a state change ───────────────────────────────────────────────────
// Shared by domain events (SwitchPositionChanged, SignalAspectChanged, …).
// Mirrors proto/common.fbs ChangeCause; kept in C++ for engine-layer use.

enum class ChangeCause : std::uint8_t
{
    COMMAND,  // result of an explicit operator command
    AUTO,     // engine-initiated (route setting, train detection, timeout)
    TIMEOUT,  // timer-driven release
};

// ── Derailer state ────────────────────────────────────────────────────────────

enum class DerailerState : std::uint8_t
{
    LOCKED,    // default safe position — derails any passing train
    UNLOCKED,  // passage allowed
};

// ── SHL-12 block direction state ──────────────────────────────────────────────
// State of a line block section in the SHL-12 automatic block system.
// Managed by libsrk_ml8; libsrk_ebilock uses only BlockSectionState (OPEN/CLOSED).
// See docs/ARCHITECTURE.md.

enum class BlockDirectionState : std::uint8_t
{
    NEUTRAL,           // No established direction
    OUTBOUND_PENDING,  // BLW sent; waiting for BLP from neighbour
    OUTBOUND,          // Direction established — this station may dispatch trains
    INBOUND_PENDING,   // Neighbour BLW received; this station must respond with BLP
    INBOUND,           // Neighbour is dispatching trains toward this station
    EMERGENCY,         // Emergency procedure active (BLAI issued)
    RESET_PENDING,     // Axle-counter reset in progress (SLI sent; waiting for SLK)
};

// ── Block section state (blok liniowy) ───────────────────────────────────────

enum class BlockSectionState : std::uint8_t
{
    CLOSED,  // blocked — no entry permitted
    OPEN,    // free to enter
};

// ── PIP (Train Identification Panel) types ───────────────────────────────────
// These types are shared between the ENGINE (producer) and PIP_WRITER (consumer).
// See docs/ARCHITECTURE.md for the full PIP threading model and
// pip.track_state schema.

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
    UID section_uid;
    UID station_uid;  // INFRASTRUCTURE/STATION UID of the owning LCS
    TrackOccupancy occupancy;
    // Present when a train enters a section or a number is assigned/removed.
    // nullopt when the section becomes free without a known train number.
    std::optional<TrainSlot> slot;
    // True when this event represents a train crossing an LCS boundary.
    // PIP_WRITER updates pip.track_state for the target section only.
    // EDR is independent — session.edr_entries is never touched by PIP_WRITER.
    bool lcs_boundary_crossing = false;
};

// Read-only view of one active train, deep-copied into EngineSnapshot every
// tick and serialised as proto::TrainState for clients (snapshot + delta).
struct TrainSnapshot
{
    UID uid;
    UID section_uid;
    float speed_kmh = 0.0f;
    float total_length_m = 0.0f;
    int total_axles = 0;
    std::vector<UID> vehicle_uids;
};

}  // namespace engine::core

// ── std::hash specialisations ────────────────────────────────────────────────
// Required for use as keys in std::unordered_map / std::unordered_set.

template<>
struct std::hash<engine::core::UID>
{
    std::size_t operator()(const engine::core::UID& id) const noexcept
    {
        return std::hash<std::uint64_t>{}(id.value);
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
