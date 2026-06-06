## ADDED Requirements

### Requirement: Browse available VehicleTypes

`VehicleTypePanel` SHALL load and display `VehicleType` records from the active data source (columns: UID, name, manufacturer, traction type, mass, length). Data loading is delegated to the application layer — `VehicleTypePanel` only displays.

#### Scenario: Load VehicleTypes from packages/

- **WHEN** the tool starts without arguments and `packages/vehicle-types/` exists
- **THEN** `VehicleTypeModel` SHALL be populated with all records from JSON files in `packages/vehicle-types/`

#### Scenario: Load VehicleTypes from directory

- **WHEN** the user provides a path via "Open Directory…" or `--data-dir <path>`
- **THEN** `DirectoryDataSource` SHALL scan recursively; `VehicleTypeModel` SHALL repopulate

#### Scenario: Sort by column

- **WHEN** the user clicks a column header in `VehicleTypePanel`
- **THEN** `QSortFilterProxyModel` SHALL sort ascending/descending by that column

### Requirement: Browse available Vehicles

`VehiclePanel` SHALL display `Vehicle` records (columns: UID, VehicleType UID, side number, carrier), filtered by the currently selected VehicleType.

#### Scenario: Filter by VehicleType

- **WHEN** the user selects a VehicleType in `VehicleTypePanel`
- **THEN** `VehicleModel` SHALL display only vehicles belonging to that VehicleType

#### Scenario: Show all Vehicles

- **WHEN** no VehicleType is selected
- **THEN** `VehicleModel` SHALL display all available vehicles

### Requirement: Create a new Vehicle

`VehicleEditDialog` SHALL provide a form for creating a new `Vehicle` (fields: side number, carrier UID, inventory number, optional notes). UID proposal and collision validation are handled by `UidGeneratorService` and `UidValidator` from `tools/shared/`.

#### Scenario: New Vehicle pre-filled with VehicleType

- **WHEN** the user selects a VehicleType and clicks "New Vehicle…"
- **THEN** `VehicleEditDialog` SHALL open with the VehicleType UID pre-populated

#### Scenario: Hard UID collision guard on Vehicle creation

- **WHEN** `VehicleEditDialog` proposes a UID for the new Vehicle
- **THEN** `UidValidator::isAvailable()` SHALL be called against `UidRegistry`
- **THEN** if the proposed UID collides, `UidGeneratorService` SHALL automatically select the next free INSTANCE
- **THEN** the dialog SHALL never present a colliding UID to the user

#### Scenario: Save Vehicle to JSON

- **WHEN** the user completes the form and clicks "Save"
- **THEN** the tool SHALL write a JSON file conforming to the `packages/vehicles/` schema to the selected output directory

### Requirement: Compose a Train from Vehicles

`TrainBuilderPanel` SHALL allow the user to compose a `Train` by adding Vehicles from `VehiclePanel` and reordering them via drag-and-drop.

#### Scenario: Add Vehicle to Train

- **WHEN** the user selects a Vehicle and clicks "Add to Train"
- **THEN** `TrainModel` SHALL append the Vehicle to the consist list

#### Scenario: Reorder consist by drag-and-drop

- **WHEN** the user drags a Vehicle within `TrainBuilderPanel`
- **THEN** `TrainModel` SHALL update the ordering to match the drop position

#### Scenario: Hard UID collision guard on Train save

- **WHEN** the user clicks "Save Train"
- **THEN** `UidValidator::isAvailable()` SHALL be called for the proposed Train UID (`Kind=TRAIN_CONSIST`)
- **THEN** if the UID collides, `UidGeneratorService` SHALL find the next free INSTANCE before saving
- **THEN** the saved JSON SHALL contain a UID not present in the loaded registry

### Requirement: Data source selection

The tool SHALL support two data source modes selectable via UI or CLI argument.

#### Scenario: Default packages/ source

- **WHEN** the tool starts without `--data-dir`
- **THEN** `PackagesDataSource` SHALL load data from `packages/` relative to the working directory

#### Scenario: CLI directory argument

- **WHEN** the tool starts with `--data-dir /path/to/symulator-data/data`
- **THEN** `DirectoryDataSource` SHALL load data from the given path

#### Scenario: Runtime source change

- **WHEN** the user clicks "Open Directory…" and selects a directory
- **THEN** all models SHALL reload from the new source without restarting the application

### Requirement: In-app UID legend panel

`UidLegendPanel` (shared with uid-generator) SHALL be accessible from the menu, independent of any loaded data source.

#### Scenario: Legend accessible before data load

- **WHEN** the tool starts and no data source is loaded
- **THEN** `UidLegendPanel` SHALL display the full bit layout and Domain/Kind table

### Requirement: SOLID module boundaries

The vehicle-browser tool SHALL be decomposed with each class having a single responsibility:

| Class | Responsibility |
|---|---|
| `MainWindow` | Shell: menu bar, toolbar, data source switching |
| `VehicleTypePanel` | Displays and manages VehicleType selection |
| `VehiclePanel` | Displays Vehicles filtered by selected VehicleType |
| `TrainBuilderPanel` | Manages Train consist composition |
| `VehicleEditDialog` | Form for creating a new Vehicle |
| `VehicleTypeModel` | `QAbstractTableModel` for VehicleType data |
| `VehicleModel` | `QAbstractTableModel` for Vehicle data |
| `TrainModel` | `QAbstractListModel` for Train consist |
| `UidLegendPanel` | Displays UID bit layout and Domain/Kind reference (shared) |
| `UidGeneratorService` | Generates next available UID (shared) |
| `UidValidator` | Validates UID availability (shared) |
| `UidRegistry` | Stores loaded UIDs (shared) |

#### Scenario: MainWindow has no model or data-loading logic

- **WHEN** `MainWindow` is instantiated
- **THEN** it SHALL delegate all data loading to the application layer and all display to panel classes — no JSON parsing or UID logic in `MainWindow`

### Requirement: Dynamic Qt6 linking

The tool SHALL be compiled with dynamic Qt6::Widgets linkage.

#### Scenario: No static Qt copy in the binary

- **WHEN** the tool is built with `cmake -DBUILD_TOOLS=ON`
- **THEN** the executable SHALL link Qt6 dynamically
