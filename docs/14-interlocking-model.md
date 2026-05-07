# Interlocking Model

**Document:** 14  
**Status:** Draft  
**Relates to:** [09-communication-contract.md](09-communication-contract.md), [08-track-topology-model.md](08-track-topology-model.md), [12-server-api.md](12-server-api.md)

---

## Purpose

This document defines the **safety rules** (interlocking logic) that the simulation engine enforces before executing any operator command.  
These rules mirror real Polish railway signalling practice (PKP Instrukcja Ie-1/Ie-4) at a level sufficient for realistic dispatcher training.

Interlocking is implemented inside the engine as a pure function:

```cpp
// Returns std::nullopt on success, or a NakReason + description on failure.
std::optional<InterlockingViolation>
    check_command(const EngineState& state, const Command& cmd);
```

The result maps directly to `COMMAND_NAK` (reason code 0x02 `SAFETY_BLOCK`).

---

## Topology model recap

The interlocking operates on the graph described in [doc 08](08-track-topology-model.md):

| Node type        | Can be occupied | Can be locked | Commands that affect it |
|------------------|:-:|:-:|---|
| `TRACK_SECTION`  | ✓ | — | none (passive) |
| `SWITCH`         | ✓ | ✓ (by route) | `SetSwitchPosition`, `RequestRoute` |
| `SIGNAL`         | — | — | `SetSignalAspect`, `RequestRoute` |
| `DERAILER`       | — | ✓ | `SetDerailerPosition` |
| `BLOCK_BOUNDARY` | — | — | `SetBlockSection` |

---

## Rule catalogue

### R1 — SetSwitchPosition

A switch may be moved **only if all of the following hold**:

| # | Condition | Reason code if violated |
|---|-----------|------------------------|
| 1 | No train axle detected on the switch node (`occupied == false`). | `SAFETY_BLOCK` |
| 2 | The switch is **not locked by an active route** (`locked_by_route == false`). | `SAFETY_BLOCK` |
| 3 | The requesting client owns the posterunek that governs this switch. | `UNAUTHORIZED` |
| 4 | The switch is not already in the requested position. | `INVALID_STATE` |
| 5 | The switch is not currently moving (`position != MOVING`). | `INVALID_STATE` |

### R2 — SetSignalAspect (manual override)

A signal aspect may be set manually **only if**:

| # | Condition | Reason code if violated |
|---|-----------|------------------------|
| 1 | Client owns the posterunek governing this signal. | `UNAUTHORIZED` |
| 2 | Requested aspect is not more permissive than the current route permits. A signal with no route set may only receive `STOP` or `OFF`. | `SAFETY_BLOCK` |
| 3 | Signal is not currently overridden by a locked route. | `SAFETY_BLOCK` |

> **Note:** Manual aspect override is intended for malfunction recovery, not for normal operation. `RequestRoute` is the primary mechanism.

### R3 — SetDerailerPosition (UNLOCKED → passage allowed)

Unlocking a derailer is permitted **only if**:

| # | Condition | Reason code if violated |
|---|-----------|------------------------|
| 1 | Client owns the posterunek. | `UNAUTHORIZED` |
| 2 | No active route passes through the derailer. | `SAFETY_BLOCK` |
| 3 | The section immediately beyond the derailer is free. | `SAFETY_BLOCK` |

### R4 — SetBlockSection (CLOSED → OPEN)

Opening a closed block section is permitted **only if**:

| # | Condition | Reason code if violated |
|---|-----------|------------------------|
| 1 | Client owns a posterunek at either boundary station. | `UNAUTHORIZED` |
| 2 | No train currently occupies any section within the block. | `SAFETY_BLOCK` |
| 3 | Both boundary signals are at `STOP`. | `SAFETY_BLOCK` |

### R5 — RequestRoute

Route setting is the most safety-critical command.  The engine executes it atomically:

#### Phase 1 — Path resolution

1. Identify the entry signal (`from_signal_g_id`) and exit signal (`to_signal_g_id`).
2. Find a route in the topology graph (breadth-first search on the track graph, switches as directed edges).
3. If no path exists → `UNKNOWN_OBJECT`.

#### Phase 2 — Conflict check (all conditions must pass)

| # | Condition | Reason code if violated |
|---|-----------|------------------------|
| 1 | Client owns all posterunki along the route path. | `UNAUTHORIZED` |
| 2 | Every track section on the path is **free** (`occupied == false`). | `SAFETY_BLOCK` |
| 3 | Every switch on the path is **not locked by a different conflicting route**. | `SAFETY_BLOCK` |
| 4 | Every derailer on the path is in `UNLOCKED` position. | `SAFETY_BLOCK` |
| 5 | No opposing route uses any section on this path. | `SAFETY_BLOCK` |
| 6 | Entry signal is currently at `STOP`. | `INVALID_STATE` |

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

| # | Condition | Reason code if violated |
|---|-----------|------------------------|
| 1 | `route_id` exists. | `UNKNOWN_OBJECT` |
| 2 | Client owns at least one posterunek on the route. | `UNAUTHORIZED` |
| 3 | No train axle is currently detected on any section of the route. If a train is present, the operator must wait for it to clear or use the forced-cancel flag. | `SAFETY_BLOCK` |

On success:
1. Set entry signal to `STOP`.
2. Unlock all switches (`locked_by_route = false`).
3. Delete the `RouteState` record.
4. Emit `RouteReleased` (reason = `OPERATOR_CANCEL`).

### R7 — AcknowledgeAlarm

No interlocking checks — always accepted if the alarm exists and the client is in the same session.  
`UNKNOWN_OBJECT` if `alarm_id` not found.

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
| Verify prerequisites (R1–R7) | **Interlocking** (this doc) |
| Move switches physically over time | **Physics engine** |
| Detect train presence via axle counters | **Physics engine** |
| Track signal aspect display | **Client renderer** (reads `SignalState` from snapshot/events) |
| Enforce speed limits after aspect | **Physics engine** (driver AI) |

---

## Open questions

| ID | Question | Priority |
|----|----------|----------|
| Q-ILK-1 | Should a forced-cancel (`force = true` flag on `CancelRoute`) be added to handle trains stopped on a route? | Medium |
| Q-ILK-2 | How to handle partial route clearing (axle-count-based release section-by-section)? Required for long freight trains. | High |
| Q-ILK-3 | Should the engine enforce speed limits per signal aspect (S1 ≈ line speed, SH1 ≤ 40 km/h)? | High |
