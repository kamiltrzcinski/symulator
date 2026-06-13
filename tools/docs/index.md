# C++/Qt6 Developer Tools

The optional developer tools provide a graphical UID generator and a browser/editor
for rolling-stock package data. Both applications use the UID definitions from
`engine/include/engine/core/types.hpp` and link to Qt6 dynamically.

## Build

Prerequisites:

- a C++20 compiler;
- CMake and Ninja or another supported generator;
- Qt6 with the Widgets and Test components available as dynamic libraries;
- the project dependencies from `vcpkg.json`.

Configure and build:

```sh
cmake -S . -B build -DBUILD_TOOLS=ON
cmake --build build --target uid-generator vehicle-browser
```

The exact executable location depends on the selected CMake generator.

## Quick Start

Use package data from `packages/`:

```sh
uid-generator
vehicle-browser
```

Use an unpacked data checkout or another JSON directory:

```sh
uid-generator --data-dir /path/to/symulator-data/data
vehicle-browser --data-dir /path/to/symulator-data/data
```

See:

- [UID legend](uid-legend.md)
- [UID generator guide](uid-generator.md)
- [Vehicle browser guide](vehicle-browser.md)
