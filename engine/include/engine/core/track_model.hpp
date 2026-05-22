#pragma once

#include "types.hpp"

#include <optional>
#include <string>
#include <vector>

// ── Track topology and runtime device state model ────────────────────────────
// Mirrors the JSON schema defined in scenarios/reference/*/topology.json and
// scenarios/reference/*/objects.json.
//
// Structs are intentionally value-types (copyable) so that EngineSnapshot can
// hold a deep copy of the whole topology at a given tick without pointer chasing.
//
// Static fields (gid, pid, sid, type_id, …) are populated once during scenario
// load and never change.  Runtime fields (occupancy, position, aspect, …) are
// mutated by the engine on each tick.

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
    bool ml8_command_active = false;
    std::string last_ml8_command_code;
};

// ── Connection port on the end of a track section ────────────────────────────
// Exactly one of it_id / iz_id is present per side (boundary ↔ section counter
// for sections that touch a boundary node, or section ↔ switch counter for
// sections adjacent to a switch).  The other field is left empty.
struct TrackPort
{
    GID neighbor_gid;  // GID of the adjacent node (TrackSection, Switch, or BoundaryNode)
    GID counter_gid;   // IT (boundary counter) or IZ (switch counter) — one must be present
    enum class CounterKind : std::uint8_t
    {
        IT,
        IZ
    } counter_kind = CounterKind::IT;
    std::vector<GID> signal_gids;  // signals facing trains entering from this side
};

// ── Track section (tor szlakowy / tor stacyjny) ───────────────────────────────
struct TrackSection
{
    // ── Static ──
    GID gid;
    std::string pid;   // local pretty-ID used in UI and dispatch forms
    SID sid;           // LCS that owns / supervises this section
    TrackPort side_a;  // one end
    TrackPort side_b;  // other end
    float length_m = 0.0f;
    bool electrified = false;
    int max_speed_kmh = 0;

    // ── Runtime ──
    TrackOccupancy occupancy = TrackOccupancy::FREE;
    int axle_count = 0;            // live axle-counter reading
    std::optional<GID> train_gid;  // GID of the train currently on this section (if known)
    OperatorCommandRuntimeState operator_state;
};

// ── Switch leg (connection port on a switch) ─────────────────────────────────
struct SwitchLeg
{
    GID neighbor_gid;              // adjacent track section or boundary node
    GID iz_gid;                    // IZ counter on this connection
    std::vector<GID> signal_gids;  // signals facing trains entering from this leg
};

// ── Switch (rozjazd) ─────────────────────────────────────────────────────────
struct Switch
{
    // ── Static ──
    GID gid;
    std::string pid;
    SID sid;
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
    std::optional<GID> locked_by_route;  // non-empty when a route locks this switch
    int moving_ticks_remaining = 0;      // countdown while position == MOVING
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
    GID gid;
    std::string pid;
    SID sid;
    std::string type_id;
    Type type = Type::BLOCK;
    GID governs_track_section_gid;  // the section behind this signal face

    // ── Runtime ──
    SignalAspect current_aspect = SignalAspect::S1_STOP;
    std::optional<GID> locked_by_route;  // non-empty when route display is active
    OperatorCommandRuntimeState operator_state;
};

// ── Derailer (wykolejnica) ───────────────────────────────────────────────────
struct Derailer
{
    // ── Static ──
    GID gid;
    std::string pid;
    SID sid;
    std::string type_id;
    GID guards_track_section_gid;  // the section this derailer protects

    // ── Runtime ──
    DerailerState state = DerailerState::LOCKED;
    std::optional<GID> locked_by_route;
    OperatorCommandRuntimeState operator_state;
};

// ── Block section (blok liniowy SHL-12) ──────────────────────────────────────
// Spans the line between two neighbouring LCS boundary points.
// The EbiLock system uses state (OPEN/CLOSED); ML8 uses direction additionally.
struct BlockSection
{
    // ── Static ──
    GID gid;
    std::string pid;
    SID sid;           // this station's SID
    SID neighbor_sid;  // the neighbouring station
    int line_number = 0;
    GID departure_signal_gid;             // outbound signal at this end
    GID entry_signal_gid;                 // inbound signal facing arriving trains
    std::vector<GID> szlak_section_gids;  // track sections forming the block

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
    GID route_id;                   // synthetic unique ID (e.g. "RTE-{from}-{to}-{tick}")
    GID from_signal_gid;            // entry signal
    GID to_signal_gid;              // exit signal
    std::vector<GID> section_gids;  // ordered list of locked track sections
    std::vector<GID> switch_gids;   // ordered list of locked switches
    std::vector<GID> derailer_gids;
    uint64_t created_tick = 0;
    bool train_entered = false;  // true once axle counter detects entry; triggers automatic release
};

// ── Active alarm ─────────────────────────────────────────────────────────────
struct AlarmState
{
    GID alarm_id;      // unique ID for this alarm instance
    std::string kind;  // e.g. "SWITCH_FAILURE", "SIGNAL_FAILURE", "AXLE_COUNTER_FAIL"
    GID object_gid;    // the device that raised the alarm
    std::string message;
    uint64_t timestamp_us = 0;
};

// ── Boundary node (węzeł graniczny) ─────────────────────────────────────────
// Terminal node in the topology — represents the edge of the local LCS area.
// A TrackSection or Switch leg that crosses into the neighbouring LCS references
// a BoundaryNode as its neighbor.
struct BoundaryNode
{
    GID gid;
    std::string pid;
    SID sid;
    std::string description;  // e.g. "boundary to Sopot (south)"
};

}  // namespace engine::core
