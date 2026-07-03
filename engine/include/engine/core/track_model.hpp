#pragma once

#include "command.hpp"
#include "types.hpp"

#include <optional>
#include <string>
#include <vector>

// ── Track topology and runtime device state model ────────────────────────────
// Mirrors the JSON schema defined in scenarios/*/topology.json and
// scenarios/*/objects.json.
//
// Structs are intentionally value-types (copyable) so that EngineSnapshot can
// hold a deep copy of the whole topology at a given tick without pointer chasing.
//
// Static fields (uid, pid, station_uid, type_id, …) are populated once during
// scenario load and never change.  Runtime fields (occupancy, position,
// aspect, …) are mutated by the engine on each tick.

namespace engine::core
{

struct OperatorCommandRuntimeState
{
    bool stopped = false;
    bool substitute_initialized = false;
    bool substitute_active = false;
    bool automatic_route_enabled = false;
    bool clamped = false;
    bool traffic_closed = false;
    bool detection_bypassed = false;
    bool special_initialized = false;
    bool special_active = false;
    bool axle_reset_initialized = false;
    std::optional<Ml8CommandCode> active_ml8_command;  // nullopt = no active ML8 command
};

// ── Connection port on the end of a track section ────────────────────────────
// Exactly one of it_id / iz_id is present per side (boundary ↔ section counter
// for sections that touch a boundary node, or section ↔ switch counter for
// sections adjacent to a switch).  The other field is left empty.
struct TrackPort
{
    UID neighbor_uid;  // UID of the adjacent node (TrackSection, Switch, or BoundaryNode)
    UID counter_uid;   // IT (boundary counter) or IZ (switch counter) — one must be present
    enum class CounterKind : std::uint8_t
    {
        IT,
        IZ
    } counter_kind = CounterKind::IT;
    std::vector<UID> signal_uids;  // signals facing trains entering from this side
};

// ── Track section (tor szlakowy / tor stacyjny) ───────────────────────────────
struct TrackSection
{
    // ── Static ──
    UID uid;
    std::string pid;   // local pretty-ID used in UI and dispatch forms
    UID station_uid;   // LCS that owns / supervises this section
    TrackPort side_a;  // one end
    TrackPort side_b;  // other end
    float length_m = 0.0f;
    bool electrified = false;
    int max_speed_kmh = 0;
    bool station_section = true;  // false = trunk-line ("szlak") section

    // ── Runtime ──
    TrackOccupancy occupancy = TrackOccupancy::FREE;
    int axle_count = 0;            // live axle-counter reading
    std::optional<UID> train_uid;  // UID of the train currently on this section (if known)
    OperatorCommandRuntimeState operator_state;
};

// ── Switch leg (connection port on a switch) ─────────────────────────────────
struct SwitchLeg
{
    UID neighbor_uid;              // adjacent track section or boundary node
    UID iz_uid;                    // IZ counter on this connection
    std::vector<UID> signal_uids;  // signals facing trains entering from this leg
};

// ── Switch (rozjazd) ─────────────────────────────────────────────────────────
struct Switch
{
    // ── Static ──
    UID uid;
    std::string pid;
    UID station_uid;
    std::string type_id;  // references data/device_types/*.json
    SwitchLeg trunk;      // stock rail end (pień)
    SwitchLeg straight;   // straight through leg
    SwitchLeg divergent;  // diverging leg
    float length_m = 0.0f;
    int max_speed_straight_kmh = 0;
    int max_speed_divergent_kmh = 0;

    // ── Runtime ──
    SwitchPosition position = SwitchPosition::STRAIGHT;
    TrackOccupancy occupancy = TrackOccupancy::FREE;
    int axle_count = 0;
    std::optional<UID> locked_by_route_uid;  // non-empty when a route locks this switch
    int moving_ticks_remaining = 0;          // countdown while position == MOVING
    OperatorCommandRuntimeState operator_state;
};

// ── Signal (semafor / tarcza) ────────────────────────────────────────────────
struct Signal
{
    enum class Type : std::uint8_t
    {
        ENTRY,      // wjazdowy
        DEPARTURE,  // wyjazdowy
        BLOCK,      // blokowy (szlakowy)
        SHUNTING,   // manewrowy (tarcza)
    };

    // ── Static ──
    UID uid;
    std::string pid;
    UID station_uid;
    std::string type_id;
    Type type = Type::BLOCK;
    UID governs_section_uid;  // the section behind this signal face

    // ── Runtime ──
    SignalAspect current_aspect = SignalAspect::S1_STOP;
    std::optional<UID> locked_by_route_uid;  // non-empty when route display is active
    OperatorCommandRuntimeState operator_state;
};

// ── Derailer (wykolejnica) ───────────────────────────────────────────────────
struct Derailer
{
    // ── Static ──
    UID uid;
    std::string pid;
    UID station_uid;
    std::string type_id;
    UID guards_section_uid;  // the section this derailer protects

    // ── Runtime ──
    DerailerState state = DerailerState::LOCKED;
    std::optional<UID> locked_by_route_uid;
    OperatorCommandRuntimeState operator_state;
};

// ── Block section (blok liniowy SHL-12) ──────────────────────────────────────
// Spans the line between two neighbouring LCS boundary points.
// The EbiLock system uses state (OPEN/CLOSED); ML8 uses direction additionally.
struct BlockSection
{
    // ── Static ──
    UID uid;
    std::string pid;
    std::string type_id;       // block system type, e.g. "SHL-12"; behaviour
                               // differentiation lives in the SRK libraries
    UID station_uid;           // this station's UID
    UID neighbor_station_uid;  // the neighbouring station
    int line_number = 0;
    UID departure_signal_uid;             // outbound signal at this end
    UID entry_signal_uid;                 // inbound signal facing arriving trains
    std::vector<UID> szlak_section_uids;  // track sections forming the block

    // ── Runtime ──
    BlockSectionState state = BlockSectionState::CLOSED;
    BlockDirectionState direction = BlockDirectionState::NEUTRAL;
    int axle_count = 0;  // aggregate axle counter for the whole block
    OperatorCommandRuntimeState operator_state;
};

// ── Route (droga przebiegu) ──────────────────────────────────────────────────
// Represents an active interlocking route between two signals.
// Created by RequestRoute and removed by CancelRoute or automatic release.
struct RouteState
{
    UID uid;                        // synthetic unique ID
    UID from_signal_uid;            // entry signal
    UID to_signal_uid;              // exit signal
    std::vector<UID> section_uids;  // ordered list of locked track sections
    std::vector<UID> switch_uids;   // ordered list of locked switches
    std::vector<UID> derailer_uids;
    uint64_t created_tick = 0;
    bool train_entered = false;  // true once axle counter detects entry; triggers automatic release
};

// ── Active alarm ─────────────────────────────────────────────────────────────
struct AlarmState
{
    UID uid;           // unique ID for this alarm instance
    std::string kind;  // e.g. "SWITCH_FAILURE", "SIGNAL_FAILURE", "AXLE_COUNTER_FAIL"
    UID object_uid;    // the device that raised the alarm
    std::string message;
    uint64_t timestamp_us = 0;
};

// ── Boundary node (węzeł graniczny) ─────────────────────────────────────────
// Terminal node in the topology — represents the edge of the local LCS area.
// A TrackSection or Switch leg that crosses into the neighbouring LCS references
// a BoundaryNode as its neighbor.
struct BoundaryNode
{
    UID uid;
    std::string pid;
    UID station_uid;
    std::string description;  // e.g. "boundary to Sopot (south)"
};

}  // namespace engine::core
