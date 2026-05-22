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
    ROUTE,
    LEVEL_CROSSING,
    INTERLOCKING_COMPUTER,
    POWER_SUPPLY,
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
    BPZ,
    DPWI,
    DPW,
    BTW,
    BTO,
    BPO,
    BKO,
    PDZ,
    PDO,
    PZT,
    PAZ,
    PZO,
    PDII,
    PDI,
    MRS,
    ODS,
    ZAL,
    POC,
    PZA,
    PZAI,
    PZW,
    SSO,
    SSS,
    UPAI,
    UPA,
    UPAO,
    UPN,
    UPO,
    ZSO,
    ZSS,
};

struct OperatorCommandCmd
{
    GID target_gid;
    OperatorTargetKind target_kind;
    OperatorCommandCode code;
};

enum class Ml8CommandCode : std::uint8_t
{
    AK,
    AZK,
    BLZ,
    BLZC,
    DOP,
    DOPS,
    DPZ,
    HMI,
    KRA,
    KSR,
    LKA,
    LOFF,
    MAN,
    NPU,
    NPW,
    NPZ,
    OGI,
    OP,
    OPO,
    OSTOP,
    OTB,
    OTE,
    OTEYYY,
    OTP,
    OTPON,
    OT,
    OTZ,
    OUZ,
    OUZ_DR,
    OUZ_DZ,
    OUZ_JN,
    OUZ_PJ,
    OUZ_X,
    OUZ_ZN,
    OWBL,
    OZCZ,
    P,
    POC,
    POT,
    PZK,
    PPN,
    PPZ,
    PZ,
    PZB,
    PZS,
    SPEC,
    STJ,
    STOJ,
    STOP,
    SZ,
    NSZ,
    WBL,
    WPN,
    WPZ,
    WZ,
    ZCZ,
    ZDM,
    ZDP,
    ZEROLO,
    ZI,
    ZO,
    ZPO,
    ZW,
    ZWBL,
    Z,
    Z_DR,
    Z_DZ,
    Z_JN,
    Z_PJ,
    Z_X,
    Z_ZN,
};

struct Ml8CommandCmd
{
    GID target_gid;
    OperatorTargetKind target_kind;
    Ml8CommandCode code;
};

// ── Variant ──────────────────────────────────────────────────────────────────

using Command =
    std::variant<SetSwitchPositionCmd, SetSignalAspectCmd, SetDerailerPositionCmd,
                 SetBlockSectionCmd, RequestRouteCmd, CancelRouteCmd, AcknowledgeAlarmCmd,
                 SetBlockDirectionCmd, InitAxleCounterResetCmd, ResetAxleCounterCmd,
                 OperatorCommandCmd, Ml8CommandCmd>;

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
