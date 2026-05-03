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
- F-011: A single operator client displays all stations assigned to that player as switchable tabs; no additional client instance is required per station.
- F-012: An operator can request and obtain permission to take over management of another station (e.g., a boundary or LCS area) within the same session.
- F-013: The system enforces station-level ownership; only the current station owner can issue commands for that station.
- F-014: Device behavior rules (interlocking logic, signal dependencies) are enforced by the engine independent of regulatory compliance.

## Non-functional requirements

- N-001: Target command propagation latency client-server-client is <= 100 ms under normal load.
- N-002: Synchronization must be deterministic (server-authoritative model).
- N-003: A single client failure must not stop the active server session.
- N-004: Critical operations must be observable (logs and metrics).
- N-005: Architecture must separate UX/UI from simulation engine logic.
- N-006: Network communication must support client reconnection.
- N-007: The database must support historical session replay.
- N-008: The architecture must follow SOLID principles at both class/function and module/system level to allow independent development, testing, and replacement of components.
- N-009: The transport layer must be replaceable; UDP with custom framing is a candidate for reducing synchronization overhead compared to TCP-based alternatives.

## Data requirements

- D-001: Station configuration stores topology and signaling device parameters.
- D-002: Timetable data stores trains, timing, and control points.
- D-003: Event log stores timestamp, type, source, and payload.
- D-004: Session snapshot stores current state and model version.

## Open questions

1. ~~Is AI part of MVP, or only a post-MVP iteration?~~ **Resolved:** AI is a separate module from the start; it is not embedded in the engine. It communicates via the engine API (F-010). Planned for post-MVP delivery.
2. How detailed should train behavior be (physics-based vs. simplified route traversal)?
3. ~~Are multi-level permissions required?~~ **Partially resolved:** station-level ownership and takeover is required (F-012, F-013). Full role hierarchy (admin, observer) is post-MVP.
4. What should be the initial station config format (JSON/YAML/protobuf)?
5. Should UDP with custom framing replace or supplement the default TCP transport? Tradeoffs: lower latency and overhead vs. manual packet loss handling and frame assembly.
6. Is Naterki station included? Final station count affects default assignment and map scope.
