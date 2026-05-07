# Contributing Guide

## Overview

This document describes how to set up a development environment and how to commit changes to the repository.

**Releases and distribution builds are managed exclusively by the project owner.** Contributors do not create tags, do not push to release branches, and do not run the full cross-platform matrix locally — that is the CI pipeline's job.

---

## Repository layout (build-relevant)

```
/
  CMakeLists.txt          ← root; drives all sub-components
  vcpkg.json              ← dependency manifest
  vcpkg/                  ← vcpkg submodule (bootstrapped on first configure)
  engine/                 ← Core Simulation Engine (Linux server, no Qt)
  server/                 ← Session Server (Linux server, no Qt)
  libtrackview/           ← shared rendering library (Qt6 Widgets)
  client/                 ← Operator client (Qt6 Widgets)
  editor/                 ← Scenario Editor (Qt6 Widgets + Qt6 Sql)
  tests/                  ← CTest-based test suite
  docs/
```

---

## Prerequisites

### Linux (primary development platform)

Install the following via your package manager. Example for Debian/Ubuntu:

```bash
sudo apt update
sudo apt install -y \
  git cmake ninja-build \
  gcc-14 g++-14 \
  libgl1-mesa-dev \       # required by Qt6 on Linux
  pkg-config zip curl unzip tar   # required by vcpkg
```

Verify versions:

```bash
cmake --version   # must be ≥ 3.25
ninja --version   # any recent version is fine
g++ --version     # must support C++20 (GCC ≥ 12 or Clang ≥ 15)
```

Qt 6 and all other libraries are fetched and built automatically by vcpkg — **do not install Qt via apt or the Qt installer**.

### Windows (contributor setup)

Required installs (all free):

| Tool | Where to get |
|---|---|
| Git | https://git-scm.com/download/win |
| CMake ≥ 3.25 | https://cmake.org/download/ |
| Ninja | `winget install Ninja-build.Ninja` or bundled with Visual Studio |
| Visual Studio 2022 Build Tools | https://visualstudio.microsoft.com/downloads/ → Build Tools; select **Desktop development with C++** workload |

Ninja is included with Visual Studio 2022 and is available in the "Developer Command Prompt". If using a plain terminal, install separately or add the Visual Studio tools to `PATH`.

Qt 6 and all other libraries are fetched and built automatically by vcpkg — **do not install Qt via the Qt installer**.

---

## Getting the source

```bash
git clone --recurse-submodules https://github.com/kamiltrzcinski/symulator.git
cd symulator
```

If you forgot `--recurse-submodules`:

```bash
git submodule update --init --recursive
```

---

## Configure and build

### Linux

```bash
cmake -G Ninja \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake \
      -B build

cmake --build build
```

On the first run vcpkg will download and compile Qt6 and other dependencies. This takes **30–60 minutes**. Subsequent builds use the cache and take seconds.

### Windows (Developer Command Prompt for VS 2022)

```bat
cmake -G Ninja ^
      -DCMAKE_BUILD_TYPE=Debug ^
      -DCMAKE_TOOLCHAIN_FILE=vcpkg\scripts\buildsystems\vcpkg.cmake ^
      -B build

cmake --build build
```

The commands are identical; only the path separator differs.

---

## Running tests

```bash
cd build
ctest --output-on-failure
```

All tests must pass before opening a pull request.

---

## Commit conventions

### Message format

```
type(scope): short imperative description

Optional body — explain *why*, not *what*. Wrap at 72 characters.
```

**type** values:

| Type | When to use |
|---|---|
| `feat` | New feature or behaviour |
| `fix` | Bug fix |
| `docs` | Documentation only |
| `refactor` | Code restructuring, no behaviour change |
| `test` | Adding or updating tests |
| `chore` | Build system, CI, tooling, dependency updates |
| `perf` | Performance improvement |

**scope** is optional; use the component name when helpful: `engine`, `server`, `client`, `editor`, `libtrackview`, `linter`, `db`.

Examples:

```
feat(engine): implement axle counter occupancy tracking
fix(client): handle TCP reconnect on server restart
docs: add libtrackview architecture to doc 13
chore(ci): cache vcpkg ports in GitHub Actions
```

### CHANGELOG.md requirement

A pre-commit hook enforces that **every commit stages a CHANGELOG.md update**. Before committing:

1. Open `CHANGELOG.md`.
2. Add a new version block at the top, or append to the current `[Unreleased]` block if one exists.
3. Use `### Added`, `### Changed`, `### Fixed`, or `### Removed` subsections.
4. `git add CHANGELOG.md` before running `git commit`.

Version numbers follow **SemVer**: `MAJOR.MINOR.PATCH`. Contributors increment `PATCH` for fixes and `MINOR` for new features. The project owner sets `MAJOR` and creates release tags.

### Branch and pull request policy

- Work on a feature branch: `git checkout -b feat/short-description`.
- Open a pull request targeting `main`.
- CI must be green (all three platform builds + tests) before merge.
- Squash merge is preferred to keep `main` history linear.
- Do not push directly to `main`.

---

## What contributors should NOT do

- Do not create or push version tags (`v*`). Releases are managed by the project owner.
- Do not run `cmake --install` or packaging steps locally — that is the CI pipeline's job.
- Do not commit generated files (`build/`, `*.scendb` project files, `*.o`, `moc_*`). The `.gitignore` covers these.
