# Tests Guide

This document is the source of truth for test structure, execution, and maintenance in this repository.

## 1. Scope and Goals

Primary goals:

- Keep unit tests fast, deterministic, and domain-focused.
- Keep each test binary small enough for quick local debug.
- Reuse fixtures/builders/helpers instead of repeating setup logic.
- Run tests by logical groups in local workflows and CI.
- Keep test naming and parameterization style consistent across modules.

## 2. Test Types

- Unit tests: single module or tightly related module set.
- Integration tests (non-DB): cross-module behavior without database I/O.
- Integration tests (DB): behavior that requires real PostgreSQL writes/reads.

## 3. Directory Layout

- `tests/engine`: engine unit tests.
- `tests/server`: server unit tests.
- `tests/srk`: SRK unit tests.
- `tests/proto`: protocol/schema tests.
- `tests/libscenario_validation`: scenario validation tests.
- `tests/qt6`: Qt6-specific tests (only when Qt6 is available).
- `tests/integration`: integration tests split into DB and non-DB targets.
- `tests/common`: shared test helpers/fixtures/builders.

## 4. Build and Run

Build (recommended local directory):

```bash
cmake --build build/ninja-debug-headless -j8
```

Run grouped profiles:

```bash
python3 scripts/run_grouped_ctest.py --test-dir build/ninja-debug-headless --profile pre-db
python3 scripts/run_grouped_ctest.py --test-dir build/ninja-debug-headless --profile db
python3 scripts/run_grouped_ctest.py --test-dir build/ninja-debug-headless --profile all
```

Run one logical group:

```bash
python3 scripts/run_grouped_ctest.py --test-dir build/ninja-debug-headless --group unit-engine
python3 scripts/run_grouped_ctest.py --test-dir build/ninja-debug-headless --group unit-server
```

Run by CTest label directly:

```bash
ctest --test-dir build/ninja-debug-headless -L '^group:unit-engine-concurrency$' --output-on-failure
```

Run a single test by name regex:

```bash
ctest --test-dir build/ninja-debug-headless -R 'FrameRoundTripCases/FrameRoundTripTest.PreservesFieldsAndPayload/Handshake' --output-on-failure
```

## 5. Grouping Model

Each test executable is discovered with exactly one CTest label (`group:*`) via `symulator_discover_gtest(...)`.

Logical groups in `scripts/run_grouped_ctest.py`:

- `unit-engine` -> `group:unit-engine-foundation|concurrency|state|snapshot|fleet|simulation`
- `unit-server` -> `group:unit-server-protocol|group:unit-server-dispatch`
- `unit-srk` -> `group:unit-srk-ebilock|ml8|common`
- `unit-proto` -> `group:unit-proto`
- `unit-validation` -> `group:unit-validation`
- `unit-qt6` -> `group:unit-qt6`
- `integration-non-db` -> `group:integration-non-db`
- `integration-db` -> `group:integration-db`

Profiles:

- `pre-db`: all unit groups + `integration-non-db`
- `db`: `integration-db`
- `all`: everything

## 6. Full Target Matrix (Current)

### Engine

- `tests_engine_foundation` (`group:unit-engine-foundation`)
  - `tests/engine/test_placeholder.cpp`
  - `tests/engine/test_uid_codec.cpp`
  - `tests/engine/test_types.cpp`
- `tests_engine_concurrency` (`group:unit-engine-concurrency`)
  - `tests/engine/test_event_queue.cpp`
  - `tests/engine/test_priority_command_queue.cpp`
  - `tests/engine/test_event_dispatcher.cpp`
- `tests_engine_state` (`group:unit-engine-state`)
  - `tests/engine/test_state_applier.cpp`
  - `tests/engine/test_topology_loader.cpp`
- `tests_engine_snapshot` (`group:unit-engine-snapshot`)
  - `tests/engine/test_snapshot_service.cpp`
- `tests_engine_fleet` (`group:unit-engine-fleet`)
  - `tests/engine/test_fleet_registry_loading.cpp`
  - `tests/engine/test_fleet_registry_coupling.cpp`
  - `tests/engine/test_fleet_registry_carriers.cpp`
- `tests_engine_simulation` (`group:unit-engine-simulation`)
  - `tests/engine/test_physics.cpp`
  - `tests/engine/test_train_sim.cpp`
  - `tests/engine/test_train_fleet.cpp`
  - `tests/engine/test_engine_loop.cpp`

### Server

- `tests_server_protocol` (`group:unit-server-protocol`)
  - `tests/server/test_placeholder.cpp`
  - `tests/server/test_frame.cpp`
  - `tests/server/test_command_ingress.cpp`
  - `tests/server/test_ownership_guard.cpp`
- `tests_server_dispatch` (`group:unit-server-dispatch`)
  - `tests/server/test_dispatch_bus.cpp`
  - `tests/server/test_dispatch_exchange_manager.cpp`
  - `tests/server/test_dispatch_coordinator.cpp`
  - `tests/server/test_dispatch_channel.cpp`
  - `tests/server/test_pip_writer.cpp`

### SRK

- `tests_srk_ebilock` (`group:unit-srk-ebilock`)
  - `tests/srk/test_ebilock_interlocking.cpp`
- `tests_srk_ml8` (`group:unit-srk-ml8`)
  - `tests/srk/test_ml8_shl12.cpp`
- `tests_srk_common` (`group:unit-srk-common`)
  - `tests/srk/test_srk_common_route_graph.cpp`
  - `tests/srk/test_srk_common_device_rules.cpp`

### Integration

- `tests_integration_non_db` (`group:integration-non-db`)
  - `tests/integration/test_placeholder.cpp`
  - `tests/integration/test_uid_registry.cpp`
- `tests_integration_db` (`group:integration-db`)
  - `tests/integration/test_pg_db_writer.cpp`
  - `tests/integration/test_edr_coordinator.cpp`
  - `tests/integration/test_pip_track_state.cpp`

### Other

- `tests_proto` (`group:unit-proto`)
  - `tests/proto/test_generated_headers.cpp`
- `tests_scenario_validation` (`group:unit-validation`)
  - `tests/libscenario_validation/test_layer1.cpp`
  - `tests/libscenario_validation/test_layer2.cpp`
- `tests_qt6` (`group:unit-qt6`, optional)
  - `tests/qt6/test_qt6_sanity.cpp`

## 7. Shared Test Infrastructure

`tests/common` provides reusable helpers through `tests_common` (INTERFACE target):

- `tests/common/include/tests/common/db_integration_fixture.hpp`
  - Base fixture for DB integration tests.
  - Reads `SYMULATOR_TEST_DB` in `SetUp()`.
  - Skips tests when env var is missing.
  - Cleans inserted session rows in `TearDown()`.
- `tests/common/include/tests/common/srk_state_builders.hpp`
  - Reusable SRK state/build helpers.
- `tests/common/include/tests/common/file_test_helpers.hpp`
  - Temporary directory and file-writing utilities for filesystem-based tests.
- `tests/common/include/tests/common/fleet_test_fixture.hpp`
  - FleetRegistry-oriented fixture with data root setup helpers.
- `tests/common/include/tests/common/fleet_test_uids.hpp`
  - Shared FleetRegistry UID constants.
- `tests/common/include/tests/common/param_test_helpers.hpp`
  - Standard named-case generator for `INSTANTIATE_TEST_SUITE_P`.

## 8. New Test Model (Required Style)

### 8.1 Fixture-first

- If setup is repeated in 2+ tests, extract to fixture.
- If setup is repeated across files/domains, extract to `tests/common`.
- Keep tests focused on behavior, not setup ceremony.

### 8.2 Named parameter cases

- Use explicit case structs with `name` as first field.
- Prefer `TEST_P` + `INSTANTIATE_TEST_SUITE_P(..., tests::common::param_name<CaseType>)`.
- Avoid anonymous loops that hide failing case identity.

Recommended pattern:

```cpp
struct MyCase {
    const char* name;
    int input;
    int expected;
};

class MyParamTest : public ::testing::TestWithParam<MyCase> {};

TEST_P(MyParamTest, Works) {
    const auto p = GetParam();
    EXPECT_EQ(run(p.input), p.expected);
}

INSTANTIATE_TEST_SUITE_P(
    MyCases,
    MyParamTest,
    ::testing::Values(
        MyCase{"Small", 1, 2},
        MyCase{"Large", 9, 10}),
    tests::common::param_name<MyCase>);
```

### 8.3 Split by behavior domain

- Keep binaries aligned with domain boundaries from section 6.
- Do not append unrelated files to a target just to balance file counts.
- If a target grows too broad or slow, split by behavior domain and assign a new label.

### 8.4 Determinism

- No dependency on wall-clock timing for correctness.
- No random behavior without fixed seed.
- Avoid hidden global state coupling between tests.

## 9. Placement Rules

- Engine logic/state/queue/simulation -> `tests/engine`
- SRK rules/interlocking/ML8 -> `tests/srk`
- Server protocol/dispatch/session -> `tests/server`
- FlatBuffers/proto contracts -> `tests/proto`
- Scenario validation rules -> `tests/libscenario_validation`
- Qt adapters -> `tests/qt6`
- Cross-module without DB -> `tests/integration` non-DB target
- Cross-module with DB I/O -> `tests/integration` DB target + `DbIntegrationFixture`

## 10. Checklists

### 10.1 Add/Change Test Checklist

- [ ] Test file is placed in the correct domain directory.
- [ ] Correct target in `tests/*/CMakeLists.txt` includes the file.
- [ ] Target has exactly one `group:*` label via `symulator_discover_gtest`.
- [ ] Repeated setup is extracted into a fixture.
- [ ] Cross-file setup duplication is extracted to `tests/common`.
- [ ] Parameter matrices use named case structs (`name` field first).
- [ ] Parameterized suites use `tests::common::param_name<CaseType>`.
- [ ] Assertions are specific and diagnose failures clearly.
- [ ] Test is deterministic (no flaky timing assumptions).
- [ ] If DB test: based on `DbIntegrationFixture` and cleans up correctly.
- [ ] If filesystem test: uses temporary directory/file helpers.

### 10.2 Refactor-to-New-Model Checklist

- [ ] Monolithic test files are split by behavior when needed.
- [ ] Duplicate literals/builders/constants are extracted.
- [ ] Ad-hoc case loops are converted to explicit `TEST_P` suites where useful.
- [ ] Legacy parameter suffix lambdas are replaced with `param_test_helpers.hpp`.
- [ ] Test names reflect behavior and state transitions.
- [ ] Updated tests still map to the same functional coverage.

### 10.3 Local Validation Checklist

- [ ] Build succeeds: `cmake --build build/ninja-debug-headless -j8`.
- [ ] Affected unit group(s) pass via grouped runner or label-filtered CTest.
- [ ] If touching DB paths, DB group/profile is executed.
- [ ] No unintended label/group drift in `scripts/run_grouped_ctest.py`.
- [ ] JUnit reports can still be generated (`--output-junit` path valid).

## 11. CI Contract

CI runs grouped executions and exports JUnit XML per logical group.

- Pre-DB stage: `--profile pre-db`
- DB stage: `--profile db`
- Typical report output: `test-reports/*.xml`

Any target/group split in CMake must be mirrored in `scripts/run_grouped_ctest.py` (`GROUP_REGEX` and, if needed, `PROFILE_GROUPS`).
