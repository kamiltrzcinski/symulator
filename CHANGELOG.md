# Changelog

All notable changes are documented here.

Entry format:
```
## [version] — YYYY-MM-DD
### Added / Changed / Fixed / Removed
- description
```

---

## [0.3.0] — 2026-05-07

### Added
- `scripts/git-hooks/pre-commit` — pre-commit hook tracked in the repository; activated automatically by CMake (no manual install required); checks: (1) `CHANGELOG.md` must be staged, (2) staged `.cpp`/`.hpp`/`.h` files must comply with `clang-format`; if formatting is wrong, prints exact commands to fix and re-stage; if `clang-format` is not installed, prints a warning and skips check 2 without blocking
- `.clang-format` — project C++ style definition (Google base, 4-space indent, Allman braces, 100-column limit)

### Changed
- `CMakeLists.txt`: added `git config core.hooksPath scripts/git-hooks` block executed on every cmake configure; activates the tracked hook for all GUI and CLI git clients that use the system git binary (VS Code, GitHub Desktop, command line)
- `docs/00-contributing.md`: "Pre-commit hook" section replaced with "Automatic commit checks" — documents what the hook checks, example output for each failure, and the `--no-verify` bypass for draft commits; Windows and Linux setup steps now include `clang-format` installation
- `scripts/pre-commit-hook.sh` removed — replaced by `scripts/git-hooks/pre-commit` (tracked hook with `core.hooksPath`)

---

## [0.2.9] — 2026-05-07

### Added
- `scripts/pre-commit-hook.sh` — bash hook that rejects commits where `CHANGELOG.md` is not staged; install with `cp scripts/pre-commit-hook.sh .git/hooks/pre-commit && chmod +x .git/hooks/pre-commit`
- `scripts/check-changelog.py` — cross-platform Python equivalent for Windows users whose Git client does not fire shell hooks; run manually with `python scripts/check-changelog.py`
- `data/vehicle_types/`, `data/vehicles/`, `data/trains/` — new fleet data directory tree (replaces `vehicles/`); accessible to engine, client, and editor
- **Vehicle type system** in `docs/10-vehicle-model.md`: type definitions carry all physical properties shared across units of the same model; individual instances reference `typeID` and override only differing fields (e.g. `massGrossT` for a loaded freight wagon); property resolution rule: instance field takes precedence when present and non-null, otherwise falls back to type
- `typeID` gID scheme: `VT-GLB-{typeName}-{7digits}`, e.g. `VT-GLB-ET22-0000001`; area is `GLB` (global) because types are not station-specific
- **Conflict detection guide** in `docs/00-contributing.md` — Step 1 now explains merge conflict markers, how to resolve them, and `git status` output to watch for
- **Pre-commit hook section** in `docs/00-contributing.md` — installation one-liner, Windows caveat (GitHub Desktop / TortoiseGit may skip hooks), manual fallback command

### Changed
- `CMakeLists.txt`: project version is now parsed from the first `## [X.Y.Z]` entry in `CHANGELOG.md` at configure time — `CHANGELOG.md` is the single source of truth; a missing entry causes a `FATAL_ERROR` to catch forgotten updates early
- `vcpkg.json`: removed the `version` field (it was not used; vcpkg only needs it for publishing to a registry, not for consuming packages)
- `docs/10-vehicle-model.md`: file layout updated from `vehicles/definitions/` and `vehicles/trains/` to `data/vehicle_types/`, `data/vehicles/`, `data/trains/`; vehicle JSON redesigned with two-level (type + instance) schema; train consist `gID` area updated from `GGO` to `TRJ`
- `docs/00-contributing.md`: project folder structure updated to reflect `data/`, `scenarios/`, `proto/`, `docker/`, `scripts/`

---

## [0.2.8] — 2026-05-08

### Added
- **Project directory skeleton** — `CMakeLists.txt` (C++20, Ninja, vcpkg integration, `BUILD_CLIENT`/`BUILD_EDITOR` options), `vcpkg.json` (flatbuffers, nlohmann-json, gtest, openssl), stub `CMakeLists.txt` + placeholder sources for `engine/`, `server/`, `libtrackview/`, `client/`, `editor/`, `tests/engine`, `tests/server`, `tests/integration`; root `.gitignore`
- **FlatBuffers schemas** — `proto/common.fbs` (enums: `Aspect`, `SwitchPosition`, `DerailerPosition`, `BlockSectionState`, `ChangeCause`, `SessionState`), `proto/session.fbs` (`Handshake` / `HandshakeAck`), `proto/commands.fbs` (all 7 command types + `CommandAck` + `CommandNak`), `proto/events.fbs` (all 15 event types 0x01–0x0F), `proto/snapshot.fbs` (`Snapshot` root table with all state arrays)
- **Docker dev stack** — `docker/docker-compose.yml` (PostgreSQL 16, server service with healthcheck), `docker/Dockerfile.server` (multi-stage: builder + minimal runtime, non-root user), `docker/init.sql` (complete schema including new `fleet.vehicles` columns), `docker/.env.example`
- **Interlocking model** (`docs/14-interlocking-model.md`) — rules R1–R7 for all command types, route lifecycle diagram (LOCKED → OCCUPIED → AUTO RELEASE), conflict detection algorithm, signal aspect selection table, open questions Q-ILK-1 to Q-ILK-3
- **Reference station data** — `scenarios/reference/gdynia_orlowo/meta.json`, `topology.json` (3 track sections, 2 switches, 2 boundary nodes), `objects.json` (6 signals, 1 posterunek `GDO_nastawnia`), `scenarios/reference/sections.json` (block sections GDO–SOP and GGO–GDO)

### Changed
- `docs/11-database-model.md`: `fleet.vehicles` table now includes `mass_gross_t REAL` (nullable, loaded freight mass) and `braking_lambda_pct INTEGER NOT NULL DEFAULT 100` (UIC λ%) — aligns DB schema with vehicle JSON model from doc 10

---

## [0.2.7] — 2026-05-07

### Added
- `brakingLambdaPct` field in vehicle JSON — UIC braking percentage λ; present on every vehicle; used to compute `a_decel` as `(λ/100) × g × 0.85`; reference table by train type added to Physics section
- `massGrossT` field in vehicle JSON — loaded mass for freight wagons; `null` for locomotives and passenger vehicles; engine uses `massGrossT` when present, `massEmptyT` otherwise (`effectiveMassT` function)
- Second JSON example in vehicle definition (loaded freight wagon `403Z`)
- Full field reference table for vehicle definition JSON
- Trapezoidal speed profile explanation with ASCII diagram — both V1 and V2 produce linear acceleration/deceleration ramps, no instant speed changes
- `consist_lambda` derived value — mass-weighted average λ across all vehicles in consist; computed at load time alongside `total_mass_t`

### Changed
- V1 physics: `a_decel` is now derived from `consist_lambda` (not a global config constant); `a_accel` remains a configurable session parameter; tick update formula clarified with `v_target` derivation
- V2 physics: braking phase unified with V1 — always λ-derived; V2 only extends the traction (acceleration) side with force-balance model
- `total_mass_t` derivation updated to use `effectiveMassT(v)` per vehicle
- Q-VEH-3 resolved: `massGrossT` + `brakingLambdaPct` together cover the loaded/empty distinction

---

## [0.2.6] — 2026-05-07

### Changed
- `docs/00-contributing.md`: complete rewrite for beginner audience — added glossary of terms (repo, commit, branch, compiler, CMake, Ninja, vcpkg, Qt, build), detailed Windows setup steps with screenshots guidance (Git installer option, VS Build Tools workload selection), SSH key generation walkthrough, explanation of what happens during a build and why the first run takes 60 minutes, step-by-step contribution workflow (8 steps from pulling latest code to opening a PR), expanded commit message guide with good/bad examples, Things You Must Not Do section

---

## [0.2.5] — 2026-05-07

### Added
- `docs/00-contributing.md`: development environment setup for Linux and Windows (prerequisites, configure + build commands), commit message format, CHANGELOG requirement, branch policy, release management note
- Cross-platform build section in `03-initial-architecture.md`: Qt 6 ≥ 6.6 explicit version and Qt modules listed; Ninja added as executor alongside CMake + vcpkg; CI/CD section — GitHub Actions free on public repo, three-platform build matrix (ubuntu-24.04 / windows-2022 / macos-14), vcpkg + Qt cache strategy, release workflow on version tags; contributor onboarding goal updated with pointer to `00-contributing.md`

### Changed
- `docs/README.md`: added `00-contributing.md` to the index

---

## [0.2.4] — 2026-05-07

### Added
- `libtrackview` shared rendering library architecture (`13-scenario-editor-architecture.md`): `TrackGrid`, `TrackScene`, `StateOverlay`, `TileSet` abstraction with `EbiScreenTileSet` (wide flat cells, coloured occupancy, arrow-head markers) and `TechnicalDiagramTileSet` (square cells, full 45° diagonals, monochrome); visual style comparison table; usage in operator client
- Station Editor native project format: `.scendb` (SQLite) with full schema — `project_meta`, `tiles`, `connections`, `objects`, `edit_history`; rationale over flat JSON (FK enforcement, SQL queries, persistent undo/redo)
- Layout style (`ebi_screen` / `technical_diagram`) selected at project creation; stored in `project_meta`; pure rendering hint, does not affect exported JSON bundles
- Editor offline operation: topology authoring and manual timetable editing require no server connection; PLK import is forwarded to server and handled by `IPLKImporter` server-side
- Component 7 (Scenario Editor) in initial architecture (`03-initial-architecture.md`): standalone C++ desktop tool, links `libtrackview`, produces station bundles + sections + timetable data, operates offline for authoring
- Cross-platform build requirements (`03-initial-architecture.md`): Linux/Windows/macOS x86-64 targets; all dependencies bundled (Qt6 via deploy tools, SQLite amalgamation, nlohmann/json header-only); CMake ≥ 3.25 + vcpkg; Windows contributor onboarding goal (clone → configure → build, no manual steps)

---

## [0.2.3] — 2026-05-07

### Added
- Scenario editor architecture (`13-scenario-editor-architecture.md`): three-editor model (Station Editor, Route Editor, Timetable Editor); `sections.json` inter-station section data model with `boundary_iz_from`/`boundary_iz_to` interlocking boundary fields; `routes.json` grouping; `IScenarioLinter` three-layer validation (L1 station bundles, L2 route topology, L3 timetable); `LintDiagnostic` structure; `ScenarioBundle` input type; scenario directory layout
- Extended area assessment: Wejherowo → Pruszcz Gdański (13 stations) feasible on 4 vCPU / 8 GB RAM; constraints are editorial, not architectural
- Four open questions (Q-EDI-1 through Q-EDI-4)

---

## [0.2.2] — 2026-05-06

### Added
- Server internal API document (`12-server-api.md`): seven C++ pure-virtual interface contracts (`ICommandHandler`, `IEventEmitter`, `ISnapshotProvider`, `ITopologyStore`, `ISessionStore`, `IEDRService`, `IPLKImporter`), module boundary diagram, session startup call sequence
- PLK Open Railway Data API integration: `IPLKImporter` fetches `GET /api/v1/schedules` and `GET /api/v1/dictionaries/stations` at server startup; 9 req/run (well within 100 req/hour basic limit); maps PLK fields to `fleet.timetable_templates`; future real-time operations mode noted (post-MVP, no architectural changes required)
- Three open questions recorded (Q-API-1 through Q-API-3)

### Changed
- Backlog P1: `Draft server API` marked resolved
- `docs/README.md` index updated

---

## [0.2.1] — 2026-05-06

### Added
- F-021: flexible posterunek assignment — player receives any number of posterunki from any stations; client shows one pulpit tab + one EDR tab per posterunek
- F-022: on-demand client topology loading — station JSON bundle loaded when player opens panel, not at session start; server loads all topologies at startup for interlocking
- N-010: SLO targets formalised — p95 command round-trip ≤100 ms, engine loop ≤5 ms/tick, packet loss <1%, reconnect ≤5 s, broadcast fanout ≤10 ms
- N-011: language policy — code/comments English-only; display strings use `{"pl": ..., "en": ...}` map in JSON for future i18n

### Changed
- Open questions OQ-2 (train physics) and OQ-6 (Naterki) marked resolved
- D-005 updated to reflect `fleet` schema decision
- Backlog: SLO targets, station list, ownership protocol, language policy all marked resolved
- Backlog: Ownership section updated (EDR owner confirmed as new native C++ component)

---

## [0.2.0] — 2026-05-06

### Added
- Database model document (`11-database-model.md`): two-schema PostgreSQL layout (`fleet` + `session`), full DDL for all tables (vehicles, train definitions, timetable templates, sessions, events, snapshots, edr_entries, posterunek_assignments, chat_log), EDR row volume estimate for Trójmiasto (~1 020 rows/session), MVP retention policy, two open questions (Q-DB-1, Q-DB-2)
- MVP load profile section in `06-server-sizing-baseline.md`: full Trójmiasto = 9 stations, 18–27 signaling operators + 9 EDR operators = ~36 peak clients, 20–100 events/s, burst 100 events/s; load assumptions updated to reflect binary FlatBuffers payloads (120 bytes vs. 500-byte JSON estimate)

### Changed
- `02-system-requirements.md`: OQ-7 (database topology) and OQ-8 (EDR integration path) marked resolved
- `05-starter-backlog.md`: four P0 items marked resolved (database model, load profile, EDR integration path, database topology)
- `docs/README.md` index updated

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
