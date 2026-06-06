## ADDED Requirements

### Requirement: Generate UID by selecting Domain / Kind / SCOPE / INSTANCE

The tool SHALL allow the user to select `UIDDomain`, `UIDKind`, `SCOPE` (uint16), and `INSTANCE` (uint16) through dedicated UI controls (`UidForm`) and display the result in `UidResultView` in both decimal and hexadecimal format.

Available Domain/Kind combinations MUST correspond exclusively to the enumerations in `engine/include/engine/core/types.hpp`. The Kind combo box SHALL be filtered to values belonging to the selected Domain.

Generation is delegated to `UidGeneratorService`, which validates against `UidRegistry` via `UidValidator` before returning. `UidForm` and `UidResultView` SHALL never receive a colliding UID.

#### Scenario: Domain filters Kind combo box

- **WHEN** the user selects `UIDDomain::ROLLING_STOCK`
- **THEN** the Kind combo box SHALL contain only: `VEHICLE_TYPE`, `VEHICLE`, `TRAIN_CONSIST`, `CARRIER`

#### Scenario: Successful UID generation

- **WHEN** the user fills Domain, Kind, SCOPE, INSTANCE and clicks "Generate"
- **THEN** `UidGeneratorService` SHALL validate the candidate against `UidRegistry`
- **THEN** `UidResultView` SHALL display the UID in decimal and hex with `0x` prefix

#### Scenario: Hard collision guard — candidate UID already in registry

- **WHEN** the candidate UID already exists in the loaded registry
- **THEN** `UidGeneratorService` SHALL increment INSTANCE and retry automatically
- **THEN** if no free INSTANCE exists in the SCOPE, the UI SHALL display: "No available INSTANCE in this SCOPE — all 65534 slots are taken"
- **THEN** the Generate button SHALL be disabled until the user changes SCOPE or Kind

#### Scenario: INSTANCE = 0 rejected

- **WHEN** the user sets INSTANCE to 0
- **THEN** the tool SHALL display "INSTANCE must be in range 0x0001–0xFFFF" and disable Generate

#### Scenario: SCOPE = 0 advisory for ROLLING_STOCK

- **WHEN** the selected Domain is `ROLLING_STOCK` and SCOPE = 0
- **THEN** the tool SHALL display a non-blocking advisory: "SCOPE=0 means global — ensure this is intentional"

### Requirement: Copy generated UID to clipboard

`UidClipboardService` SHALL copy the generated UID decimal value to the system clipboard when the user activates the Copy action. This class has no other responsibility.

#### Scenario: Copy UID

- **WHEN** the user clicks "Copy" after a UID has been generated
- **THEN** `UidClipboardService` SHALL place the decimal UID string in the system clipboard via `QClipboard`

### Requirement: Display loaded UID registry

`UidRegistryView` SHALL display all UIDs loaded from the active data source in a table (columns: UID hex, Domain, Kind, SCOPE, INSTANCE, source file). It has no responsibility for generation or validation.

#### Scenario: Load from packages/

- **WHEN** the tool starts without arguments and `packages/` exists
- **THEN** `UidRegistry` SHALL be populated from all JSON files in `packages/vehicles/`, `packages/trains/`, `packages/vehicle-types/`
- **THEN** `UidRegistryView` SHALL display the loaded entries

#### Scenario: Load from directory

- **WHEN** the user provides a path via "Open Directory…" or `--data-dir <path>`
- **THEN** `DirectoryDataSource` SHALL scan recursively for JSON files
- **THEN** `UidRegistry` SHALL be repopulated and `UidRegistryView` SHALL refresh

### Requirement: In-app UID legend panel

`UidLegendPanel` SHALL display the full UID bit layout and a complete Domain/Kind reference table, available immediately on startup without any data source loaded.

#### Scenario: Legend accessible before data load

- **WHEN** the tool starts and no data source is loaded
- **THEN** `UidLegendPanel` SHALL display the bit layout diagram and the full Domain/Kind table

### Requirement: SOLID module boundaries

The uid-generator tool SHALL be decomposed with each class having a single responsibility:

| Class | Responsibility |
|---|---|
| `UidForm` | Captures Domain / Kind / SCOPE / INSTANCE input |
| `UidResultView` | Displays generated UID and Copy button |
| `UidRegistryView` | Displays loaded UID table |
| `UidLegendPanel` | Displays UID bit layout and Domain/Kind reference |
| `UidGeneratorService` | Generates next available UID, delegates validation |
| `UidValidator` | Checks whether a candidate UID is free in the registry |
| `UidRegistry` | Stores loaded UIDs, provides lookup by value |
| `UidClipboardService` | Copies text to system clipboard |
| `IDataSource` | Interface for data access |
| `PackagesDataSource` | Loads data from `packages/` |
| `DirectoryDataSource` | Loads data from a user-supplied directory |
| `JsonLoader` | Parses JSON files into domain structs |

#### Scenario: UidGeneratorService has no UI dependency

- **WHEN** `UidGeneratorService::generate()` is called
- **THEN** it SHALL interact only with `UidValidator` and `UidRegistry` — no Qt widget types in its interface

#### Scenario: UidValidator has no knowledge of data sources

- **WHEN** `UidValidator::isAvailable(uid)` is called
- **THEN** it SHALL query only `UidRegistry` — it SHALL NOT open files or call `IDataSource`

### Requirement: Unit tests

The shared service and registry classes SHALL be covered by unit tests in `tools/shared/tests/` using Qt Test, registered with CTest.

#### Scenario: UidRegistry rejects duplicate insert

- **WHEN** `UidRegistry::insert(uid, file)` is called with a UID already present
- **THEN** `contains(uid)` SHALL return true and no second entry SHALL be created

#### Scenario: UidValidator returns correct availability

- **WHEN** `UidValidator::isAvailable(uid)` is called
- **THEN** it SHALL return false if `uid` is in the registry, true otherwise

#### Scenario: UidGeneratorService increments on collision

- **WHEN** `UidGeneratorService::generate(domain, kind, scope)` is called and the first candidate collides
- **THEN** INSTANCE SHALL be incremented and the next free UID returned

#### Scenario: UidGeneratorService throws when SCOPE is full

- **WHEN** all 65534 INSTANCE values are occupied for the given SCOPE
- **THEN** `UidGeneratorService::generate()` SHALL throw `UidExhaustedException`

#### Scenario: JsonLoader maps fields correctly

- **WHEN** `JsonLoader` parses a minimal JSON fixture for `VehicleType`, `Vehicle`, and `Train`
- **THEN** each returned struct SHALL contain the expected field values with no data loss

### Requirement: Dynamic Qt6 linking

The tool SHALL be compiled with dynamic Qt6::Widgets linkage.

#### Scenario: No static Qt copy in the binary

- **WHEN** the tool is built with `cmake -DBUILD_TOOLS=ON`
- **THEN** the executable SHALL link Qt6 dynamically (no `_STATIC` cmake target)
