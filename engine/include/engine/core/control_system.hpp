#pragma once

#include "command.hpp"
#include "state_view.hpp"
#include "track_model.hpp"
#include "types.hpp"

#include <optional>
#include <string>
#include <variant>
#include <vector>

// ── IControlSystem — universal interlocking interface ────────────────────────
// Each supported SRK (System Sterowania Ruchem Kolejowym) is compiled into a
// separate shared library (libsrk_ebilock, libsrk_ml8, …) that provides a
// concrete implementation of IControlSystem.
//
// The interface is *stateless*: IControlSystem never holds mutable world state.
// Instead it reads the world via IStateView (passed on each call) and returns
// a list of DeviceStateChange values that the ENGINE thread applies atomically.
//
// Threading: all methods are called exclusively on the ENGINE thread.
// IControlSystem implementations must not spawn threads or do I/O.
//
// See docs/17-control-system-interface.md for the full contract.

namespace engine::core
{

// ── DeviceStateChange variant ────────────────────────────────────────────────
// Every observable mutation the interlocking can produce.
// The ENGINE applies changes via std::visit(StateApplier, change) —
// no system_id branches anywhere in the engine core.

struct SignalAspectChange
{
    GID gid;
    SignalAspect new_aspect;
    ChangeCause cause;
    std::optional<GID> route_id;
};

struct SwitchPositionChange
{
    GID gid;
    SwitchPosition new_position;
    ChangeCause cause;
    int moving_ticks_remaining = 0;  // >0 only when new_position == MOVING
};

struct SwitchLocked
{
    GID switch_gid;
    GID route_id;
};

struct SwitchUnlocked
{
    GID switch_gid;
    GID route_id;
};

struct DerailerStateChange
{
    GID gid;
    DerailerState new_state;
    ChangeCause cause;
    std::optional<GID> route_id;
};

struct BlockSectionStateChange
{
    GID gid;
    BlockSectionState new_state;
};

struct BlockDirectionChange
{
    GID gid;
    BlockDirectionState new_direction;
    bool requires_neighbor_confirmation = false;  // true after BLW (waiting for BLP)
};

struct AxleCounterResetChange
{
    GID gid;
    OperatorTargetKind target_kind;
};

struct OperatorCommandStateChange
{
    GID gid;
    OperatorTargetKind target_kind;
    OperatorCommandCode code;
    bool active = true;
};

struct Ml8CommandStateChange
{
    GID gid;
    OperatorTargetKind target_kind;
    Ml8CommandCode code;
    bool active = true;
};

struct RouteAdded
{
    RouteState route;
};

struct RouteRemoved
{
    GID route_id;
    std::string reason;  // "OPERATOR_CANCEL" | "TRAIN_CLEARED" | "TIMEOUT" | "FORCE"
};

struct AlarmRaised
{
    AlarmState alarm;
};

struct AlarmCleared
{
    GID alarm_id;
};

using DeviceStateChange =
    std::variant<SignalAspectChange, SwitchPositionChange, SwitchLocked, SwitchUnlocked,
                 DerailerStateChange, BlockSectionStateChange, BlockDirectionChange,
                 AxleCounterResetChange, OperatorCommandStateChange, Ml8CommandStateChange,
                 RouteAdded, RouteRemoved, AlarmRaised, AlarmCleared>;

// ── InterlockingViolation ────────────────────────────────────────────────────
// Returned by check_command when the command is rejected.

struct InterlockingViolation
{
    std::uint8_t reason_code;  // matches COMMAND_NAK reason codes from doc 09
    std::string reason_text;   // human-readable for logging / dispatcher display
    GID offending_gid;         // the device that caused the violation (may be empty)
};

// ── IControlSystem ───────────────────────────────────────────────────────────
class IControlSystem
{
public:
    virtual ~IControlSystem() = default;

    // Unique identifier for this SRK type (matches meta.json "control_system" value).
    virtual ControlSystemID system_id() const = 0;

    // Returns nullopt if the command is safe to execute.
    // Returns InterlockingViolation if any interlocking rule is violated.
    // Must not modify any state.  Called on ENGINE thread.
    virtual std::optional<InterlockingViolation> check_command(const IStateView& state,
                                                               const Command& cmd) const = 0;

    // Executes the command and returns the resulting state changes.
    // Called only after check_command returned nullopt.
    // Must not read EngineState directly — all reads go through IStateView.
    virtual std::vector<DeviceStateChange> execute_command(const IStateView& state,
                                                           const Command& cmd) = 0;

    // Called once per ENGINE tick.  Used for time-driven transitions
    // (e.g. EEA-4 switch machine timing, SHL-12 timeouts, alarm auto-clear).
    virtual std::vector<DeviceStateChange> on_tick(const IStateView& state, uint64_t tick) = 0;

    // Returns the set of command type names handled by this system.
    // Used by CommandIngress to route commands to the correct SRK library.
    virtual std::vector<std::string> supported_command_types() const = 0;
};

}  // namespace engine::core
