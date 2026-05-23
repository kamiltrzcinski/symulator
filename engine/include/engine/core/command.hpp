#pragma once

#include "types.hpp"

#include <cstdint>
#include <string_view>
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

// ── Command-code name helpers ─────────────────────────────────────────────────
// Return the short ASCII code name for logging and diagnostics.
// Returns an empty string_view for out-of-range values.

namespace engine::core
{

constexpr std::string_view operator_command_code_name(OperatorCommandCode code) noexcept
{
    switch (code)
    {
        case OperatorCommandCode::SES:   return "SES";
        case OperatorCommandCode::SEO:   return "SEO";
        case OperatorCommandCode::SZI:   return "SZI";
        case OperatorCommandCode::SZW:   return "SZW";
        case OperatorCommandCode::SZN:   return "SZN";
        case OperatorCommandCode::SZO:   return "SZO";
        case OperatorCommandCode::SAM:   return "SAM";
        case OperatorCommandCode::SAW:   return "SAW";
        case OperatorCommandCode::ZWP:   return "ZWP";
        case OperatorCommandCode::ZWM:   return "ZWM";
        case OperatorCommandCode::ZWS:   return "ZWS";
        case OperatorCommandCode::ZWO:   return "ZWO";
        case OperatorCommandCode::ITS:   return "ITS";
        case OperatorCommandCode::ITO:   return "ITO";
        case OperatorCommandCode::ZWB:   return "ZWB";
        case OperatorCommandCode::ZBP:   return "ZBP";
        case OperatorCommandCode::ZBM:   return "ZBM";
        case OperatorCommandCode::ZRI:   return "ZRI";
        case OperatorCommandCode::ZRK:   return "ZRK";
        case OperatorCommandCode::OPS:   return "OPS";
        case OperatorCommandCode::SLI:   return "SLI";
        case OperatorCommandCode::SLK:   return "SLK";
        case OperatorCommandCode::BLW:   return "BLW";
        case OperatorCommandCode::BLP:   return "BLP";
        case OperatorCommandCode::BLO:   return "BLO";
        case OperatorCommandCode::BLZ:   return "BLZ";
        case OperatorCommandCode::BLAI:  return "BLAI";
        case OperatorCommandCode::BLA:   return "BLA";
        case OperatorCommandCode::BLS:   return "BLS";
        case OperatorCommandCode::OST:   return "OST";
        case OperatorCommandCode::ZKB:   return "ZKB";
        case OperatorCommandCode::PZM:   return "PZM";
        case OperatorCommandCode::OZK:   return "OZK";
        case OperatorCommandCode::ZESI:  return "ZESI";
        case OperatorCommandCode::ZES:   return "ZES";
        case OperatorCommandCode::PZZI:  return "PZZI";
        case OperatorCommandCode::PZZ:   return "PZZ";
        case OperatorCommandCode::POZ:   return "POZ";
        case OperatorCommandCode::DPOI:  return "DPOI";
        case OperatorCommandCode::DPO:   return "DPO";
        case OperatorCommandCode::DKOI:  return "DKOI";
        case OperatorCommandCode::DKO:   return "DKO";
        case OperatorCommandCode::DKPI:  return "DKPI";
        case OperatorCommandCode::DKP:   return "DKP";
        case OperatorCommandCode::BPZ:   return "BPZ";
        case OperatorCommandCode::DPWI:  return "DPWI";
        case OperatorCommandCode::DPW:   return "DPW";
        case OperatorCommandCode::BTW:   return "BTW";
        case OperatorCommandCode::BTO:   return "BTO";
        case OperatorCommandCode::BPO:   return "BPO";
        case OperatorCommandCode::BKO:   return "BKO";
        case OperatorCommandCode::PDZ:   return "PDZ";
        case OperatorCommandCode::PDO:   return "PDO";
        case OperatorCommandCode::PZT:   return "PZT";
        case OperatorCommandCode::PAZ:   return "PAZ";
        case OperatorCommandCode::PZO:   return "PZO";
        case OperatorCommandCode::PDII:  return "PDII";
        case OperatorCommandCode::PDI:   return "PDI";
        case OperatorCommandCode::MRS:   return "MRS";
        case OperatorCommandCode::ODS:   return "ODS";
        case OperatorCommandCode::ZAL:   return "ZAL";
        case OperatorCommandCode::POC:   return "POC";
        case OperatorCommandCode::PZA:   return "PZA";
        case OperatorCommandCode::PZAI:  return "PZAI";
        case OperatorCommandCode::PZW:   return "PZW";
        case OperatorCommandCode::SSO:   return "SSO";
        case OperatorCommandCode::SSS:   return "SSS";
        case OperatorCommandCode::UPAI:  return "UPAI";
        case OperatorCommandCode::UPA:   return "UPA";
        case OperatorCommandCode::UPAO:  return "UPAO";
        case OperatorCommandCode::UPN:   return "UPN";
        case OperatorCommandCode::UPO:   return "UPO";
        case OperatorCommandCode::ZSO:   return "ZSO";
        case OperatorCommandCode::ZSS:   return "ZSS";
    }
    return {};
}

constexpr std::string_view ml8_command_code_name(Ml8CommandCode code) noexcept
{
    switch (code)
    {
        case Ml8CommandCode::AK:      return "AK";
        case Ml8CommandCode::AZK:     return "AZK";
        case Ml8CommandCode::BLZ:     return "BLZ";
        case Ml8CommandCode::BLZC:    return "BLZC";
        case Ml8CommandCode::DOP:     return "DOP";
        case Ml8CommandCode::DOPS:    return "DOPS";
        case Ml8CommandCode::DPZ:     return "DPZ";
        case Ml8CommandCode::HMI:     return "HMI";
        case Ml8CommandCode::KRA:     return "KRA";
        case Ml8CommandCode::KSR:     return "KSR";
        case Ml8CommandCode::LKA:     return "LKA";
        case Ml8CommandCode::LOFF:    return "LOFF";
        case Ml8CommandCode::MAN:     return "MAN";
        case Ml8CommandCode::NPU:     return "NPU";
        case Ml8CommandCode::NPW:     return "NPW";
        case Ml8CommandCode::NPZ:     return "NPZ";
        case Ml8CommandCode::OGI:     return "OGI";
        case Ml8CommandCode::OP:      return "OP";
        case Ml8CommandCode::OPO:     return "OPO";
        case Ml8CommandCode::OSTOP:   return "OSTOP";
        case Ml8CommandCode::OTB:     return "OTB";
        case Ml8CommandCode::OTE:     return "OTE";
        case Ml8CommandCode::OTEYYY:  return "OTEYYY";
        case Ml8CommandCode::OTP:     return "OTP";
        case Ml8CommandCode::OTPON:   return "OTPON";
        case Ml8CommandCode::OT:      return "OT";
        case Ml8CommandCode::OTZ:     return "OTZ";
        case Ml8CommandCode::OUZ:     return "OUZ";
        case Ml8CommandCode::OUZ_DR:  return "OUZ_DR";
        case Ml8CommandCode::OUZ_DZ:  return "OUZ_DZ";
        case Ml8CommandCode::OUZ_JN:  return "OUZ_JN";
        case Ml8CommandCode::OUZ_PJ:  return "OUZ_PJ";
        case Ml8CommandCode::OUZ_X:   return "OUZ_X";
        case Ml8CommandCode::OUZ_ZN:  return "OUZ_ZN";
        case Ml8CommandCode::OWBL:    return "OWBL";
        case Ml8CommandCode::OZCZ:    return "OZCZ";
        case Ml8CommandCode::P:       return "P";
        case Ml8CommandCode::POC:     return "POC";
        case Ml8CommandCode::POT:     return "POT";
        case Ml8CommandCode::PZK:     return "PZK";
        case Ml8CommandCode::PPN:     return "PPN";
        case Ml8CommandCode::PPZ:     return "PPZ";
        case Ml8CommandCode::PZ:      return "PZ";
        case Ml8CommandCode::PZB:     return "PZB";
        case Ml8CommandCode::PZS:     return "PZS";
        case Ml8CommandCode::SPEC:    return "SPEC";
        case Ml8CommandCode::STJ:     return "STJ";
        case Ml8CommandCode::STOJ:    return "STOJ";
        case Ml8CommandCode::STOP:    return "STOP";
        case Ml8CommandCode::SZ:      return "SZ";
        case Ml8CommandCode::NSZ:     return "NSZ";
        case Ml8CommandCode::WBL:     return "WBL";
        case Ml8CommandCode::WPN:     return "WPN";
        case Ml8CommandCode::WPZ:     return "WPZ";
        case Ml8CommandCode::WZ:      return "WZ";
        case Ml8CommandCode::ZCZ:     return "ZCZ";
        case Ml8CommandCode::ZDM:     return "ZDM";
        case Ml8CommandCode::ZDP:     return "ZDP";
        case Ml8CommandCode::ZEROLO:  return "ZEROLO";
        case Ml8CommandCode::ZI:      return "ZI";
        case Ml8CommandCode::ZO:      return "ZO";
        case Ml8CommandCode::ZPO:     return "ZPO";
        case Ml8CommandCode::ZW:      return "ZW";
        case Ml8CommandCode::ZWBL:    return "ZWBL";
        case Ml8CommandCode::Z:       return "Z";
        case Ml8CommandCode::Z_DR:    return "Z_DR";
        case Ml8CommandCode::Z_DZ:    return "Z_DZ";
        case Ml8CommandCode::Z_JN:    return "Z_JN";
        case Ml8CommandCode::Z_PJ:    return "Z_PJ";
        case Ml8CommandCode::Z_X:     return "Z_X";
        case Ml8CommandCode::Z_ZN:    return "Z_ZN";
    }
    return {};
}

}  // namespace engine::core
