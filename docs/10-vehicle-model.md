# Vehicle and Train Model

## Overview

A **train** is a `std::vector<Vehicle>` assembled at runtime from individual vehicle definitions stored in JSON.

The fleet data is split into three levels:

| Level | Directory | Purpose |
|---|---|---|
| **Vehicle type** | `data/vehicle_types/` | Physical properties shared by all units of the same model (e.g. every ET22 has the same axle count, mass, speed limit). One JSON file per type. |
| **Vehicle instance** | `data/vehicles/` | An individual, numbered vehicle. References its type and overrides only what differs (e.g. loaded mass for a freight wagon). One directory per unit, with the canonical data stored in `vehicle.json`. |
| **Train consist** | `data/trains/` | Ordered list of vehicle `gID`s forming a specific trainset. |

Station devices (signals, switches, derailers) are part of the scenario topology and live in `scenarios/`. They are **not** fleet data.

---

## Where the data directory is used

| Component | Usage |
|---|---|
| **Engine** | Loads types + instances at startup to build the fleet registry; assembles consists from train JSON |
| **Client** | Reads types + instance display names to populate the rolling-stock info panel |
| **Editor** | Reads types to populate the vehicle palette when building timetable entries |

All three components read the same `data/` tree from the filesystem (path configurable at runtime). No component writes to it during a session.

---

## File layout

```
data/
  vehicle_types/
    locomotive/
      electric/        ← electric mainline and shunting locos
        111e/
          111eb.json
          111ed.json
          111eo.json
        et/
          et22.json
        eu/
          eu07.json
        ep/
          ep09.json
        …
      diesel/          ← diesel mainline and shunting locos
        sm/
          sm42.json
        sp/
          sp32.json
        …
      steam/           ← steam locomotives
        ty/
          ty2.json
        …
    emu_unit/
      motor/           ← electric multiple unit motor/control cars
        we/
          14we.json
          21we.json
          36we.json
        en/
          en57.json
        ed/
          ed250.json
        …
    dmu_unit/
      motor/           ← diesel multiple unit motor cars
        36we/
          36weh.json
          36wehd.json
        sa/
          sa134.json
        …
    freight_wagon/
      hopper/
        452w/
          452w.json
        …
    service_wagon/     ← track machines, maintenance vehicles
        wm15/
          wm15.json
        …
  vehicles/
    locomotive/
      electric/
        et22/
          et22-001/
            vehicle.json
            photos/        <- optional per-vehicle assets
          et22-002/
            vehicle.json
          ...
    freight_wagon/
      hopper/
        452w/
          452w-5375001/
            vehicle.json
          ...
    emu_unit/
      motor/
        en57/
          en57-001/
            vehicle.json
          ...
  trains/
    passenger/
      ic_12345.json    ← one file per consist
      regional_5401.json
      …
    freight/
      tow54321.json
      …
    maintenance/
      tm_2001.json
      …
scenarios/
  reference/
    gdynia_orlowo/
      topology.json    ← track sections, switches
      objects.json     ← signals, derailers, posterunki
```

The engine scans `data/vehicle_types/` and `data/trains/` **recursively** for JSON files. Vehicle instances are loaded only from files named `data/vehicles/**/vehicle.json`; other JSON files inside a vehicle directory are treated as sidecar metadata and ignored by the fleet loader. Vehicle type subdirectories are named after `vehicleType` (lowercase), `vehicleSubtype` where applicable, and then a family/series folder such as `sa`, `we`, `111e`, `en`, `eu`, `ty`, or `wm15`. For trains, the folder name must match `trainCategory` (`passenger`, `freight`, `maintenance`).

---

## Global identifier scheme for vehicle types

Vehicle types use the same `generateGID` function as other objects (see [docs/07-ebiscreen-description.md](07-ebiscreen-description.md)), with:

| Segment | Value | Rationale |
|---|---|---|
| `type` | `VT` | Vehicle Type |
| `area` | `GLB` | Global — types are not area-specific |
| `pID` | type designation | e.g. `ET22`, `EP09`, `EN57AL` |
| number | 7-digit sequence | unique across all `VT` objects |

**Example:** `VT-GLB-ET22-0000001`

Individual vehicle instances use `VEH` as the type segment and the deployment area as `area`:

**Example:** `VEH-TRJ-ET22-001-0000001`

---

## Vehicle type definition

A type file declares all physical properties that are identical across every unit of that model.

### JSON schema

```json
{
    "typeID":           "VT-GLB-ET22-0000001",
    "typeName":         "ET22",
    "vehicleType":      "LOCOMOTIVE",
    "vehicleSubtype":   "ELECTRIC",
  "pkpSeries":        "ET22",
  "family":           "Pafawag 201E",
    "lengthM":          19.24,
    "axleCount":        6,
    "massEmptyT":       84.0,
    "massGrossT":       null,
    "maxSpeedKmh":      125,
    "brakingLambdaPct": 100,
    "powerKW":          2000.0,
  "tractionForceKN":  196.0,
  "multipleCouplingCapable": true,
  "davisA":           39.24,
  "davisB":           0.1962,
  "davisC":           0.0017658
}
```

A covered freight wagon type:

```json
{
    "typeID":           "VT-GLB-403Z-0000020",
    "typeName":         "403Z",
    "vehicleType":      "FREIGHT_WAGON",
    "vehicleSubtype":   "COVERED",
  "pkpSeries":        null,
  "family":           "Wagony Swidnica",
    "lengthM":          14.02,
    "axleCount":        4,
    "massEmptyT":       22.0,
    "massGrossT":       null,
    "maxSpeedKmh":      100,
    "brakingLambdaPct": 65,
    "powerKW":          null,
  "tractionForceKN":  null,
  "davisA":           44.145,
  "davisB":           0.21582,
  "davisC":           0.001962
}
```

### Type field reference

| Field | Type | Required | Notes |
|---|---|---|---|
| `typeID` | string | yes | generated by `generateGID("VT", "GLB", typeName)` |
| `typeName` | string | yes | human-readable designation, e.g. `ET22` |
| `vehicleType` | string | yes | see table below |
| `vehicleSubtype` | string | depends | required for `LOCOMOTIVE`, `EMU_UNIT`, `DMU_UNIT`, `FREIGHT_WAGON` |
| `pkpSeries` | string-or-null | no | railway series label used in UI and reports |
| `family` | string-or-null | no | human-readable manufacturer/family grouping |
| `lengthM` | float | yes | over buffers |
| `axleCount` | int | yes | |
| `massEmptyT` | float | yes | tare mass in tonnes |
| `massGrossT` | float‑or‑null | no | default loaded mass; `null` = vehicle is never loaded |
| `maxSpeedKmh` | int | yes | design maximum |
| `brakingLambdaPct` | int | yes | UIC braking percentage λ |
| `powerKW` | float‑or‑null | no | `null` for unpowered vehicles |
| `tractionForceKN` | float‑or‑null | no | `null` for unpowered vehicles |
| `multipleCouplingCapable` | bool‑or‑null | no | traction-capable type categories (`LOCOMOTIVE`, `EMU_UNIT`/`DMU_UNIT` + `MOTOR`); for locomotives it gates same-type coupled traction gain, for MU units it documents type-level MU compatibility |
| `davisA` | float | no | per-tonne Davis A coefficient; defaults are applied if missing |
| `davisB` | float | no | per-tonne Davis B coefficient; defaults are applied if missing |
| `davisC` | float | no | per-tonne Davis C coefficient; defaults are applied if missing |

### `vehicleType` and `vehicleSubtype` values

| `vehicleType` | `vehicleSubtype` | Notes |
|---|---|---|
| `LOCOMOTIVE` | `ELECTRIC` \| `DIESEL` \| `STEAM` | |
| `EMU_UNIT` | `MOTOR` \| `TRAILER` | Multiple-unit train car |
| `DMU_UNIT` | `MOTOR` \| `TRAILER` | Diesel multiple-unit car |
| `PASSENGER_WAGON` | — | |
| `FREIGHT_WAGON` | `FLAT` \| `COVERED` \| `TANK` \| `HOPPER` | |
| `SERVICE_WAGON` | — | Maintenance, snow plough, etc. |

---

## Vehicle instance definition

An instance references its type by `typeID`. Only fields that **differ from the type** need to be present (e.g. `massGrossT` when a specific wagon is known to be loaded).

### JSON schema — locomotive

```json
{
    "gID":         "VEH-TRJ-ET22-001-0000001",
    "pID":         "ET22-001",
    "typeID":      "VT-GLB-ET22-0000001",
  "displayName": "ET22-001",
  "tractionStatus": "OPERATIONAL"
}
```

`tractionStatus` applies only to traction-capable instances:
- `LOCOMOTIVE`
- `EMU_UNIT` + `MOTOR`
- `DMU_UNIT` + `MOTOR`

When omitted for a traction-capable unit, the loader defaults it to `OPERATIONAL`.
`DEFECTIVE` units contribute mass and drag, but not traction or power.

### JSON schema — freight wagon with a loaded-state override

```json
{
    "gID":         "VEH-TRJ-403Z-001-0000050",
    "pID":         "403Z-001",
    "typeID":      "VT-GLB-403Z-0000020",
    "displayName": "403Z-001",
    "massGrossT":  58.0
}
```

`massGrossT` overrides the type’s `null` default, telling the engine this wagon is carrying 58 t gross.

### Property resolution rule

```
effective(field) = instance.field   if present and not null
                   type.field       otherwise
```

The engine performs this merge once at load time and caches the resolved `Vehicle` struct. Instances never need to repeat fields that match the type.

### Instance field reference

| Field | Type | Required | Notes |
|---|---|---|---|
| `gID` | string | yes | generated by `generateGID("VEH", area, pID)` |
| `pID` | string | yes | operational number, e.g. `ET22-001` |
| `typeID` | string | yes | must match a `typeID` in `data/vehicle_types/` |
| `displayName` | string | yes | shown on screen |
| `tractionStatus` | enum (`OPERATIONAL`\|`DEFECTIVE`) | no | valid only for traction-capable units; default `OPERATIONAL` |
| any type field | — | no | present only when this unit deviates from the type default |

Fields `powerKW` and `tractionForceKN` are used by the **v2 physics model** and ignored by v1. They must be present on the type so the registry is forward-compatible.

---

## Train composition

A consist file lists vehicle `gID`s in order from **front to rear**. The engine assembles `std::vector<Vehicle>` by looking up each instance in the fleet registry.

```json
{
    "gID":         "TRN-TRJ-IC12345-0000100",
    "pID":         "IC 12345",
  "trainCategory": "PASSENGER",
    "carrier":     "PKP Intercity",
    "displayName": "IC 12345 Kraków — Gdynia",
    "vehicles": [
        "VEH-TRJ-ET22-001-0000001",
        "VEH-TRJ-112A-001-0000010",
        "VEH-TRJ-112A-002-0000011",
        "VEH-TRJ-112A-003-0000012",
        "VEH-TRJ-134A-001-0000020"
    ]
}
```

`trainCategory` is required and must be one of `PASSENGER`, `FREIGHT`, `MAINTENANCE`. The loader validates that the folder path and field value are consistent.

`carrier` is optional. When present, it must match one of the names from `data/carriers.json`.
If the field is missing or `null`, the consist is loaded without an assigned carrier.

The engine computes derived properties at load time:

```
effectiveMassT(v) = v.massGrossT  if massGrossT is not null
                    v.massEmptyT  otherwise

total_length_m  = sum of vehicle.lengthM
total_axles     = sum of vehicle.axleCount
total_mass_t    = sum of effectiveMassT(v)          ← physics input
consist_lambda  = sum(v.brakingLambdaPct × effectiveMassT(v)) / total_mass_t
                                                     ← mass-weighted average λ [%]
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

The current implementation in `engine/physics` uses a deterministic Newton/Davis model with one integration step per simulation tick.

### Consist aggregation

At consist build time, the engine resolves per-vehicle physics into one `TrainPhysicsParams` structure:

```
total_mass_t      = Σ mass_t
max_speed_ms      = min(vehicle max speed)
davis_A/B/C       = Σ (vehicle_davis * vehicle_mass_t)
```

`max_traction_kn` is resolved with traction status and coupling rules:
- `EMU_UNIT/DMU_UNIT MOTOR`: sum traction only for `OPERATIONAL` units.
- `LOCOMOTIVE`:
  - one operational locomotive: use it,
  - multiple operational locomotives: sum all only if every operational locomotive has the same `typeID` and that type has `multipleCouplingCapable = true`,
  - otherwise: only the first operational locomotive contributes traction (all other locomotives are ballast-only).
- `DEFECTIVE` traction-capable units always contribute mass and resistance, but zero traction/power.

In current engine logic, `multipleCouplingCapable` directly gates traction gain for locomotives. For EMU/DMU motor-car types, it is currently stored as verified type metadata and used by planning/editor validation.

This keeps runtime simulation constant-time per train.

### Tick integration

For each tick with step `dt`:

```
F_resistance = davis_A + davis_B * v_kmh + davis_C * v_kmh^2
F_net        = F_traction - F_resistance - F_brake
a            = F_net / (mass_t * 1000)
v_new        = clamp(v + a * dt, 0, v_max)
x_new        = x + 0.5 * (v + v_new) * dt
```

`F_traction` and `F_brake` are provided by DriverAI (or another control policy).

### Braking model

The service brake force limit is derived from braking percentage λ:

```
a_brake_max = clamp((lambda_pct / 100) * g * 0.85, 0.3, 8.0)
F_brake_max = a_brake_max * total_mass_t   [kN]
```

`DriverAI` also uses `braking_distance()` and `braking_distance_to_speed()` helpers for anticipation.

## DriverAI

`DriverAI` is a deterministic four-state automaton:

- `STOPPED`
- `ACCELERATING`
- `CRUISING`
- `BRAKING`

Inputs include current aspect, distance to signal, current physics state, consist limits, and advisory context from the next signal (`next_aspect`, `distance_to_next_signal_m`).

Behavior highlights:

- `S1_STOP` triggers braking only when braking distance reaches the mast; a far red signal does not force immediate braking.
- Warning/expect aspects can trigger proactive braking toward the expected speed at the next signal.
- In `CRUISING`, traction is set to approximately match Davis resistance, which avoids oscillation around target speed.

---

## DOMAIN_EVENT integration

| Event           | Added fields (from this document)                      |
|-----------------|--------------------------------------------------------|
| `TrainComposed` | `vehicle_gIDs[]`, `total_length_m`, `total_axles`      |
| `TrainMovement` | `speed_kmh` (already in event 0x09 as of contract v1.1)|
| `TrackSectionOccupancyChanged` | `train_gID` (which train caused the change) |
| `SwitchOccupancyChanged`       | `train_gID`                                 |

---

## Resolved decisions

- Q-VEH-1: **Resolved.** EMU/DMU traction is modeled per powered car (`vehicleSubtype = MOTOR`), not per whole unit.
- Q-VEH-2: **Resolved.** Multiple locomotives are allowed in a consist, but effective coupled traction is granted only for same-type locomotives with `multipleCouplingCapable = true`; otherwise additional locomotives are ballast-only.
- Q-VEH-3: **Resolved.** Freight wagons carry `massGrossT` (loaded mass) and `massEmptyT` (tare). The engine uses `massGrossT` when present, `massEmptyT` otherwise. `brakingLambdaPct` covers the loaded/empty braking difference — a loaded wagon has a lower λ, so `a_decel` is automatically smaller.
