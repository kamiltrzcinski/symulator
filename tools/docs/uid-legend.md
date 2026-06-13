# UID Legend

The tools use the `engine::core::UID` type and `make_uid()` implementation from
`engine/include/engine/core/types.hpp`. The engine header is the source of truth.

## Bit Layout

```text
63        48 47    40 39    32 31             16 15              0
+------------+--------+--------+----------------+------------------+
|  reserved  | DOMAIN |  KIND  |     SCOPE      |     INSTANCE     |
|   16 bits  | 8 bits | 8 bits |    16 bits     |      16 bits     |
+------------+--------+--------+----------------+------------------+
```

Bits 63-48 must be zero. `SCOPE` is an unsigned 16-bit value. `INSTANCE`
must be in the range `0x0001` through `0xFFFF`; zero is reserved.

```cpp
UID make_uid(UIDDomain domain, UIDKind kind,
             std::uint16_t scope, std::uint16_t instance);
```

All persisted UIDs must be less than or equal to
`UID_MAX_SAFE_JSON_INTEGER` (`2^53 - 1`, or `9007199254740991`) so they can be
represented exactly as JSON numbers.

## Domains

| Domain | Hex | Description |
|---|---:|---|
| `ROLLING_STOCK` | `0x01` | Vehicle types, vehicles, train consists and carriers |
| `INFRASTRUCTURE` | `0x02` | Stations and fixed railway infrastructure |
| `OPERATIONS` | `0x03` | Session-scoped routes, alarms and dispatch exchanges |

## Kinds and SCOPE

| Domain | Kind | Hex | Description | SCOPE meaning |
|---|---|---:|---|---|
| `ROLLING_STOCK` | `VEHICLE_TYPE` | `0x01` | Vehicle type definition | Vehicle series/family code, or `0` |
| `ROLLING_STOCK` | `VEHICLE` | `0x02` | Individual vehicle | Vehicle series/family code, or `0` |
| `ROLLING_STOCK` | `TRAIN_CONSIST` | `0x03` | Assembled train | Vehicle series/family code, or `0` |
| `ROLLING_STOCK` | `CARRIER` | `0x04` | Railway carrier | Always `0` |
| `INFRASTRUCTURE` | `STATION` | `0x11` | Line control station | Station instance number |
| `INFRASTRUCTURE` | `DISPATCH_AREA` | `0x12` | Dispatch/interlocking area | Station instance number |
| `INFRASTRUCTURE` | `TRACK_SECTION` | `0x13` | Track section | Station instance number |
| `INFRASTRUCTURE` | `SWITCH` | `0x14` | Points or turnout | Station instance number |
| `INFRASTRUCTURE` | `SIGNAL` | `0x15` | Signal head | Station instance number |
| `INFRASTRUCTURE` | `DERAILER` | `0x16` | Derailer | Station instance number |
| `INFRASTRUCTURE` | `BLOCK_SECTION` | `0x17` | Line block section | Station instance number |
| `INFRASTRUCTURE` | `BOUNDARY_NODE` | `0x18` | LCS boundary | Station instance number |
| `INFRASTRUCTURE` | `LEVEL_CROSSING` | `0x19` | Level crossing | Station instance number |
| `INFRASTRUCTURE` | `AXLE_COUNTER` | `0x1A` | Axle counter | Station instance number |
| `INFRASTRUCTURE` | `INTERLOCKING` | `0x1B` | Control system | Station instance number |
| `INFRASTRUCTURE` | `POWER_SUPPLY` | `0x1C` | Traction power section | Station instance number |
| `OPERATIONS` | `ROUTE` | `0x21` | Set route | Station instance, or `0` for session-global |
| `OPERATIONS` | `ALARM` | `0x22` | Active alarm/event | Station instance, or `0` for session-global |
| `OPERATIONS` | `DISPATCH_EXCHANGE` | `0x23` | S-form exchange | Station instance, or `0` for session-global |

`INSTANCE` is sequential within a `(DOMAIN, KIND, SCOPE)` bucket. The tools
reserve zero and search the positive range for the first available value.
