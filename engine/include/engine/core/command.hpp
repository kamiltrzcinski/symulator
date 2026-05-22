#pragma once

#include "types.hpp"

#include <cstdint>
#include <variant>

// ── Operator command payloads ────────────────────────────────────────────────
// Each struct carries the minimal information needed to describe the desired
// state change.  Ownership (player_id / dispatch_area_id) and sequencing
// (seq_id, priority, timestamp_us) are stored separately in CommandMeta and
// never duplicated inside individual command structs.
//
// Command variants are processed on the ENGINE thread via std::visit against
// IControlSystem::check_command and IControlSystem::execute_command.
//
// See docs/09-communication-contract.md for wire-level mapping and
// docs/14-interlocking-model.md for per-command interlocking rules.

namespace engine::core
{

// ── Station / device commands ────────────────────────────────────────────────

struct SetSwitchPositionCmd
{
    GID gid;
    SwitchPosition position;  // only STRAIGHT or DIVERGENT — not MOVING
};

struct SetSignalAspectCmd
{
    GID gid;
    SignalAspect aspect;
};

struct SetDerailerPositionCmd
{
    GID gid;
    DerailerState position;
};

struct SetBlockSectionCmd
{
    GID gid;
    BlockSectionState state;  // OPEN / CLOSED (EbiLock block sections)
};

struct RequestRouteCmd
{
    GID from_signal_gid;
    GID to_signal_gid;
};

struct CancelRouteCmd
{
    GID route_id;
    bool force = false;  // force-cancel even when a train is on the route
};

struct AcknowledgeAlarmCmd
{
    GID alarm_id;
};

// ── SHL-12 block direction commands (ML8 only) ────────────────────────────────
// These correspond to the SHL-12 telegraph operations documented in the ML8
// instruction manual.  See docs/14-interlocking-model.md §SHL-12.

enum class Shl12Op : std::uint8_t
{
    BLW,   // Request outbound direction (NEUTRAL → OUTBOUND_PENDING)
    BLP,   // Confirm direction — response to neighbour's BLW (→ INBOUND / OUTBOUND)
    BLO,   // Cancel pending direction request (OUTBOUND_PENDING → NEUTRAL)
    BLZ,   // Release established direction (OUTBOUND / INBOUND → NEUTRAL)
    BLAI,  // Initialise emergency direction change
    BLA,   // Execute emergency direction change
    OPS,   // Cancel special procedure
};

struct SetBlockDirectionCmd
{
    GID block_section_gid;
    Shl12Op operation;
};

struct InitAxleCounterResetCmd  // SLI
{
    GID block_section_gid;
};

struct ResetAxleCounterCmd  // SLK
{
    GID block_section_gid;
};

enum class OperatorTargetKind : std::uint8_t
{
    SIGNAL,
    SWITCH,
    DERAILER,
    TRACK_SECTION,
    BLOCK_SECTION,
    AXLE_COUNTER_SYSTEM,
    STATION,
};

enum class OperatorCommandCode : std::uint8_t
{
    SES,
    SEO,
    SZI,
    SZW,
    SZN,
    SZO,
    SAM,
    SAW,
    ZWP,
    ZWM,
    ZWS,
    ZWO,
    ITS,
    ITO,
    ZWB,
    ZBP,
    ZBM,
    ZRI,
    ZRK,
    OPS,
    SLI,
    SLK,
    BLW,
    BLP,
    BLO,
    BLZ,
    BLAI,
    BLA,
    BLS,
    OST,
    ZKB,
    PZM,
    OZK,
    ZESI,
    ZES,
    PZZI,
    PZZ,
    POZ,
    DPOI,
    DPO,
    DKOI,
    DKO,
    DKPI,
    DKP,
};

struct OperatorCommandCmd
{
    GID target_gid;
    OperatorTargetKind target_kind;
    OperatorCommandCode code;
};

// ── Variant ──────────────────────────────────────────────────────────────────

using Command =
    std::variant<SetSwitchPositionCmd, SetSignalAspectCmd, SetDerailerPositionCmd,
                 SetBlockSectionCmd, RequestRouteCmd, CancelRouteCmd, AcknowledgeAlarmCmd,
                 SetBlockDirectionCmd, InitAxleCounterResetCmd, ResetAxleCounterCmd,
                 OperatorCommandCmd>;

// ── Envelope ─────────────────────────────────────────────────────────────────
// Wraps a Command with the sequencing/identity metadata added by CommandIngress
// on the WORK_POOL thread before the command enters PriorityCommandQueue.

struct CommandMeta
{
    std::uint32_t seq_id = 0;
    CommandPriority priority = CommandPriority::NORMAL;
    PlayerID player_id;
    DispatchAreaID dispatch_area_id;
    std::uint64_t timestamp_us = 0;
};

struct EnvelopedCommand
{
    CommandMeta meta;
    Command payload;
};

}  // namespace engine::core
