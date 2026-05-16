# Interlocking Model

**Document:** 14  
**Status:** Draft  
**Relates to:** [09-communication-contract.md](09-communication-contract.md), [08-track-topology-model.md](08-track-topology-model.md), [12-server-api.md](12-server-api.md)

---

## Purpose

This document defines the **safety rules** (interlocking logic) that the simulation engine enforces before executing any operator command.  
These rules mirror real Polish railway signalling practice (PKP Instrukcja Ie-1/Ie-4) at a level sufficient for realistic dispatcher training.

Interlocking is implemented as a method on `IControlSystem`, the universal SRK interface:

```cpp
// Returns std::nullopt on success, or an InterlockingViolation on failure.
std::optional<InterlockingViolation>
    check_command(const IStateView& state, const Command& cmd) const;
```

The violation maps directly to `COMMAND_NAK`.  See [doc 17](17-control-system-interface.md) for the full interface contract.

Shared logic (R1–R7) lives in `libsrk_common` (`srk/common/device_rules.hpp`).  Each SRK library (`libsrk_ebilock`, `libsrk_ml8`) delegates to these helpers and adds system-specific rules.

---

## Topology model recap

The interlocking operates on the graph described in [doc 08](08-track-topology-model.md):

| Node type        | Can be occupied | Can be locked | Commands that affect it |
|------------------|:-:|:-:|---|
| `TRACK_SECTION`  | ✓ | — | none (passive) |
| `SWITCH`         | ✓ | ✓ (by route) | `SetSwitchPosition`, `RequestRoute` |
| `SIGNAL`         | — | — | `SetSignalAspect`, `RequestRoute` |
| `DERAILER`       | — | ✓ | `SetDerailerPosition` |
| `BLOCK_SECTION`  | ✓ (axle count) | — | `SetBlockSection`, `SetBlockDirection`, `InitAxleCounterReset`, `ResetAxleCounter` |
| `BOUNDARY_NODE`  | — | — | — (topology endpoint only) |

---

## Rule catalogue

### R1 — SetSwitchPosition

A switch may be moved **only if all of the following hold**:

| # | Condition | NAK code |
|---|-----------|----------|
| 1 | No train axle detected on the switch node (`occupied == false`). | `SAFETY_BLOCK` (0x02) |
| 2 | The switch is **not locked by an active route** (`locked_by_route == false`). | `ROUTE_LOCKED` (0x04) |
| 3 | The requesting client owns the posterunek that governs this switch. | `UNAUTHORIZED` (0x08) |
| 4 | The switch is not already in the requested position. | `INVALID_STATE` (0x03) |
| 5 | The switch is not currently moving (`position != MOVING`). | `SWITCH_MOVING` (0x06) |

### R2 — SetSignalAspect (manual override)

A signal aspect may be set manually **only if**:

| # | Condition | NAK code |
|---|-----------|----------|
| 1 | Client owns the posterunek governing this signal. | `UNAUTHORIZED` (0x08) |
| 2 | Requested aspect is not more permissive than the current route permits. A signal with no route set may only receive `S1_STOP` or `MS1_STOP`. | `SAFETY_BLOCK` (0x02) |
| 3 | Signal is not currently overridden by a locked route. | `SAFETY_BLOCK` (0x02) |

> **Note:** Manual aspect override is intended for malfunction recovery, not for normal operation. `RequestRoute` is the primary mechanism.

### R3 — SetDerailerPosition (UNLOCKED → passage allowed)

Unlocking a derailer is permitted **only if**:

| # | Condition | NAK code |
|---|-----------|----------|
| 1 | Client owns the posterunek. | `UNAUTHORIZED` (0x08) |
| 2 | No active route passes through the derailer. | `ROUTE_LOCKED` (0x04) |
| 3 | The section immediately beyond the derailer is free. | `SAFETY_BLOCK` (0x02) |

### R4 — SetBlockSection (CLOSED → OPEN)

Opening a closed block section is permitted **only if**:

| # | Condition | NAK code |
|---|-----------|----------|
| 1 | Client owns a posterunek at either boundary station. | `UNAUTHORIZED` (0x08) |
| 2 | No train currently occupies any section within the block. | `SAFETY_BLOCK` (0x02) |
| 3 | Both boundary signals are at `S1_STOP` or `MS1_STOP`. | `SAFETY_BLOCK` (0x02) |

### R5 — RequestRoute

Route setting is the most safety-critical command.  The engine executes it atomically:

#### Phase 1 — Path resolution

1. Identify the entry signal (`from_signal_g_id`) and exit signal (`to_signal_g_id`).
2. Find a route in the topology graph (breadth-first search on the track graph, switches as directed edges).
3. If no path exists → `UNKNOWN_OBJECT`.

#### Phase 2 — Conflict check (all conditions must pass)

| # | Condition | NAK code |
|---|-----------|----------|
| 1 | Client owns all posterunki along the route path. | `UNAUTHORIZED` (0x08) |
| 2 | Every track section on the path is **free** (`occupied == false`). | `SAFETY_BLOCK` (0x02) |
| 3 | Every switch on the path is **not locked by a different conflicting route**. | `ROUTE_LOCKED` (0x04) |
| 4 | Every derailer on the path is in `UNLOCKED` position. | `SAFETY_BLOCK` (0x02) |
| 5 | No opposing route uses any section on this path. | `SAFETY_BLOCK` (0x02) |
| 6 | Entry signal is currently at `S1_STOP` or `MS1_STOP`. | `INVALID_STATE` (0x03) |

#### Phase 3 — Atomic state mutation (on success)

Executed only if all Phase 2 checks pass.  No partial mutation:

1. Lock each switch to its required position (`locked_by_route = true`).  
   If a switch is in the wrong position and is free → move it automatically.
2. Set entry signal aspect to the appropriate permissive aspect (see §Signal Aspect Selection below).
3. Create a `RouteState` record with a unique `route_id`.
4. Emit `RouteSet` domain event.
5. Emit `SwitchPositionChanged` (cause = `AUTO`) for any switches moved.
6. Emit `SignalAspectChanged` (cause = `AUTO`) for the entry signal.

#### Signal aspect selection

| Route type | Aspect granted |
|------------|---------------|
| Full entry from line, departure track free | `S1` (one white — proceed at line speed) |
| Entry from line, departure track occupied | `SH1` (shunt — proceed at max 40 km/h) |
| Shunting move within station | `SH1` |
| Departure to line, line block open | `S2` (two whites — depart) |
| Departure to line, caution (restricted speed) | `MS2` |

### R6 — CancelRoute

| # | Condition | NAK code |
|---|-----------|----------|
| 1 | `route_id` exists. | `NOT_FOUND` (0x01) |
| 2 | Client owns at least one posterunek on the route. | `UNAUTHORIZED` (0x08) |
| 3 | No train axle is currently detected on any section of the route. If a train is present, the operator must wait for it to clear or use `force = true`. | `SAFETY_BLOCK` (0x02) |

On success:
1. Set entry signal to `STOP`.
2. Unlock all switches (`locked_by_route = false`).
3. Delete the `RouteState` record.
4. Emit `RouteReleased` (reason = `OPERATOR_CANCEL`).

### R7 — AcknowledgeAlarm

No interlocking checks — always accepted if the alarm exists and the client is in the same session.  
`NOT_FOUND` (0x01) if `alarm_id` not found.

---

## SHL-12 block telegraph rules (ML8 only)

These rules apply only when the scenario's `control_system` is `"estw_ml8"`.  Commands 0x08–0x0A are rejected with NAK 0x07 (`UNSUPPORTED`) by EbiLock X4.

`BlockDirectionState` encodes the current phase of a SHL-12 block:

```
NEUTRAL            → no established direction
OUTBOUND_PENDING   → BLW sent; waiting for BLP from neighbour
OUTBOUND           → this station dispatches trains outbound
INBOUND_PENDING    → neighbour BLW received; must reply with BLP
INBOUND            → neighbour dispatches trains toward this station
EMERGENCY          → emergency change in progress (BLAI issued)
RESET_PENDING      → axle-counter reset initiated (SLI sent)
```

### R8 — SetBlockDirection (Shl12Op)

| Operation | Preconditions | NAK code | Resulting state | Side-effects |
|-----------|---------------|----------|-----------------|--------------|
| BLW  | direction == NEUTRAL | `INVALID_STATE` (0x03) | OUTBOUND_PENDING | `requires_neighbor_confirmation = true` |
| BLP  | direction == OUTBOUND_PENDING ∨ INBOUND_PENDING | `INVALID_STATE` | OUTBOUND / INBOUND + block OPEN | |
| BLO  | direction == OUTBOUND_PENDING | `INVALID_STATE` | NEUTRAL | |
| BLZ  | direction == OUTBOUND ∨ INBOUND; axle_count == 0 | `SAFETY_BLOCK` (axles≠0), `INVALID_STATE` (wrong dir) | NEUTRAL + block CLOSED | |
| BLAI | direction != RESET_PENDING | `INVALID_STATE` | EMERGENCY | |
| BLA  | direction == EMERGENCY | `INVALID_STATE` | NEUTRAL + block CLOSED | |
| OPS  | direction == EMERGENCY ∨ RESET_PENDING | `INVALID_STATE` | NEUTRAL + block CLOSED | |

### R9 — InitAxleCounterReset (SLI)

| # | Condition | NAK code |
|---|-----------|----------|
| 1 | Block direction == NEUTRAL. | `INVALID_STATE` (0x03) |

On success: direction → `RESET_PENDING`.

### R10 — ResetAxleCounter (SLK)

| # | Condition | NAK code |
|---|-----------|----------|
| 1 | Block direction == RESET_PENDING. | `INVALID_STATE` (0x03) |

On success: direction → `NEUTRAL`, axle_count reset to 0, block CLOSED.

---

## Route lifecycle

```
           ┌─────────────────────────────────────────────────────────┐
           │                     route_id = X                         │
           │                                                           │
  RequestRoute ──▶ [LOCKED] ──▶ train enters entry section            │
           │           │                                               │
           │           ▼                                               │
           │      [OCCUPIED] ──▶ train clears last section             │
           │                             │                             │
           │                             ▼                             │
           │                    [AUTO RELEASE] ──▶ RouteReleased       │
           │                       (cause=TRAIN_CLEARED)               │
           │                                                           │
           │           ┌── CancelRoute (no train present) ─────────▶  RouteReleased
           │           └── Timeout (train never entered) ───────────▶  RouteReleased
           └─────────────────────────────────────────────────────────┘
```

**Auto-release:** Once the train's last axle clears the final section on the route, the engine automatically releases the route (unlocks switches, does **not** change the exit signal).

**Timeout:** If no axle enters the entry section within `route_hold_timeout_s` (default: 120 s), the route is auto-cancelled and `RouteReleased` (reason = `TIMEOUT`) is emitted.

---

## Conflicting route detection

Two routes conflict if they share at least one track section **and** are in opposite directions, **or** both lock the same switch to different positions.

The engine maintains an in-memory `RouteIndex` (a map from section_g_id → set of route_ids that cover it) to check conflicts in O(sections_on_route) time.

---

## Interlocking vs. engine boundary

| Responsibility | Owner |
|----------------|-------|
| Verify prerequisites (R1–R10) | **IControlSystem** implementations (`libsrk_ebilock`, `libsrk_ml8`) |
| Shared rule helpers (R1–R7) | **libsrk_common** (`srk/common/device_rules.hpp`) |
| SRK factory / session binding | **ControlSystemRegistry** |
| Apply DeviceStateChange to world state | **ENGINE** (`StateApplier` visitor) |
| Move switches physically over time | **Physics engine** |
| Detect train presence via axle counters | **Physics engine** |
| Track signal aspect display | **Client renderer** (reads `SignalState` from snapshot/events) |
| Enforce speed limits after aspect | **Physics engine** (driver AI) |

---

## Open questions

| ID | Question | Priority |
|----|----------|----------|
| Q-ILK-1 | Forced-cancel (`force = true` flag) is defined in `CancelRouteCmd` but not yet enforced in `device_rules.cpp`. | Medium |
| Q-ILK-2 | How to handle partial route clearing (axle-count-based release section-by-section)? Required for long freight trains. | High |
| Q-ILK-3 | Should the engine enforce speed limits per signal aspect (S2_PROCEED ≈ line speed, MS2_SHUNTING_ALLOWED ≤ 40 km/h)? | High |
| Q-ILK-4 | R6 (CancelRoute) is specified but not yet implemented in `device_rules.cpp`. | Medium |
| Q-ILK-5 | INBOUND_PENDING state — neighbour's BLW confirmation protocol is not yet modelled in the multi-station session layer. | Medium |
