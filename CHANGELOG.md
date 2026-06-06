# Changelog

All notable changes are documented here.

## [0.5.10] - 2026-06-06

### Fixed
- Test and script paths updated after data migration: `data/stations.json` → `scenarios/stations.json`,
  `data/vehicle_types` → `packages/vehicle-types`, `data/vehicles` → `packages/vehicles`,
  `data/trains` → `packages/trains`, `data/carriers.json` → `packages/carriers/carriers.json`
- CI workflow: replaced old zip-based symulator-data download with `scripts/fetch_packages.py`

### Changed
- README: documented git hooks activation and initial package fetch step

## [0.5.9] - 2026-06-06

### Changed
- External data (trains, vehicles, vehicle-types, schedules, carriers) moved to `symulator-data` repo, distributed as versioned packages
- `data/stations.json` moved to `scenarios/stations.json`
- `scenarios/trains/` removed — schedules are now in the `schedules` package

### Added
- `packages/` directory for downloaded data packages (git-ignored)
- `scripts/fetch_packages.py` — fetches latest packages from GitHub Releases, cleans old versions
- `.githooks/post-merge` — auto-fetches packages after `git pull`

## [0.5.8] - 2026-06-04

### Changed
- **Grouped CTest execution**: unit/integration suites are now organized into logical groups with dedicated labels:
  - `group:unit-engine-core`, `group:unit-engine-sim`
  - `group:unit-server-protocol`, `group:unit-server-dispatch`
  - `group:unit-srk`, `group:unit-proto`, `group:unit-validation`
  - `group:integration-non-db`, `group:integration-db`
- **Test targets split**:
  - `tests_engine` split into `tests_engine_core` and `tests_engine_sim`.
  - `tests_server` split into `tests_server_protocol` and `tests_server_dispatch`.
  - `tests_integration` split into `tests_integration_non_db` and `tests_integration_db`.
- **CI test flow**: GitHub Actions now runs tests by logical groups using `scripts/run_grouped_ctest.py` and publishes per-group JUnit artifacts.
- **Integration helper script**: `scripts/run_integration_tests.sh` now selects integration groups via `group:integration-` labels.

### Added
- **Grouped test runner**: new `scripts/run_grouped_ctest.py` runs CTest in explicit groups, prints `PASS`/`FAILED` summary per group, and lists failing test names.
- **Test docs**: tests/README.md new test guide with know-how 

## [0.5.7] - 2026-06-03

### Changed
- **Universal UID refactor**: replaced all string-based identity types (`GID`, `SID`,
  `DispatchAreaID`, `ControlSystemID`) with a single 48-bit, JSON-safe `UID` (`uint64_t`)
  across the entire codebase — engine, server, SRK, proto schemas, JSON data files, and
  database schema.
- **UID bit layout** (`docs/21-uid-legend.md`): DOMAIN\[47:40\] | KIND\[39:32\] | SCOPE\[31:16\] | INSTANCE\[15:0\].
  19 KIND values cover rolling-stock, infrastructure, and operational entities.
- **Station registry** (`data/stations.json`): added numeric instance map (GOr=1, Sp=2, GGO=3, OT=4);
  infrastructure UIDs encode the owning station in the SCOPE field.
- **JSON data migration**: `data/vehicle_types`, `data/vehicles`, `data/trains`, and
  `scenarios/*/topology.json` + `objects.json` now use numeric `uid`, `type_uid`,
  `vehicle_uids`, `neighborUID`, `itUID`, `izUID`, `governs_section`, etc.
- **FlatBuffers proto schemas**: all `string g_id`/`route_id`/`alarm_id` fields replaced
  with `uint64 uid`/`route_uid`/`alarm_uid`; all generated headers regenerated.
- **Database schema**: `station_sid TEXT` → `station_uid BIGINT`, `object_gid TEXT` →
  `object_uid BIGINT`, `section_gid TEXT` → `section_uid BIGINT` across all tables.
- **Layer1/Layer2 validators**: updated to validate numeric `uid` fields and UID-encoded
  cross-references instead of string GIDs and sID fields.

### Added
- **UID codec tests** (`tests/engine/test_uid_codec.cpp`): encode/decode roundtrip,
  boundary values, JSON-safety, and no-collision assertions for every domain/kind.
- **UID registry validator** (`scripts/validate_uid_registry.py`): enforces valid UIDs,
  no duplicates, and correct SCOPE in all data and topology JSON files; wired into CI as
  a cmake target after FlatBuffers schema validation.
- **UID registry integration test** (`tests/integration/test_uid_registry.cpp`): loads
  the real scenario files and verifies UID validity, uniqueness, and station-SCOPE
  consistency.

### Fixed
- **Integration tests**: fixed db schema and integration tests with db interface 
  rework.

## [0.5.6] - 2026-06-01

### Changed
- **Carrier database**: changed `data/carriers.json` from a flat name list into a `carriers`
  catalog of carrier objects with numeric semantic `id`, `name`, `type`, and `logo`, merging
  passenger/freight variants where they represent the same carrier.
- **Fleet carrier assignment**: train consists now reference carriers with numeric `carrierId`
  UIDs instead of validating the human-readable carrier name string.
- **UID model**: added a 48-bit, JSON-safe `UID` helper layout for numeric domain identifiers.

## [0.5.5] - 2026-05-28

### Added
- **Timetable operating days**: `fleet.timetable_templates` now stores ISO weekday-based
  `operating_days` (`1`=Monday ... `7`=Sunday), with server startup seeding `session.edr_entries`
  only for timetable rows active on the current local day.
- **Operating-days documentation**: added `docs/20-timetable-operating-days.md` with the
  `1-7`, `1-5`, and `6,7` notation legend.
- **Train scenarios**: added TME 551462 (`ST48`) and MPE 5600 (`EU160`) scenario train JSON files.
- **EDR journal persistence**: added backend-only `session.edr_journal_entries` storage for
  operator-visible registrations, corrections, cancellations, telephonegram notes, track occupancy
  notes, and crossing notifications, independent of PIP/ZPR.
- **Carrier database**: implemented a flat JSON array `data/carriers.json` storing 145 carrier names, loaded by `FleetRegistry` on startup.
- **Carrier assignment**: `TrainConsist` structure and JSON schemas now support an optional `carrier` field, verified against the database.

### Changed
- **Scenario train routes**: expanded ROJ 50622 and TDE 592599 route definitions with additional
  technical operating points (`R`, `PZS`, transit groups) using parent-station times.
- **TME 551462 timings**: filled freight-train route times from Gdańsk Osowa at 12:32 using
  80 km/h and PLK line kilometre marks for lines 201/202; Lębork remains marked `PT`.

### Fixed
- **Integration test isolation**: `PgDbWriterFixture.SeedEdrEntriesForOperatingDay_NoTemplatesForDay`
  now verifies absence of seeded rows for a test-specific train number in the current session,
  removing a brittle dependency on globally persisted `fleet.timetable_templates` rows across tests.

## [0.5.4] - 2026-05-25

### Added
- **FleetRegistry carrier test coverage**: added 6 unit tests in
  `tests/engine/test_fleet_registry.cpp` covering `carriers.json` loading,
  acceptance of known carriers, rejection of unknown carriers, null/missing
  `carrier` handling, and invalid carrier dictionary formats.
- **PgDbWriter negative-path integration coverage**: added 15 integration tests
  in `tests/integration/test_pg_db_writer.cpp` for `operating_days` validation,
  ISO weekday range checks, zero-row seed behavior, `ON CONFLICT` update
  semantics, and EDR journal status/constraint handling.
- **Dispatch channel split**: former dispatch-wire/domain handling was split into two server modules:
  - `DispatchChannel` (wire/protocol layer): FlatBuffers decode + sender verification + outbound result frame broadcast,
  - `DispatchCoordinator` (domain layer): S-form state transitions, `session.dispatch_telegrams` persistence, and EDR handoff.
- **Wire-protocol rename (0x61 unchanged)**: protocol/schema naming migrated to dispatch-channel terminology:
  - schema file renamed to `proto/dispatch_channel.fbs`,
  - root/table/union enums renamed to `DispatchChannelMessage*`,
  - generated header `autogens/proto/dispatch_channel_generated.h` replaced the previous legacy header,
  - gateway API renamed to `set_dispatch_channel_handler()` and frame constant to `msg_type::kDispatchChannel`.
- **CI schema guard**: `.github/workflows/ci.yml` now runs `validate_proto_schemas` to enforce FlatBuffers schema/generated-header synchronisation.
- **DB writer contract extension**: `IDbWriter` gained five persistence methods used by server boundaries:
  - `save_snapshot(...)`,
  - `append_chat_message(...)`,
  - `assign_operating_point(...)`,
  - `release_operating_point(...)`,
  - `upsert_timetable_template(...)`.
  `NullDbWriter` and `PgDbWriter` were updated accordingly.
- **Scenario validation library**: new `libscenario_validation` static library with layered validators:
  - Layer 1 (station bundle integrity checks),
  - Layer 2 (inter-station/route consistency checks),
  - Layer 3 scaffold (reserved for timetable-layer checks).
- **New test suites**:
  - `tests/server/test_dispatch_coordinator.cpp` (12 tests),
  - `tests/server/test_dispatch_channel.cpp` (12 tests),
  - `tests/libscenario_validation/test_layer1.cpp` (9 tests),
  - `tests/libscenario_validation/test_layer2.cpp` (6 tests),
  - integration additions in `tests/integration/test_pg_db_writer.cpp` for new `IDbWriter` methods.
- **Build wiring**: `libscenario_validation` and its tests were added to CMake target graph (`CMakeLists.txt` + `tests/CMakeLists.txt`).
- **Scope correction**: removed server-side PLK importer implementation files and related HTTP dependency from build/dependency manifests; documentation updated to reflect that PLK import belongs to a separate integration path.

- **Unit tests — srk::common**: new test suite `tests/srk/test_srk_common_route_graph.cpp`
  (11 tests) covering `find_route_path()` and `make_route_id()` in detail: straight/divergent
  switch traversal, derailer collection, null-opt cases, ID determinism and uniqueness.
- **Unit tests — srk::common device rules**: new test suite `tests/srk/test_srk_common_device_rules.cpp`
  (24 tests) exercising R1–R4 and R7 check and execute helpers directly: switch position
  (valid, occupied, moving, route-locked, already-in-position, not-found, instant and delayed throw),
  signal aspect (valid, not-found, route-locked proceed vs stop), derailer (valid, not-found,
  route-locked, guarded section occupied), block section (open valid, close with axles), and
  alarm acknowledgement (exists, not-found, cleared change).
- **Integration tests — EdrCoordinator**: new test suite `tests/integration/test_edr_coordinator.cpp`
  (5 tests) verifying that `EdrCoordinator::on_telegram_accepted()` writes the correct
  departure/arrival timestamp and status to `session.edr_entries` for S25/S26 in both SENT and
  RECEIVED direction; unhandled form types leave the row untouched.
- **Integration tests — pip.track_state**: new test suite `tests/integration/test_pip_track_state.cpp`
  (5 tests) validating `PgDbWriter::upsert_pip_track_state()` against a live PostgreSQL instance:
  initial INSERT, free-section empty-array write, ON CONFLICT DO UPDATE, multi-section isolation,
  and `updated_at` persistence.
- **DB schema**: `pip` schema and `pip.track_state` table added to `docker/init.sql`; previously the
  table was written at runtime but absent from the init script, causing a startup failure on fresh
  containers.
- **Rename `posterunek` → `operating_point`** (continued): two missed occurrences fixed —
  `single-posterunek` in F-019 (`02-system-requirements.md`) and `revokePosterunek` in the
  `IDispatchAI` interface sketch (`03-initial-architecture.md`).
- **PipWriter** (`server/include/server/pip_writer.hpp` + `server/src/pip_writer.cpp`): new class
  consuming `PipEvent` batches from the ENGINE thread and persisting them to `pip.track_state`.
  - `IDbWriter::upsert_pip_track_state(session_id, section_gid, trains_json)` added as a pure
    virtual; executes `INSERT … ON CONFLICT (session_id, section_gid) DO UPDATE SET trains, updated_at`
    — `path_confirmed` is intentionally not touched (managed by route-confirmation commands).
  - `NullDbWriter` captures calls in `pip_upserts` (`vector<PipUpsert{section_gid, trains_json}>`).
  - `PgDbWriter` implements the UPSERT with `$3::jsonb` cast.
  - `TrainSlot` serialised to JSON array: `[{"number":…,"has_extra_info":…,"manually_placed":…,"entry_side":"LEFT"|"RIGHT"}]`;
    free section or absent slot → `"[]"`.
  - `SessionServer` constructs `pip_writer_` (after `edr_coordinator_`, before `dispatch_channel_`
    in LIFO order); `pip_cb` lambda replaced with `pip_writer_->on_pip_events(events)`.
  - 4 unit tests in `tests/server/test_pip_writer.cpp`: `FreeSection_UpsertWithEmptyTrains`,
    `OccupiedSection_UpsertWithTrainSlot`, `LcsBoundaryCrossing_UpsertTargetSection`,
    `MultipleBatch_UpsertAllSections`. **333/333 tests pass**.
- **FleetRegistry tests**: Updated fleet registry fixtures for the family-based vehicle type layout
  and covered ignored sidecar JSON files.

### Changed
- **Git pre-commit clang-format scope**: formatting enforcement now targets only
  staged C/C++ source and header files (`.c`, `.cc`, `.cpp`, `.cxx`, `.h`,
  `.hh`, `.hpp`, `.hxx`) and passes file paths as a safe array to
  `git clang-format --diff`.
- **Vehicle model documentation**: `docs/10-vehicle-model.md` now documents
  optional train-consist field `carrier`, including dictionary source
  `data/carriers.json` and null/missing behavior.
- **FlatBuffers autogen refresh**: regenerated
  `autogens/proto/common_generated.h` with project `flatc` (v25.12.19);
  output change is formatting-only (no schema/wire semantic change).
- **Warnings cleanup**: fixed clean-build warnings in the dispatch stack by handling all `DispatchFormType` enum values in `DispatchCoordinator::form_type_str()` and by consuming `[[nodiscard]]` results in `test_dispatch_coordinator.cpp`.
- **Rename `posterunek` → `operating_point`** throughout the entire codebase:
  - `docker/init.sql`: `session.posterunek_assignments` table renamed to
    `session.operating_point_assignments`; column `posterunek_id` renamed to `operating_point_id`
    in both `fleet.timetable_templates` and `session.edr_entries`; index renamed accordingly.
  - All 10 documentation files updated: `posterunek_id` → `operating_point_id`,
    `PosterunekOwnership` → `OperatingPointOwnership`, `assignPosterunek` → `assignOperatingPoint`,
    `releasePosterunek` → `releaseOperatingPoint`, etc.
- **Fleet data**: Reworked `data/vehicles` so each physical vehicle instance lives in its own
  directory with a canonical `vehicle.json`, aligned with the `data/vehicle_types` hierarchy.
- **Fleet data**: Grouped vehicle type JSON files by model family/series under `data/vehicle_types`,
  including shared folders for EMU `WE` variants and electric locomotive `111E` variants.
- **Fleet loader**: Updated vehicle loading to read only `vehicle.json` files under `data/vehicles`,
  leaving room for per-vehicle sidecar assets and metadata.
- **Build**: Scoped compiler warning flags per compiler so MSVC builds use native warning options.

### Fixed
- **Carrier validation load order**: `FleetRegistry::load()` now loads
  `data/carriers.json` before loading train consists, so optional `carrier`
  validation uses the populated dictionary instead of an empty in-memory list.
- **Timetable template UPSERT compatibility**: `PgDbWriter::upsert_timetable_template(...)` now writes to schema-accurate `scheduled_arrival` / `scheduled_departure` `INTERVAL` columns (via `make_interval(secs => ...)`) instead of non-existent `*_secs` columns.
- **Schema conflict target for UPSERT**: added unique index `uq_timetable_train_station` on `fleet.timetable_templates(train_number, station_sid)` so `ON CONFLICT (train_number, station_sid)` is valid and integration test `PgDbWriterFixture.UpsertTimetableTemplate_InsertsRow` passes.

## [0.5.3]

### Fixed
- **DB schema**: added `ON DELETE CASCADE` to all foreign keys referencing `session.sessions(id)` in
  `docker/init.sql` (`events`, `snapshots`, `edr_entries`, `operating_point_assignments`, `chat_log`,
  `dispatch_telegrams`); integration test `TearDown` no longer hits FK constraint violations when
  deleting the session row.
- **CI**: bumped `actions/checkout@v4` → `v6` and `actions/cache@v4` → `v5`; both now run on
  Node.js 24 (v4 actions run on deprecated Node.js 20 which is removed from runners on 2026-09-16).

### Added
- **TrainFleet switch & boundary traversal** (`engine`): trains now move through the topology
  graph correctly.
  - `resolve_next_section()` extended to handle Switch neighbours: trunk→straight/divergent based on
    `SwitchPosition`; leg→trunk regardless of position.  A `MOVING` switch pins the train at the
    section boundary until the throw completes.
  - `BoundaryNode` neighbours trigger a boundary crossing: the section is freed, a
    `PipEvent{lcs_boundary_crossing=true}` is emitted, and the train is removed from the fleet in
    the same tick.
  - `NextSectionInfo` struct (replaces the previous `optional<GID>` return type) carries the next
    section GID, the new `from_gid` (switch GID after a switch crossing — required for correct
    `ahead_port()` resolution on the new section), and the `is_boundary_crossing` flag.
  - 8 new unit tests in `tests/engine/test_train_fleet.cpp` covering: direct section, trunk→straight,
    trunk→divergent, MOVING switch, leg→trunk, boundary detection, full tick traversal through a
    switch, and boundary removal with `PipEvent` emission.

## [0.5.2] - 2026-05-24

### Added
- **LICENSE**: proprietary All Rights Reserved — source available for inspection only; no license is granted to use, copy, modify, or distribute without written permission from both copyright holders.

### Fixed
- **CI**: `3rdParty/vcpkg` added as a git submodule so `actions/checkout` with `submodules: recursive` can download it; previously `bootstrap-vcpkg.sh` was missing in the runner environment.
- **Compiler warnings**: migrated all `pqxx::transaction_base::exec_params()` call sites to the non-deprecated `exec(sql, pqxx::params{...})` API (pqxx 8.x); silenced `[[nodiscard]]` setup-call warnings in `test_dispatch_exchange_manager.cpp` with explicit `std::ignore =`.



### Added
- **EventLog persistence**: every `DeviceStateChange` emitted by `DispatchBus` is now persisted to `session.events`.
  - `server/db_writer.hpp`: `DomainEventRow` struct + `write_domain_event()` pure-virtual; `NullDbWriter` captures to `written_events`.
  - `server/pg_db_writer.cpp`: INSERT INTO `session.events` (session_id, event_type, event_id, timestamp_us, object_gid, payload BYTEA) using `pqxx::bytes_view`.
  - `server/dispatch_bus.cpp`: extracts 13-byte metadata header from wire frame bytes 16–28 (event_type, event_id, timestamp_us) + resolves `object_gid` via `object_gid_from_change()` C++20 requires-clauses helper covering all 14 `DeviceStateChange` variants.
  - 2 new integration tests: `WriteDomainEvent_InsertsRow`, `WriteDomainEvent_NullObjectGid`.
- **EdrCoordinator** (`server/include/server/edr_coordinator.hpp` + `server/src/edr_coordinator.cpp`): new class tracking actual train arrival/departure times in `session.edr_entries`.
  - S25 accepted → `update_edr_departure()` for the departing station.
  - S26 accepted → `update_edr_arrival()` for the destination station.
  - Direction-aware station resolution: SENT → `src_area`, RECEIVED → `dst_area`.
  - `IDbWriter` extended with `update_edr_departure()` and `update_edr_arrival()`; both use `make_interval(secs => ...)` time-of-day pattern identical to `update_edr_track_clear_time`.
  - `DispatchChannel` constructor takes `EdrCoordinator&`; `SessionServer` owns and wires it.
  - `NullDbWriter`: `edr_departures` and `edr_arrivals` capture vectors.
  - 2 new unit tests: `S25_Sent_SetsEdrDepartureForSrcArea`, `S26_Received_SetsEdrArrivalForDstArea`.
  - 3 new integration tests: `UpdateEdrDeparture_SetsActualDepartureAndStatus`, `UpdateEdrArrival_SetsActualArrivalAndStatus`, `UpdateEdrDeparture_IdempotentOnAlreadyDeparted` (second call with status=DEPARTED is a no-op).
- **Docker test environment** (`docker/docker-compose.test.yml`): ephemeral PostgreSQL 16 on port 5433 with `tmpfs` storage; no state persists between runs.
- **Integration test runner** (`scripts/run_integration_tests.sh`): launches the Docker service, waits for `pg_isready`, runs `ctest -L integration`, tears down; usable locally and in CI.
- **GitHub Actions CI** (`.github/workflows/ci.yml`): single job on `ubuntu-24.04` — bootstraps vcpkg, caches binary packages by `vcpkg.json` hash, compiles headless build, runs unit tests, applies `docker/init.sql` to a service-container PostgreSQL 16, then runs integration tests; triggers on push/PR to `main`. **332/332 tests pass (9 integration tests verified against live DB)**.

## [0.5.1] - 2026-05-23

### Added
- **DbWriter PostgreSQL pipeline**: Production `PgDbWriter : IDbWriter` using libpqxx 8.x.
  - `server/include/server/pg_db_writer.hpp` + `server/src/pg_db_writer.cpp`: connects via libpq connection string; `init_session()` inserts into `session.sessions` and returns UUID; `write_dispatch_telegram()` inserts to `session.dispatch_telegrams`; `update_edr_track_clear_time()` updates `track_clear_time` as time-of-day `INTERVAL`.
  - `server/CMakeLists.txt`: new `server_db_lib` static library links `server_lib + libpqxx::pqxx`; unit tests remain libpqxx-free.
  - `server/session_server.cpp`: `--db` CLI flag + `DB_HOST`/`DB_PORT`/`DB_USER`/`DB_PASSWORD`/`DB_NAME` env-var fallback; `DispatchChannel` wired into `SessionServer` for the first time.
  - `tests/integration/test_pg_db_writer.cpp`: 4 integration tests (auto-skipped when `SYMULATOR_TEST_DB` is not set).
  - `vcpkg.json`: added `libpqxx` dependency.
- **IDbWriter**: added `virtual std::string init_session(const std::string& display_name, int schema_version) = 0;` to the interface; `NullDbWriter` returns a deterministic UUID sentinel.

## [0.5.0] - 2026-05-23

### Refactored
- **srk_common: NAK codes**: Extracted all `COMMAND_NAK` reason codes (0x00–0x09) into a new `srk/common/include/srk/common/nak_codes.hpp`; removed triple-definition from `device_rules.cpp`, `ebilock_system.cpp`, and `ml8_system.cpp`.
- **srk_common: R8-R10 helpers**: Moved `check/execute_set_block_direction` (SHL-12), `check/execute_init_axle_counter_reset` (SLI), and `check/execute_reset_axle_counter` (SLK) from EbiLock and ML8 into `srk::common`; ~280 lines of copy-pasted code deleted.
- **srk_common: tick_switch_machines**: Updated signature to accept `pending_targets&`; now fully lands switches at the correct position at end-of-throw instead of just decrementing the counter; `on_tick()` in both systems reduced to a two-liner.
- **Tests**: Replaced `Engine` and `Server` placeholder tests with real round-trip tests — `EngineState` insert/find for `TrackSection`/`Switch` (4 tests) and `Frame` encode/decode, CRC corruption, partial-buffer (5 tests); total 313/313 passing.

### Added
- **Dispatch channel (`0x61`)**: New `msg_type 0x61 DISPATCH_CHANNEL_MESSAGE` for inter-posterunek S-form and free-text communication scoped to neighbouring `(src_area, dst_area)` pairs.
  - `proto/dispatch_channel.fbs`: `DispatchChannelMessage` with `DispatchChannelMessageKind` union (`DispatchFormPayload` | `FreeTextPayload`).
  - `proto/common.fbs`: `DispatchFormType`, `TelegramDirection`, `ExchangeStatus` enums added (reuse C++ enums from `engine::core::types.hpp`).
  - `proto/events.fbs`: `event_type 0x13 DispatchTelegramStateChanged`.
  - `server/db_writer.hpp`: `IDbWriter` interface + `NullDbWriter` test double.
  - `server/dispatch_channel.hpp` + `dispatch_channel.cpp`: parses inbound frame, drives `DispatchExchangeManager`, writes via `IDbWriter`, broadcasts to pair.
  - `server/transport_gateway`: `dispatch_area_id` in `ClientInfo`; `broadcast_to_pair()`; `set_dispatch_channel_handler()`.
  - `docker/init.sql`: `track_clear_time INTERVAL` in `session.edr_entries`; new `session.dispatch_telegrams` table.
  - 10 new unit tests (`tests/server/test_dispatch_channel.cpp`); **323/323 tests pass**.
- **Events**: Added `OperatorCommandStateChanged` (`event_type 0x11`) and `Ml8CommandStateChanged` (`event_type 0x12`) to `events.fbs` and `dispatch_bus.cpp`; both carry `g_id`, `target_kind`, `command_code`, and `active`.
- **Snapshot**: Added `OperatorCommandRuntimeState` table to `snapshot.fbs`; field `operator_state` added to `SwitchState`, `TrackSectionState`, `SignalState`, `DerailerState`, and `BlockSectionSnapshotState` so clients receive full command-flag state on reconnect without replaying events.
- **Track model**: `OperatorCommandRuntimeState` now holds `std::optional<OperatorCommandCode> active_operator_command` and `std::optional<Ml8CommandCode> active_ml8_command` — replaces the previous `bool ml8_command_active` + `std::string last_ml8_command_code` pair with type-safe optionals.
- **Command helpers**: `constexpr std::string_view operator_command_code_name(OperatorCommandCode)` and `ml8_command_code_name(Ml8CommandCode)` added to `command.hpp` (74 EbiLock codes, 71 ML8 codes).
- **DispatchExchangeManager**: New `server::DispatchExchangeManager` class (`server/include/server/dispatch_exchange_manager.hpp`) — pure-logic S-form exchange state machine per `(src_area, dst_area)` pair; supports S2/S24/S25/S26, S55/S56 (dangerous goods), and S35 (cancellation); `exchange_id` format `"exch-0000001"`.
- **E2E smoke test**: Added `scripts/e2e_smoke_test.py` — Python 3 script that connects to a running server, performs HANDSHAKE + SNAPSHOT round-trip, and validates wire format, CRC, and session_id.
- **Build**: Added `-Wall -Wextra -Wpedantic -Wno-unused-parameter` compile flags to root `CMakeLists.txt`.
- **Tests**: 17 new tests in `tests/server/test_dispatch_exchange_manager.cpp`; updated `test_state_applier.cpp` for `optional<Ml8CommandCode>`.

### Fixed
- **TopologyLoader tests**: Added `SCENARIO_DIR` compile definition to `tests/engine/CMakeLists.txt` so `test_topology_loader.cpp` finds scenario files correctly; updated all GIDs in the test to the `l202-` prefix scheme and corrected numeric values (length, maxSpeed, divergent speed).
- **Static init / SRK registration**: `symulator-server` now links `srk_ebilock` and `srk_ml8` with `$<LINK_LIBRARY:WHOLE_ARCHIVE,...>` (CMake 3.24+) so `ControlSystemRegistry::register_static()` executes before `main()`; previously the linker dropped the object files and the server crashed with `"Unknown control_system: ebilock_x4"`.
- **Fleet load**: Changed `"axleCount": null` to `"axleCount": 4` in `data/vehicle_types/dmu_unit/motor/dsb.json`; the null value caused a `FleetLoadError` crash at server startup.
- **CRC-32 thread safety**: Replaced the `static bool ready` lazy-init pattern in `server/src/frame.cpp` with an IIFE-initialised `static const` array; the old pattern was a formal C++ data race (undefined behaviour, detectable by ThreadSanitizer).
- **Macro redefinition warning**: Removed redundant `#define ASIO_STANDALONE` from `server/include/server/transport_gateway.hpp` and `server/src/transport_gateway.cpp`; the macro is already set project-wide as a CMake compile definition.

## [0.4.4] - 2026-05-22

### Added
- **ML8**: Added a dedicated `Ml8Command` protocol path (`cmd_type 0x21`), ML8 command enum, SRK execution path, state application, and parser coverage for the ML8 operator-command catalog.
- **Documentation**: Added the English ML8 system description and command catalog in `docs/19-ml8-description.md`.
- **Command Catalogs**: Added the English `data/command_types/ml8_command_types.json` catalog for ML8-only operator commands.

### Changed
- **Language**: Converted the newly added ML8 command descriptions and documentation to English.

## [0.4.3] — 2026-05-22

### Added
- **Commands**: Added the `OperatorCommand` protocol command (`cmd_type 0x20`) and a complete operator-command catalog covering signal, switch/derailer, track, axle-counter, Eac, SHL-12, and Eap procedures from the X-4-02850 command documentation.
- **Engine**: Added persistent operator-command runtime flags for signals, switches, derailers, track sections, and block sections, including stop, substitute-signal, automatic-route, clamp, traffic-closure, detection-bypass, special-procedure, and axle-counter reset states.
- **SRK**: Added shared execution support for documented operator commands in EbiLock and ML8, including command-state changes, signal stopping/substitute operations, switch/derailer plus/minus commands, axle-counter reset handling, and line-block direction/stop/special procedures.
- **Protocol**: Regenerated FlatBuffers command headers and extended the JSON command catalog with the full `OperatorCommand` schema and all supported command codes.
- **Tests**: Added coverage for `OperatorCommand` parsing, state application, and representative EbiLock operator-command execution.

### Changed
- **EbiLock**: Extended EbiLock command support to include SHL-12-style block direction and axle-counter commands in addition to the generic operator-command path.
- **ML8**: Registered the generic operator-command path alongside existing ML8 and SHL-12 commands.

Entry format:
```
## [version] — YYYY-MM-DD
### Added / Changed / Fixed / Removed
- description
```

---

## [0.4.2] — 2026-05-21

### Added
- **Documentation**: Added the comprehensive ETCS/RBC Supervisory System Specification (`docs/18-etcs-rbc-ertms-description.md`), fully formatted for GitHub rendering with detailed specifications of the RBC topology, ETCS session models, Movement Authority schemas, operational commands, train positioning/stop/deregistration protocols, and communication timeouts.

### Changed
- **Documentation**: Renamed the raw extensionless file `docs/18-etcs-rbc-ertms-description` to `.md` to ensure native GitHub Markdown rendering, and updated `docs/README.md` to index the new document.

---

## [0.4.1] — 2026-05-20

### Added
- **Developer Experience**: Added `.clangd` configuration and a helper script `scripts/generate_compile_commands.py` to generate `compile_commands.json` for editor/LSP support.

### Changed
- **Code Formatting**: Applied `clang-format` to generated FlatBuffers headers (`autogens/proto/*_generated.h`) to enforce uniform style, triggered by the newly installed formatting tools in the pre-commit hook.

### Fixed
- **Cross-Platform Portability**: Completely eliminated POSIX-specific signal handling (`<pthread.h>`, `pthread_sigmask`, `sigwait`) in `server/src/session_server.cpp`, replacing it with `asio::signal_set`. The codebase now builds natively and operates identically across **Windows (MSVC)**, **Linux (GCC/Clang)**, and **macOS (Clang)** without any platform-dependent preprocessor guards.
- **Universal Build & Environment Integration**: Verified build configurations for all three major OS targets (Windows, Linux, macOS) using the universal build validation tool (`verify_universal_build.py`), ensuring a 100% test success rate (270/270 unit and integration tests passing).

---

## [0.4.0] — 2026-05-17



### Added
- `engine/include/engine/core/train_fleet.hpp` / `engine/src/train_fleet.cpp` — `TrainFleet`: container of active `TrainSim` instances; `add_train(TrainSimState, GID from_gid)` called before `EngineLoop::start()`; `tick_all(EngineState&, tick_num, PipCallback)` called on ENGINE thread: builds `DriverInput` from topology + signal state, calls `TrainSim::tick()`, applies `apply_track_section_occupancy` on crossing, emits `PipEvent`s
- `EngineLoop::PipCallback` type alias; `add_train()` public method; `pip_cb` constructor parameter; `train_fleet_` member — ticked in `do_tick()` after `IControlSystem::on_tick()`, before snapshot publish
- `SessionServer` wires a `pip_cb` that logs all `PipEvent`s to `std::cerr` (`[PIP] section=... station=... occ=... train=...`)

### Changed
- `engine/src/engine_loop.cpp` — `do_tick()` step order: cmd drain → `on_tick` → **TrainFleet tick** → `set_current_tick` → snapshot publish → `changes_cb`
- `engine/CMakeLists.txt` — added `src/train_fleet.cpp` to the `engine` static library

---

## [0.3.20] — 2026-05-17

### Added
- `server/include/server/session_server.hpp` / `server/src/session_server.cpp` — `SessionServer`: composition root; owns all subsystems; `from_args(argc, argv)` parses CLI; `run()` blocks on SIGINT/SIGTERM, then stops; `start()` / `stop()` private
- `server/src/main.cpp` — single-responsibility entry point: `SessionServer::from_args(argc, argv).run(); return 0;`

### Changed
- `server/CMakeLists.txt` — `symulator-server` compiles `session_server.cpp`; links `srk_ebilock` + `srk_ml8` for static-init `ControlSystemRegistry` self-registration

---

## [0.3.19] — 2026-05-17

### Added
- `server/src/transport_gateway.cpp` / `server/include/server/transport_gateway.hpp` — Phase 5: TCP server with per-client async session state machine (HANDSHAKE → ACTIVE); handles snapshot chunking, command ingestion, heartbeat, and broadcast
- `server/src/dispatch_bus.cpp` / `server/include/server/dispatch_bus.hpp` — Phase 6: bridges `EngineLoop::StateChangesCallback` to wire DOMAIN_EVENT frames broadcast to all ACTIVE clients
- `tests/server/test_dispatch_bus.cpp` — unit tests for DispatchBus event serialisation (switches, signals, derailers, block sections, routes, alarms, block direction)

### Changed
- `proto/session.fbs` — renamed `PosterunekInfo` → `DispatchAreaInfo`, field `posterunek_id` → `dispatch_area_id`, vector `assigned_posterunki` → `assigned_dispatch_areas`; same rename in `SessionNotice`
- `proto/ownership.fbs` — field `posterunek_id` → `dispatch_area_id` in `TakeoverRequest` / `TakeoverResponse`
- `proto/events.fbs` — `PosterunekOwnershipChanged` → `DispatchAreaOwnershipChanged`; field `posterunek_id` → `dispatch_area_id`
- `proto/snapshot.fbs` — `PosterunekOwnership` → `DispatchAreaOwnership`; fields and vector renamed accordingly
- `engine/include/engine/core/command.hpp` — `CommandMeta::posterunek_id` → `CommandMeta::dispatch_area_id`
- `server/include/server/ownership_guard.hpp` / `server/src/ownership_guard.cpp` — all parameter names `posterunek` → `dispatch_area`
- `scenarios/reference/gdynia_orlowo/objects.json` — key `posterunek_id` → `dispatch_area_id`
- `server/CMakeLists.txt` — added `transport_gateway.cpp`, `dispatch_bus.cpp`; links `asio::asio`
- `CMakeLists.txt` (root) — asio `find_path` now also searches `3rdParty/asio/include/` for cross-platform fallback
- `scripts/configure_ninja.py` — added `ensure_asio()`: auto-downloads standalone asio 1.28.1 headers to `3rdParty/asio/include/` when not found in system paths
- `scripts/deps/install_linux.sh` — added `libasio-dev`
- `scripts/deps/install_macos.sh` — added `asio` to brew install
- `scripts/install_system_deps.py` — added `libasio-dev` (Linux) and `asio` (macOS) to print-only dependency lists

---

## [0.3.18] — 2026-05-16

### Added
- `engine/core/track_model.hpp` — value-type device structs: `TrackSection`, `Switch`, `Signal`, `Derailer`, `BlockSection`, `RouteState`, `AlarmState`, `BoundaryNode`
- `engine/core/command.hpp` — `Command` variant (10 types, cmd_type 0x01–0x0A), `Shl12Op`, `CommandMeta`, `EnvelopedCommand`
- `engine/core/state_view.hpp` — `IStateView` pure read-only interface
- `engine/core/engine_state.hpp` / `.cpp` — mutable world state owned by ENGINE thread; implements `IStateView`; `apply_*` and `insert_*` mutators
- `engine/core/engine_snapshot.hpp` / `.cpp` — immutable deep-copy snapshot; `AtomicSnapshot` for lock-free cross-thread reads
- `engine/core/control_system.hpp` — `IControlSystem` universal interface; `DeviceStateChange` variant (11 types); `InterlockingViolation`
- `engine/core/control_system_registry.hpp` / `.cpp` — singleton factory; `register_static()` for static-init self-registration
- `srk/common/` (`libsrk_common`) — shared interlocking helpers: `device_rules` (R1–R7 check/execute), `route_graph` (BFS path finder)
- `srk/ebilock/` (`libsrk_ebilock`) — `EbiLockSystem`: R1–R7, EEA-4 throw timer (90 ticks), self-registration as `"ebilock_x4"`
- `srk/ml8/` (`libsrk_ml8`) — `Ml8System`: R1–R7 + full SHL-12 state machine (R8–R10: BLW/BLP/BLO/BLZ/BLAI/BLA/OPS/SLI/SLK), self-registration as `"estw_ml8"`
- `tests/srk/test_ebilock_interlocking.cpp` — 22 tests: R1–R7, EEA-4 timer, ControlSystemRegistry
- `tests/srk/test_ml8_shl12.cpp` — 17 tests: full SHL-12 state machine
- `data/device_types/wykolejnica.json` — `DERAILER` device type (`DVT-GLB-WK-0000004`)
- `data/device_types/blok_liniowy_shl12.json` — `BLOCK_SECTION` device type (`DVT-GLB-BL-SHL12-0000005`)
- `docs/17-control-system-interface.md` — IControlSystem/IStateView/DeviceStateChange contract, ControlSystemRegistry, AtomicSnapshot, EbiLock X4, ML8 SHL-12

### Changed
- `engine/core/types.hpp` — added `ControlSystemID`, `MS1_STOP` signal aspect, `BlockDirectionState` enum and `std::hash` specialisation
- `engine/CMakeLists.txt` — added `engine_state.cpp`, `engine_snapshot.cpp`, `control_system_registry.cpp`
- `CMakeLists.txt` (root) and `tests/CMakeLists.txt` — `add_subdirectory(srk)`
- `data/device_types/device_type_catalog.json` — entries for `wykolejnica` and `blok_liniowy_shl12`
- `scenarios/reference/gdynia_orlowo/meta.json` — added `"control_system": "ebilock_x4"` field
- `scenarios/reference/gdynia_orlowo/topology.json`, `scenarios/reference/lines/250.json` — corrected station code `GDO` → `GOr` in all GIDs
- `docs/09-communication-contract.md` — commands 0x08–0x0A; NAK codes 0x01–0x09; `BlockDirectionStateChanged` event 0x10; AtomicSnapshot description
- `docs/14-interlocking-model.md` — `IControlSystem` method signature; updated NAK codes; SHL-12 rules R8–R10
- `docs/16-implementation-skeleton.md` — reflects actual implementation; remaining ENGINE integration TODO list
- `docs/README.md` — added doc 17; updated descriptions for docs 14 and 16

### Fixed
- `engine/core/engine_snapshot.hpp` — renamed `session_id` data member to `session` to resolve name conflict with `IStateView::session_id()` virtual method
- `tests/srk/test_ebilock_interlocking.cpp` — added missing `#include <engine/core/control_system_registry.hpp>`

---

## [0.3.17] — 2026-05-14

### Added
- `data/device_types/signal_templates.json` — master template containing the full suite of PKP signal aspects and descriptions (renamed from `semafor.json`)
- `data/device_types/semafor_zaporowy.json` — new signal type for barrier signals supporting S1, Sz, and Ms2 aspects
- `data/device_types/tarcza_manewrowa_tm.json` — new shunting shield type (Ms1, Ms2)
- `data/device_types/tarcza_ostrzegawcza.json` — new warning signal type (To1-To4) with green flashing aspect (To3 — expecting 100 km/h)
- `data/device_types/sygnalizator_powtarzajacy.json` — new repeating signal type (Sp1-Sp4) with white auxiliary light
- `data/device_types/semafor_m_*.json` — 15 new shunting-enabled variants (`Ms2`) for all existing semaphore configurations (2, 3, 4, and 5-chamber)

### Changed
- Refactored all semaphore configurations to use official PKP technical descriptions for aspects S1 through S13a.
- **Hardware-Enforced Signaling**: Implemented logic where speed-specific aspects require corresponding physical stripes (100 km/h → green, 60 km/h → orange).
- **Expectation Signaling**: Enabled flashing aspects (S3, S4) on all multi-chamber semaphores to allow warning about upcoming speed restrictions regardless of local stripe presence.
- Updated `S2` description to "Jazda z największą dozwoloną prędkością" and added `SZ_SUBSTITUTION` (Sygnał zastępczy, 40 km/h) to all semaphores.
- Enabled `additionalAspectAllowed` across all complex signal types to support flexible route signaling and simulation of non-organized movements

---

## [0.3.16] — 2026-05-11

### Added
- `data/device_types/semafor_3komorowy.json`, `semafor_4komorowy.json` — multi-chamber signal types for standard aspect sequences
- `data/device_types/semafor_3komorowy_pas_{zielony,pomaranczowy,zielony_pomaranczowy}.json` — 3-chamber signal variants with speed-indicating stripes
- `data/device_types/semafor_4komorowy_pas_{zielony,pomaranczowy,zielony_pomaranczowy}.json` — 4-chamber signal variants with speed-indicating stripes
- `data/device_types/semafor_5komorowy_{zielony,pomaranczowy,zielony_pomaranczowy}.json` — 5-chamber signals with complex stripe configurations (DVT-GLB-SEM-5K series)

### Changed
- `data/device_types/semafor_3komorowy_pasz.json` — renamed to `semafor_3komorowy_pas_zielony.json` for naming consistency across signal variants

---

## [0.3.15] — 2026-05-10

### Added
- `data/command_types/command_types.json` — canonical JSON command type catalog with strict per-command `payload_schema` (required fields, enums, `additionalProperties: false`)
- `data/device_types/device_type_catalog.json` — canonical JSON registry of loadable device type definitions
- `data/device_types/semafor_2komorowy.json` — dedicated 2-chamber signal type with constrained `supportedAspects`
- `docs/examples/json/commands/command_types.example.json` and `docs/examples/json/devices/device_types.example.json` — updated JSON type-catalog examples for authoring and validation

### Changed
- `data/device_types/semafor.json` — redefined as an explicit 5-chamber signal type (`headType`, `chamberCount`) with formal control capabilities (`acceptedCommandTypes`, `controlBehavior`, `supportedAspects`)
- `data/device_types/naped_eea4.json` — added explicit switch-machine behavior contract (`acceptedCommandTypes`, `stateModel`, `controlBehavior`, `safetyInterlocks`)

## [0.3.14] — 2026-05-09

### Added
- `scripts/configure_ninja.py` — unified Ninja/vcpkg bootstrap flow with a single `3rdParty/` root, host override dry-run support, and dependency install orchestration
- `scripts/install_system_deps.py` and `scripts/deps/{install_linux.sh,install_macos.sh,install_windows.ps1}` — cross-platform system dependency installation entrypoints
- `scripts/verify_universal_build.py` — dry-run validation of Linux/Windows/macOS host bootstrap/configure command paths
- `tests/qt6/test_qt6_sanity.cpp` and `tests/qt6/CMakeLists.txt` — Qt6 Core sanity coverage integrated into CTest when `Qt6::Core` is available
- `proto/{chat.fbs,ownership.fbs,voice.fbs}` and `tests/proto/test_generated_headers.cpp` — additional protocol schemas with generated-header regression tests
- `docs/16-implementation-skeleton.md` — implementation skeleton for pre-command/pre-device server wiring

### Changed
- `CMakeLists.txt` (root) — introduced `generate_proto_headers` and `symulator_proto`; FlatBuffers headers are generated into `build/generated/proto`
- `engine/CMakeLists.txt`, `client/CMakeLists.txt`, `server/CMakeLists.txt` — linked against `symulator_proto`
- `tests/CMakeLists.txt` — added `proto` tests and conditional `qt6` test subtree
- `vcpkg.json` — added `qtbase` dependency and pinned `builtin-baseline`
- `.gitignore` and `3rdParty/README.md` — switched to shared `3rdParty/` cache model
- `docs/00-contributing.md`, `docs/03-initial-architecture.md`, `docs/09-communication-contract.md`, `docs/README.md` — updated contributor workflow and architecture/contract documentation
- `proto/{commands.fbs,session.fbs,snapshot.fbs}` — extended protocol payloads and clarified enum/default/schema details

### Fixed
- `docs/00-contributing.md` — corrected stale clone/CI/project-structure instructions and repaired a broken section heading

## [0.3.13] — 2026-05-09

### Added
- `engine/include/engine/core/types.hpp` — `TractionStatus` enum (`OPERATIONAL`, `DEFECTIVE`) for traction-capable vehicle instances
- `engine/include/engine/core/fleet_registry.hpp` — new fleet fields: type-level `multiple_coupling_capable`, instance-level `traction_capable`, `traction_status`, and resolved coupling capability copy
- `tests/engine/test_fleet_registry.cpp` — regression coverage for default traction status, defective-units-as-ballast behavior, same-type coupling enabled path, unknown capability fallback, and mixed-type fallback
- `tests/engine/test_train_sim.cpp` — simulation aggregation tests for same-type coupled locomotives, unknown coupling fallback, and defective EMU motor behavior

### Changed
- `engine/src/fleet_registry.cpp` — loader/parser support for `multipleCouplingCapable` (type) and `tractionStatus` (instance); `multipleCouplingCapable` is now accepted for all traction-capable type categories (`LOCOMOTIVE`, `EMU/DMU MOTOR`), while non-traction types are rejected; consist traction aggregation applies operational-state and locomotive coupling rules (same-type + capability gate)
- `engine/src/train_sim.cpp` — `make_train_sim_state()` now mirrors consist traction rules: defective traction units contribute only ballast mass/drag; locomotive coupling applies only for same-type capability-enabled sets
- `scripts/migrate_fleet_layout.py` — migration/backfill support for explicit `multipleCouplingCapable` boolean values across all traction-capable type files (`LOCOMOTIVE`, `EMU/DMU MOTOR`) and `tractionStatus` in traction-capable vehicle files
- `scripts/generate_vehicle_types.py` and `scripts/update_vehicle_physics.py` — generation/update scripts now include `multipleCouplingCapable` handling for all traction-capable type categories
- `data/vehicle_types/locomotive/**/*.json` and `data/vehicle_types/{emu_unit,dmu_unit}/motor/**/*.json` — `multipleCouplingCapable` fully backfilled to explicit boolean values (no `null` left in traction-capable type files)
- `data/vehicles/locomotive/et22_001.json` — added `tractionStatus: OPERATIONAL`
- `docs/10-vehicle-model.md` — resolved Q-VEH-1/Q-VEH-2; documented per-motor-unit traction semantics, `tractionStatus`, and same-type coupling rule with unknown-capability fallback
- `docs/02-system-requirements.md`, `docs/11-database-model.md`, `docs/13-scenario-editor-architecture.md`, `docker/init.sql` — aligned requirements/schema/editor validation notes with new traction status and coupling capability model

### Fixed
- Consist and simulation traction computation no longer overestimates multi-locomotive traction when locomotives are defective, mixed-type, or have unknown coupling capability

## [0.3.12] — 2026-05-09

### Added
- `engine/src/fleet_registry.cpp` — full fleet loader implementation with recursive discovery of `vehicle_types`, `vehicles`, and `trains`; strict schema validation; category/folder consistency checks; derived consist metrics
- `engine/include/engine/sim/train_sim.hpp` and `engine/src/train_sim.cpp` — orchestration layer for per-train simulation ticks with DI ports (`ITrainControlPolicy`, `IPhysicsIntegrator`, `ITrainEventSink`), default adapters, section-crossing events, and dead-end handling
- `tests/engine/test_fleet_registry.cpp` — regression coverage for recursive loading, Davis defaulting, derived consist aggregation, and invalid-data rejection
- `tests/engine/test_train_sim.cpp` — orchestration tests with fake policy/integrator/sink; crossing emission and dead-end stop behavior coverage
- `scripts/migrate_fleet_layout.py` — idempotent migration script for fleet layout/schema updates (`sourceReliability` removal, train category fill, folder moves)

### Changed
- `engine/include/engine/core/fleet_registry.hpp` — extended model fields (`pkp_series`, `family`, `train_category`) and updated Davis default helper contract
- `engine/include/engine/physics/driver_ai.hpp` — advisory-aware control inputs (`next_aspect`, `distance_to_next_signal_m`), proactive warning braking, and corrected stop-signal behavior for far-ahead red aspects
- `engine/include/engine/physics/physics_model.hpp` — default initialization for `TrainPhysicsParams` to ensure correct consist speed aggregation and stable parameter construction
- `engine/CMakeLists.txt` and `tests/engine/CMakeLists.txt` — registered new engine sources and test targets
- `scripts/generate_vehicle_types.py` — removed `sourceReliability` output and aligned subtype/type generation templates
- `data/vehicle_types/**/*.json` — removed legacy `sourceReliability`; retained normalized physics-identification fields (`pkpSeries`, `family`, `davisA/B/C`)
- `data/vehicles/` and `data/trains/` — reorganized into typed/category subdirectories; train files now carry required `trainCategory`
- `docs/10-vehicle-model.md` and `docs/00-contributing.md` — updated data tree layout, schema requirements, and current physics/DriverAI description

### Fixed
- `tests/engine/test_train_sim.cpp` / `engine/include/engine/physics/physics_model.hpp` — fixed `MakeTrainSimStateAggregatesVehicles` failure (`max_speed_ms` could collapse to `0` due to zero-initialized aggregation)

## [0.3.11] — 2026-05-09

### Added
- `engine/include/engine/physics/physics_model.hpp` — header-only Newton/Davis physics model: `TrainPhysicsParams`, `TrainPhysicsState`, `PhysicsModel` (Davis resistance, braking distance, forward-Euler integration), `VehiclePhysicsContrib`, `build_train_params()`
- `engine/include/engine/physics/driver_ai.hpp` — deterministic 4-state driver automaton (STOPPED / ACCELERATING / CRUISING / BRAKING); maps all `SignalAspect` values to speed limits; pure/stateless `DriverAI::tick()`
- `tests/engine/test_physics.cpp` — 15 GTest cases for physics model and DriverAI (107 tests total, all passing)

### Changed
- `data/vehicle_types/` — reorganised from a flat directory into a typed subdirectory tree:
  - `locomotive/electric/` (81 files) — electric mainline and shunting locomotives
  - `locomotive/diesel/`  (67 files) — diesel mainline and shunting locomotives
  - `locomotive/steam/`   (21 files) — steam locomotives
  - `emu_unit/motor/`     (73 files) — electric multiple unit motor/control cars
  - `dmu_unit/motor/`     (38 files) — diesel multiple unit motor cars
  - `freight_wagon/hopper/` (1 file)
  - `service_wagon/`      (19 files) — track machines and maintenance vehicles
- `scripts/update_vehicle_physics.py` — updated to scan `vehicle_types/**/*.json` recursively
- `scripts/generate_vehicle_types.py` — updated to write new files into the correct subdirectory (`SUBDIR_MAP`); generator now sets correct `vehicleType`/`vehicleSubtype` per group
- `engine/include/engine/core/fleet_registry.hpp` — updated `load_types_` comment: uses recursive scan (`**/*.json`)
- `docs/10-vehicle-model.md` — updated File Layout section to show new subdirectory structure
- `docs/00-contributing.md` — updated `vehicle_types/` description to reflect subdirs



### Fixed
- `docs/03-initial-architecture.md`: removed incorrect claim that PIP_WRITER auto-creates `session.edr_entry` on LCS boundary crossing — PIP and EDR are independent subsystems
- `docs/11-database-model.md`: corrected `TrainCrossedLcsBoundary` action in PIP_WRITER event table — PIP_WRITER only UPSERTs `pip.track_state`, never touches `session.edr_entries`
- `engine/include/engine/core/types.hpp`: corrected `lcs_boundary_crossing` comment accordingly

---

## [0.3.9] — 2026-05-08

### Added
- `engine/include/engine/core/types.hpp`: dispatch-form vocabulary — `TrainCategory` enum (`PASSENGER | FREIGHT | MAINTENANCE`); `DispatchFormType` enum (S2/S24/S25/S26/S35/S51/S52/S55/S56/S76); `TelegramDirection` (`SENT | RECEIVED`); `TelegramStatus` (`PENDING | CONFIRMED | REJECTED | SUPERSEDED`); `ExchangeStatus` (full S-form state machine: `IDLE → S2_SENT → S24_RECEIVED → S25_SENT → S26_RECEIVED → CLOSED`; cancellation: `CANCELLED`)
- `tests/engine/test_types.cpp`: 17 new tests for `TrainCategory`, `DispatchFormType`, `TelegramDirection`, `TelegramStatus`, `ExchangeStatus` (distinct-values, all-forms-reachable, standard-path, cancellation-path); total 92/92
- `docs/15-dispatch-forms.md`: full dispatch-form specification — S-form catalogue (S2–S76), S2/S24/S25/S26 state machine diagram, cancellation path, duplicate-confirmation guard ("droga wolna" already filled), level-crossing notifications (S51/S52 + km_markers), `DispatchCoordinator` responsibilities, engine types table, open questions Q-SF-1 to Q-SF-3
- `docs/11-database-model.md`: `fleet.train_definitions` extended with `train_category`, `classification`, `supplement`, `description` columns; `session.edr_entries` extended with `track_clear_time INTERVAL` (set by S24/S56 confirmation, duplicate guarded by server); `session.dispatch_telegrams` table (form_type, exchange_id, from_sid/to_sid, direction, status, km_markers TEXT[], body, timestamp_us); retention policy entry for `dispatch_telegrams`

---

## [0.3.8] — 2026-05-08

### Added
- `engine/include/engine/core/types.hpp`: PIP vocabulary types — `EntrySide` enum (`LEFT`/`RIGHT`); `TrainSlot` struct (train number ≤6 chars, `has_extra_info`, `manually_placed`, `entry_side`; `operator==` defaulted); `PipEvent` struct (section GID, station SID, `TrackOccupancy`, `std::optional<TrainSlot>`, `lcs_boundary_crossing` flag) — produced by ENGINE, consumed exclusively by PIP_WRITER thread
- `tests/engine/test_types.cpp`: 10 tests covering `EntrySide` distinctness, `TrainSlot` default-init / field roundtrip / 6-char number / manually-placed flag / equality operator, `PipEvent` with absent slot / present slot / LCS boundary crossing / section and station IDs / extra-info flag
- `docs/03-initial-architecture.md`: PIP_WRITER as 4th fixed thread; `EventQueue<PipEvent>` in inter-thread communication map; updated total thread count (4 fixed + 2 pools = 8 at 4 vCPU); PIP_WRITER step added to shutdown sequence
- `docs/11-database-model.md`: "three schemas" decision; `pip` schema with `pip.track_state` table (JSONB `trains` array, `path_confirmed` flag), `TrainSlot` JSONB shape documented, train-number lifecycle table, column-display aggregation rule; `pip.track_state` added to retention policy

### Changed
- `engine/include/engine/core/types.hpp`: added `<optional>` and `<vector>` includes (required by `PipEvent` and future `TrackDisplayState`)
- `tests/engine/CMakeLists.txt`: register `test_types.cpp` in `tests_engine` executable

---

## [0.3.7] — 2026-05-08

### Added
- `engine/include/engine/core/types.hpp`: domain vocabulary shared across all engine modules — strong-ID wrappers (`GID`, `SID`, `DispatchAreaID`, `PlayerID`) with three-way comparison and `std::hash`; `CommandPriority` enum; `TrackOccupancy`, `SwitchPosition`, `SignalAspect` (S1–S13 + Ms2), `RandomEventType` enums
- `engine/include/engine/core/event_queue.hpp`: thread-safe MPSC FIFO queue (`EventQueue<T>`) with mutex + condition_variable; `push`, `try_pop`, `wait_and_pop`, `close`; consumer receives `nullopt` when queue is closed and drained; `push` after close throws `std::runtime_error`
- `engine/include/engine/core/priority_command_queue.hpp`: four-bucket priority MPSC queue (`PriorityCommandQueue<T>`); buckets ordered EMERGENCY → SAFETY → NORMAL → BACKGROUND; always dequeues highest non-empty bucket; FIFO preserved within each priority
- `engine/include/engine/core/event_dispatcher.hpp`: synchronous type-safe intra-thread event dispatcher (`EventDispatcher<EventT>`) — `subscribe` returns `SubscriptionToken`, `unsubscribe` by token, `publish` snapshots subscriber list under shared lock then releases before calling handlers (deadlock-safe; changes take effect next round)
- `tests/engine/test_event_queue.cpp`: parameterised — `TYPED_TEST_SUITE<int, std::string>` for 7 basic-behaviour cases; `TEST_P` MPSC suite over 4 producer/item-count configurations (1×2000, 2×1000, 4×500, 8×250); non-parameterised: close-unblocks, move-only type
- `tests/engine/test_priority_command_queue.cpp`: parameterised — `TYPED_TEST_SUITE<int, std::string>` for push/pop/close/isClosed; `TEST_P` per-priority suite (EMERGENCY/SAFETY/NORMAL/BACKGROUND) for push-at-priority and FIFO-within-bucket; `TEST_P` MPSC suite over 3 configurations (1×800, 4×200, 8×100); non-parameterised: multi-priority ordering, EMERGENCY preemption, size, close/drain, `kBucketCount == 4` guard
- `tests/engine/test_event_dispatcher.cpp`: parameterised — `TYPED_TEST_SUITE<IntEvent, StringEvent>` for delivery, noop, unsubscribe, count-tracking; `TEST_P` subscriber-count suite over {1,2,5,10,50}; `TEST_P` concurrency stress suite (subscribe+publish / unsubscribe+publish) over 3 thread configurations; non-parameterised: unknown-token noop, resubscribe, snapshot semantics

### Changed
- `CMakeLists.txt`: add `find_package(OpenSSL REQUIRED)`; add quiet fallback so `flatbuffers` package resolves under both vcpkg (`flatbuffersConfig.cmake`) and Ubuntu apt (`FlatBuffersConfig.cmake`) — both export the same `flatbuffers::flatbuffers` target
- `engine/CMakeLists.txt`: add `find_package(Threads REQUIRED)` and link `Threads::Threads`; required for `EventQueue` and `PriorityCommandQueue` threading primitives
- `tests/engine/CMakeLists.txt`: register `test_event_queue.cpp`, `test_priority_command_queue.cpp`, `test_event_dispatcher.cpp` in `tests_engine` executable

---

## [0.3.6] — 2026-05-07

### Added
- `data/device_types/semafor.json`: global signal aspect catalogue — all 14 PKP aspects (S1–S13 + Ms2) valid across all semaphore types; individual semaphore type definitions (post-MVP) will reference a subset of these aspects; `typeID: DVT-GLB-SEM-0000001`
- `docs/02-system-requirements.md`: F-023 (random events engine hook — `IRandomEventSource`, built-in event types, no engine implementation); F-024 (AI Dispatch Module — virtual operator, `IDispatchAI`, ONNX Runtime candidate, GPU inference)
- `docs/03-initial-architecture.md`: Threading model section — 3 fixed threads (ENGINE/DISPATCHER/DB_WRITER), 2 configurable Boost.Asio thread pools (IO_POOL/WORK_POOL), `PriorityCommandQueue` (4 buckets), `EventQueue<T>` with mutex+cv, `OwnershipRegistry` with `shared_mutex`, shutdown sequence
- `docs/03-initial-architecture.md`: AI Dispatch Module section — `IDispatchAI` interface, non-blocking `pollCommands` contract, framework comparison table (ONNX Runtime / LibTorch / llama.cpp), training strategy outline
- `docs/03-initial-architecture.md`: Random Events Module section — `IRandomEventSource` + `NullRandomEventSource` default, `RandomEvent` with open enum, dependency injection at server init
- `docs/03-initial-architecture.md`: dependency table extended — Boost.Asio ≥1.84, FlatBuffers ≥23.x, libpqxx ≥7.x, ONNX Runtime ≥1.18 (post-MVP)

### Changed
- `docs/02-system-requirements.md`: F-013 rewritten — posterunek-level exclusive ownership (one posterunek → exactly one player); engine rejects commands on ownership mismatch; TAKEOVER flow is the only ownership transfer path
- `docs/02-system-requirements.md`: F-019 rewritten — one player may hold multiple posterunki; `OwnershipRegistry` (`posterunek_id` → `player_id`) maintained atomically by session server

---

## [0.3.5] — 2026-05-07

### Changed
- `scenarios/reference/gdynia_orlowo/topology.json`:
  - track sections: replaced `startItID`/`endItID` with `sideA`/`sideB` (bidirectional, no implied direction); each side carries `neighborID`, `itID` or `izID`, and `signals[]`; added `occupied: false`; removed flat top-level `signals[]`
  - switches: replaced flat `trunkIzID`/`straightIzID`/`divergentIzID` fields with nested `trunk`/`straight`/`divergent` leg objects (each with `neighborID`, `izID`, `signals[]`); added `typeID` → `data/device_types/naped_eea4.json`
- `scenarios/reference/gdynia_orlowo/objects.json`: added `typeID` → `data/device_types/semafor_ksztaltowy_pkp.json` to all 6 signal instances; removed redundant `direction` field (direction is now encoded by sideA/sideB placement in topology)
- `docs/07-ebiscreen-description.md`: appended "Track section topology model" section — bidirectional sideA/sideB schema, switch leg schema, device type reference table

---

## [0.3.4] — 2026-05-07

### Changed
- `scenarios/reference/lines/250.json`: complete redesign of track section model —
  bidirectional `sideA`/`sideB` endpoints (no implied direction), each side carries
  `itID` + `signals[]`; `initial_state` replaced by `occupied: bool`;
  szlak GDO–SOP split into two sections × 1800 m;
  appended `$switch_types` illustration for STANDARD / CROSSING / SCISSORS

---

## [0.3.3] — 2026-05-07

### Added
- `scenarios/reference/lines/250.json` — block sections for line 250 (one file per line replaces flat sections.json); added `line_number` field at root
- `data/device_types/semafor_ksztaltowy_pkp.json` — signal type: PKP shape-light semaphore with aspect table
- `data/device_types/naped_eea4.json` — switch machine type: EEA-4
- `data/devices/` — new directory for individual device instances (mirrors vehicle_types/vehicles split)

### Removed
- `scenarios/reference/sections.json` — replaced by `scenarios/reference/lines/<number>.json`

---

## [0.3.2] — 2026-05-07

### Added
- `data/trains/tow54321.json` — consist Tow 54321 Gdynia–Katowice: ET22-001 + 452W-5375001 + 452W-5375002

---

## [0.3.1] — 2026-05-07

### Added
- `data/vehicle_types/201e.json` — type Pafawag 201E (PKP series ET22); fields: `typeID`, `typeName`, `pkpSeries`, 6-axle electric locomotive, 120 t, 125 km/h, 3000 kW, 411 kN
- `data/vehicle_types/452w.json` — type Wagony Świdnica 452W; 4-axle hopper wagon, 22 t tare, 90 t gross, 120 km/h
- `data/vehicles/et22_001.json` — locomotive ET22-001 (type 201E), no deviations from type
- `data/vehicles/452w_5375001.json` — wagon 452W-5375001, fully loaded (uses type default massGrossT 90 t)
- `data/vehicles/452w_5375002.json` — wagon 452W-5375002, tare only (massGrossT overridden to 22.0 t)

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
- Editor offline operation: topology authoring and manual timetable editing require no server connection; PLK schedule import requires a server connection
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
- Server internal API document (`12-server-api.md`): six C++ pure-virtual interface contracts (`ICommandHandler`, `IEventEmitter`, `ISnapshotProvider`, `ITopologyStore`, `ISessionStore`, `IEDRService`), module boundary diagram, session startup call sequence

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
