# Changelog

All notable changes are documented here.

Entry format:
```
## [version] — YYYY-MM-DD
### Added / Changed / Fixed / Removed
- description
```

---

## [0.1.9] — 2026-05-06

### Added
- Communication contract extended (`09-communication-contract.md`):
  - Message types: `CHAT_MESSAGE (0x60)`, `VOICE_CHAN_JOIN/LEAVE/STATE (0x70-0x72)`; range 0x80–0xFF reserved
  - Multi-operator model: posterunek (sub-post) concept; `HANDSHAKE_ACK` now returns `assigned_posterunki[]`; `TAKEOVER_REQUEST` targets a `posterunek_id`
  - `PosterunekOwnershipChanged (0x0E)` replaces `StationOwnershipChanged`; `TrainComposed (0x0E)`, `TrainDecomposed (0x0F)` added to domain event catalog
  - Player communication section: chat payload spec; voice architecture decision (audio on separate UDP/DTLS, TCP reserved for signaling only)
  - External access path: TLS 1.3 wraps TCP frame unchanged; auth_token hook already present in HANDSHAKE
  - Frame design validation table: all extensions confirmed compatible with 16-byte header
  - Open questions Q-COM-5 and Q-COM-6 added; Q-COM-2 resolved
- Vehicle and train model document (`10-vehicle-model.md`): vehicle JSON definition (gID, type, lengthM, axleCount, massEmptyT, powerKW, tractionForceKN), train composition JSON (ordered vehicle list), derived properties (total_length_m, total_axles, total_mass_t), axle-counting occupancy model, physics v1 (constant acceleration, MVP) and v2 (F=P/v minus resistances, post-MVP), DOMAIN_EVENT integration table, three open questions (Q-VEH-1 through Q-VEH-3)

### Changed
- `09-communication-contract.md`: `train_gID` added to TrackSection and Switch occupancy events; `speed_kmh` added to TrainMovement event; snapshot updated with `posterunek_assignments`
- `docs/README.md` index updated

---

## [0.1.8] — 2026-05-06

### Added
- Communication contract document (`09-communication-contract.md`): 16-byte binary frame layout (magic 0x5352, msg_type, flags, seq_id, payload_len, CRC-32), full message type catalog with direction arrows, command catalog (7 types), domain event catalog (13 types), snapshot structure, ownership protocol, handshake/reconnect flow, heartbeat rules, sequencing and error handling rules
- Serialization format decision: FlatBuffers selected (zero-copy, schema = contract document, forward-compatible, C++20 native); `.fbs` schema files in `proto/` as single source of truth
- F-018 added to system requirements covering binary frame and FlatBuffers decision
- Four open questions recorded (Q-COM-1 through Q-COM-4)

### Changed
- Starter backlog P0: command/event/snapshot contract item marked resolved
- `docs/README.md` index updated

---

## [0.1.7] — 2026-05-06

### Added
- Track topology object model documented (`08-track-topology-model.md`): track section (`OT`) and switch (`ZWR`) as graph edges; `It` and `iz` axle counters as graph nodes; field definitions for length, electrification, max speed, signals
- Station config file format decided: split JSON bundle (`meta.json` / `topology.json` / `objects.json`) per station, loaded by engine via nlohmann/json, validated by editor against JSON Schema; future option noted (SQLite as editor-native format with JSON export)
- Topology graph model: layout as directed multigraph, occupancy and route checking reduce to graph traversal
- Three open topology questions recorded (Q-TOPo-1 double slips, Q-TOPo-2 diamond crossings, Q-TOPo-3 coordinate system)

### Changed
- Open question #4 (`02-system-requirements.md`): marked resolved — split JSON format selected
- Starter backlog P0: station map configuration format item marked resolved

---

## [0.1.6] — 2026-05-03

### Changed
- Protocol consolidation: removed REST/HTTP from engine and all inter-module channels; one network protocol for the entire system (TCP persistent socket + custom binary framing for client↔server; direct call/Unix socket for intra-server EDR↔engine)
- F-017: rewritten — no REST API on engine; inter-module communication uses direct calls or TCP framing reuse
- N-009: rewritten — single TCP protocol decision, rationale for ruling out REST as second stack
- Initial architecture Channel 2 (EDR↔engine): changed from REST to direct call/Unix socket; Channel 3 (AI) deferred to post-MVP design phase
- Initial architecture transport decisions summary: consolidated to one protocol rule
- Decisions to make #1: updated resolved entry to reflect no-REST outcome
- Starter backlog: replaced REST API design item with EDR↔engine data contract item (direct call/Unix socket boundary)

## [0.1.5] — 2026-05-03

### Changed
- Client↔server transport decision closed: TCP persistent socket with custom binary framing selected; UDP ruled out (domain events are safety-critical and must all be delivered; UDP + mandatory ACK = reimplementing TCP with more complexity and no gain)
- N-009: updated to reflect TCP decision and rationale
- Open question 5: marked resolved
- Initial architecture Channel 1: updated from "deferred" to decided TCP with full rationale
- Initial architecture: "Display/rendering options" section replaced with single resolved decision; removed option listings
- Initial architecture "Decisions to make": items 1, 5, 6 marked resolved
- Starter backlog: TCP vs UDP item marked resolved with rationale summary

## [0.1.4] — 2026-05-03

### Changed
- Client delivery model: consolidated from two separate clients (native C++ + browser) to a single native C++ desktop application containing both Pulpity (station panel tabs) and EDR (register tabs); browser client removed entirely
- F-011: updated — single C++ application, both areas tab-based, no browser client
- F-016: updated — EDR view is part of the C++ application, not a browser interface
- N-009: updated — browser constraint removed; TCP/UDP both viable for native client (later resolved in 0.1.5)
- Open question 5: restored as open (browser constraint no longer applies)
- Initial architecture component 3: renamed and rewritten as "Client Application (native C++ desktop)" with Pulpity and EDR sub-areas described
- Initial architecture component 4 (EDR): removed browser interface note
- Initial architecture Channel 1: updated rationale (no browser constraint)
- Initial architecture minimum deployment layout and working decisions: updated to reflect single C++ client
- Starter backlog: rendering model checkbox updated to reflect native C++ decision

## [0.1.3] — 2026-05-03

### Added
- Vision and scope: EDR as 4th MVP component (server-side train data provider, browser-based management UI); client delivery model section (browser-based confirmed)
- System requirements: F-015 EDR server component, F-016 EDR browser interface, F-017 engine REST API for inter-module communication
- System requirements: D-005 master train database; open questions 7-9 (database topology, EDR integration path, supervisor module)
- Initial architecture: REST API added to Core Simulation Engine component; EDR as component 4 (Persistence becomes 5, AI becomes 6); minimum deployment layout updated; rendering and EDR decisions recorded in working decisions
- Starter backlog: rendering model marked resolved; P0 items for EDR integration path, database topology, language policy; P1 items for EDR↔engine contract, REST API design, supervisor module scope; EDR ownership note

## [0.1.2] — 2026-05-03

### Added
- Vision and scope: station assignment model (5-6 stations, default 3 per player, tabbed client, permission-based takeover)
- System requirements: F-011 tabbed multi-panel client, F-012/F-013 station ownership and takeover, F-014 device behavior rules
- System requirements: N-008 SOLID at code and architecture level, N-009 replaceable transport layer (UDP candidate)
- System requirements: resolved open questions for AI module and permissions; added Naterki and UDP open questions
- Initial architecture: SOLID/modularity as core principles
- Initial architecture: tabbed panel design in Client UI component
- Initial architecture: AI Module defined as separate process from day one with explicit rationale
- Initial architecture: Transport layer options section (TCP/WS default vs UDP with custom framing, working decision)
- Starter backlog: P0 items for station list confirmation and station ownership protocol
- Starter backlog: P1 items for tabbed client layout and TCP vs UDP evaluation

## [0.1.1] — 2026-05-03

### Changed
- README: added one-time hook setup step for new contributors

## [0.1.0] — 2026-05-03

### Added
- Initial project planning documentation baseline
- Vision and scope with MVP definition and success criteria
- System requirements: functional (F-001–F-010), non-functional (N-001–N-007), data (D-001–D-004)
- Initial architecture: component split, communication contracts, deployment and rendering options
- Roadmap with 5 delivery stages (Stage 0–4) and definition of done per stage
- Starter backlog with P0/P1/P2 prioritization
- Dedicated server sizing baseline with first-principles load analysis and RPi4/RPi5 assessments
