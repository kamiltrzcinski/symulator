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
- F-013: The system enforces posterunek-level exclusive ownership. Each posterunek has exactly one assigned operator at any given time; the engine rejects any command whose `posterunek_id` does not match the issuing player's current assignment. One player may hold any number of posterunki simultaneously; one posterunek may never be held by more than one player simultaneously. Ownership transfer follows the TAKEOVER flow (F-012).
- F-014: Device behavior rules (interlocking logic, signal dependencies) are enforced by the engine independent of regulatory compliance.
- F-015: The EDR (Electronic Traffic Register) component runs on the server and communicates with the simulation engine as a master data provider (train definitions, routes, timetable inputs).
- F-016: The EDR functional area is part of the native C++ client application, not a separate browser-based interface.
- F-017: Inter-module communication on the server (EDR↔engine, future AI↔engine) uses the same TCP framing protocol or direct in-process calls depending on deployment topology. No separate HTTP/REST layer is introduced for MVP.
- F-018: The binary protocol uses a 16-byte frame header (magic 0x5352, msg_type, flags, seq_id, payload_len, CRC-32) with FlatBuffers-serialized payloads. Schema files in `proto/` are the canonical contract. Message types: HANDSHAKE, HEARTBEAT, COMMAND, COMMAND_ACK, COMMAND_NAK, DOMAIN_EVENT, SNAPSHOT_CHUNK, SESSION_NOTICE, TAKEOVER_REQUEST, TAKEOVER_RESPONSE, CHAT_MESSAGE, VOICE_CHAN_JOIN/LEAVE/STATE. Details in `09-communication-contract.md`.
- F-019: Operators are assigned to posterunki (sub-posts) within a station. One player may control multiple posterunki from multiple stations simultaneously; each posterunek is controlled by exactly one player at any time. For MVP with single-posterunek stations the model degenerates to per-station ownership. The server maintains an `OwnershipRegistry` (`posterunek_id` → `player_id` mapping) updated atomically on assignment and TAKEOVER.
- F-020: A train is a `std::vector<Vehicle>` assembled from ordered vehicle `gID` references in a composition JSON. Each vehicle has `lengthM` and `axleCount`; the engine derives section occupancy from axle counting at `It`/`iz` devices. Physics v1 uses constant acceleration; v2 (post-MVP) uses per-vehicle `powerKW` and `tractionForceKN` with total consist mass, `tractionStatus` (`OPERATIONAL`/`DEFECTIVE`) for traction-capable units, and type-level `multipleCouplingCapable` metadata for traction-capable categories (with same-type locomotive traction-gain gating in runtime aggregation). Details in `10-vehicle-model.md`.
- F-021: A player client may be assigned any number of posterunki from any stations in the session, regardless of geographic grouping. The client presents one pulpit tab per assigned posterunek and one EDR tab per assigned posterunek; the operator switches freely between tabs. Posterunek assignment is managed by the session server and communicated in `HANDSHAKE_ACK`.
- F-022: Station topology (JSON bundle) is loaded by the client on demand — when the player opens a panel for that station — not pre-loaded for all stations at session start. The server loads all topologies at session start for interlocking validation.
- F-023: The simulation engine exposes an `IRandomEventSource` interface. Any module implementing this interface can inject `RandomEvent` payloads into the engine's priority command queue at `EMERGENCY` priority. The engine does not implement random events itself; it only provides the injection hook. Built-in event types (track closure, infrastructure failure, emergency services call) are defined as an open enum extensible without engine changes. This requirement exists to make the feature implementable post-MVP without modifying the engine core.
- F-024: An AI Dispatch Module (post-MVP) will implement `IDispatchAI` and operate as a separate server-side process. It observes all `DOMAIN_EVENT` broadcasts (read-only subscribe), generates `COMMAND` payloads, and submits them through the standard command queue as a virtual operator. The AI holds posterunek assignments via the same `OwnershipRegistry` mechanism as human players (F-019). The AI module is trained and runs locally on GPU hardware; the inference process communicates with the engine via the same binary framing as any other component (Channel 3). No AI logic is embedded in the simulation engine.

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
- N-010: SLO targets — p95 command round-trip ≤100 ms; engine loop ≤5 ms/tick at 20 Hz; packet loss tolerance <1%; client reconnect + snapshot sync ≤5 s; broadcast fanout lag (server to all clients) ≤10 ms.
- N-011: All display strings (device names, vehicle names, station names) use a language-keyed map (`{"pl": "...", "en": "..."}`) in JSON definitions. Code, identifiers, and comments are English-only. This structure supports future UI i18n without schema changes.

## Data requirements

- D-001: Station configuration stores topology and signaling device parameters.
- D-002: Timetable data stores trains, timing, and control points.
- D-003: Event log stores timestamp, type, source, and payload.
- D-004: Session snapshot stores current state and model version.
- D-005: Master train database (`fleet` schema) stores fleet-wide vehicle definitions and timetable templates. Populated by importing JSON config files at session start; read-only during an active session.

## Open questions

1. ~~Is AI part of MVP, or only a post-MVP iteration?~~ **Resolved:** AI is a separate module from the start; it is not embedded in the engine. It communicates via the engine API (F-010). Planned for post-MVP delivery.
2. ~~How detailed should train behavior be?~~ **Partially resolved:** v1 uses constant acceleration (MVP); v2 adds power/mass physics post-MVP. Details in `10-vehicle-model.md`.
3. ~~Are multi-level permissions required?~~ **Partially resolved:** station-level ownership and takeover is required (F-012, F-013). Full role hierarchy (admin, observer) is post-MVP.
4. ~~What should be the initial station config format (JSON/YAML/protobuf)?~~ **Resolved:** split JSON (three files per station: `meta.json`, `topology.json`, `objects.json`). Engine loads via nlohmann/json. Schema validated by the editor. Details in `08-track-topology-model.md`.
5. ~~Operator client↔server transport: TCP vs UDP?~~ **Resolved:** TCP persistent socket. Domain events are safety-critical and must all be delivered; UDP + mandatory ACK adds complexity with no gain at this traffic volume and frequency.
6. ~~Is Naterki station included?~~ **Resolved:** excluded from MVP. Post-MVP addition. Final MVP station list: 9 stations, Gdynia Chylonia → Gdańsk Orunia.
7. ~~Database topology: single database instance with separate schemas for master train data and session state, or two distinct database instances?~~ **Resolved:** one PostgreSQL instance, two schemas — `fleet` and `session`. Details in `11-database-model.md`.
8. ~~EDR integration path: adapt and wrap the existing C# prototype, or rewrite as a native server-side component?~~ **Resolved:** new native C++ component. Owns timetable templates and live EDR data. Communicates with engine via direct in-process call or Unix socket.
9. Supervisor/monitoring module: is a dedicated module needed to coordinate EDR↔engine data flow and oversee session integrity? What is its scope and placement?
