# System requirements

## Functional requirements

- F-001: The system must maintain a railway topology model (track sections, switches, signals, dependencies).
- F-002: An operator can send commands that change signaling device states.
- F-003: The system validates command safety rules before execution.
- F-004: The simulation engine updates track occupancy over time.
- F-005: The system supports timetable planning and actual execution timestamps.
- F-006: The server synchronizes state across multiple clients.
- F-007: All critical events are stored in an event log.
- F-008: Session state and station configuration are persisted in a database.
- F-009: A client can request a full snapshot when joining/rejoining a session.
- F-010: The system exposes a basic API for an external AI/traffic module.

## Non-functional requirements

- N-001: Target command propagation latency client-server-client is <= 100 ms under normal load.
- N-002: Synchronization must be deterministic (server-authoritative model).
- N-003: A single client failure must not stop the active server session.
- N-004: Critical operations must be observable (logs and metrics).
- N-005: Architecture must separate UX/UI from simulation engine logic.
- N-006: Network communication must support client reconnection.
- N-007: The database must support historical session replay.

## Data requirements

- D-001: Station configuration stores topology and signaling device parameters.
- D-002: Timetable data stores trains, timing, and control points.
- D-003: Event log stores timestamp, type, source, and payload.
- D-004: Session snapshot stores current state and model version.

## Open questions

1. Is AI part of MVP, or only a post-MVP iteration?
2. How detailed should train behavior be (physics-based vs. simplified route traversal)?
3. Are multi-level permissions required (operator, admin, observer)?
4. What should be the initial station config format (JSON/YAML/protobuf)?
