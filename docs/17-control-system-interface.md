# Control System Interface (SRK Universal Interface)

**Document:** 17  
**Status:** Implemented  
**Relates to:** [03-initial-architecture.md](03-initial-architecture.md), [09-communication-contract.md](09-communication-contract.md), [14-interlocking-model.md](14-interlocking-model.md)

---

## Purpose

This document defines the `IControlSystem` interface — the single integration point between the simulation engine and any SRK (System Sterowania Ruchem Kolejowym) implementation.

The interface decouples the engine from railway signalling standards.  Adding a new SRK type requires no changes to the engine core: a new shared library implementing `IControlSystem` is compiled and linked, and the engine creates it by name via `ControlSystemRegistry`.

---

## Module layout

```
srk/
├── common/           libsrk_common  — shared safety rules and BFS path finder
│   ├── include/srk/common/
│   │   ├── device_rules.hpp   — R1–R7 check/execute helpers
│   │   └── route_graph.hpp    — RoutePathNode, RoutePath, find_route_path()
│   └── src/
│       ├── device_rules.cpp
│       └── route_graph.cpp
├── ebilock/          libsrk_ebilock — EbiLock X4 (SZRP interlocking)
│   ├── include/srk/ebilock/ebilock_system.hpp
│   └── src/ebilock_system.cpp
└── ml8/              libsrk_ml8     — ESTW ML8 (SHL-12 block telegraph)
    ├── include/srk/ml8/ml8_system.hpp
    └── src/ml8_system.cpp
```

Tests: `tests/srk/` — `test_ebilock_interlocking.cpp`, `test_ml8_shl12.cpp`.

---

## IControlSystem

```cpp
class IControlSystem {
public:
    virtual ~IControlSystem() = default;

    // Unique string key matching meta.json "control_system" value.
    virtual ControlSystemID system_id() const = 0;

    // Returns nullopt if the command may proceed.
    // Returns InterlockingViolation if any rule is violated.
    // Must not modify any state.  Called on ENGINE thread.
    virtual std::optional<InterlockingViolation>
    check_command(const IStateView& state, const Command& cmd) const = 0;

    // Executes the command; returns the list of resulting state changes.
    // Called only after check_command returned nullopt.
    virtual std::vector<DeviceStateChange>
    execute_command(const IStateView& state, const Command& cmd) = 0;

    // Called once per ENGINE tick (20 Hz).
    // Drives time-dependent transitions: EEA-4 timers, SHL-12 timeouts, alarm auto-clear.
    virtual std::vector<DeviceStateChange>
    on_tick(const IStateView& state, uint64_t tick) = 0;

    // List of command type names accepted by this system.
    virtual std::vector<std::string> supported_command_types() const = 0;
};
```

**Threading contract:** all methods are called exclusively on the ENGINE thread.  Implementations must not spawn threads or perform I/O.

**State contract:** `IControlSystem` holds no mutable world state.  It may hold system-internal per-session state (e.g. EEA-4 countdown counters, SHL-12 pending-direction records), but all world state reads go through `IStateView`.

---

## IStateView

Pure read-only view of the live world state.  Passed by const-ref to every `IControlSystem` call.

```cpp
class IStateView {
public:
    virtual ~IStateView() = default;

    virtual const TrackSection* find_section    (const GID&) const noexcept = 0;
    virtual const Switch*       find_switch     (const GID&) const noexcept = 0;
    virtual const Signal*       find_signal     (const GID&) const noexcept = 0;
    virtual const Derailer*     find_derailer   (const GID&) const noexcept = 0;
    virtual const BlockSection* find_block      (const GID&) const noexcept = 0;
    virtual const RouteState*   find_route      (const GID&) const noexcept = 0;
    virtual const AlarmState*   find_alarm      (const GID&) const noexcept = 0;
    virtual const BoundaryNode* find_boundary   (const GID&) const noexcept = 0;

    virtual void for_each_section (std::function<void(const TrackSection&)>) const = 0;
    virtual void for_each_switch  (std::function<void(const Switch&)>)        const = 0;
    virtual void for_each_signal  (std::function<void(const Signal&)>)        const = 0;
    virtual void for_each_route   (std::function<void(const RouteState&)>)    const = 0;

    virtual const std::string& session_id()    const noexcept = 0;
    virtual uint64_t           current_tick()  const noexcept = 0;
};
```

Pointers returned by `find_*` are valid only for the duration of the current call.  `IControlSystem` must not cache them across calls.

Two implementations exist:

| Type | Used by |
|------|---------|
| `EngineState`    | ENGINE thread (live mutable state) |
| `EngineSnapshot` | any other thread; read-only deep copy; published via `AtomicSnapshot` |

---

## DeviceStateChange

Every observable mutation an SRK system may produce:

```cpp
using DeviceStateChange = std::variant<
    SignalAspectChange,      // signal aspect updated
    SwitchPositionChange,    // switch moving or snapped to position
    SwitchLocked,            // switch locked by a route
    SwitchUnlocked,          // switch unlocked when route is removed
    DerailerStateChange,     // derailer locked/unlocked
    BlockSectionStateChange, // block section OPEN / CLOSED
    BlockDirectionChange,    // SHL-12 direction state updated
    RouteAdded,              // new RouteState created
    RouteRemoved,            // RouteState removed
    AlarmRaised,             // new AlarmState created
    AlarmCleared>;           // AlarmState removed
```

The ENGINE applies changes via `std::visit(StateApplier, change)`.  There are no `system_id` branches anywhere in the engine core.

---

## InterlockingViolation

Returned by `check_command` when a rule is violated:

```cpp
struct InterlockingViolation {
    uint8_t     reason_code;   // matches COMMAND_NAK reason codes (doc 09)
    std::string reason_text;   // human-readable, included in COMMAND_NAK payload
    GID         offending_gid; // the device that caused the violation
};
```

NAK reason codes used by SRK implementations:

| Code | Name            | Meaning |
|------|-----------------|---------|
| 0x01 | NOT_FOUND       | object GID not in state |
| 0x02 | SAFETY_BLOCK    | interlocking rule violated |
| 0x03 | INVALID_STATE   | device is in a state that rejects this command |
| 0x04 | ROUTE_LOCKED    | a route locks the device |
| 0x05 | NO_PATH         | BFS found no route between entry and exit signals |
| 0x06 | SWITCH_MOVING   | switch is currently in MOVING state |
| 0x07 | UNSUPPORTED     | command type not handled by this SRK |

---

## ControlSystemRegistry

Singleton factory.  Maps `ControlSystemID` strings → `IControlSystem` factory functions.

```cpp
class ControlSystemRegistry {
public:
    static ControlSystemRegistry& instance();
    void register_system(ControlSystemID, FactoryFn);
    std::unique_ptr<IControlSystem> create(const ControlSystemID&) const;
    bool has(const ControlSystemID&) const;
    static bool register_static(ControlSystemID, FactoryFn);  // for static-init use
};
```

Each SRK library self-registers at static-initialisation time:

```cpp
// inside ebilock_system.cpp
static const bool kRegistered = ControlSystemRegistry::register_static(
    ControlSystemID{"ebilock_x4"},
    [] { return std::make_unique<EbiLockSystem>(); });
```

The engine calls `ControlSystemRegistry::instance().create(system_id)` at session start, where `system_id` comes from `meta.json → "control_system"`.

**Linking requirement:** any binary that needs a given SRK type must link that library.  Static initialisation of an unlinked library will not happen, so its system ID will not appear in the registry.

---

## AtomicSnapshot

Lock-free snapshot publication for cross-thread reads.

```cpp
class AtomicSnapshot {
public:
    // Called by ENGINE after each tick — publishes a new immutable snapshot.
    void publish(std::shared_ptr<const EngineSnapshot> snap);

    // Called from any thread (IO_POOL, WORK_POOL, …) — returns the most recent snapshot.
    std::shared_ptr<const EngineSnapshot> load() const;
};
```

`EngineSnapshot` is a deep copy of all world-state maps plus `session_id` and `tick`.  It implements `IStateView`, so it can be passed directly to `IControlSystem` for read-only inspection (e.g. from WORK_POOL before enqueuing a command for validation pre-checks).

---

## Implemented SRK systems

### EbiLock X4 (`"ebilock_x4"`)

Used by scenarios with `"control_system": "ebilock_x4"` in `meta.json`.

Rules implemented: R1 (switch throw), R2 (signal aspect), R3 (derailer unlock), R5 (route set), R7 (alarm acknowledge).

Time-driven behavior:
- **EEA-4 timer** — after `SetSwitchPosition` is executed, the switch enters `MOVING` state for a configurable number of ticks (default: 90 ticks = 4.5 s at 20 Hz) before snapping to the target position.
- **Route auto-release** — once a train has entered and fully cleared all sections of a route (`train_entered = true`, all sections `FREE`), the route is automatically released.

Does **not** handle `SetBlockDirectionCmd`, `InitAxleCounterResetCmd`, or `ResetAxleCounterCmd` — returns NAK 0x07 (`UNSUPPORTED`) for those.

### ESTW ML8 (`"estw_ml8"`)

Used by scenarios with `"control_system": "estw_ml8"` in `meta.json`.

Implements everything `ebilock_x4` does, plus the full **SHL-12 block telegraph** state machine:

| Operation | Transition | Condition |
|-----------|------------|-----------|
| BLW  | `NEUTRAL → OUTBOUND_PENDING` | block must be NEUTRAL |
| BLP  | `OUTBOUND_PENDING → OUTBOUND + OPEN` | from either side |
|      | `INBOUND_PENDING → INBOUND + OPEN`   | |
| BLO  | `OUTBOUND_PENDING → NEUTRAL` | cancel pending request |
| BLZ  | `OUTBOUND / INBOUND → NEUTRAL + CLOSED` | axle_count must be 0 |
| BLAI | any → `EMERGENCY` | rejected in RESET_PENDING |
| BLA  | `EMERGENCY → NEUTRAL + CLOSED` | |
| OPS  | `EMERGENCY / RESET_PENDING → NEUTRAL + CLOSED` | |

Axle counter reset (SHL-12):

| Command | Transition | Condition |
|---------|------------|-----------|
| SLI (`InitAxleCounterResetCmd`) | `NEUTRAL → RESET_PENDING` | |
| SLK (`ResetAxleCounterCmd`)     | `RESET_PENDING → NEUTRAL + CLOSED, axle_count = 0` | |

---

## BFS route path finder (`srk::common`)

`find_route_path(state, from_gid, to_gid)` performs breadth-first search on the topology graph to find a path between two signals (entry and exit).

- **Nodes:** `TrackSection`, `Switch`, `BoundaryNode`
- **Directionality:** tracked via `came_from` to determine which switch leg is used
- `required_switch_position(switch, prev_node, next_node)` → `STRAIGHT | DIVERGENT`
- Returns a `RoutePath` (ordered list of `RoutePathNode` with type + gid + required switch position) or `std::nullopt` if no path exists

`make_route_id(from_gid, to_gid)` produces a deterministic route GID string from the two signal GIDs.

---

## Scenario binding

The `meta.json` file for each scenario includes a `"control_system"` field:

```json
{
  "schema_version": 1,
  "scenario_id": "gdynia_orlowo_reference",
  "control_system": "ebilock_x4",
  ...
}
```

The server reads this field at session load time and calls `ControlSystemRegistry::instance().create({"ebilock_x4"})`.

---

## Open questions

| ID | Question | Priority |
|----|----------|----------|
| Q-SRK-1 | Phase 7 (engine integration): wire `IControlSystem` into the ENGINE tick loop — `StateApplier` visitor, session-load `create()`, per-tick `on_tick()`, per-command `check_command()` + `execute_command()`. | High |
| Q-SRK-2 | `CancelRoute` (R6) is specified in doc 14 but not yet implemented in `device_rules.cpp`. | Medium |
| Q-SRK-3 | `tick_route_auto_release` re-locks derailers on route removal — currently noted as "handled by engine" since the helper lacks full derailer context. | Low |
| Q-SRK-4 | `requires_neighbor_confirmation = true` in `BlockDirectionChange` after BLW is set but the confirmation protocol (waiting for the neighbour station's BLP) is not yet modelled in the multi-station session layer. | Medium |
