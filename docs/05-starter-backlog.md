# Starter backlog

## Priority P0

- [x] Agree on station map configuration format. **Resolved:** split JSON — `meta.json`, `topology.json`, `objects.json` per station; engine loads via nlohmann/json; schema validated by editor. Details in `08-track-topology-model.md`.
- [x] Define command/event/snapshot communication contract. **Resolved:** 16-byte binary frame (magic + type + flags + seq_id + payload_len + CRC32); FlatBuffers payloads; message catalog: COMMAND / COMMAND_ACK / COMMAND_NAK / DOMAIN_EVENT / SNAPSHOT_CHUNK / HEARTBEAT / HANDSHAKE / TAKEOVER. Details in `09-communication-contract.md`.
- [x] Define minimal database model. **Resolved:** one PostgreSQL instance, two schemas — `fleet` (static reference data) and `session` (events, snapshots, EDR live, posterunek assignments, chat). Details in `11-database-model.md`.
- [ ] Prepare one reference traffic scenario for testing. **Scope defined:** two posterunki — Gdynia Orłowo and Sopot, plus the line section between them. Blocked on: editor ready to compose station topology from JSON objects. Deferred to Stage 1 completion.
- [x] Choose deployment model: dedicated authoritative server.
- [x] Confirm client delivery model: single native C++ desktop application per player, containing both Pulpity (station panel tabs) and EDR (register tabs). No browser client.
- [x] Define MVP load profile (sessions, clients, update rate, payload size). **Resolved:** full Trójmiasto = 9 stations, ~36 peak clients, 20–100 events/s, 50 kB snapshot, ~1 020 EDR rows/session. Details in `06-server-sizing-baseline.md`.
- [x] Define server SLO targets for p95 latency, loop time, and packet loss tolerance. **Resolved:** p95 command round-trip ≤100 ms; engine loop ≤5 ms/tick at 20 Hz; packet loss tolerance <1%; reconnect+snapshot sync ≤5 s; broadcast fanout lag ≤10 ms.
- [x] Confirm final station list (include or exclude Naterki) and define default station-to-player assignment. **Resolved:** 9 Trójmiasto stations (Gdynia Chylonia → Gdańsk Orunia); Naterki excluded from MVP (post-MVP). Station assignment is **flexible** — a player (client) receives a list of posterunki regardless of geographic station; the client displays one pulpit tab and one EDR tab per assigned posterunek and switches freely between them. No fixed geographic split.
- [x] Define station ownership and takeover protocol (request, grant, revoke). **Resolved:** lifecycle — Request: C→S `TAKEOVER_REQUEST(posterunek_id)`; Grant: S→C `TAKEOVER_RESPONSE(granted=true)` + broadcast `PosterunekOwnershipChanged`; Voluntary release: `TAKEOVER_REQUEST(release=true)` or graceful disconnect; Server revoke: after 30 s grace period on disconnect → `PosterunekOwnershipChanged(owner=null)`. Details in `09-communication-contract.md`.
- [x] Decide EDR integration path: **Resolved:** new native C++ component (not adapted from C# prototype); owns `fleet.timetable_templates` and `session.edr_entries`; communicates with engine via direct call / Unix socket (Channel 2).
- [x] Decide database topology: **Resolved:** single PostgreSQL instance, two schemas (`fleet` / `session`). See `11-database-model.md`.
- [x] Establish project language policy. **Resolved:** all code, identifiers, and comments in English. Display strings (device names, vehicle names, station names) in Polish as base language; `display` field in JSON objects uses a language-keyed map (`{"pl": "...", "en": "..."}`) from the start to allow future i18n without schema changes.

## Priority P1

- [ ] Draft server API (endpoints or channels).
- [ ] Build command validation prototype in the SRK engine.
- [ ] Define snapshot and event-log retention strategy.
- [ ] Create client-server integration test plan.
- [ ] Benchmark dedicated server on 2 vCPU and 4 GB RAM baseline.
- [ ] Benchmark dedicated server on 4 vCPU and 8 GB RAM reference tier.
- [ ] Define alert thresholds and scaling trigger points.
- [ ] Design tabbed multi-panel client layout (number of tabs, active-station switching UX, state refresh per tab).
- [x] Evaluate TCP vs UDP transport: resolved — TCP persistent socket with custom binary framing. UDP ruled out: events are safety-critical (must all deliver), UDP + mandatory ACK = reimplementing TCP with more complexity.
- [ ] Define EDR ↔ engine data contract (train definitions, routes, timetable inputs format and update frequency; direct call or Unix socket boundary).
- [ ] Define scope and placement of supervisor/monitoring module (EDR↔engine coordination, session integrity oversight).

## Priority P2

- [ ] Define scope and acceptance criteria for first AI module.
- [ ] Create operator panel concept (without detailed UI work).
- [ ] Define session quality metrics and scoring approach.

## Ownership (draft for dedicated-server path)

- Person A: core engine and signaling rules.
- Person B: session server, persistence, and deployment.
- EDR: new native C++ component (Tymon); new implementation, not adaptation of C# prototype.
- Shared: protocol contracts, integration tests, and performance profiling.
