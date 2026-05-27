# Server Internal API

## Context

The system has no REST/HTTP external API (resolved, N-009). "Server API" means the **C++ interface contracts between internal modules**. The client-facing wire protocol is fully defined in `09-communication-contract.md`.

This document defines module boundaries as pure virtual C++ interfaces. Each interface is the unit of substitution (mock in tests, alternative implementation in production).

---

## Module boundary diagram

```
┌─────────────────────────────────────────────────────────────┐
│                      Server process                         │
│                                                             │
│  ┌──────────────┐   ICommandHandler   ┌──────────────────┐ │
│  │ SessionServer│ ──────────────────▶ │  SimulationEngine│ │
│  │              │ ◀────────────────── │                  │ │
│  │  (TCP loop,  │   IEventEmitter     │  (tick loop,     │ │
│  │   fanout,    │                     │   interlocking,  │ │
│  │   snapshot)  │ ◀────────────────── │   occupancy)     │ │
│  │              │   ISnapshotProvider │                  │ │
│  └──────────────┘                     └────────┬─────────┘ │
│         │                                      │           │
│  ISessionStore                        ITopologyStore        │
│         │                                      │           │
│  ┌──────▼──────────────────────────────────────▼─────────┐ │
│  │                  PostgreSQL (libpqxx)                  │ │
│  │            fleet schema │ session schema               │ │
│  └────────────────────────────────────────────────────────┘ │
│                                                             │
│  ┌──────────────┐   IEDRService                            │
│  │  EDRComponent│ ◀──────────── SimulationEngine (events) │ │
│  │              │ ──────────────▶ fleet + session schemas  │ │
│  └──────────────┘                                          │
│         ▲                                                   │

└─────────────────────────────────────────────────────────────┘
```

---

## Interface definitions

### ICommandHandler

Receives a validated, deserialized command from the SessionServer. Returns synchronously.

```cpp
struct CommandResult {
    bool        accepted;
    uint8_t     reason_code;   // 0 = OK; see COMMAND_NAK reason codes
    std::string reason_text;
    uint32_t    event_id;      // populated when accepted = true
};

class ICommandHandler {
public:
    virtual CommandResult handle(const Command& cmd) = 0;
    virtual ~ICommandHandler() = default;
};
```

### IEventEmitter

Engine calls this when state changes. SessionServer implements it; broadcasts to all connected clients and writes to `session.events`.

```cpp
class IEventEmitter {
public:
    virtual void emit(const DomainEvent& event) = 0;
    virtual ~IEventEmitter() = default;
};
```

### ISnapshotProvider

SessionServer calls this on client join/reconnect.

```cpp
class ISnapshotProvider {
public:
    virtual Snapshot buildSnapshot() const = 0;
    virtual ~ISnapshotProvider() = default;
};
```

### ITopologyStore

Loads and queries station topology. Implemented over the JSON bundle (loaded at session start, cached in memory). The engine uses this for interlocking graph traversal; the client has its own read-only copy loaded on demand (F-022).

```cpp
class ITopologyStore {
public:
    virtual void            loadStation(const std::string& stationSID) = 0;
    virtual const Station&  getStation(const std::string& stationSID) const = 0;
    virtual const Object&   getObject(const std::string& gID) const = 0;
    virtual ~ITopologyStore() = default;
};
```

### ISessionStore

Persistence operations. Implemented over PostgreSQL `session` schema.

```cpp
class ISessionStore {
public:
    virtual void appendEvent(const DomainEvent& event) = 0;
    virtual void saveSnapshot(const Snapshot& snapshot) = 0;
    virtual void assignOperatingPoint(const OperatingPointAssignment& assignment) = 0;
    virtual void releaseOperatingPoint(const std::string& operatingPointId,
                                   const std::string& clientID) = 0;
    virtual void appendChat(const ChatMessage& msg) = 0;
    virtual ~ISessionStore() = default;
};
```

### IEDRService

EDR component interface. Exposes timetable data to engine and accepts live updates from the operator.

```cpp
class IEDRService {
public:
    // Called at session start — loads from fleet.timetable_templates into session.edr_entries
    virtual void initSession(const std::string& sessionID,
                             const std::vector<std::string>& stationSIDs) = 0;

    // Called by engine on TrainMovement events
    virtual void notifyTrainArrival(const std::string& sessionID,
                                    const std::string& trainNumber,
                                    const std::string& stationSID,
                                    int64_t timestampUs) = 0;
    virtual void notifyTrainDeparture(const std::string& sessionID,
                                      const std::string& trainNumber,
                                      const std::string& stationSID,
                                      int64_t timestampUs) = 0;

    // Called by EDR operator (via client command)
    virtual void updateEntry(const EDREntryUpdate& update) = 0;

    // Read path for client EDR tab
    virtual std::vector<EDREntry> getEntries(const std::string& sessionID,
                                              const std::string& stationSID) const = 0;
    virtual ~IEDRService() = default;
};
};
```

---

## Call sequence: session startup

```
1. Server starts
2. ITopologyStore::loadStation()   × 9 stations  → topology graph in memory
3. Client connects → HANDSHAKE
4. IEDRService::initSession()      → session.edr_entries seeded from fleet.timetable_templates
5. ISnapshotProvider::buildSnapshot() → sent to client as SNAPSHOT_CHUNK(s)
6. Normal operation: client sends COMMAND → ICommandHandler::handle()
                     engine calls IEventEmitter::emit() → broadcast + ISessionStore::appendEvent()
```

---

## Open questions

- Q-API-1: Should `IEDRService` live in the same process as the engine (direct call) or in a separate process (Unix socket)? Direct call is simpler for MVP; separate process allows independent restart of EDR without stopping the engine.

- Q-API-3: Should the PLK station code → internal `sID` mapping be hardcoded in import configuration, or stored as a dictionary in `fleet.station_map`?

---

## Implemented module additions (as of 0.5.0)

### DispatchExchangeManager

`server::DispatchExchangeManager` (`server/include/server/dispatch_exchange_manager.hpp`) is a pure-logic, single-threaded manager for S-form dispatch exchange state machines.  It has no I/O and no engine state dependency.  See [doc 15](15-dispatch-forms.md) for the full S-form catalogue and state-machine description.

```cpp
// Submit a dispatch telegram; advances the state machine.
// Returns ACCEPTED, REJECTED_WRONG_STATE, or REJECTED_DUPLICATE.
TelegramOutcome submit_telegram(
    const std::string& src_area, const std::string& dst_area,
    DispatchFormType form, TelegramDirection direction,
    const std::string& train_number);

// Query current ExchangeStatus for a directed pair (returns IDLE if no exchange exists).
ExchangeStatus status(const std::string& src_area, const std::string& dst_area) const noexcept;

// Advance S26_RECEIVED → CLOSED (arrival confirmed).
void close(const std::string& src_area, const std::string& dst_area);
```

`exchange_id` format: `"exch-0000001"` (zero-padded counter; monotonically increasing per server instance).

### Operator command state in snapshots and events

Each device state in the snapshot (`SwitchState`, `TrackSectionState`, `SignalState`, `DerailerState`, `BlockSectionSnapshotState`) carries:

```cpp
struct OperatorCommandRuntimeState {
    std::optional<OperatorCommandCode> active_operator_command;
    std::optional<Ml8CommandCode>      active_ml8_command;
};
```

This replaces the earlier `bool ml8_command_active` + `std::string last_ml8_command_code` pair with a type-safe optional.

Incremental updates are delivered as `DOMAIN_EVENT (0x20)` with:
- `event_type 0x11 OperatorCommandStateChanged` — EbiLock operator command flag changed
- `event_type 0x12 Ml8CommandStateChanged` — ML8 command flag changed

Both events carry `g_id`, `target_kind`, `command_code`, and `active` (bool).  See [doc 09](09-communication-contract.md) for the full wire definition.

### Helper functions in command.hpp

```cpp
// Returns the display name for a command code (e.g. "S1" for OperatorCommandCode::S1).
// Used in NAK reason_text and logging.
constexpr std::string_view operator_command_code_name(OperatorCommandCode);
constexpr std::string_view ml8_command_code_name(Ml8CommandCode);
```
