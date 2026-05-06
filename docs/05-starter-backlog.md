# Starter backlog

## Priority P0

- [x] Agree on station map configuration format. **Resolved:** split JSON — `meta.json`, `topology.json`, `objects.json` per station; engine loads via nlohmann/json; schema validated by editor. Details in `08-track-topology-model.md`.
- [x] Define command/event/snapshot communication contract. **Resolved:** 16-byte binary frame (magic + type + flags + seq_id + payload_len + CRC32); FlatBuffers payloads; message catalog: COMMAND / COMMAND_ACK / COMMAND_NAK / DOMAIN_EVENT / SNAPSHOT_CHUNK / HEARTBEAT / HANDSHAKE / TAKEOVER. Details in `09-communication-contract.md`.
- [x] Define minimal database model. **Resolved:** one PostgreSQL instance, two schemas — `fleet` (static reference data) and `session` (events, snapshots, EDR live, posterunek assignments, chat). Details in `11-database-model.md`.
- [ ] Prepare one reference traffic scenario for testing.
- [x] Choose deployment model: dedicated authoritative server.
- [x] Confirm client delivery model: single native C++ desktop application per player, containing both Pulpity (station panel tabs) and EDR (register tabs). No browser client.
- [x] Define MVP load profile (sessions, clients, update rate, payload size). **Resolved:** full Trójmiasto = 9 stations, ~36 peak clients, 20–100 events/s, 50 kB snapshot, ~1 020 EDR rows/session. Details in `06-server-sizing-baseline.md`.
- [ ] Define server SLO targets for p95 latency, loop time, and packet loss tolerance.
- [ ] Confirm final station list (include or exclude Naterki) and define default station-to-player assignment.
- [ ] Define station ownership and takeover protocol (request, grant, revoke).
- [x] Decide EDR integration path: **Resolved:** new native C++ component (not adapted from C# prototype); owns `fleet.timetable_templates` and `session.edr_entries`; communicates with engine via direct call / Unix socket (Channel 2).
- [x] Decide database topology: **Resolved:** single PostgreSQL instance, two schemas (`fleet` / `session`). See `11-database-model.md`.
- [ ] Establish project language policy: English for all code, identifiers, comments, and documentation (translation tools are acceptable).

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
- EDR: C# prototype owner (Tymon); final ownership and integration path to be confirmed.
- Shared: protocol contracts, REST API design, integration tests, and performance profiling.
