## 1. Survey existing tools and data schemas

- [ ] 1.1 Read `tools/uid-generator/` (Python) — note all features and UI flows to replicate in C++; these files are **reference only** and will be fully removed in task 5.1
- [ ] 1.2 Read `tools/vehicle-browser/` (Python) — note all features and UI flows to replicate in C++; these files are **reference only** and will be fully removed in task 5.1
- [ ] 1.3 Read `engine/include/engine/core/types.hpp` — catalogue all `UIDDomain` and `UIDKind` enumerations with numeric values for `uid_legend_table.hpp`
- [ ] 1.4 Read JSON files in `packages/vehicle-types/`, `packages/vehicles/`, `packages/trains/` — note field names and types needed by `JsonLoader`

## 2. Shared layer — domain types and UID infrastructure

- [ ] 2.1 Create `tools/shared/domain/uid_types.hpp` — re-exports `UIDDomain`, `UIDKind`, `make_uid` from `engine/include/engine/core/types.hpp`
- [ ] 2.2 Create `tools/shared/domain/uid_legend_table.hpp` — constexpr table mapping every `UIDDomain`+`UIDKind` to display name, hex, and SCOPE semantics; add `static_assert` for completeness
- [ ] 2.3 Create `tools/shared/registry/uid_registry.hpp/.cpp` — stores loaded UIDs, provides `contains(UID)` and `insert(UID, sourceFile)` (SRP: registry only)
- [ ] 2.4 Create `tools/shared/registry/uid_validator.hpp/.cpp` — `isAvailable(UID)` queries `UidRegistry` (SRP: validation only, no file I/O)
- [ ] 2.5 Create `tools/shared/services/uid_generator_service.hpp/.cpp` — `generate(domain, kind, scope)` retries until `UidValidator::isAvailable()` returns true or throws `UidExhaustedException` (SRP: generation only)
- [ ] 2.6 Create `tools/shared/services/clipboard_service.hpp/.cpp` — wraps `QClipboard::setText()` (SRP: clipboard only)

## 3. Shared layer — data access

- [ ] 3.1 Create `tools/shared/data/i_data_source.hpp` — pure abstract interface: `loadVehicleTypes()`, `loadVehicles()`, `loadTrains()`
- [ ] 3.2 Create `tools/shared/data/json_loader.hpp/.cpp` — parses JSON → domain structs `VehicleType`, `Vehicle`, `Train` (SRP: parsing only)
- [ ] 3.3 Create `tools/shared/data/packages_data_source.hpp/.cpp` — implements `IDataSource`, reads from `packages/`
- [ ] 3.4 Create `tools/shared/data/directory_data_source.hpp/.cpp` — implements `IDataSource`, reads recursively from a given path
- [ ] 3.5 Create `tools/shared/CMakeLists.txt` — static library `tools_shared` linking Qt6::Widgets and engine include path

## 4. Shared layer — UID legend widget

- [ ] 4.1 Create `tools/shared/ui/uid_legend_panel.hpp/.cpp` — `QWidget` displaying bit layout diagram and Domain/Kind table from `uid_legend_table.hpp` (SRP: legend display only)

## 5. Project scaffolding

- [ ] 5.1 Remove Python files from `tools/uid-generator/` and `tools/vehicle-browser/`
- [ ] 5.2 Create `tools/COPYING` with GPL-2.0-or-later full text
- [ ] 5.3 Create `tools/CMakeLists.txt` with `BUILD_TOOLS` option, add subdirectories for shared, uid-generator, vehicle-browser
- [ ] 5.4 Wire `tools/CMakeLists.txt` into root `CMakeLists.txt` under `if(BUILD_TOOLS)`

## 6. uid-generator — CMake and skeleton

- [ ] 6.1 Create `tools/uid-generator/CMakeLists.txt` — target links `tools_shared`, Qt6::Widgets (dynamic)
- [ ] 6.2 Create `tools/uid-generator/main.cpp` — `QApplication` init, parse `--data-dir`, construct `MainWindow`

## 7. uid-generator — UI components (one class per file)

- [ ] 7.1 Create `tools/uid-generator/ui/main_window.hpp/.cpp` — `QMainWindow` shell: menu ("File", "Help/UID Legend"), tab widget hosting `UidForm`+`UidResultView`, `UidRegistryView`, `UidLegendPanel`
- [ ] 7.2 Create `tools/uid-generator/ui/uid_form.hpp/.cpp` — Domain/Kind combos, SCOPE/INSTANCE spinboxes, Generate button; emits `generateRequested(domain, kind, scope, instance)` signal
- [ ] 7.3 Create `tools/uid-generator/ui/uid_result_view.hpp/.cpp` — displays decimal + hex UID, Copy button; calls `UidClipboardService` on copy
- [ ] 7.4 Create `tools/uid-generator/ui/uid_registry_view.hpp/.cpp` — `QTableView` backed by `UidRegistry` (read-only display, no logic)
- [ ] 7.5 Wire `UidForm::generateRequested` → `UidGeneratorService::generate()` → `UidResultView::showUid()` in `MainWindow`

## 8. vehicle-browser — CMake and skeleton

- [ ] 8.1 Create `tools/vehicle-browser/CMakeLists.txt` — target links `tools_shared`, Qt6::Widgets (dynamic)
- [ ] 8.2 Create `tools/vehicle-browser/main.cpp` — `QApplication` init, parse `--data-dir`, construct `MainWindow`

## 9. vehicle-browser — models (one class per file)

- [ ] 9.1 Create `tools/vehicle-browser/models/vehicle_type_model.hpp/.cpp` — `QAbstractTableModel` for `VehicleType` (SRP: model only)
- [ ] 9.2 Create `tools/vehicle-browser/models/vehicle_model.hpp/.cpp` — `QAbstractTableModel` for `Vehicle` with `setFilterVehicleType()` (SRP: model only)
- [ ] 9.3 Create `tools/vehicle-browser/models/train_model.hpp/.cpp` — `QAbstractListModel` for Train consist with drag-and-drop support (SRP: model only)

## 10. vehicle-browser — UI components (one class per file)

- [ ] 10.1 Create `tools/vehicle-browser/ui/main_window.hpp/.cpp` — shell: menu, toolbar, data source switching; no model or loading logic
- [ ] 10.2 Create `tools/vehicle-browser/ui/vehicle_type_panel.hpp/.cpp` — `QTableView` + `QSortFilterProxyModel` for VehicleType; emits `vehicleTypeSelected` signal
- [ ] 10.3 Create `tools/vehicle-browser/ui/vehicle_panel.hpp/.cpp` — `QTableView` for Vehicle, connected to `vehicleTypeSelected`; emits `addToTrainRequested` signal
- [ ] 10.4 Create `tools/vehicle-browser/ui/train_builder_panel.hpp/.cpp` — `QListView` with drag-and-drop reordering, Save Train button
- [ ] 10.5 Create `tools/vehicle-browser/ui/vehicle_edit_dialog.hpp/.cpp` — new Vehicle form; calls `UidGeneratorService` for UID proposal; calls `UidValidator` before enabling Save

## 11. User documentation

- [ ] 11.1 Create `tools/docs/uid-legend.md` — full UID bit layout, all Domain/Kind/SCOPE values from `types.hpp`, JSON safety limit, `make_uid()` reference
- [ ] 11.2 Create `tools/docs/uid-generator.md` — installation, CLI args, data source modes, step-by-step workflow, collision guard behaviour, clipboard, legend panel
- [ ] 11.3 Create `tools/docs/vehicle-browser.md` — installation, CLI args, data source modes, browse/filter, create Vehicle, compose Train, JSON output format, legend panel
- [ ] 11.4 Create `tools/docs/index.md` — overview, quick-start for both tools, link to uid-legend.md

## 12. Verification

- [ ] 12.1 Build both tools locally with `cmake -DBUILD_TOOLS=ON` and verify dynamic Qt linkage
- [ ] 12.2 Verify `static_assert` in `uid_legend_table.hpp` fires when a `UIDKind` entry is removed from the table
- [ ] 12.3 Run uid-generator: generate a UID that already exists in the registry — verify it is never presented to the user
- [ ] 12.4 Run vehicle-browser: create a Vehicle with a colliding UID — verify it is auto-incremented before the dialog closes
- [ ] 12.5 Run both tools with `--data-dir /path/to/symulator-data/data` and verify data loads correctly
- [ ] 12.6 Verify `UidLegendPanel` displays before any data source is loaded
- [ ] 12.7 Update `CHANGELOG.md`
