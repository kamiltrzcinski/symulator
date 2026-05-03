# Starter backlog

## Priority P0

- [ ] Agree on station map configuration format.
- [ ] Define command/event/snapshot communication contract.
- [ ] Define minimal database model.
- [ ] Prepare one reference traffic scenario for testing.
- [x] Choose deployment model: dedicated authoritative server.
- [ ] Confirm rendering model after UI prototype (default candidate: client-side rendering).
- [ ] Define MVP load profile (sessions, clients, update rate, payload size).
- [ ] Define server SLO targets for p95 latency, loop time, and packet loss tolerance.

## Priority P1

- [ ] Draft server API (endpoints or channels).
- [ ] Build command validation prototype in the SRK engine.
- [ ] Define snapshot and event-log retention strategy.
- [ ] Create client-server integration test plan.
- [ ] Benchmark dedicated server on 2 vCPU and 4 GB RAM baseline.
- [ ] Benchmark dedicated server on 4 vCPU and 8 GB RAM reference tier.
- [ ] Define alert thresholds and scaling trigger points.

## Priority P2

- [ ] Define scope and acceptance criteria for first AI module.
- [ ] Create operator panel concept (without detailed UI work).
- [ ] Define session quality metrics and scoring approach.

## Ownership (draft for dedicated-server path)

- Person A: core engine and signaling rules.
- Person B: session server, persistence, and deployment.
- Shared: protocol contracts, integration tests, and performance profiling.
