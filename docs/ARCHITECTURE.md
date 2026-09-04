# Symulator — Architecture & Reference

This is the single reference document for the project. It replaces the previous
`docs/00-*` … `docs/21-*` series, `docs/devices/000-list.md`, and the old
`docs/README.md` index, which described the intended design at various points
in time and had drifted from what the code actually does. Everything below was
checked against the current source tree, not against the older docs.

Out of scope for this file (unchanged, left as-is):
- `docs/instructions/` — proprietary PKP reference PDFs (gitignored).
- `docs/plan_*.md` and `docs/devlog.md` — private, gitignored working notes
  used during implementation sessions; not part of the published documentation
  set and not merged here.
- `docs/examples/json/` — example JSON fixtures for hand-authoring device/command
  type files; kept as data, not prose.

Where the original design and the current implementation disagree, this doc
describes **what the code does today** and calls out the gap explicitly rather
than silently picking one version.

---

## 1. Vision and scope

A networked railway signalling simulator inspired by Ebilock-style dispatcher
workflows: operators send commands, the engine simulates outcomes (signals,
switches, track occupancy, train movement) and enforces interlocking safety
rules, and a session server keeps every connected client in sync.

MVP target: one shared session, up to a handful of stations, 2+ operator
clients, timetable-driven trains, persistent event history. Domain logic is
independent of any UI — the engine has no rendering or transport code, and the
wire protocol carries only serialized state and commands.

Explicitly **not** goals for MVP: full vendor-system compliance, advanced
dispatcher-panel graphics, an AI opponent with online learning, large
multi-station production scenarios beyond the reference pair described below.

---

## 2. Repository layout

```
symulator/
  engine/                 core simulation (topology, physics, interlocking glue) — implemented
  srk/                     interlocking rule libraries (common / ebilock / ml8)   — implemented
  server/                  session server: transport, dispatch, persistence, terminal — implemented
  libscenario_validation/  three-layer scenario linter (layer1/2/3_validator)     — implemented
  proto/                   FlatBuffers wire schemas (.fbs)                        — implemented
  autogens/proto/          generated FlatBuffers C++ headers (committed)         — generated, committed
  tests/                   engine / server / srk / integration / proto / libscenario_validation / qt6
  scripts/                 build helpers, git hooks, e2e smoke test, UID/proto validators
  docker/                  docker-compose + PostgreSQL init.sql
  client/                  operator desktop app                                  — STUB (3-line main.cpp)
  editor/                  scenario editor                                       — STUB (3-line main.cpp)
  libtrackview/            SOLID-separated track layout and Qt rendering library
```

Fleet/vehicle data (`vehicle_types/`, `vehicles/`, `trains/`, `carriers/`,
`command_types/`, `device_types/`) and scenario bundles (`scenarios/`) live in
this repo under `data/` and `scenarios/`, but the canonical, larger data set is
maintained in the sibling repo **`symulator-data`** and pulled in via
`scripts/fetch_packages.py` / `packages/` (gitignored, refreshed on
`git pull` through a `post-merge` hook). `data/` in this repo currently holds
only signalling reference data (`command_types/`, `device_types/`); the vehicle
and train JSON described in §8 lives in `symulator-data`.

Build system: CMake ≥ 3.25 + Ninja + vcpkg, one root `CMakeLists.txt`. Options
`BUILD_CLIENT` / `BUILD_EDITOR` (default ON, pull in Qt6) and `BUILD_TOOLS`
(OFF) gate the GUI stubs; a headless configure (`--headless` via
`scripts/configure_ninja.py`) builds only `engine`, `server`, and tests,
skipping Qt6 entirely. See §13 for the full build/test workflow.

---

## 3. Identifiers — the UID system

All entities (infrastructure, rolling stock, session-scoped objects) are
identified by a single 48-bit-significant `UID` (a `uint64_t`), defined in
`engine/include/engine/core/types.hpp`. This replaced an earlier mix of
string identifiers (`GID`, `SID`, `DispatchAreaID`, `ControlSystemID`) —
**those string types no longer exist anywhere in the code.** Any example still
showing `"OT-TRJ-GOr-l202-t1a"`-style strings for track sections, switches,
signals, etc. describes the pre-migration scheme and is obsolete.

### Bit layout

```
 63      48 47    40 39    32 31           16 15            0
┌──────────┬────────┬────────┬──────────────┬────────────────┐
│ reserved │ DOMAIN │  KIND  │    SCOPE     │    INSTANCE    │
│  16 bits │ 8 bits │ 8 bits │   16 bits    │    16 bits     │
└──────────┴────────┴────────┴──────────────┴────────────────┘
```

`reserved` must be `0`. Max value `0x00FF_FFFF_FFFF_FFFF` < 2^53, so every UID
is a safe JavaScript/JSON integer.

```cpp
UID make_uid(UIDDomain domain, UIDKind kind, std::uint16_t scope, std::uint16_t instance);

UIDDomain     uid_domain(UID);
UIDKind       uid_kind(UID);
std::uint16_t uid_scope(UID);
std::uint16_t uid_instance(UID);
bool          uid_is_safe_json_integer(UID);
bool          uid_has_kind(UID, UIDDomain, UIDKind);
```

### DOMAIN values

| Hex | Name | Description |
|---|---|---|
| `0x01` | `ROLLING_STOCK` | Vehicles, vehicle types, train consists, carriers |
| `0x02` | `INFRASTRUCTURE` | Fixed railway infrastructure |
| `0x03` | `OPERATIONS` | Session-scoped operational entities (routes, alarms, dispatch exchanges) |

### KIND values (globally unique across domains)

| Hex | Name | Domain |
|---|---|---|
| `0x01` | `VEHICLE_TYPE` | ROLLING_STOCK |
| `0x02` | `VEHICLE` | ROLLING_STOCK |
| `0x03` | `TRAIN_CONSIST` | ROLLING_STOCK |
| `0x04` | `CARRIER` | ROLLING_STOCK |
| `0x11` | `STATION` | INFRASTRUCTURE |
| `0x12` | `DISPATCH_AREA` | INFRASTRUCTURE |
| `0x13` | `TRACK_SECTION` | INFRASTRUCTURE |
| `0x14` | `SWITCH` | INFRASTRUCTURE |
| `0x15` | `SIGNAL` | INFRASTRUCTURE |
| `0x16` | `DERAILER` | INFRASTRUCTURE |
| `0x17` | `BLOCK_SECTION` | INFRASTRUCTURE |
| `0x18` | `BOUNDARY_NODE` | INFRASTRUCTURE |
| `0x19` | `LEVEL_CROSSING` | INFRASTRUCTURE |
| `0x1A` | `AXLE_COUNTER` | INFRASTRUCTURE |
| `0x1B` | `INTERLOCKING` | INFRASTRUCTURE |
| `0x1C` | `POWER_SUPPLY` | INFRASTRUCTURE |
| `0x21` | `ROUTE` | OPERATIONS |
| `0x22` | `ALARM` | OPERATIONS |
| `0x23` | `DISPATCH_EXCHANGE` | OPERATIONS |

`SCOPE` means the station instance number for `INFRASTRUCTURE`/`OPERATIONS`
UIDs (see the registry below), or a vehicle series code / `0` for
`ROLLING_STOCK`. `INSTANCE` is a sequential number starting at `1`; `0` is
invalid and never emitted. OPERATIONS UIDs are session-scoped — unique only as
the pair `(session_uuid, uid)`, not globally.

### Station instance registry

Defined in `scenarios/stations.json` (not `data/stations.json` — an earlier
doc revision had the wrong path):

| Instance | Code | Name |
|---|---|---|
| 1 | `GOr` | Gdynia Orłowo |
| 2 | `Sp` | Sopot |
| 3 | `GGO` | Gdynia Główna Osobowa |
| 4 | `OT` | (test/template scenario) |

Numbers are never reused. Adding a station: append to `scenarios/stations.json`,
then run `scripts/validate_uid_registry.py` (also wired as a CMake target and
run in CI) which checks every `uid` field in JSON data is a valid, in-range,
non-duplicate value and that infrastructure SCOPE values resolve to a known
station.

One catalog that is **not** UID-based: device *type* definitions
(`data/device_types/*.json`, e.g. signal or switch-machine models) still use a
descriptive string id such as `"DVT-GLB-SEM-0000001"`. That's a global type
catalog, not an instance identifier, and hasn't been migrated — the codebase
only stores those strings as opaque references, so this is a deliberate
distinction rather than an oversight.

---

## 4. Engine core

Headers live in `engine/include/engine/core/`, implementation in `engine/src/`.

| Header | Purpose |
|---|---|
| `types.hpp` | `UID` and friends; enums (`SwitchPosition`, `SignalAspect`, `TrackOccupancy`, `BlockSectionState`, `BlockDirectionState`, `TrainCategory`, dispatch-form enums, …) |
| `track_model.hpp` | Value-type structs for device state: `TrackSection`, `Switch`, `Signal`, `Derailer`, `BlockSection`, `RouteState`, `AlarmState`, `BoundaryNode` |
| `command.hpp` | `Command = std::variant<...>` (cmd_type 0x01–0x0A, 0x20, 0x21) |
| `state_view.hpp` | `IStateView` — read-only interface |
| `engine_state.hpp/.cpp` | `EngineState : IStateView` — mutable world state, owned by the ENGINE thread |
| `engine_snapshot.hpp/.cpp` | `EngineSnapshot : IStateView` — immutable deep copy; `AtomicSnapshot` for lock-free cross-thread reads |
| `control_system.hpp` | `DeviceStateChange` variant; `InterlockingViolation`; `IControlSystem` |
| `control_system_registry.hpp/.cpp` | Factory singleton, `register_static()` for self-registration |
| `state_applier.hpp` | `std::visit` functor applying `DeviceStateChange` to `EngineState` |
| `topology_loader.hpp/.cpp` | `topology.json` + `objects.json` → `EngineState::insert_*` |
| `fleet_registry.hpp/.cpp` | Loads vehicle types/instances/consists into `FleetRegistry` |
| `train_fleet.hpp/.cpp` | Active trains: spawn/despawn, per-tick physics + occupancy |
| `spawn_resolver.hpp/.cpp` | Resolves the free track section behind a boundary node for spawning |
| `fleet_command.hpp` | `FleetCommand = std::variant<SpawnRequest, DespawnRequest>` |
| `snapshot_service.hpp` / `engine/src/snapshot_service.cpp` | `EngineSnapshot` → FlatBuffers `Snapshot` table |
| `engine_loop.hpp/.cpp` | The 20 Hz tick scheduler (§5) |
| `timetable_catalog_loader.hpp/.cpp` | Loads timetable points and connections (`data/timetable_points`, `data/timetable_connections`) |
| `train_scheduler.hpp/.cpp` | Validates and schedules timetable-driven spawning |

### `IStateView`

```cpp
class IStateView {
public:
    virtual const TrackSection*  find_track_section(UID) const noexcept = 0;
    virtual const Switch*        find_switch(UID)        const noexcept = 0;
    virtual const Signal*        find_signal(UID)         const noexcept = 0;
    virtual const Derailer*      find_derailer(UID)       const noexcept = 0;
    virtual const BlockSection*  find_block_section(UID)  const noexcept = 0;
    virtual const RouteState*    find_route(UID)          const noexcept = 0;
    virtual const AlarmState*    find_alarm(UID)          const noexcept = 0;
    virtual const BoundaryNode*  find_boundary_node(UID)  const noexcept = 0;

    virtual void for_each_section(std::function<void(const TrackSection&)>) const = 0;
    virtual void for_each_switch (std::function<void(const Switch&)>)       const = 0;
    virtual void for_each_signal (std::function<void(const Signal&)>)       const = 0;
    virtual void for_each_route  (std::function<void(const RouteState&)>)   const = 0;

    virtual const std::string& session_id()   const noexcept = 0;
    virtual uint64_t           current_tick() const noexcept = 0;
};
```

Two implementations: `EngineState` (live, mutable, ENGINE-thread-only) and
`EngineSnapshot` (read-only deep copy, safe from any thread). Active trains
(`TrainSnapshot`) live on `EngineSnapshot` as a plain vector — there is no
`find_train`/`for_each_train` on the interface itself yet.

### Track model — sideA/sideB, not startItID/endItID

A track section has two named ends, `side_a`/`side_b` (the labels carry no
directional meaning — the engine determines travel direction at runtime from
which side a train entered). Each `TrackPort` carries `neighbor_uid`,
`counter_uid` with a `CounterKind{IT, IZ}` tag, and `signal_uids[]`. A switch
has three legs (`trunk`, `straight`, `divergent`), each shaped like a track
port. `BlockSection` carries a `type_id` string (e.g. `"SHL-12"`),
`neighbor_station_uid` (the cross-station link, see §9), and the szlak
section UIDs it spans.

### `DeviceStateChange` (current, 15 variants)

```cpp
using DeviceStateChange = std::variant<
    SignalAspectChange, SwitchPositionChange, SwitchLocked, SwitchUnlocked,
    DerailerStateChange, BlockSectionStateChange, TrackSectionOccupancyChange,
    BlockDirectionChange, AxleCounterResetChange, OperatorCommandStateChange,
    Ml8CommandStateChange, RouteAdded, RouteRemoved, AlarmRaised, AlarmCleared>;
```

`TrackSectionOccupancyChange`, `AxleCounterResetChange`,
`OperatorCommandStateChange`, and `Ml8CommandStateChange` were added after the
original 11-type design; `StateApplier` has one `operator()` arm per type and
the ENGINE core has zero `if (system_id == ...)` branches — every mutation
flows through this visitor.

### `IControlSystem`

```cpp
class IControlSystem {
public:
    virtual ControlSystemID system_id() const = 0;
    virtual std::optional<InterlockingViolation>
        check_command(const IStateView&, const Command&) const = 0;   // no mutation
    virtual std::vector<DeviceStateChange>
        execute_command(const IStateView&, const Command&) = 0;       // only after check_command() == nullopt
    virtual std::vector<DeviceStateChange>
        on_tick(const IStateView&, uint64_t tick) = 0;                // once per ENGINE tick
    virtual std::vector<std::string> supported_command_types() const = 0;
};
```

All methods run exclusively on the ENGINE thread and must not block or spawn
threads. `ControlSystemRegistry` is a singleton mapping a string
`ControlSystemID` (from `meta.json → "control_system"`, e.g. `"ebilock_x4"` /
`"estw_ml8"`) to a factory; each SRK library self-registers at static-init via
`register_static()`. `AtomicSnapshot::publish()`/`load()` is a lock-free
`std::atomic<std::shared_ptr<const EngineSnapshot>>`.

### `EngineLoop::do_tick()` — the actual tick sequence

Runs on a `std::jthread` at a fixed 50 ms period (`TICK_PERIOD`, 20 Hz):

1. Drain up to `MAX_CMDS_PER_TICK` commands from `PriorityCommandQueue`; for
   each, call `check_command()` → NAK callback on violation, else
   `execute_command()` → `apply_all()` (StateApplier) → collect changes.
2. Call `IControlSystem::on_tick()` (timers, SHL-12 state machine, alarm
   auto-clear) → apply → collect changes.
3. Drain `EventQueue<FleetCommand>` (spawn/despawn requests from the
   terminal), re-validating against current state; resulting occupancy
   changes join the tick's change list, `PipEvent`s go to the PIP callback.
4. If any trains are active, `TrainFleet::tick_all()` — physics integration,
   axle-counter occupancy, PipEvents.
5. Advance the logical tick counter.
6. Deep-copy world state (including `train_fleet_.snapshot_trains()`) into a
   new `EngineSnapshot` and `AtomicSnapshot::publish()` it.
7. If any changes accumulated this tick, call `changes_cb_` once with the
   full list — this is what `DispatchBus` turns into `DOMAIN_EVENT` frames
   (§7) and `PipWriter` turns into `pip.track_state` UPSERTs (§11).

`FleetCommand` (`SpawnRequest`/`DespawnRequest`) is enqueued via
`EngineLoop::enqueue_fleet_command()` — currently the only producer is the
built-in terminal (§12); this is also the seam a future `TrainScheduler`
(timetable-driven spawning, tracked as E5, header exists but is unconnected)
would plug into without any `EngineLoop` changes.

---

## 5. Threading model — as implemented

The original design (still reflected in some doc history) called for four
fixed threads (`ENGINE`, `DISPATCH_BUS`, `DB_WRITER`, `PIP_WRITER`) plus
Boost.Asio `IO_POOL`/`WORK_POOL` thread pools. **That is not what's running
today.** The actual runtime has:

| Thread | What it does |
|---|---|
| **ENGINE** | `std::jthread`, 20 Hz tick loop (§4). Never sleeps on I/O by design. |
| **io_thread_** | One `std::jthread` running a single Boost.Asio `io_context` (`server/src/transport_gateway.cpp`) — connection accept, frame read/write. There is no pool; it's one thread. |
| **terminal** | One `std::jthread` for `StdinTerminal`, polling stdin with `poll()` around a blocking `getline` so it can honor `stop_token`. |

`DispatchBus`, `PipWriter`, and the `IDbWriter` implementations
(`NullDbWriter`/`PgDbWriter`) are **plain objects, not threads.** They are
invoked synchronously as callbacks from inside `EngineLoop::do_tick()` (step 7
above) and `PipWriter::on_pip_events()` — see the wiring in
`server/src/session_server.cpp`. `PgDbWriter` executes blocking `pqxx`
transactions inline, which means **DB I/O currently happens on the ENGINE
thread**, contradicting the original "ENGINE never blocks on I/O" principle.
This is a known simplification, not a documented feature — treat it as tech
debt if per-tick latency ever becomes a problem with `PgDbWriter` enabled
(`NullDbWriter`, the default with no `--db` flag, has no I/O cost).

`EventQueue<T>` (thread-safe MPSC, mutex + condvar, no heap allocation on the
hot path) exists and is used — but today its only real producer/consumer pair
is `EventQueue<FleetCommand>` inside `EngineLoop`. It is not used for
`DomainEvent`, `EventLogEntry`, or `PipEvent` fan-out; those go through direct
synchronous calls instead.

**Startup order** (`SessionServer::start()`, `server/src/session_server.cpp`):
load scenario → load fleet → resolve `IControlSystem` from `meta.json` →
construct network layer (`OwnershipGuard` → `IDbWriter` → `TransportGateway` →
`DispatchBus` → `DispatchExchangeManager` → `EdrCoordinator` → `PipWriter` →
`DispatchCoordinator` → `DispatchChannel`) → construct `EngineLoop` → construct
the terminal (`InMemoryUserStore` → `CommandRegistry` → `TerminalSession` →
`StdinTerminal`) → start IO thread → start ENGINE thread → start terminal
thread.

**Shutdown order** is the reverse, enforced by C++ declaration order in
`SessionServer` (destructors run LIFO): terminal first (its commands enqueue
into `EngineLoop`, so it must stop before the loop is torn down), then
`engine_loop_`, then the dispatch/network chain
(`dispatch_channel_ → dispatch_coordinator_ → pip_writer_ → edr_coordinator_ →
exchange_mgr_ → dispatch_bus_ → gateway_ → db_writer_ → ownership_`).

---

## 6. Interlocking (SRK)

`srk/` contains the safety-rule libraries, each implementing `IControlSystem`:

```
srk/
├── common/   libsrk_common — shared R1–R10 helpers, NAK codes, BFS route finder
├── ebilock/  libsrk_ebilock — EbiLock X4 ("ebilock_x4")
└── ml8/      libsrk_ml8     — ESTW ML8 ("estw_ml8"), adds the SHL-12 block telegraph
```

`ControlSystemRegistry::register_static()` runs at static-init time in each
library's `.cpp`. **Linker note:** `libsrk_ebilock`/`libsrk_ml8` must be linked
into `symulator-server` via `$<LINK_LIBRARY:WHOLE_ARCHIVE,...>` (CMake 3.24+)
— without it the linker drops the object files containing that static
initializer and the registry stays empty (`"Unknown control_system"` at
startup). See `server/CMakeLists.txt`.

### Rule catalogue (R1–R10)

| Rule | Command | Core preconditions | Typical NAK |
|---|---|---|---|
| R1 | `SetSwitchPosition` | not occupied, not route-locked, owner matches, not already there, not MOVING | `SAFETY_BLOCK` / `ROUTE_LOCKED` / `SWITCH_MOVING` |
| R2 | `SetSignalAspect` (manual) | owner matches, aspect not more permissive than route allows, not route-overridden | `SAFETY_BLOCK` |
| R3 | `SetDerailerPosition` (unlock) | owner matches, no active route, section beyond is free | `SAFETY_BLOCK` / `ROUTE_LOCKED` |
| R4 | `SetBlockSection` (open) | owner matches, no train in block, both boundary signals at stop | `SAFETY_BLOCK` |
| R5 | `RequestRoute` | BFS path exists; every section free, every switch unconflicted, every derailer unlocked, no opposing route, entry signal at stop → atomically locks switches, sets aspect, creates `RouteState`, emits `RouteSet` | `NO_PATH` / `SAFETY_BLOCK` / `ROUTE_LOCKED` |
| R6 | `CancelRoute` | route exists, owner matches, no train present (or `force=true`) | `NOT_FOUND` / `SAFETY_BLOCK` |
| R7 | `AcknowledgeAlarm` | alarm exists | `NOT_FOUND` |
| R8 | `SetBlockDirection` (SHL-12, ML8 only) | see `BlockDirectionState` transitions below | `INVALID_STATE` / `SAFETY_BLOCK` |
| R9 | `InitAxleCounterReset` (SLI, ML8 only) | direction == NEUTRAL | `INVALID_STATE` |
| R10 | `ResetAxleCounter` (SLK, ML8 only) | direction == RESET_PENDING | `INVALID_STATE` |

R1–R7 live in `srk::common::device_rules`, used by both EbiLock and ML8.
R8–R10 (SHL-12) are ML8-only; EbiLock rejects those command types with NAK
`UNSUPPORTED (0x07)`.

Auto-release: once a train's last axle clears the final section of a route,
the route releases automatically (unlocks switches, does not touch the exit
signal). Timeout: an unentered route auto-cancels after
`route_hold_timeout_s` (default 120 s). EbiLock additionally drives an EEA-4
switch-machine timer (default 90 ticks = 4.5 s) between `SetSwitchPosition`
and the switch snapping to its target.

### `BlockDirectionState` (SHL-12)

```
NEUTRAL → OUTBOUND_PENDING (BLW) → OUTBOUND + OPEN (BLP)
NEUTRAL → INBOUND_PENDING  (peer's BLW) → INBOUND + OPEN (BLP)
OUTBOUND/INBOUND → NEUTRAL + CLOSED (BLZ, requires axle_count == 0)
any → EMERGENCY (BLAI) → NEUTRAL + CLOSED (BLA)
NEUTRAL → RESET_PENDING (SLI) → NEUTRAL, axle_count = 0 (SLK)
```

### Operator command runtime state

Every device (signal, switch, derailer, track section, block section) carries
an `OperatorCommandRuntimeState { optional<OperatorCommandCode>,
optional<Ml8CommandCode> }` — a typed-optional pair (replacing an earlier
`bool + string` pair that could go out of sync). Set by cmd_type `0x20`
(`OperatorCommand`, EbiLock catalog) or `0x21` (`Ml8Command`, ML8 catalog);
mirrored to clients as `DOMAIN_EVENT` 0x11/0x12 and persisted in every
snapshot's `operator_state` field. `engine/core/command.hpp` exposes
`operator_command_code_name()` / `ml8_command_code_name()` constexpr helpers
(74 EbiLock codes, 71 ML8 codes) for logging and NAK text.

The canonical catalogs for these codes are data files, not prose:
`data/command_types/command_types.json` (EbiLock/EbiScreen operator catalog)
and `data/command_types/ml8_command_types.json` (ML8; command mnemonics like
`STJ`, `PZ`, `WBL`, `ZEROLO`, `NPW`, etc. — see the ML8 operator manual,
chapter 11, for full meanings). `data/device_types/device_type_catalog.json`
indexes the individual signal/switch-machine/derailer/block type files.

---

## 7. Wire protocol (client ↔ server)

Single TCP persistent socket, custom binary framing, FlatBuffers payloads
(`proto/*.fbs`, generated headers committed under `autogens/proto/`). No
REST/HTTP layer anywhere.

### Frame header (16 bytes) — implemented as documented

```
Offset  Size  Field          Notes
──────  ────  ─────────────  ─────────────────────────────────────────
 0       2    magic          0x53 0x52 ('S','R')
 2       1    msg_type
 3       1    flags          bit 0 = IS_LAST_CHUNK
 4       4    seq_id         uint32, per-connection, wraps at 2^32
 8       4    payload_len    uint32, MVP cap 65535 bytes
12       4    crc32          CRC-32/ISO-HDLC over bytes [0..11] + payload
16       N    payload
```

`server/include/server/frame.hpp` / `server/src/frame.cpp` implement exactly
this. A bad CRC or `payload_len > 65535` means the client must close and
reconnect.

### msg_type — defined vs. actually handled

| Value | Name | Defined in frame.hpp | Handled by `TransportGateway` |
|---|---|:-:|:-:|
| 0x01 | HANDSHAKE | ✓ | ✓ |
| 0x03 | HEARTBEAT | ✓ | ✓ |
| 0x10 | COMMAND | ✓ | ✓ |
| 0x11/0x12 | COMMAND_ACK/NAK | ✓ | ✓ (outbound) |
| 0x20 | DOMAIN_EVENT | ✓ | ✓ (outbound, broadcast) |
| 0x30/0x31 | SNAPSHOT_REQUEST/CHUNK | ✓ | ✓ |
| 0x50/0x51 | TAKEOVER_REQUEST/RESPONSE | ✓ | **no read/write path** |
| 0x60 | CHAT_MESSAGE | ✓ | **no read/write path** |
| 0x61 | DISPATCH_CHANNEL_MESSAGE | ✓ | ✓ |
| 0x70–0x72 | VOICE_CHAN_JOIN/LEAVE/STATE | ✓ | **no read/write path** |

TAKEOVER, CHAT, and VOICE constants and FlatBuffers schemas
(`proto/ownership.fbs`, `proto/chat.fbs`, `proto/voice.fbs`) exist so the wire
format is stable, but `TransportGateway`'s dispatch only handles `kHandshake`,
`kSnapshotRequest`, `kCommand`, `kHeartbeat`, `kDispatchChannel`
(`server/src/transport_gateway.cpp`). Building these features is adding a
handler branch plus the domain logic behind it, not designing a new protocol.

### Commands (cmd_type, C → S, msg_type 0x10)

| cmd_type | Name | Notes |
|---|---|---|
| 0x01 | SetSwitchPosition | |
| 0x02 | SetSignalAspect | manual override; `RequestRoute` is the primary path |
| 0x03 | SetDerailerPosition | |
| 0x04 | SetBlockSection | |
| 0x05 | RequestRoute | BFS path resolution + atomic lock/aspect/route creation |
| 0x06 | CancelRoute | |
| 0x07 | AcknowledgeAlarm | |
| 0x08 | SetBlockDirection | ML8 only — NAK `UNSUPPORTED` on EbiLock |
| 0x09 | InitAxleCounterReset | ML8 only (SLI) |
| 0x0A | ResetAxleCounter | ML8 only (SLK) |
| 0x20 | OperatorCommand | EbiLock/EbiScreen catalog |
| 0x21 | Ml8Command | ML8 catalog |

### COMMAND_NAK reason codes

| Code | Name | Checked by |
|---|---|---|
| 0x00 | UNSPECIFIED | fallback, not emitted intentionally |
| 0x01 | NOT_FOUND | SRK library |
| 0x02 | SAFETY_BLOCK | SRK library |
| 0x03 | INVALID_STATE | SRK library |
| 0x04 | ROUTE_LOCKED | SRK library |
| 0x05 | NO_PATH | SRK library |
| 0x06 | SWITCH_MOVING | SRK library |
| 0x07 | UNSUPPORTED | SRK library |
| 0x08 | UNAUTHORIZED | `OwnershipGuard` — see caveat in §14 |
| 0x09 | SESSION_PAUSED | session state, pre-engine |

Single source of truth: `srk/common/include/srk/common/nak_codes.hpp`.

### DOMAIN_EVENT catalog (event_type, S → C broadcast, msg_type 0x20)

Payload starts with `event_type` (1 byte), `event_id` (4 bytes, monotonic),
`timestamp_us` (8 bytes), then the FlatBuffers body. Implemented in
`server/src/dispatch_bus.cpp`:

| event_type | Name | Status |
|---|---|---|
| 0x01 | SwitchPositionChanged | implemented |
| 0x03 | SignalAspectChanged | implemented |
| 0x04 | TrackSectionOccupancyChanged | implemented |
| 0x05 | DerailerPositionChanged | implemented |
| 0x06 | BlockSectionStateChanged | implemented |
| 0x07 | RouteSet | implemented |
| 0x08 | RouteReleased | implemented |
| 0x09 | TrainMovement | **reserved, not emitted** (per-section occupancy via 0x04 is the only train-position signal today) |
| 0x0A | AlarmRaised | implemented |
| 0x0B | AlarmCleared | implemented |
| 0x0C | OperatingPointOwnershipChanged | **not implemented** |
| 0x0D | SessionStateChanged | **not implemented** |
| 0x0E | TrainComposed | **not implemented** |
| 0x0F | TrainDecomposed | **not implemented** |
| 0x10 | BlockDirectionStateChanged | implemented |
| 0x11 | OperatorCommandStateChanged | implemented |
| 0x12 | Ml8CommandStateChanged | implemented |

### Snapshot (msg_type 0x31)

Built from `AtomicSnapshot::load()` → FlatBuffers `Snapshot` table, chunked at
≤64 kB with `IS_LAST_CHUNK`. Contains `switches`, `track_sections` (with a
`train_uid` field joined from the active-trains list), `signals`, `derailers`,
`block_sections` (with `BlockDirectionState` for SHL-12), `active_routes`,
`active_alarms`, and `trains` (`TrainState`, one per active train). Each
device record carries `operator_state: OperatorCommandRuntimeState` so a
reconnecting client doesn't need to replay events to reconstruct active
command flags.

---

## 8. Vehicle and train model

Fleet data (`symulator-data` repo, `data/` root) is split three ways:

```
data/
  vehicle_types/   physical properties shared by a model (one JSON per type)
  vehicles/        individual numbered units; vehicle.json + optional photos/
  trains/          ordered vehicle-UID lists forming a consist (passenger/, freight/, maintenance/)
  carriers/carriers.json   optional carrier/operator registry
```

`engine/src/fleet_registry.cpp` scans `vehicle_types/` and `trains/`
recursively; vehicle instances are loaded only from files literally named
`vehicle.json`. Today only `trains/freight/` has populated data; the
`passenger/`/`maintenance/` folders are supported by the loader but empty.

### Vehicle type / instance

Both use a numeric `uid` field (see §3) — **not** the string `typeID`/`gID`
scheme shown in older examples. Everything else lines up with the loader
(`engine/include/engine/core/fleet_registry.hpp`, `VehicleType`/`VehicleInstance`):
`typeName`, `vehicleType`/`vehicleSubtype`, `pkpSeries`, `family`, `lengthM`,
`axleCount`, `massEmptyT`/`massGrossT`, `maxSpeedKmh`, `brakingLambdaPct`,
`powerKW`, `tractionForceKN`, `multipleCouplingCapable`, `davisA/B/C`.
Instance fields override type fields only where present
(`effective(field) = instance.field ?? type.field`), resolved once at load
time.

`tractionStatus` (`OPERATIONAL`/`DEFECTIVE`) applies only to traction-capable
instances (`LOCOMOTIVE`, `EMU_UNIT`+`MOTOR`, `DMU_UNIT`+`MOTOR`); defaults to
`OPERATIONAL`. A train consist (`trains/**/*.json`) has a required
`trainCategory` (`PASSENGER | FREIGHT | MAINTENANCE` — matches the
`TrainCategory` enum in `types.hpp`; the loader checks the folder path
matches), an ordered `vehicle_uids` list, and an optional numeric `carrierId`
resolved against `carriers/carriers.json`.

At consist build time the engine derives:

```
total_length_m  = Σ vehicle.lengthM
total_axles     = Σ vehicle.axleCount
total_mass_t    = Σ effectiveMassT(v)
max_traction_kn : OPERATIONAL EMU/DMU motor cars sum; for LOCOMOTIVE — a single
                  operational unit uses its own traction, multiple operational
                  units of the same type only sum if that type has
                  multipleCouplingCapable=true, otherwise only the first
                  contributes (rest are ballast); DEFECTIVE units always add
                  mass/drag but never traction.
```

### Physics (`engine/include/engine/physics/`)

`TrainPhysicsParams{total_mass_t, max_traction_kn, max_speed_ms, davis_A/B/C}`
aggregated once per consist. Per tick:

```
F_resistance = davis_A + davis_B*v_kmh + davis_C*v_kmh^2
a            = (F_traction - F_resistance - F_brake) / mass_kg
v_new        = clamp(v + a*dt, 0, v_max)
x_new        = x + 0.5*(v + v_new)*dt
```

Braking: `a_brake_max = clamp(lambda_pct/100 * g * 0.85, 0.3, 8.0)`,
`F_brake_max = a_brake_max * total_mass_t`. `DriverAI` is a deterministic
four-state automaton — `STOPPED / ACCELERATING / CRUISING / BRAKING` — driven
by current aspect, distance to the next signal, and consist limits; in
`CRUISING` it targets traction ≈ Davis resistance to avoid speed oscillation.

### Axle counting / occupancy

`TrainFleet` (`engine/src/train_fleet.cpp`) tracks each active train's
position against the topology graph. The leading edge occupies the section
ahead as soon as it crosses a boundary; the trailing edge frees the section
behind once the running axle count reaches `total_axles` — mirroring real
axle-counter behavior. This is what emits `TrackSectionOccupancyChange`
(§4/§7), with a real per-consist axle count, not a hardcoded value.

`TrainFleet::spawn()`/`despawn()` validate against the current `IStateView`
(section must exist and be `FREE` to spawn), update occupancy, and append
both a `DeviceStateChange` and a DB-only `PipEvent`. `spawn_resolver.hpp`
resolves "the one free section behind this boundary node" so a caller only
needs to name a boundary, not a specific section.

---

## 9. Track topology and scenario data

Each station is a directory with three files, loaded by
`engine/src/topology_loader.cpp` via a per-object-kind parser table (adding a
new topology object kind means one parser function plus one registry row, no
changes to the public loader API):

```
scenarios/<station>/
  meta.json       station_sid, area, line_numbers, control_system, schema_version
  topology.json   boundary_nodes, track_sections, switches, block_sections
  objects.json    signals, derailers
  layouts/*.json  versioned presentation geometry referencing infrastructure UIDs
```

`meta.json → "control_system"` (`"ebilock_x4"` or `"estw_ml8"`) selects the
`IControlSystem` implementation via `ControlSystemRegistry` at session start.

### `topology.json` shapes (real, current)

```json
// boundary node
{ "uid": 2302102536193, "pID": "l202_granica_poludniowa", "description": "..." }

// track section — sideA/sideB, numeric neighbor/counter/signal UIDs
{
  "uid": 2280627699713, "pID": "l202_tor1a",
  "sideA": { "neighborUID": 2280627699731, "itUID": 2310692470785, "signalUIDs": [2289217634305] },
  "sideB": { "neighborUID": 2284922667009, "izUID": 2310692470786, "signalUIDs": [2289217634307] },
  "lengthM": 200.0, "electrified": true, "maxSpeedKmh": 110, "occupied": false
}

// switch — trunk/straight/divergent legs; typeID references data/device_types (string, see §3)
{
  "uid": 2284922667009, "pID": "l202_zwr1", "typeID": "DVT-GLB-ZWR-EEA4-0000002",
  "trunk":     { "neighborUID": 2280627699713, "izUID": 2310692470786, "signalUIDs": [] },
  "straight":  { "neighborUID": 2280627699715, "izUID": 2310692470789, "signalUIDs": [] },
  "divergent": { "neighborUID": 2280627699717, "izUID": 2310692470793, "signalUIDs": [] },
  "lengthM": 28.5, "maxSpeedStraightKmh": 110, "maxSpeedDivergentKmh": 60
}

// block section (SHL-12) — the cross-station link
{
  "uid": 2297807568897, "pID": "blk_l202_gor_sp", "type_id": "SHL-12",
  "neighborStationUID": 2272037830657, "lineNumber": 202,
  "departureSignalUID": 2289217634307, "entrySignalUID": 2289217634305,
  "szlakSectionUIDs": [2280627699729, 2280627699730, 2280627699731]
}
```

`objects.json` holds `signals` (`uid`, `pID`, `typeID`, `type` —
`ENTRY`/`DEPARTURE`/`BLOCK`/`SHUNTING`, `initial_aspect`,
`governs_section`) and `derailers`.

Track geometry is deliberately absent from `topology.json`. `layouts/*.json`
stores logical-unit paths, named switch ports, signal attachments and labels, so
one physical graph may have EbiScreen, technical and future control-panel
presentations. The machine-readable contract is
`schemas/track-layout.schema.json`; the module boundaries, SOLID guarantees and
rendering test strategy are documented in
[`22-track-layout-and-rendering.md`](22-track-layout-and-rendering.md).

### Cross-station connectivity ("Model A")

Adjacent stations reference each other directly: a `BlockSection`'s
`neighborStationUID` points at the other station's UID (from
`scenarios/stations.json`, §3), and its `szlakSectionUIDs` list the shared
line-section chain. There is **no separate `sections.json` / Route Editor
format** — an earlier design considered one, but the implemented approach is
this direct UID cross-reference between each station's own `topology.json`,
requiring no extra data structure. `scenarios/gdynia_orlowo` and
`scenarios/sopot` are the current reference pair (L202/L250 lines).

---

## 10. Dispatch forms (S-forms) and EDR

Dispatch forms (*formularze zapowiedniowe*, prefix **S**) implement: *a train
may not depart toward a neighbouring line-control section until that section
confirms the line is clear.*

| Form | Direction | Purpose |
|---|---|---|
| S2 | A→B | Dispatch request |
| S24 | B→A | Line-clear grant |
| S25 | A→B | Departure notice |
| S26 | B→A | Arrival confirmation (closes the exchange) |
| S35 | A→B | Cancel a pending S2 |
| S51/S52 | A→B / B→A | Level-crossing notification / ack |
| S55/S56 | A→B / B→A | Dangerous-goods variant of S2/S24 |
| S76 | A↔B | Free-form remark, stored alongside the other forms for one audit trail |

State machine: `IDLE → S2_SENT → S24_RECEIVED → S25_SENT → S26_RECEIVED →
CLOSED`, with `S35` cancelling from `S2_SENT`. `session.edr_entries.track_clear_time`
is set at `S24_RECEIVED`; a duplicate S24/S56 for an already-cleared entry is
rejected with a dispatcher-visible warning instead of silently overwritten.

Server components (all implemented, `server/include/server/` +
`server/src/`):

- **`DispatchExchangeManager`** — pure logic, no I/O, one state machine per
  `(src_area, dst_area)` pair. `submit_telegram(...)`, `status(...)`,
  `close(...)`. `exchange_id` format: `"exch-0000001"`.
- **`DispatchCoordinator`** — wraps the manager, persists accepted telegrams
  to `session.dispatch_telegrams` via `IDbWriter`, updates
  `edr_entries.track_clear_time` on S24/S56, notifies `EdrCoordinator` on
  S25/S26.
- **`DispatchChannel`** — wire layer: parses `DISPATCH_CHANNEL_MESSAGE`
  (msg_type 0x61), verifies sender, delegates to `DispatchCoordinator`,
  broadcasts the result to the `(src, dst)` pair.
- **`EdrCoordinator`** — updates `session.edr_entries` for departure/arrival.
- **`PipWriter`** — see §11.

---

## 11. Persistence (PostgreSQL)

One instance, three schemas (`docker/init.sql` is the actual DDL). `fleet` is
read-mostly static reference data imported at startup; `session` is the
write-heavy operational log; `pip` is written exclusively by `PipWriter` and
never by anything else (kept separate specifically so a stall there can't
share a write path with the append-only `session.events` log).

| Table | Written by | Status |
|---|---|---|
| `fleet.vehicles`, `fleet.train_definitions`, `fleet.train_definition_vehicles` | startup import | implemented |
| `fleet.timetable_templates` | intended: startup import | **DDL exists, nothing writes to it yet** |
| `session.sessions` | `PgDbWriter::init_session()` | implemented |
| `session.events` | `DispatchBus` (via `IDbWriter`) | implemented |
| `session.snapshots` | intended: periodic snapshot persistence | **DDL exists, `save_snapshot()` is never called** |
| `session.edr_entries`, `session.edr_journal_entries` | `EdrCoordinator` | implemented |
| `session.dispatch_telegrams` | `DispatchCoordinator` | implemented |
| `session.operating_point_assignments` | intended: `OwnershipGuard`/takeover flow | **DDL exists, never written** (matches §14 — takeover isn't wired) |
| `session.chat_log` | intended: CHAT_MESSAGE handling | **DDL exists, never written** (matches §7 — chat isn't wired) |
| `pip.track_state` | `PipWriter` | implemented |

`pip.track_state` holds one row per `(session_id, section_gid)` with a JSONB
`trains` array of `TrainSlot{number, has_extra_info, manually_placed,
entry_side}` — 0 entries means the track is empty, the server never truncates
the list, and the client is responsible for building any column/alternating
view from the full array. Retention: `pip.track_state` rows are deleted when
a session ends (fully reconstructable from `session.events`); `fleet.*` is
never deleted except on explicit re-import; everything else in `session.*`
retains for session lifetime + 30 days (enforced by an external maintenance
job, not application code).

---

## 12. Server terminal (operator CLI)

A built-in stdin terminal (`server/include/server/terminal/`), reachable once
the server process is running — no separate tool. Every command is its own
class behind a common interface so permissions and the command set can grow
without touching a central dispatcher:

- **`ITerminalCommand`** — `name()`, `help()`, `required_permission()`,
  `execute(args)`.
- **`Permission`** enum — `SPAWN, DESPAWN, KICK, BAN, RESET, SETTINGS, LOGS`.
- **`CommandRegistry`** — `add()` (throws on duplicate name), `find()`,
  `for_each()`.
- **`IUserStore`** — swappable auth backend. Today: `InMemoryUserStore`, one
  built-in account, **plaintext `admin`/`admin123`**. Planned: a DB-backed
  store with hashed passwords and a real user table (schema not yet decided).
- **`TerminalSession`** — tokenizes input, handles `login`/`logout`/`help`,
  dispatches to the registry only after login and a permission check (e.g. a
  moderator role could be granted `SPAWN`/`DESPAWN` but not `RESET` without
  any engine change).
- **`ITerminalTransport`** / **`StdinTerminal`** — the transport is abstracted
  too; stdin is the only implementation today.

Implemented commands: `spawn <consist> <boundary>`, `despawn <train>`,
`trains` (list active trains). Arguments accept either a decimal UID or a
`pID` string, resolved via `server/include/server/terminal/lookup.hpp`
against `FleetRegistry`/`EngineSnapshot`. **Caveat:** boundary-node `pID`s are
not unique across stations (e.g. both `gdynia_orlowo` and `sopot` can have a
`l202_granica_*`-style name) — prefer UID when a scenario has more than one
station loaded.

---

## 13. Build, test, and contribute
    
Full setup instructions (OS-specific installers, SSH keys, etc.) are not
repeated here — run `python3 scripts/install_system_deps.py` (or the Windows
`py` equivalent) once, then:

> **Windows Note**: `scripts/configure_ninja.py` automatically detects and sets up the MSVC environment using `vswhere`. There's no need to manually launch the 'x64 Native Tools Command Prompt'.

```bash
# Full build (Qt6 client+editor):
python3 scripts/configure_ninja.py --build-type Debug --configure-only
cmake --build build/ninja-debug

# Headless build (server + tests only, no Qt6):
python3 scripts/configure_ninja.py --build-type Debug --headless --configure-only
cmake --build build/ninja-debug-headless
```

Tests (GoogleTest, grouped by `scripts/run_grouped_ctest.py`):

```bash
python3 scripts/run_grouped_ctest.py \
  --test-dir build/ninja-debug-headless \
  --reports-dir build/ninja-debug-headless/test-reports \
  --profile pre-db          # unit + non-DB integration

python3 scripts/run_grouped_ctest.py ... --profile db   # requires SYMULATOR_TEST_DB
```

**Git hooks** (installed automatically the first time you run `cmake`, via
`git config core.hooksPath scripts/git-hooks/`):
1. `CHANGELOG.md` must be staged on every commit.
2. Staged `.cpp`/`.hpp`/`.h` files must pass `clang-format` (skipped with a
   warning if clang-format isn't installed).

**FlatBuffers codegen**: edit `proto/*.fbs`, then run
`bash scripts/autogens/generate_proto_headers.sh` (or just build — the
`generate_proto_headers` CMake target regenerates automatically when a
schema is newer than its header) and commit the regenerated files under
`autogens/proto/`. CI's `validate_proto_schemas` target checks committed
headers match the schemas.

**UID data validation**: `scripts/validate_uid_registry.py` (also a CMake
target, run in CI) — see §3.

Commit convention: `type(scope): imperative description`
(`feat`/`fix`/`docs`/`refactor`/`test`/`chore`; scope is the touched module).
Never push to `main` directly, never push version tags (release-only, project
owner), never commit `build/` or `.scendb` files (`.scendb` doesn't currently
exist in the repo — see §14).

---

## 14. Not implemented / explicitly out of scope today

Kept as one list so it's easy to check before assuming something exists:

- **Operator client, scenario editor, `libtrackview`** — all three are
  3-line stub `main()`s / an empty namespace. Qt6 is wired into CMake and
  vcpkg but no window, canvas, or rendering code exists yet.
- **Scenario editor toolchain** — no `.scendb` SQLite format, no
  `IScenarioLinter`/`LintDiagnostic`/`ScenarioBundle` types, no `sections.json`
  Route Editor format. `libscenario_validation` **does** exist and is real
  (three-layer validator: `layer1_validator`, `layer2_validator`,
  `layer3_validator`), but it's a different, simpler shape than the
  interface once sketched for the editor.
- **ETCS/RBC supervisory layer** — zero code anywhere (`engine/`, `server/`,
  `srk/`, `proto/`, `tests/`); a full concept spec existed describing RBC
  areas, ETCS sessions, Movement Authorities, GSM-R, DSAT, TSR, but none of
  it has been started.
- **TAKEOVER / ownership enforcement** — `OwnershipGuard` exists and is
  constructed, but no command path actually calls it; `check`/`set_owner`/
  `get_owner`/`release` are exercised only by unit tests that build it as a
  DI dependency. TAKEOVER_REQUEST/RESPONSE frames are unhandled (§7). In
  practice, no ownership/authorization is enforced on commands yet, despite
  `UNAUTHORIZED (0x08)` existing as a NAK code.
- **Chat and voice** — wire formats defined (`proto/chat.fbs`,
  `proto/voice.fbs`, msg_types 0x60/0x70–0x72) but zero handling code.
- **`TrainMovement` (0x09), `TrainComposed`/`TrainDecomposed`,
  `OperatingPointOwnershipChanged`, `SessionStateChanged`** — reserved
  event_type values with no emitter.
- **`session.snapshots`, `session.chat_log`,
  `session.operating_point_assignments`, `fleet.timetable_templates`** — DDL
  exists in `docker/init.sql`; nothing in the codebase writes to them yet.

- **Password hashing / DB-backed user store** — terminal auth is a single
  hardcoded plaintext account today (§12).
- **AI Dispatch Module** — post-MVP by design; no code, and the engine has no
  AI-specific hook implemented (a `RandomEvent`/`IRandomEventSource`-style
  injection point was discussed but is not present in `types.hpp` today —
  verify before relying on it if you pick this up).

---

## 15. Open questions worth resolving

Trimmed to the ones that still block a specific piece of future work (many
older open questions were already resolved by the implementation above and
are omitted):

- **Route partial clearing.** Long freight trains crossing multiple sections
  on one route currently release the whole route only after the *last*
  section clears — section-by-section release isn't implemented.
- **Forced route cancel.** `CancelRoute`'s `force=true` flag is part of the
  command shape but not enforced in `device_rules.cpp` (a train present still
  blocks cancellation unconditionally).
- **SHL-12 neighbour confirmation.** `BlockDirectionChange`'s
  `requires_neighbor_confirmation` flag is set after `BLW`, but waiting for
  the neighbouring station's `BLP` isn't modelled at the multi-station
  session layer yet — today each station's SHL-12 state machine is
  effectively independent.
- **Speed-limit enforcement per signal aspect.** `DriverAI` reacts to stop
  aspects but there's no explicit tie between e.g. `MS2` and a 40 km/h cap
  enforced by the physics layer.
- **PLK timetable import.** Format and station-code mapping for pulling real
  PKP/PLK schedules is unresolved; explicitly out of the server binary's
  responsibility per an earlier decision (a separate integration tool, not
  `server/`).
- **Multi-LCS dispatch chains.** Whether a train crossing more than one
  line-control-section boundary before the previous S26 arrives opens
  independent exchanges per pair (current assumption) or needs a chained
  model.
