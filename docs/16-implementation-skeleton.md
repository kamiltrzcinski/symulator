# Implementation Skeleton (Pre-Commands / Pre-Devices)

## Purpose

This document defines **class-level scaffolding** and **module registration flow** that can be prepared now, without guessing device behavior or command semantics.

Current rule:
- no speculative interlocking/device logic,
- no speculative command payload semantics,
- only stable wiring, boundaries, and responsibilities.

---

## Current constraint baseline

Not yet available (blocking functional implementation):
- final command catalog and command semantics,
- complete device catalog and runtime representation,
- final topology/device coupling rules.

Therefore, this stage focuses on:
- interfaces,
- bus/thread ownership,
- lifecycle wiring,
- error propagation contracts,
- observability points.

---

## Server-side module map (target)

| Module | Thread / Context | Responsibility now | Responsibility later |
|---|---|---|---|
| `TransportGateway` | `IO_POOL` | frame ingress/egress, connection lifecycle | full protocol endpoint behavior |
| `CommandIngress` | `WORK_POOL` | deserialize + envelope validation | full command parsing + schema upgrades |
| `OwnershipGuard` | `WORK_POOL` | ownership check hook point | real posterunek authorization matrix |
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
2. `CommandIngress` deserializes envelope.
3. `OwnershipGuard` validates ownership.
4. Accepted command goes to priority queue for `EngineLoop`.
5. `EngineLoop` emits resulting domain events.
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
2. `SnapshotService` builds response using snapshot provider/store.
3. snapshot chunks sent back to client.

---

## Interface placeholders (safe to create before command/device commit)

Use these as stable seams:

```cpp
class ICommandEnvelopeParser;
class ICommandAuthorizer;
class ICommandQueue;
class IEngineTickRunner;
class IDomainEventStore;
class ISnapshotStore;
class IPipProjectionStore;
class IClientBroadcastSink;
```

At this stage they should define only:
- input/output ownership,
- error model,
- threading guarantees.

Do not lock payload schemas yet.

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

## What must wait for the command/device commit

Blocked (intentionally):
- concrete `Command` variant/types,
- concrete `DomainEvent` variant/types,
- device state model and mutation rules,
- interlocking checks and route conflict indexing,
- command-to-device execution mapping.

---

## Definition of done for this skeleton stage

1. All module boundaries are named and owned by a thread/context.
2. Registration and shutdown sequence are explicit and documented.
3. No class assumes unknown command/device semantics.
4. Protocol and architecture docs are internally consistent with this model.
