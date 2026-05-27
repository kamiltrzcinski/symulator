# Implementation Skeleton

**Status:** Domain layer, SRK interface, ENGINE tick loop, server composition root, and first-pass dispatch exchange manager complete.  Remaining work: EDR coordinator, DB persistence pipeline, PLK importer, full client.

## Purpose

This document describes the server-side class structure and module wiring as of the current implementation state.  Earlier revisions were marked "pre-commands / pre-devices" scaffolding.  The domain layer is now implemented; remaining work is the ENGINE tick-loop integration and client broadcast pipeline.

---

## Implemented domain layer

### engine/core headers

| Header | Description |
|--------|-------------|
| `types.hpp` | Vocabulary types: `GID`, `SID`, `DispatchAreaID`, `PlayerID`, `ControlSystemID`; all enums (`SwitchPosition`, `SignalAspect`, `DerailerState`, `BlockSectionState`, `BlockDirectionState`, `TrackOccupancy`, `ChangeCause`, …) |
| `track_model.hpp` | Value-type structs for runtime device state: `TrackSection`, `Switch`, `Signal`, `Derailer`, `BlockSection`, `RouteState`, `AlarmState`, `BoundaryNode`; `OperatorCommandRuntimeState` (holds `optional<OperatorCommandCode>` + `optional<Ml8CommandCode>`) |
| `command.hpp` | `Command = std::variant<...>` (10 types, cmd_type 0x01–0x0A); `Shl12Op` enum; `CommandMeta`; `EnvelopedCommand`; `operator_command_code_name()` + `ml8_command_code_name()` helpers |
| `state_view.hpp` | `IStateView` — pure read-only interface; `find_*` and `for_each_*` methods |
| `engine_state.hpp` / `.cpp` | `EngineState : IStateView` — mutable world state owned by ENGINE thread; `insert_*`, `apply_*` mutators |
| `engine_snapshot.hpp` / `.cpp` | `EngineSnapshot : IStateView` — immutable deep copy; `AtomicSnapshot` for lock-free cross-thread reads |
| `control_system.hpp` | `DeviceStateChange` variant (11 types); `InterlockingViolation`; `IControlSystem` interface |
| `control_system_registry.hpp` / `.cpp` | Singleton factory; `register_static()` for static-init self-registration |

### srk/ libraries

| Library | Registered ID | Rules |
|---------|---------------|-------|
| `libsrk_common` | — (helper, not standalone) | R1–R10 check/execute helpers; `check/execute_operator_command`; `tick_switch_machines`; `find_route_path()` BFS; `nak_codes.hpp` (0x00–0x09) |
| `libsrk_ebilock` | `"ebilock_x4"` | R1–R10 via srk_common; EEA-4 throw timer; route auto-release |
| `libsrk_ml8` | `"estw_ml8"` | R1–R10 via srk_common + ML8-specific commands (R8–R10: BLW/BLP/BLO/BLZ/BLAI/BLA/OPS/SLI/SLK) |

> **Linker note:** `libsrk_ebilock` and `libsrk_ml8` must be linked into `symulator-server` with `$<LINK_LIBRARY:WHOLE_ARCHIVE,...>` (CMake 3.24+) to prevent the linker from dropping their static-init object files.  Without this, `ControlSystemRegistry::register_static()` never runs and the server crashes with "Unknown control_system".  See `server/CMakeLists.txt`.

### server/ additions

| File | Description |
|------|-------------|
| `server/include/server/dispatch_exchange_manager.hpp` | `DispatchExchangeManager` — pure-logic S-form state machine per `(src_area, dst_area)` pair; `TelegramResult`, `TelegramOutcome` types |
| `server/src/dispatch_exchange_manager.cpp` | Implementation; `generate_exchange_id()` produces `"exch-0000001"` format |

---

## Implementation status

**Completed:**
- ENGINE tick loop: `StateApplier` visitor, `IControlSystem::on_tick()`, `check_command()` + `execute_command()` ✅
- Topology loader: `topology.json` + `objects.json` → `EngineState` inserts (GID scheme: `l202-` prefix) ✅
- `SnapshotService`: `AtomicSnapshot::load()` → FlatBuffers → chunked `SNAPSHOT_CHUNK` ✅
- `DeviceStateChange` → `DomainEvent` wire encoding → `DOMAIN_EVENT (0x20)` broadcast ✅
- Operator command events `0x11 OperatorCommandStateChanged`, `0x12 Ml8CommandStateChanged` ✅
- `DispatchExchangeManager`: S-form state machine, 17 unit tests ✅
- `scripts/e2e_smoke_test.py`: HANDSHAKE + SNAPSHOT round-trip smoke test ✅

**Remaining (next phases):**
- `ZapowiedniowiecManager`: DB persistence for `session.dispatch_telegrams`, EDR `track_clear_time` update
- `DbWriter` pipeline: event/snapshot/chat retention
- `EdrCoordinator`: EDR view integration
- PLK importer (`IPLKImporter`)
- Full Qt6 client broadcast rendering

> **Test infra note:** `tests/engine/CMakeLists.txt` sets `SCENARIO_DIR="${CMAKE_SOURCE_DIR}/scenarios/gdynia_orlowo"` via compile definition so `test_topology_loader.cpp` can locate scenario files without hardcoding paths.

The sections below describe the remaining wiring and interface contracts.

---

## Server-side module map (target)

| Module | Thread / Context | Responsibility now | Responsibility later |
|---|---|---|---|
| `TransportGateway` | `IO_POOL` | frame ingress/egress, connection lifecycle | full protocol endpoint behavior |
| `CommandIngress` | `WORK_POOL` | deserialize + envelope validation | full command parsing + schema upgrades |
| `OwnershipGuard` | `WORK_POOL` | ownership check hook point | real operating point authorization matrix |
| `EngineLoop` | `ENGINE` | tick scheduler + queue drain | device/interlocking mutation logic |
| `DispatchBus` | dedicated `DISPATCH_BUS` thread | subscriber fan-out, backpressure boundary | routing policies per event family |
| `SnapshotService` | `DISPATCH_BUS` + storage | snapshot request handling hook point | snapshot delta/full strategy |
| `DbWriter` | dedicated `DB_WRITER` thread | async persistence sink | event/snapshot/chat retention jobs |
| `PipWriter` | dedicated `PIP_WRITER` thread | async PIP projection sink | full PIP lifecycle projections |
| `EdrCoordinator` | `DISPATCH_BUS` subscriber | integration boundary only | full EDR exchange manager |
| `AiAdapter` | optional subscriber | integration boundary only | AI virtual operator runtime |

---

## Internal bus contract

### Design intent

`DispatchBus` is the **single in-process publication thread** for domain events.

Benefits:
- deterministic subscriber ordering,
- one ownership point for event envelope lifetime,
- no cross-thread free/reuse races,
- bounded fan-out backpressure.

### Minimal interface skeleton

```cpp
class IDispatchBus {
public:
    virtual ~IDispatchBus() = default;

    virtual void publish_domain_event(DomainEventEnvelope event) = 0;
    virtual SubscriptionToken subscribe_domain_events(DomainEventHandler handler) = 0;
    virtual void unsubscribe(SubscriptionToken token) = 0;
};
```

`DomainEventEnvelope` remains generic at this stage (no speculative payload fields).

---

## Registration model

All modules register in startup order through one composition root:

```cpp
class ModuleRegistry {
public:
    void register_transport(TransportGateway&);
    void register_command_ingress(CommandIngress&);
    void register_snapshot_service(SnapshotService&);
    void register_db_writer(DbWriter&);
    void register_pip_writer(PipWriter&);
    void register_edr(EdrCoordinator&);
    void register_ai(AiAdapter&);
};
```

### Startup wiring sequence

1. Start fixed queues and dedicated threads (`ENGINE`, `DISPATCH_BUS`, `DB_WRITER`, `PIP_WRITER`).
2. Construct core service boundaries (`IDispatchBus`, queue adapters, stores).
3. Register subscribers on `DispatchBus` (DB, PIP, transport fan-out, optional EDR/AI).
4. Start network ingress (`IO_POOL`, `WORK_POOL`).
5. Accept sessions and commands.

### Shutdown wiring sequence

1. Stop ingress (`IO_POOL`, `WORK_POOL`).
2. Close command queue.
3. Stop `ENGINE` loop.
4. Drain and close `DispatchBus` queue.
5. Drain `DB_WRITER` and `PIP_WRITER` queues.
6. Join all dedicated threads.

---

## Request handling skeletons

### A) COMMAND frame

1. `TransportGateway` receives frame.
2. `CommandIngress` deserializes envelope; maps `cmd_type` byte to `Command` variant.
3. `OwnershipGuard` checks `player_id` owns `operating_point_id` (NAK 0x08 on failure).
4. Accepted command goes to `PriorityCommandQueue` for `EngineLoop`.
5. **[TODO]** `EngineLoop` calls `IControlSystem::check_command(engine_state, cmd)` — NAK on violation.
6. **[TODO]** `EngineLoop` calls `IControlSystem::execute_command(engine_state, cmd)` — applies each `DeviceStateChange` via `StateApplier`.
7. `EngineLoop` emits resulting domain events.
6. `DispatchBus` fans out to:
   - client broadcast,
   - DB writer,
   - PIP projection,
   - optional EDR/AI listeners.

### B) TAKEOVER flow

1. `TransportGateway` receives request.
2. `OwnershipGuard` applies policy.
3. response returned to requester.
4. ownership-change domain event published to `DispatchBus`.

### C) SNAPSHOT request

1. `TransportGateway` receives request.
2. `SnapshotService` calls `AtomicSnapshot::load()` to get the latest `EngineSnapshot`.
3. Snapshot serialized to FlatBuffers and sent as `SNAPSHOT_CHUNK` frames.

---

## Interface placeholders (safe to create before ENGINE integration)

Use these as stable seams for the remaining implementation phases:

```cpp
class ICommandEnvelopeParser;   // FlatBuffers bytes → Command variant
class ICommandAuthorizer;        // ownership + session-state pre-check
class ICommandQueue;             // PriorityCommandQueue<EnvelopedCommand>
class IEngineTickRunner;         // 20 Hz timer; calls IControlSystem + StateApplier
class IDomainEventStore;         // DeviceStateChange → DomainEventEnvelope
class ISnapshotStore;            // AtomicSnapshot access
class IPipProjectionStore;       // pip.track_state UPSERT sink
class IClientBroadcastSink;      // per-station broadcast to IO_POOL strands
```

---

## Error model skeleton

Recommended envelope status codes (transport-agnostic):
- `OK`
- `UNAUTHORIZED`
- `INVALID_FRAME`
- `UNSUPPORTED_MESSAGE`
- `SAFETY_BLOCKED`
- `SESSION_UNAVAILABLE`
- `INTERNAL_ERROR`

These can map later to protocol-level ACK/NAK reasons.

---

## Observability points to add immediately

Even before functional device logic:
- ingress frames/s by msg family,
- queue depth (`command`, `dispatch`, `db`, `pip`),
- dispatch fan-out latency,
- per-thread loop duration (`ENGINE`, `DISPATCH_BUS`),
- rejected request counters by status code.

---

## What must wait for ENGINE tick-loop integration

Intentionally not yet done (see Q-SRK-1 in [doc 17](17-control-system-interface.md)):
- `StateApplier` visitor (`std::visit` over `DeviceStateChange` → `EngineState` mutation),
- `IControlSystem::on_tick()` call in the 20 Hz loop,
- `IControlSystem::check_command()` + `execute_command()` call-chain in `EngineLoop`,
- topology loader (`topology.json` → `EngineState::insert_*`),
- `DeviceStateChange` → FlatBuffers wire encoding → `DOMAIN_EVENT` broadcast.

---

## Definition of done for this skeleton stage

1. All module boundaries are named and owned by a thread/context.
2. Registration and shutdown sequence are explicit and documented.
3. No class assumes unknown command/device semantics.
4. Protocol and architecture docs are internally consistent with this model.
