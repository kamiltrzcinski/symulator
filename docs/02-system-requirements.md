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
- F-011: A single native C++ desktop application serves both the station panel (Pulpity) and EDR functional areas. Station panels are displayed as tabs labelled by station name; the EDR area uses the same tab pattern per station. No second client instance or browser client is required.
- F-012: An operator can request and obtain permission to take over management of another station (e.g., a boundary or LCS area) within the same session.
- F-013: The system enforces station-level ownership; only the current station owner can issue commands for that station.
- F-014: Device behavior rules (interlocking logic, signal dependencies) are enforced by the engine independent of regulatory compliance.
- F-015: The EDR (Electronic Traffic Register) component runs on the server and communicates with the simulation engine as a master data provider (train definitions, routes, timetable inputs).
- F-016: The EDR functional area is part of the native C++ client application, not a separate browser-based interface.
- F-017: Inter-module communication on the server (EDR↔engine, future AI↔engine) uses the same TCP framing protocol or direct in-process calls depending on deployment topology. No separate HTTP/REST layer is introduced for MVP.
- F-018: The binary protocol uses a 16-byte frame header (magic 0x5352, msg_type, flags, seq_id, payload_len, CRC-32) with FlatBuffers-serialized payloads. Schema files in `proto/` are the canonical contract. Message types: HANDSHAKE, HEARTBEAT, COMMAND, COMMAND_ACK, COMMAND_NAK, DOMAIN_EVENT, SNAPSHOT_CHUNK, SESSION_NOTICE, TAKEOVER_REQUEST, TAKEOVER_RESPONSE, CHAT_MESSAGE, VOICE_CHAN_JOIN/LEAVE/STATE. Details in `09-communication-contract.md`.
- F-019: Operators are assigned to posterunki (sub-posts) within a station. Multiple operators may work the same station simultaneously, each controlling objects within their posterunek scope. For MVP with single-posterunek stations the model degenerates to per-station ownership.
- F-020: A train is a `std::vector<Vehicle>` assembled from ordered vehicle `gID` references in a composition JSON. Each vehicle has `lengthM` and `axleCount`; the engine derives section occupancy from axle counting at `It`/`iz` devices. Physics v1 uses constant acceleration; v2 (post-MVP) uses locomotive `powerKW` and `tractionForceKN` with total consist mass. Details in `10-vehicle-model.md`.

## Non-functional requirements

- N-001: Target command propagation latency client-server-client is <= 100 ms under normal load.
- N-002: Synchronization must be deterministic (server-authoritative model).
- N-003: A single client failure must not stop the active server session.
- N-004: Critical operations must be observable (logs and metrics).
- N-005: Architecture must separate UX/UI from simulation engine logic.
- N-006: Network communication must support client reconnection.
- N-007: The database must support historical session replay.
- N-008: The architecture must follow SOLID principles at both class/function and module/system level to allow independent development, testing, and replacement of components.
- N-009: A single TCP persistent socket with custom binary framing is the network communication protocol for the system. It is used for client↔server real-time sync. Intra-server modules (EDR↔engine) on the same host communicate via direct in-process calls or a Unix socket. No separate REST/HTTP layer is introduced for MVP; this avoids maintaining two protocol stacks and two serialization formats.

## Data requirements

- D-001: Station configuration stores topology and signaling device parameters.
- D-002: Timetable data stores trains, timing, and control points.
- D-003: Event log stores timestamp, type, source, and payload.
- D-004: Session snapshot stores current state and model version.
- D-005: Master train database stores fleet-wide train definitions and route data independent of active session state (exact relationship to session database — separate instance or separate schema — is an open question).

## Open questions

1. ~~Is AI part of MVP, or only a post-MVP iteration?~~ **Resolved:** AI is a separate module from the start; it is not embedded in the engine. It communicates via the engine API (F-010). Planned for post-MVP delivery.
2. How detailed should train behavior be (physics-based vs. simplified route traversal)?
3. ~~Are multi-level permissions required?~~ **Partially resolved:** station-level ownership and takeover is required (F-012, F-013). Full role hierarchy (admin, observer) is post-MVP.
4. ~~What should be the initial station config format (JSON/YAML/protobuf)?~~ **Resolved:** split JSON (three files per station: `meta.json`, `topology.json`, `objects.json`). Engine loads via nlohmann/json. Schema validated by the editor. Details in `08-track-topology-model.md`.
5. ~~Operator client↔server transport: TCP vs UDP?~~ **Resolved:** TCP persistent socket. Domain events are safety-critical and must all be delivered; UDP + mandatory ACK adds complexity with no gain at this traffic volume and frequency.
6. Is Naterki station included? Final station count affects default assignment and map scope.
7. ~~Database topology: single database instance with separate schemas for master train data and session state, or two distinct database instances?~~ **Resolved:** one PostgreSQL instance, two schemas — `fleet` and `session`. Details in `11-database-model.md`.
8. ~~EDR integration path: adapt and wrap the existing C# prototype, or rewrite as a native server-side component?~~ **Resolved:** new native C++ component. Owns timetable templates and live EDR data. Communicates with engine via direct in-process call or Unix socket.
9. Supervisor/monitoring module: is a dedicated module needed to coordinate EDR↔engine data flow and oversee session integrity? What is its scope and placement?
