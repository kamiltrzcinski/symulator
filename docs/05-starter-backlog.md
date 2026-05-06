# Starter backlog

## Priority P0

- [x] Agree on station map configuration format. **Resolved:** split JSON — `meta.json`, `topology.json`, `objects.json` per station; engine loads via nlohmann/json; schema validated by editor. Details in `08-track-topology-model.md`.
- [ ] Define command/event/snapshot communication contract.
- [ ] Define minimal database model.
- [ ] Prepare one reference traffic scenario for testing.
- [x] Choose deployment model: dedicated authoritative server.
- [x] Confirm client delivery model: single native C++ desktop application per player, containing both Pulpity (station panel tabs) and EDR (register tabs). No browser client.
- [ ] Define MVP load profile (sessions, clients, update rate, payload size).
- [ ] Define server SLO targets for p95 latency, loop time, and packet loss tolerance.
- [ ] Confirm final station list (include or exclude Naterki) and define default station-to-player assignment.
- [ ] Define station ownership and takeover protocol (request, grant, revoke).
- [ ] Decide EDR integration path: adapt existing C# prototype or rewrite as a new server-side component.
- [ ] Decide database topology: single DB with separate schemas vs. two distinct instances (master train data vs. session state).
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
