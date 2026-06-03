# Universal UID — Bit-Layout Legend

> **Version**: 1.0  
> **Status**: authoritative — all code, JSON data, and proto schemas must conform.

## 1. Motivation

The codebase previously used several incompatible identifier types:

| Old type | Kind | Replaced by |
|---|---|---|
| `GID` | string, hierarchical (`"OT-TRJ-GOr-l202-t1a"`) | `UID` |
| `SID` | string, station code (`"GOr"`) | SCOPE field of INFRA UIDs |
| `DispatchAreaID` | string | `UID` INFRASTRUCTURE/DISPATCH_AREA |
| `ControlSystemID` | string | `UID` INFRASTRUCTURE/INTERLOCKING |
| `UID` (old) | numeric, ROLLING_STOCK only | `UID` (universal) |

`PlayerID` (`std::string`, Steam) and `session_id` (PostgreSQL UUID string) are **not** replaced.

---

## 2. Bit Layout

```
 63      48 47    40 39    32 31           16 15            0
┌──────────┬────────┬────────┬──────────────┬────────────────┐
│ reserved │ DOMAIN │  KIND  │    SCOPE     │    INSTANCE    │
│  16 bits │ 8 bits │ 8 bits │   16 bits    │    16 bits     │
└──────────┴────────┴────────┴──────────────┴────────────────┘
```

| Field    | Bits  | Width | Range       | Notes |
|----------|-------|-------|-------------|-------|
| reserved | 63–48 | 16    | must be `0` | kept zero for future expansion |
| DOMAIN   | 47–40 | 8     | `0x01–0xFF` | top-level entity category |
| KIND     | 39–32 | 8     | `0x01–0xFF` | entity sub-type (globally unique) |
| SCOPE    | 31–16 | 16    | `0x0000–0xFFFF` | context-dependent (see §5) |
| INSTANCE | 15–0  | 16    | `0x0001–0xFFFF` | sequential number within scope |

**JSON safety**: max value `0x00FF_FFFF_FFFF_FFFF` = 72 057 594 037 927 935 < 2^53 = 9 007 199 254 740 992 ✓  
All UID values are safe JavaScript integers (no precision loss in JSON / FlatBuffers `uint64`).

### C++ constructor

```cpp
UID make_uid(UIDDomain domain, UIDKind kind,
             std::uint16_t scope, std::uint16_t instance);

// Accessors
UIDDomain    uid_domain(UID);
UIDKind      uid_kind(UID);
std::uint16_t uid_scope(UID);      // was: uid_type_code()
std::uint16_t uid_instance(UID);   // was: uid_item_number()
bool          uid_is_safe_json_integer(UID);
bool          uid_has_kind(UID, UIDDomain, UIDKind);
```

---

## 3. DOMAIN Values

| Hex    | Name            | Description |
|--------|-----------------|-------------|
| `0x01` | `ROLLING_STOCK` | Vehicles, vehicle types, train consists, carriers |
| `0x02` | `INFRASTRUCTURE` | Fixed railway infrastructure |
| `0x03` | `OPERATIONS`    | Session-scoped operational entities |

Reserved: `0x04–0xFE`. `0x00` and `0xFF` are invalid.

---

## 4. KIND Values

KIND values are **globally unique** across all domains (domain-prefixed nibble: RS=`0x0_`, INFRA=`0x1_`, OPS=`0x2_`).  
This allows `uid_kind()` alone to identify entity type without first checking domain.

### 4.1 ROLLING_STOCK domain (`0x01`)

| Hex    | Name            | Old identifier | Description |
|--------|-----------------|----------------|-------------|
| `0x01` | `VEHICLE_TYPE`  | `typeID` string (`"VT-GLB-452W-…"`) | Vehicle type definition |
| `0x02` | `VEHICLE`       | `gID` string (`"VEH-TRJ-…"`) | Individual vehicle instance |
| `0x03` | `TRAIN_CONSIST` | `gID` string (`"TRN-…"`) | Assembled train set |
| `0x04` | `CARRIER`       | numeric UID (old scheme, compatible) | Railway carrier / operator |

### 4.2 INFRASTRUCTURE domain (`0x02`)

| Hex    | Name              | Old identifier | Description |
|--------|-------------------|----------------|-------------|
| `0x11` | `STATION`         | `sID` (`"GOr"`) | Line control station (LCS) |
| `0x12` | `DISPATCH_AREA`   | `DispatchAreaID` (`"GGO_nastawnia_A"`) | Interlocking area within a station |
| `0x13` | `TRACK_SECTION`   | `gID` (`"OT-TRJ-GOr-…"`) | Named track section |
| `0x14` | `SWITCH`          | `gID` (`"OT-ZWR-GOr-…"`) | Points / turnout |
| `0x15` | `SIGNAL`          | `gID` (`"OT-SEM-GOr-…"`) | Semaphore / signal head |
| `0x16` | `DERAILER`        | `gID` (`"OT-WYK-…"`) | Derail / scotch block |
| `0x17` | `BLOCK_SECTION`   | `gID` (`"OT-BL-…"`) | Line block section |
| `0x18` | `BOUNDARY_NODE`   | `gID` (`"OT-BND-…"`) | LCS boundary / handover point |
| `0x19` | `LEVEL_CROSSING`  | `gID` (`"OT-LO-…"`) | Level crossing |
| `0x1A` | `AXLE_COUNTER`    | `gID` (`"OT-PZD-…"`) | Axle counter |
| `0x1B` | `INTERLOCKING`    | `ControlSystemID` (`"GGO_ebilock"`) | Interlocking / control system |
| `0x1C` | `POWER_SUPPLY`    | `gID` (`"OT-POZ-…"`) | Traction power supply section |

### 4.3 OPERATIONS domain (`0x03`)

| Hex    | Name                | Old identifier | Description |
|--------|---------------------|----------------|-------------|
| `0x21` | `ROUTE`             | `route_id` string | Set route through interlocking |
| `0x22` | `ALARM`             | `alarm_id` string | Active alarm / event |
| `0x23` | `DISPATCH_EXCHANGE` | exchange string ID | S-form dispatch exchange |

Reserved: `0x05–0x10`, `0x1D–0x20`, `0x24–0xFE`.

---

## 5. SCOPE Semantics

SCOPE meaning depends on DOMAIN:

| DOMAIN           | SCOPE value | Meaning |
|------------------|-------------|---------|
| `ROLLING_STOCK`  | vehicle series / family code, or `0` | e.g. `0x01B3` for EN57 series |
| `INFRASTRUCTURE` | **station instance number** (see §6) | `1`=GOr, `2`=Sp, `3`=GGO, `4`=OT |
| `OPERATIONS`     | station instance number, or `0` | `0` for session-global operations |

For ROLLING_STOCK CARRIER the SCOPE is always `0` (carriers are not station-scoped).

---

## 6. Station Instance Registry

Station instance numbers are defined in `data/stations.json`.  
The mapping is static — never reuse or reassign a number.

| Instance | Code  | Name |
|----------|-------|------|
| `1`      | `GOr` | Gdynia Orłowo |
| `2`      | `Sp`  | Sopot |
| `3`      | `GGO` | Gdynia Główna Osobowa |
| `4`      | `OT`  | (test/template scenario) |

New stations: append to the list with the next unused integer.

---

## 7. INSTANCE Semantics

- INSTANCE is a **sequential number** starting at `1` within a given `(DOMAIN, KIND, SCOPE)` triple.
- INSTANCE `0x0000` is **reserved / invalid** — never emit it.
- Maximum 65 535 instances per `(KIND, SCOPE)` bucket (16-bit field).

### OPERATIONS domain — session scope

OPERATIONS UIDs (routes, alarms, exchanges) are **session-scoped**:
- INSTANCE is a monotonic per-session counter starting at `1`.
- Global uniqueness is the pair `(session_uuid, uid)` — the UID alone is not globally unique across sessions.
- Do **not** persist OPERATIONS UIDs beyond session lifetime without also persisting `session_uuid`.

---

## 8. Worked Examples

```
CARRIER, carrier #1:
  make_uid(ROLLING_STOCK, CARRIER, 0, 1)
  = (0x01 << 40) | (0x04 << 32) | (0x0000 << 16) | 0x0001
  = 0x0000_0104_0000_0001
  = 1_116_691_496_961

STATION GOr:
  make_uid(INFRASTRUCTURE, STATION, 1, 1)
  = (0x02 << 40) | (0x11 << 32) | (0x0001 << 16) | 0x0001
  = 0x0000_0211_0001_0001
  = 2_272_037_765_121

TRACK_SECTION on GOr, instance 202:
  make_uid(INFRASTRUCTURE, TRACK_SECTION, 1, 202)
  = (0x02 << 40) | (0x13 << 32) | (0x0001 << 16) | 0x00CA
  = 0x0000_0213_0001_00CA
  = 2_306_001_764_290

SIGNAL on GOr, instance 1:
  make_uid(INFRASTRUCTURE, SIGNAL, 1, 1)
  = (0x02 << 40) | (0x15 << 32) | (0x0001 << 16) | 0x0001
  = 0x0000_0215_0001_0001
  = 2_289_217_634_305

ROUTE (session-scoped, station GOr), instance 1:
  make_uid(OPERATIONS, ROUTE, 1, 1)
  = (0x03 << 40) | (0x21 << 32) | (0x0001 << 16) | 0x0001
  = 0x0000_0321_0001_0001
  = 3_463_780_335_617
```

---

## 9. Extension Rules

### Adding a new KIND
1. Choose the next unused hex value in the correct nibble range (RS: `0x05–0x0F`, INFRA: `0x1D–0x1F`, OPS: `0x24–0x2F`, or the next decade `0x30–…`).
2. Add the constant to `UIDKind` enum in `engine/include/engine/core/types.hpp`.
3. Document it in §4 of this file with the old identifier it replaces (if any).
4. Add encode/decode roundtrip tests in `tests/engine/test_uid_codec.cpp`.
5. Update `scripts/validate_uid_registry.py` with the new valid KIND value.

### Adding a new DOMAIN
1. Choose the next unused value (`0x04`, `0x05`, …).
2. Add to `UIDDomain` enum in `types.hpp`.
3. Document in §3, define KIND sub-range (nibble convention: domain*16 for first kind).
4. Follow the same KIND extension steps above.

### Adding a new station
1. Append to `data/stations.json` with the next unused instance number.
2. Update §6 of this file.
3. Run `scripts/validate_uid_registry.py` to verify consistency.

---

## 10. Validation

`scripts/validate_uid_registry.py` enforces:
- All `uid` fields in JSON data files are valid uint64 values ≤ `2^53 - 1`.
- DOMAIN, KIND, SCOPE, INSTANCE fields decode to known valid values.
- No duplicate UIDs within a single file.
- Infrastructure UIDs: SCOPE must be a known station instance (present in `data/stations.json`).
- INSTANCE `0` is rejected.

Run via CMake target `validate_uid_registry` (also executed in CI after FlatBuffers validation).
