## ADDED Requirements

### Requirement: In-app UID legend panel

Both `uid-generator` and `vehicle-browser` SHALL include a `UidLegendPanel` widget that displays the full UID bit layout and a complete reference table of all `UIDDomain` / `UIDKind` values with their hex codes and SCOPE semantics, derived from `engine/include/engine/core/types.hpp`.

The legend SHALL be populated at compile time from a constexpr table in `tools/shared/domain/uid_legend_table.hpp`. A `static_assert` SHALL verify that every value in `UIDDomain` and `UIDKind` is covered by the table — if a new enum value is added to `types.hpp` without updating the table, compilation SHALL fail.

#### Scenario: Bit layout display

- **WHEN** the user opens the UID Legend panel (menu or tab)
- **THEN** `UidLegendPanel` SHALL display the 64-bit field diagram:
  ```
  63      48 47    40 39    32 31           16 15            0
  ┌──────────┬────────┬────────┬──────────────┬────────────────┐
  │ reserved │ DOMAIN │  KIND  │    SCOPE     │    INSTANCE    │
  │  16 bits │ 8 bits │ 8 bits │   16 bits    │    16 bits     │
  └──────────┴────────┴────────┴──────────────┴────────────────┘
  ```

#### Scenario: Domain/Kind reference table

- **WHEN** the user views the legend
- **THEN** the panel SHALL display a table with columns: Domain name, Domain hex, Kind name, Kind hex, SCOPE semantics
- **THEN** the table SHALL cover all values from `UIDDomain` and `UIDKind` as defined in `types.hpp`

#### Scenario: Static completeness check at build time

- **WHEN** a new value is added to `UIDKind` in `types.hpp` and `uid_legend_table.hpp` is not updated
- **THEN** compilation of `tools/` SHALL fail with a `static_assert` error identifying the missing entry

### Requirement: Legend accessible without data source

The `UidLegendPanel` SHALL be available immediately on application startup, independent of whether a data source has been loaded.

#### Scenario: Legend visible before data load

- **WHEN** the tool starts and no data source is loaded yet
- **THEN** the UID Legend panel SHALL still display the full bit layout and reference table
