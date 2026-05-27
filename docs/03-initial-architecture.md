# Initial architecture

## Core principles

- The simulation engine and domain model are independent from the presentation layer.
- SOLID principles apply at every level: individual classes/functions and the overall module/system decomposition. This keeps each component independently testable, replaceable, and extensible without cascade rewrites.

## Components

1. Core Simulation Engine (C++20)
   - track and signaling model,
   - command safety validation,
   - state updates in ticks or event-driven mode.
2. Session Server
   - authoritative state owner,
   - client synchronization,
   - session lifecycle and command queue handling.
3. Client Application (native C++ desktop)
   A single application with two functional areas, both tab-based:

   **Pulpity (station panels):**
   - one tab per assigned station, labelled by station name,
   - clicking a tab switches the active dispatcher panel,
   - panel state visualization, operator command input, alarms and logs view.

   **EDR (Electronic Traffic Register):**
   - separate top-level area within the same application,
   - one tab per station for train register data,
   - operator can switch station context without leaving the application.

   The operator client communicates with the Session Server over a persistent TCP connection (see Channel 1).
4. EDR — Electronic Traffic Register (server-side)
   - master train data provider for the simulation engine (train definitions, routes, timetable inputs),
   - runs as a server-side module (or sibling process) and communicates with the engine via in-process calls or the internal subscriber bus,
   - the client-side EDR view is part of the native C++ application (see component 3); no separate browser interface,
   - implemented as a native C++ component (no C# runtime dependency in MVP),
   - a supervisor/monitoring sub-module may be needed to coordinate EDR↔engine data flow (open question).
5. Persistence Layer
   - station configuration storage,
   - event log,
   - session snapshots.
6. AI Module (separate process/service, post-MVP)
   - designed as an independent module from day one; not embedded in the engine,
   - communicates with the Core Simulation Engine via the same command/event contract as human operators (TCP framing when remote, internal bus when co-located),
   - traffic planning and movement management,
   - handling of LCS boundary interactions,
   - optional random events generation.

   Rationale: keeping AI decoupled means the engine remains deterministic and testable without an AI runtime; the AI can be developed, replaced, or disabled independently.

7. Scenario Editor (standalone desktop tool)
   - Standalone C++ desktop application; independent of the operator client binary.
   - Links `libtrackview` (shared rendering library) for the tile-based canvas; the operator client links the same library.
   - Produces per-station topology bundles (`meta.json`, `topology.json`, `objects.json`), inter-station section definitions, and timetable template data consumed by the Session Server.
   - Native working format: `.scendb` (SQLite per station project); exports to JSON bundles for the engine.
   - Operates fully offline for topology and manual timetable authoring; a server connection is required only for PLK schedule import (handled server-side via `IPLKImporter`).
   - Subject to the same cross-platform and dependency-bundling requirements as the operator client (see below).

## Threading model

The server process runs a fixed set of named threads with clearly defined ownership boundaries. No code may block a thread that does not own that blocking operation.

### Fixed threads (always present)

```
Thread          Owner              Blocking allowed    Notes
──────────────  ─────────────────  ──────────────────  ──────────────────────────────────────────
ENGINE          SimulationEngine   NO                  20 Hz tick loop; must not wait on I/O
DISPATCH_BUS    DispatchBus        YES (cv::wait)      Drains EventQueue<DomainEvent> and fan-outs to subscribers
DB_WRITER       DatabaseManager    YES (libpqxx I/O)   Drains EventQueue<EventLogEntry>
PIP_WRITER      PipStateManager    YES (libpqxx I/O)   Drains EventQueue<PipEvent>; UPSERT pip.track_state
```

The engine thread never holds a mutex for longer than a single `push()` into an outbound queue. All I/O, serialization, and database work happens exclusively on DISPATCH_BUS, DB_WRITER, or PIP_WRITER.

**DISPATCH_BUS** is a dedicated subscriber bus thread. Modules register handlers during startup and receive immutable event envelopes. Publication is single-writer (the bus thread), and payload lifetime is released only after all registered subscribers complete, which avoids cross-thread memory-ownership races.

**PIP_WRITER** is dedicated to the Train Identification Panel (PIP) subsystem. It receives `PipEvent` records from the ENGINE whenever a track section changes occupancy or a train number is assigned/removed. Its sole responsibility is maintaining the `pip.track_state` table (UPSERT per section). PIP and EDR are independent subsystems — PIP_WRITER never touches `session.edr_entries`. PIP_WRITER never communicates back to the ENGINE — it is a write-only sink. An awaria (crash/stall) of PIP_WRITER does not affect the engine tick loop or the client broadcast path; at worst the PIP display in reconnecting clients will be stale until the writer recovers.

### I/O and work thread pools (Boost.Asio)

```
Thread pool     Default size                    Role
──────────────  ──────────────────────────────  ──────────────────────────────────────────────
IO_POOL         max(2, hardware_concurrency/2)  Boost.Asio io_context; accepts TCP connections,
                                                 reads/writes frames; one strand per connection
WORK_POOL       max(1, hardware_concurrency/2)  Deserializes FlatBuffers payloads, validates
                                                 COMMAND ownership, pushes into CommandQueue
```

Pool sizes are read from a config file at startup and capped by `hardware_concurrency`. At 4 vCPU the defaults yield 2 + 2 = 4 pool threads plus 4 fixed threads = 8 total.

### Inter-thread communication

```
Producer → Queue → Consumer
────────────────────────────────────────────────────────────────
WORK_POOL    → PriorityCommandQueue<Command>   → ENGINE
ENGINE       → EventQueue<DomainEvent>         → DISPATCH_BUS
ENGINE       → EventQueue<PipEvent>            → PIP_WRITER
DISPATCH_BUS → EventQueue<EventLogEntry>       → DB_WRITER
DISPATCH_BUS → per-station broadcast sets      → IO_POOL (strands)
```

**`EventQueue<PipEvent>`** is a dedicated MPSC queue (ENGINE is the sole producer). `PipEvent` carries the section `gID`, new `TrackOccupancy`, and an optional `train_number` (present on entry/exit). The queue is separate from `EventQueue<DomainEvent>` so that a PIP_WRITER stall cannot apply backpressure to DISPATCH_BUS and vice versa.

**`PriorityCommandQueue`** uses four FIFO buckets (EMERGENCY=0, SAFETY=1, NORMAL=2, BACKGROUND=3). The engine always drains the highest non-empty bucket first. Within one priority level FIFO order is preserved. A configurable `MAX_CMDS_PER_TICK` cap (default 32) ensures the tick duration stays within the 5 ms SLO regardless of input burst.

**`EventQueue<T>`** is a single-producer single-consumer ring with a `std::mutex` + `std::condition_variable`. The consumer calls `wait_and_pop`; the producer calls `push` and notifies. No heap allocation on the hot path.

### Operating point ownership and command rejection

`OwnershipRegistry` is a `std::unordered_map<operating_point_id, player_id>` protected by a `std::shared_mutex`. Reads (ownership check during command validation on WORK_POOL) take a shared lock. Writes (TAKEOVER, session join/leave) take an exclusive lock. The engine itself never reads `OwnershipRegistry` — rejection happens on WORK_POOL before the command enters `PriorityCommandQueue`, so the engine thread is never involved in ownership bookkeeping.

### Shutdown sequence

1. `io_context.stop()` — IO_POOL and WORK_POOL drain and exit.
2. Engine tick loop exits on stop flag.
3. `EventQueue<DomainEvent>` is flushed — DISPATCH_BUS processes remaining events and exits.
4. `EventQueue<PipEvent>` is flushed — PIP_WRITER commits remaining UPSERTs and exits.
5. `EventQueue<EventLogEntry>` is flushed — DB_WRITER commits remaining inserts and exits.

All queues support a `close()` call that unblocks `wait_and_pop` with a sentinel; no `std::terminate` or forced joins.

---

## Cross-platform build requirements

### Development environment

Primary development platform: **Linux x86-64**.
Target platforms: **Linux x86-64, Windows x86-64, macOS x86-64**.

The server runs on Linux only. All end-user components (operator client, scenario editor) must build and run on all three target platforms.

### Dependency bundling policy

**All third-party libraries must be bundled with the client and editor distributions.** No dependency on system-installed versions of Qt, SQLite, or any other library. This removes installation friction for end users and ensures version consistency across all platforms.

| Dependency | Version | Scope | Bundling method |
|---|---|---|---|
| **Qt 6** | ≥ 6.6 | client, editor, `libtrackview` | vcpkg; bundled via `windeployqt` / `linuxdeployqt` / `macdeployqt` |
| SQLite | amalgamation | editor (`.scendb`) | Single-file amalgamation compiled directly into the editor binary |
| nlohmann/json | ≥ 3.11 | engine, editor, server | Header-only via vcpkg; no separate binary |
| OpenSSL | system | server only | System-provided on Linux server; not required in client distributions |
| **Boost.Asio** | ≥ 1.84 | server | Header-only (Asio standalone mode); vcpkg `boost-asio` port; not linked into client or editor |
| FlatBuffers | ≥ 23.x | engine, server, client | vcpkg; `flatc` compiler run as CMake custom target (`generate_proto_headers`); headers generated into `build/generated/proto` (not checked in) |
| libpqxx | ≥ 7.x | server | vcpkg; server only; links against system `libpq` on Linux |
| ONNX Runtime | ≥ 1.18 (post-MVP) | AI module | vcpkg `onnxruntime-gpu` port; AI process only; CUDA provider for RTX GPU |

Qt modules used: `Qt6::Widgets`, `Qt6::OpenGLWidgets` (canvas), `Qt6::Sql` (SQLite in editor), `Qt6::Network` (client TCP transport). No Qt Quick / QML — pure Widgets.

### Build system

**CMake** (≥ 3.25) + **Ninja** + **vcpkg**.

- Ninja replaces Make as the executor. On all three target platforms CMake generates the same `build.ninja` file — one configure command, no platform-specific Make variants (no `nmake`, no `mingw32-make`).
- A single root `CMakeLists.txt` drives all components via `add_subdirectory`.
- No cross-compilation from Linux to Windows. Windows builds run on native Windows CI runners.
- macOS builds run on native macOS CI runners.

```bash
# Configure (all platforms, identical command)
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
      -B build

# Build
cmake --build build
```

### CI/CD — GitHub Actions

The repository is public. GitHub Actions is **free and unlimited** for public repositories.

**Build matrix** — three platforms run in parallel on every pull request and every push to `main`:

```
ubuntu-24.04   →  build  →  test  →  artifact (Linux binary)
windows-2022   →  build  →  test  →  artifact (Windows binary)
macos-14       →  build  →  test  →  artifact (macOS binary)
```

**Cache strategy:** vcpkg port builds and the Qt6 build are cached via `actions/cache`. After the first run (30–60 min for Qt) subsequent runs take a few minutes.

**Release workflow:** triggered by pushing a version tag (`git tag v0.3.0 && git push --tags`). Runs the full matrix; on success creates a GitHub Release and attaches the three platform binaries as assets. Release tags are created exclusively by the project owner — contributors do not push tags.

**No minutes cost.** GitHub's 2 000 free minutes/month applies only to private repositories.

### Contributor onboarding goal

A new contributor on **Windows** must be able to clone the repo, run one CMake configure command, and obtain a working build — without installing anything beyond: Git, CMake, Ninja, a C++20 compiler (MSVC or Clang), and vcpkg. Requirements:
- All library dependencies resolved automatically by vcpkg on configure.
- No hard-coded UNIX paths in build scripts or source.
- CI pipeline validates all three platforms on every pull request, catching regressions before merge.

Detailed step-by-step setup instructions for both Linux and Windows are in `docs/00-contributing.md`.

1. Client sends command to server.
2. Server validates and forwards to engine.
3. Engine computes the next state and emits events.
4. Server stores events and broadcasts updates to clients.
5. Clients update view from events/snapshots.

## Communication contracts (v0)

- Command: operator intent (for example set switch state).
- Domain Event: accepted command result or autonomous state change.
- Snapshot: full session state for reconnect.
- Heartbeat: liveness signal for client and server.

## Transport layer per communication channel

### Channel 1: Operator Client ↔ Session Server (real-time game state)
- Protocol: **TCP persistent socket with custom binary framing**.
- Rationale: every domain event (switch state, signal change, track occupancy) must be delivered exactly once and in order — none can be dropped. This rules out lossy UDP. UDP with mandatory per-packet ACK would be a manual reimplementation of TCP without its battle-tested congestion control and retransmit logic. At 1–10 Hz and ~500 bytes per update, TCP overhead is negligible and N-001 (≤ 100 ms) is easily achievable.

### Channel 2: EDR ↔ Engine (server-side, same host)
- Protocol: **direct in-process call or Unix socket** (if separated into distinct processes on the same machine).
- Rationale: both components run on the same server. Introducing HTTP/REST would add a second protocol stack and a second serialization format with no benefit. The same binary framing used for client↔server can be reused if a socket boundary is needed; more likely a direct function call or message bus suffices.

### Channel 3: AI Module ↔ Engine (post-MVP)
- Protocol: **deferred.** Decided when the AI module is actually being designed. If co-located: same approach as Channel 2. If remote: TCP with the existing framing is the default candidate.

### Channel 4: Engine ↔ Session Server (intra-server)
- Same process or same host: direct function call or in-process event bus — no network protocol needed.

**Protocol consolidation decision: one network protocol (TCP + custom binary framing) for all socket-based communication. No REST/HTTP layer in MVP. Intra-server components use direct calls.**

## AI Dispatch Module (post-MVP)

The AI Dispatch Module is a separate server-side process that acts as a virtual operator. It communicates with the engine through the identical mechanisms used by human players — no special engine-internal paths exist for AI.

### Role

- Holds operating point assignments via `OwnershipRegistry` (F-019, F-024).
- Subscribes to `DOMAIN_EVENT` broadcasts as a read-only observer.
- Generates `COMMAND` payloads (set signal, set route, takeover, etc.) and submits them through `PriorityCommandQueue` at the appropriate priority level.
- May hand off operating points to a human player on demand; re-acquires them when the player leaves.

### Inference hardware

Training and inference targets a local GPU (NVIDIA RTX 5070 Ti, 16 GB VRAM). The choice of ML framework is deferred until post-MVP design, but the interface boundary is fixed now:

```
IDispatchAI
  + onDomainEvent(DomainEvent) → void       // observe state changes
  + pollCommands() → std::vector<Command>   // engine calls this each tick
  + assignOperatingPoint(operating_point_id) → void
  + revokeOperatingPoint(operating_point_id) → void
```

`pollCommands` is called by the ENGINE thread at the start of each tick. The implementation must be non-blocking (inference result is prepared asynchronously on a GPU thread and staged in a lock-free buffer; `pollCommands` only reads the buffer).

### Candidate frameworks (decision post-MVP)

| Framework | VRAM usage | C++ API | Notes |
|---|---|---|---|
| LibTorch (PyTorch C++) | moderate | native | vcpkg port available; same model format as Python training |
| ONNX Runtime | low | native | inference-only; model trained in any framework then exported |
| llama.cpp | very low | native | for LLM-based planner variants |

ONNX Runtime is the current default candidate: small binary footprint, GPU via CUDA provider, no Python runtime dependency at inference time.

### Training strategy (outline)

1. Record human operator sessions from the simulation (event log → replay files).
2. Train an imitation learning baseline on recorded sessions.
3. Fine-tune with reinforcement learning against a reward function encoding: train punctuality, safety rule compliance, and throughput.
4. Evaluation: run AI against recorded scenarios and compare to human baseline.

Training runs outside the main repository in a separate Python workspace. Only the exported ONNX (or LibTorch `.pt`) model is committed to this repo under `ai/models/`.

---

## Random Events Module (post-MVP, extension hook)

The engine core does not implement random events. It provides `IRandomEventSource`:

```cpp
// Engine calls poll() once per tick; the result is injected at EMERGENCY priority.
class IRandomEventSource {
public:
    virtual ~IRandomEventSource() = default;
    virtual std::optional<RandomEvent> poll() = 0;
};

struct RandomEvent {
    RandomEventType type;      // open enum; built-in types below
    std::string affectedGID;   // gID of the affected track section, device, or train
    std::string description;   // human-readable, language-keyed JSON string
};

// Built-in RandomEventType values (extensible without engine changes):
//   TRACK_CLOSURE        — section must be taken out of service
//   INFRASTRUCTURE_FAIL  — switch or signal failure
//   MEDICAL_EMERGENCY    — ambulance required at station
//   POLICE_CALL          — police required at station
//   RECOVERY_TRAIN       — recovery train must be dispatched
//   PASSENGER_ALARM      — passenger emergency stop pulled
```

A `NullRandomEventSource` (returns `std::nullopt` always) is the default at startup. The Random Events Module replaces it via dependency injection at server init — no engine code changes required. The module generates events according to a probability table loaded from a config file, producing realistic incident scenarios during gameplay.

---

## Minimum deployment layout

- One server process (engine + session server).
- EDR process on the same server.
- 2+ operator clients (native C++ desktop application; one per player machine; contains both Pulpity and EDR views).
- One database instance (local or same VPS); two-instance topology under evaluation (see open question 7 in requirements).

## Deployment options for server and engine

### Option A: Dedicated authoritative server (recommended)

- Session server runs on separate infrastructure (VPS or dedicated machine).
- Simulation engine runs on the same server process (or tightly coupled service).
- Clients are thin: command input + state rendering only.

Pros:
- clean anti-cheat and anti-desync model,
- no dependency on player host stability,
- best fit for persistence, replay, and multi-session growth.

Cons:
- requires deployment and ops setup from day one,
- higher initial backend complexity.

### Option B: Player-hosted authoritative session

- One player machine runs server + engine.
- Other players connect peer-to-host.

Pros:
- fastest MVP bootstrap,
- no mandatory infrastructure costs initially.

Cons:
- host disconnect ends the session,
- weaker reliability and more NAT/network edge cases,
- difficult migration path if state and persistence are host-tied.

### Option C: Hybrid migration path

- Early MVP supports player-hosted mode.
- Protocol and persistence are designed as if dedicated server already exists.
- Later, runtime moves to dedicated server without protocol rewrite.

Pros:
- practical speed now + scalable target architecture,
- lower rewrite risk if contracts are stable.

Cons:
- requires discipline to avoid host-only shortcuts,
- temporary dual-mode complexity.

## Current working decisions

- Deployment model: Option A (Dedicated authoritative server).
- Engine placement: simulation engine runs on the authoritative server.
- Client: single native C++ desktop application per player; contains both Pulpity (station panel tabs) and EDR (register tabs). No browser client anywhere.
- Operator client transport: TCP persistent socket (see Channel 1 rationale).

Rationale:
- dedicated authority avoids host disconnect issues,
- engine on server prevents state divergence between players,
- native C++ client keeps rendering and state management fast and self-contained.

## Dedicated server minimum requirements (MVP baseline)

Assumptions for this baseline:
- 1 to 2 concurrent sessions,
- up to 8 connected operator clients total,
- up to 20 state update pushes per second per session,
- average payload size 300 to 600 bytes per update,
- session server, engine, and database on the same machine.

### Hardware and network tiers

1. Minimum viable tier
   - CPU: 2 vCPU (modern x86_64 or ARM, about 3.0 GHz class)
   - RAM: 4 GB
   - Storage: 20 GB SSD
   - Link throughput: 20 Mbps symmetric
   - Link quality: RTT to most players under 80 ms, jitter under 20 ms, packet loss under 1%
2. Recommended production starter tier
   - CPU: 4 vCPU
   - RAM: 8 GB
   - Storage: 40 GB NVMe SSD
   - Link throughput: 50 Mbps symmetric
   - Link quality: RTT under 60 ms, jitter under 10 ms, packet loss under 0.5%
3. Growth tier (4+ sessions or heavier AI worker)
   - CPU: 8 vCPU
   - RAM: 16 GB
   - Storage: 80+ GB NVMe SSD
   - Link throughput: 100 Mbps+ symmetric

### Capacity notes

- Outbound bandwidth estimate:
  BW_out ~= clients * updates_per_sec * payload_bytes * 8 * 1.4
- Example:
  8 clients * 20 updates * 500 bytes * 8 * 1.4 ~= 896000 bps (~0.9 Mbps)
- Throughput is usually not the bottleneck at MVP scale; latency, jitter, and packet loss are more critical.
- Keep at least 30% CPU headroom to absorb spikes.

### Runtime SLO targets for validation

- p95 command acknowledge latency under 120 ms.
- p95 server update loop time under 20 ms.
- Average CPU usage under 65% during peak test load.
- Average memory usage under 70% during peak test load.

## Display/rendering decision

**Selected: client-side panel rendering with server-pushed state deltas/events (Option 1).**

- Server sends state deltas and domain events over the TCP persistent socket.
- Client renders panel from local layout config + live state.
- Low bandwidth, responsive UX, easy to support multiple UI styles on the same engine.
- Server-side rendered stream (Option 2) and hybrid static overlay (Option 3) are not pursued: unnecessary bandwidth and complexity at MVP scale.

## Technical risks

- Tight coupling between UI and domain logic.
- Non-deterministic behavior under concurrent commands.
- Unbounded event log growth without snapshot policy.
- Missing integration tests for multiplayer synchronization.

## Decisions to make

1. ~~Transport: WebSocket or gRPC?~~ **Resolved:** TCP persistent socket with custom binary framing for all socket communication; direct calls for intra-server. No REST/HTTP in MVP.
2. Message format: JSON first, or protobuf from day one?
3. Database: PostgreSQL (preferred) or SQLite for bootstrap?
4. Engine timing: fixed tick or purely event-driven model?
5. ~~Deployment model~~ **Resolved:** dedicated authoritative server (Option A).
6. ~~Rendering model~~ **Resolved:** client-side rendering with server-pushed events.
