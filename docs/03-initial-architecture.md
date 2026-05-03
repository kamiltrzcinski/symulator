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
3. Client UI
   - tab-based multi-panel display (one tab per assigned station),
   - operator can switch active station without launching a second client instance,
   - panel/state visualization,
   - operator command input,
   - alarms and logs view.
4. Persistence Layer
   - station configuration storage,
   - event log,
   - session snapshots.
5. AI Module (separate process/service, post-MVP)
   - designed as an independent module from day one; not embedded in the engine,
   - communicates with the Session Server via the engine API (see F-010),
   - traffic planning and movement management,
   - handling of LCS boundary interactions,
   - optional random events generation.

   Rationale: keeping AI decoupled means the engine remains deterministic and testable without an AI runtime; the AI can be developed, replaced, or disabled independently.

## Proposed flow

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

## Transport layer options

### Option T-A: TCP / WebSocket (default)
- Simpler implementation, reliable delivery.
- Higher per-message overhead; may be acceptable for initial load profile.

### Option T-B: UDP with custom framing
- Lower overhead and potentially lower latency.
- Requires manual frame assembly and lost-packet recovery strategy.
- Candidate for optimization once baseline latency is measured against N-001 (≤ 100 ms).

Current decision: start with TCP/WebSocket; evaluate UDP if profiling shows N-001 is at risk.

## Minimum deployment layout

- One server process.
- 2+ operator clients.
- One database instance (local or same VPS).

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
- Rendering direction: client-side panel rendering is the default candidate for MVP.

Rationale:
- dedicated authority avoids host disconnect issues,
- engine on server prevents state divergence between players,
- client-side rendering keeps bandwidth low and UI responsive.

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

## Display/rendering options

### Rendering option 1: Client-side panel rendering (recommended)

- Server sends state deltas/events.
- Client renders panel from local layout config + live state.

Pros:
- low bandwidth,
- responsive UX,
- easy to support multiple UI styles on the same engine.

### Rendering option 2: Server-side rendered stream

- Server renders frames and streams image/video to clients.

Pros:
- fully centralized visual consistency.

Cons:
- high bandwidth and latency pressure,
- poor fit for operator interaction,
- unnecessary complexity for MVP.

### Rendering option 3: Hybrid static layout + dynamic overlays

- Layout is static asset distributed to clients.
- Dynamic signaling/occupancy state comes from server events.

Pros:
- clean split between content and runtime,
- straightforward cache/version strategy.

## Technical risks

- Tight coupling between UI and domain logic.
- Non-deterministic behavior under concurrent commands.
- Unbounded event log growth without snapshot policy.
- Missing integration tests for multiplayer synchronization.

## Decisions to make

1. Transport: WebSocket or gRPC?
2. Message format: JSON first, or protobuf from day one?
3. Database: PostgreSQL (preferred) or SQLite for bootstrap?
4. Engine timing: fixed tick or purely event-driven model?
5. Deployment model: dedicated server, player-hosted, or hybrid migration?
6. Rendering model: client-side, server-streamed, or hybrid static layout?
