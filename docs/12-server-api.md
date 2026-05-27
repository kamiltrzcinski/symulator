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
│  IPLKImporter (HTTP client, startup only)                   │
│         │                                                   │
└─────────┼───────────────────────────────────────────────────┘
          │ HTTPS / X-API-Key
   pdp-api.plk-sa.pl
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
```

### IPLKImporter

Fetches timetable data from the PLK Open Railway Data API and writes to `fleet.timetable_templates`. Called **once at server startup** (or on explicit admin re-import), not during active sessions.

```cpp
struct PLKImportConfig {
    std::string apiKey;
    std::string baseUrl = "https://pdp-api.plk-sa.pl";
    std::string date;                          // YYYY-MM-DD
    std::vector<std::string> stationCodes;     // PLK station codes for the 9 Trójmiasto stations
};

class IPLKImporter {
public:
    virtual void import(const PLKImportConfig& config) = 0;
    virtual ~IPLKImporter() = default;
};
```

---

## PLK Open Railway Data API integration

**Source:** `https://pdp-api.plk-sa.pl` — PLK public API, authenticated via `X-API-Key` header.  
**Documentation:** `https://pdp-api.plk-sa.pl/scalar/v1`

### Endpoints used

| Endpoint | Purpose |
|---|---|
| `GET /api/v1/dictionaries/stations` | Resolve station names → PLK station codes for the 9 Trójmiasto stations |
| `GET /api/v1/schedules?station=...&date=...` | Fetch planned timetable for each station |

### Mapping to `fleet.timetable_templates`

| PLK field | DB column |
|---|---|
| `trainNumber` / `scheduleId` | `train_number` |
| station code | `station_sid` (mapped to internal sID) |
| `arrivalTime` | `scheduled_arrival` (converted to INTERVAL from session epoch) |
| `departureTime` | `scheduled_departure` |
| `trackNumber` / `platform` | `track_number` |
| `stopType` | `stop_type` (COMMERCIAL / TECHNICAL / PASS_THROUGH) |

### Rate limits and import strategy

- Basic tier: 100 req/hour, 1 000 req/day.
- 9 stations × 1 request each = **9 requests per import run** — trivially within limits.
- Import is triggered manually before a session, not automatically on session start.
- Results are cached in `fleet.timetable_templates`; re-import only when timetable changes (new timetable year, ad-hoc correction).
- API key stored in server config file (not hardcoded); loaded at startup.

### Future: real-time operations mode

`GET /api/v1/operations` provides real-time train positions and delays. Post-MVP option: a "live timetable" mode where the simulator seeds train positions from PLK real-time data instead of a fixed timetable. Requires no architectural changes — `IEDRService.initSession` would call a different import source.

---

## Call sequence: session startup

```
1. Server starts
2. IPLKImporter::import()          → fleet.timetable_templates populated (if not already current)
3. ITopologyStore::loadStation()   × 9 stations  → topology graph in memory
4. Client connects → HANDSHAKE
5. IEDRService::initSession()      → session.edr_entries seeded from fleet.timetable_templates
6. ISnapshotProvider::buildSnapshot() → sent to client as SNAPSHOT_CHUNK(s)
7. Normal operation: client sends COMMAND → ICommandHandler::handle()
                     engine calls IEventEmitter::emit() → broadcast + ISessionStore::appendEvent()
```

---

## Open questions

- Q-API-1: Should `IEDRService` live in the same process as the engine (direct call) or in a separate process (Unix socket)? Direct call is simpler for MVP; separate process allows independent restart of EDR without stopping the engine.
- Q-API-2: HTTP client library for `IPLKImporter`: **libcurl** (C, battle-tested, widely packaged) vs. **cpp-httplib** (header-only, C++11). Libcurl preferred for production; cpp-httplib acceptable for MVP given its zero-dependency appeal.
- Q-API-3: Should the PLK station code → internal `sID` mapping be hardcoded in the importer config, or stored as a dictionary in `fleet.station_map`?

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
