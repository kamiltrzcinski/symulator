# Vehicle and Train Model

## Overview

A **train** is a `std::vector<Vehicle>` assembled at runtime from individual vehicle definitions stored in JSON. Vehicle definitions are the canonical source; train compositions reference vehicle `gID`s. This separates the fleet registry from session-specific consists.

---

## File layout

```
vehicles/
  definitions/
    et22.json          ← one file per vehicle class or individual unit
    ep09.json
    112a.json
    134a.json
    …
  trains/
    ic_12345.json      ← one file per train consist
    pospiech_express.json
    …
```

---

## Vehicle definition

Each vehicle is an individual unit with a unique `gID`.

### JSON schema

```json
{
  "gID":            "VEH-GGO-ET22-001-0000001",
  "pID":            "ET22-001",
  "type":           "LOCOMOTIVE",
  "subtype":        "ELECTRIC",
  "displayName":    "ET22-001",
  "lengthM":        19.24,
  "axleCount":      6,
  "massEmptyT":     84.0,
  "maxSpeedKmh":    125,
  "powerKW":        2000.0,
  "tractionForceKN": 196.0
}
```

### `type` and `subtype` values

| `type`           | `subtype`                      | Notes                               |
|------------------|--------------------------------|-------------------------------------|
| `LOCOMOTIVE`     | `ELECTRIC` \| `DIESEL` \| `STEAM` |                                  |
| `EMU_UNIT`       | `MOTOR` \| `TRAILER`           | Multiple-unit train car             |
| `PASSENGER_WAGON`| —                              |                                     |
| `FREIGHT_WAGON`  | `FLAT` \| `COVERED` \| `TANK` \| `HOPPER` | |
| `SERVICE_WAGON`  | —                              | Maintenance, snow plough, etc.      |

Fields `powerKW` and `tractionForceKN` are used by the **v2 physics model** and ignored by v1. They must still be present in locomotive definitions so the fleet registry is forward-compatible.

---

## Train composition

A train composition lists vehicle `gID`s in order from **front to rear**. The engine assembles `std::vector<Vehicle>` by iterating the list and looking up each vehicle definition.

```json
{
  "gID":         "TRN-GGO-IC12345-0000100",
  "pID":         "IC 12345",
  "displayName": "IC 12345 Kraków — Gdynia",
  "vehicles": [
    "VEH-GGO-ET22-001-0000001",
    "VEH-GGO-112A-001-0000010",
    "VEH-GGO-112A-002-0000011",
    "VEH-GGO-112A-003-0000012",
    "VEH-GGO-134A-001-0000020"
  ]
}
```

The engine computes derived properties at load time:

```
total_length_m  = sum of vehicle.lengthM
total_axles     = sum of vehicle.axleCount
total_mass_t    = sum of vehicle.massEmptyT       ← v2 physics input
```

---

## Axle counting and section occupancy

`It` and `iz` devices count axes, not trains. The engine tracks:

1. When the **leading axle** of the train crosses an `It`/`iz`, the section ahead becomes **occupied**.
2. As each vehicle's axles pass, the device increments its internal axle counter.
3. When the running total equals `train.total_axles`, the **trailing axle** has cleared the device — the section behind becomes **free**.

This mirrors real SRK behavior and makes track section occupancy derived directly from the physical device model, not from train position bookkeeping.

---

## Physics

### V1 — constant acceleration (MVP)

The engine applies a fixed acceleration profile per train regardless of vehicle composition:

```
a_accel  = configurable constant (default: 0.4 m/s²)
a_decel  = configurable constant (default: 0.8 m/s²)
v_target = min(section.maxSpeedKmh, signal_aspect_speed) converted to m/s
```

At each simulation tick `Δt`:
- If `v < v_target`: `v += a_accel * Δt`
- If `v > v_target`: `v -= a_decel * Δt`
- `v` is clamped to `[0, v_target]`

No mass, no gradient, no curve resistance. Suitable for dispatcher training where train dynamics are not the focus.

### V2 — power and mass (post-MVP)

When `subtype = ELECTRIC | DIESEL` locomotives are present, v2 computes traction force:

```
F_traction  = min(tractionForceKN × 1000,  powerKW × 1000 / max(v, 1.0))   [N]
F_rolling   = total_mass_t × 1000 × 0.002 × 9.81                            [N]  (rolling resistance coefficient ≈ 0.002)
F_gradient  = total_mass_t × 1000 × 9.81 × sin(gradient_permille / 1000)    [N]
F_net       = F_traction − F_rolling − F_gradient
a           = F_net / (total_mass_t × 1000)                                  [m/s²]
```

Braking in v2 uses `a_decel` (constant for MVP extension); regenerative braking is post-v2.

The v1 → v2 switch is a configuration flag per session, not a code branch — the engine selects the physics strategy at startup.

---

## DOMAIN_EVENT integration

| Event           | Added fields (from this document)                      |
|-----------------|--------------------------------------------------------|
| `TrainComposed` | `vehicle_gIDs[]`, `total_length_m`, `total_axles`      |
| `TrainMovement` | `speed_kmh` (already in event 0x09 as of contract v1.1)|
| `TrackSectionOccupancyChanged` | `train_gID` (which train caused the change) |
| `SwitchOccupancyChanged`       | `train_gID`                                 |

---

## Open questions

- Q-VEH-1: Should multi-unit (EMU) vehicles define their power across the whole unit or per motor car? Per-unit is simpler for v2 physics.
- Q-VEH-2: Should a single train consist allow multiple locomotives (e.g., double-headed freight)? V1 ignores this; V2 would need to sum `tractionForceKN` across all `LOCOMOTIVE` entries.
- Q-VEH-3: Loaded vs. empty wagon mass — is `massEmptyT` sufficient for dispatcher training, or should freight wagons have a `massLoadedT` field for v2 accuracy?
