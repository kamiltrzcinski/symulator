## Why

Developer tools (`uid-generator`, `vehicle-browser`) are written in Python, which has no place in this project's C++/Qt6 stack. Rewriting them in C++/Qt6 removes the external language dependency, enables dynamic Qt linking (smaller distribution packages), and opens the path to a full Vehicle/Train editor in `vehicle-browser`.

## What Changes

- Remove `tools/uid-generator/` (Python) — replace with C++/Qt6 implementation
- Remove `tools/vehicle-browser/` (Python) — replace with C++/Qt6 implementation
- Both tools link Qt dynamically (no static Qt copy in the binary)
- Both tools are covered by GPL-2.0-or-later (`tools/COPYING`), required by Qt open-source terms
- Both tools support two data source modes:
  - **packed** — from `packages/` (extracted by `fetch_packages.py`)
  - **directory** — directly from any JSON directory (for developers working in `symulator-data`)
- Both tools include an in-app **UID Legend** panel derived at compile time from `engine/include/engine/core/types.hpp`
- Full user documentation added in `tools/docs/`
- CI/CD for tools — separate task, out of scope here

## Capabilities

### New Capabilities

- `uid-generator-cpp`: Qt6 Widgets tool for generating valid UIDs conforming to the bit layout in `docs/21-uid-legend.md` and enum values in `engine/include/engine/core/types.hpp`. Includes hard collision guard, clipboard copy, registry view, and in-app UID legend.
- `vehicle-browser-cpp`: Qt6 Widgets tool for browsing, creating `Vehicle` (from `VehicleType`), and composing `Train` from Vehicles. Hard UID collision guard on save. Supports both data source modes.
- `uid-legend`: In-app panel rendering the full UID bit layout and Domain/Kind/SCOPE reference table, derived from `types.hpp` at compile time via a constexpr table with `static_assert` completeness check.
- `user-documentation`: Static docs in `tools/docs/` covering installation, data source modes, uid-generator workflow, vehicle-browser workflow, and the UID legend.

### Modified Capabilities

## Impact

- `tools/` — Python files removed, new C++/CMake structure
- `tools/CMakeLists.txt` — new optional `BUILD_TOOLS` targets
- `tools/COPYING` — GPL-2.0-or-later
- `tools/docs/` — new user documentation directory
- Dependency: Qt6 Widgets (dynamic link), already present in the project
- Input data: `packages/` or any directory of JSON files
