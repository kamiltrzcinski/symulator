## ADDED Requirements

### Requirement: User documentation in tools/docs/

The repository SHALL contain user-facing documentation in `tools/docs/` (Markdown) covering both tools. The documentation SHALL be self-contained — no external links required to understand normal usage.

#### Scenario: Documentation structure present

- **WHEN** a developer checks out the repository with `BUILD_TOOLS=ON`
- **THEN** `tools/docs/` SHALL contain at minimum:
  - `index.md` — overview and quick-start for both tools
  - `uid-legend.md` — full UID bit layout and Domain/Kind/SCOPE reference
  - `uid-generator.md` — complete uid-generator usage guide
  - `vehicle-browser.md` — complete vehicle-browser usage guide

### Requirement: uid-legend.md covers full UID specification

`tools/docs/uid-legend.md` SHALL document the complete UID bit layout, all `UIDDomain` values, all `UIDKind` values with their Domain groupings, SCOPE semantics per Kind, INSTANCE constraints, JSON safety limit, and the `make_uid()` function signature. It SHALL serve as a standalone reference identical in content to `docs/21-uid-legend.md` but oriented toward tool users rather than engine developers.

#### Scenario: All Domain/Kind values documented

- **WHEN** a user reads `tools/docs/uid-legend.md`
- **THEN** they SHALL find a table containing every `UIDDomain` and `UIDKind` value from `types.hpp` with hex code, description, and SCOPE meaning

#### Scenario: JSON safety limit documented

- **WHEN** a user reads `tools/docs/uid-legend.md`
- **THEN** they SHALL find the statement that all UIDs must be ≤ `UID_MAX_SAFE_JSON_INTEGER` (2^53 − 1) to be safely representable as JSON numbers

### Requirement: uid-generator.md covers complete usage

`tools/docs/uid-generator.md` SHALL document: installation prerequisites (Qt6 dynamic libraries), CLI invocation, data source modes (`packages/` vs `--data-dir`), step-by-step UID generation workflow, collision guard behaviour, clipboard copy, and the in-app legend panel.

#### Scenario: CLI reference documented

- **WHEN** a user reads `uid-generator.md`
- **THEN** they SHALL find the full list of CLI arguments: `--data-dir <path>`, `--help`

#### Scenario: Collision guard behaviour documented

- **WHEN** a user reads `uid-generator.md`
- **THEN** they SHALL find an explanation that the tool automatically finds the next free INSTANCE and will error if the SCOPE is exhausted

### Requirement: vehicle-browser.md covers complete usage

`tools/docs/vehicle-browser.md` SHALL document: installation prerequisites, CLI invocation, data source modes, browsing VehicleTypes and Vehicles, creating a new Vehicle (form fields, UID auto-proposal), composing a Train (add, reorder, save), JSON output format, and the in-app legend panel.

#### Scenario: Data source modes documented

- **WHEN** a user reads `vehicle-browser.md`
- **THEN** they SHALL find instructions for both modes: running against `packages/` and running against a `symulator-data` checkout with `--data-dir`

#### Scenario: JSON output format documented

- **WHEN** a user reads `vehicle-browser.md`
- **THEN** they SHALL find the expected JSON schema for saved Vehicle and Train files, with field descriptions
