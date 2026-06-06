## Context

The project uses C++20 and Qt6. Developer tools were previously Python. JSON data comes from two sources: `packages/` (extracted tarballs) or a raw `symulator-data` checkout. Qt is already a project dependency — dynamic linking under GPL-2.0-or-later is required for open-source Qt usage.

UIDs have a fixed bit layout defined in `engine/include/engine/core/types.hpp` (`make_uid`, `UIDDomain`, `UIDKind`). Tools must use the same enumerations — include, never copy.

## Goals / Non-Goals

**Goals:**
- Rewrite both tools in C++20/Qt6 Widgets, SOLID and Clean Code throughout
- Hard UID collision guard: `UidGeneratorService` never returns a UID that already exists in the registry
- In-app UID legend panel derived from `types.hpp` at compile time
- Dynamic Qt linking, GPL-2.0-or-later license
- Full user documentation in `tools/docs/`
- Optional build via `-DBUILD_TOOLS=ON`

**Non-Goals:**
- CI/CD for tools (separate task)
- Infrastructure editor (stations, tracks)
- Qt static linking

## Decisions

### 1. SOLID module structure

Each class has exactly one responsibility. Dependencies flow inward: data ← domain ← application ← UI.

```
tools/
  COPYING
  CMakeLists.txt                      ← BUILD_TOOLS option
  docs/
    index.md
    uid-legend.md
    uid-generator.md
    vehicle-browser.md
  shared/
    CMakeLists.txt                    ← static lib: tools_shared
    domain/
      uid_types.hpp                   ← re-exports UIDDomain, UIDKind, make_uid
      uid_legend_table.hpp            ← constexpr table + static_assert completeness
    data/
      i_data_source.hpp               ← IDataSource interface (SRP: data contract)
      packages_data_source.hpp/.cpp   ← reads packages/
      directory_data_source.hpp/.cpp  ← reads arbitrary directory recursively
      json_loader.hpp/.cpp            ← JSON → VehicleType, Vehicle, Train (SRP: parsing)
    registry/
      uid_registry.hpp/.cpp           ← stores loaded UIDs, lookup by value (SRP: registry)
      uid_validator.hpp/.cpp          ← isAvailable(UID) against registry (SRP: validation)
    services/
      uid_generator_service.hpp/.cpp  ← generates next free UID, throws on exhaustion (SRP)
      clipboard_service.hpp/.cpp      ← wraps QClipboard (SRP: clipboard)
    ui/
      uid_legend_panel.hpp/.cpp       ← QWidget: bit layout + Domain/Kind table (SRP: legend)
  uid-generator/
    CMakeLists.txt
    main.cpp
    ui/
      main_window.hpp/.cpp            ← QMainWindow shell, menus, tab layout
      uid_form.hpp/.cpp               ← Domain/Kind/SCOPE/INSTANCE inputs (SRP: form)
      uid_result_view.hpp/.cpp        ← displays UID decimal+hex, Copy button (SRP: display)
      uid_registry_view.hpp/.cpp      ← QTableView of loaded UIDs (SRP: registry display)
  vehicle-browser/
    CMakeLists.txt
    main.cpp
    models/
      vehicle_type_model.hpp/.cpp     ← QAbstractTableModel for VehicleType (SRP: model)
      vehicle_model.hpp/.cpp          ← QAbstractTableModel for Vehicle (SRP: model)
      train_model.hpp/.cpp            ← QAbstractListModel for Train consist (SRP: model)
    ui/
      main_window.hpp/.cpp            ← QMainWindow shell, menus, data source switching
      vehicle_type_panel.hpp/.cpp     ← VehicleType table + selection (SRP: VehicleType view)
      vehicle_panel.hpp/.cpp          ← Vehicle table + filter (SRP: Vehicle view)
      train_builder_panel.hpp/.cpp    ← Train composition, drag-and-drop (SRP: Train assembly)
      vehicle_edit_dialog.hpp/.cpp    ← new Vehicle form (SRP: Vehicle creation)
```

### 2. Hard UID collision guard

**Decision**: `UidGeneratorService::generate()` calls `UidValidator::isAvailable()` before returning. If the candidate collides, it increments INSTANCE and retries up to 65534 times. If no free INSTANCE is found it throws `UidExhaustedException`. The UI layer never receives a colliding UID.

**Why hard guard, not soft warning**: Warnings are ignored. The guard belongs in the service layer — independent of UI, reusable by vehicle-browser.

**Alternative rejected**: UI-layer check only — violates SRP and leaves the service unsafe.

### 3. In-app UID legend — compile-time derivation

**Decision**: `UidLegendPanel` reads from a `constexpr` table in `uid_legend_table.hpp` that maps every `UIDDomain`+`UIDKind` to display name, hex value, and SCOPE semantics. A `static_assert` verifies completeness — adding a new enum value without updating the table breaks the build.

**Why compile-time**: `types.hpp` is the single source of truth. If it changes, the legend recompiles automatically. Runtime file reading would require deployment and could go stale.

### 4. Open/Closed for data sources

**Decision**: `IDataSource` is a pure abstract interface. Adding a new source requires only a new class — no changes to `UidRegistry`, models, or UI.

### 5. Dynamic Qt linking

**Decision**: `find_package(Qt6 REQUIRED COMPONENTS Widgets)` without `STATIC`. Qt provided by system package manager or Qt Online Installer on Windows.

**Rationale**: Static Qt requires a commercial licence or full Qt source redistribution. Dynamic + GPL complies with Qt open-source terms.

## Risks / Trade-offs

- **`static_assert` legend gap** → If a `UIDKind` is added to `types.hpp` and the table is not updated, compilation fails. This is intentional — it is a feature, not a risk.
- **Qt6 on Windows** → Requires Qt Online Installer or vcpkg `qt6-base`+`qt6-widgets`. Already in project vcpkg — check if entries exist; add if missing.
- **GPL on tools** → Acceptable — tools are developer-only, not shipped as product. Engine stays on its own licence.
- **`UidExhaustedException` on dense SCOPE** → UI shows a clear error with remaining slot count.
