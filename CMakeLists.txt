cmake_minimum_required(VERSION 3.25)

# ── Version — single source of truth is CHANGELOG.md ───────────────────────
# Look for the first line matching "## [X.Y.Z]"
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/CHANGELOG.md" _cl_lines LIMIT_COUNT 30)
foreach(_cl_line IN LISTS _cl_lines)
    if(_cl_line MATCHES "^## \\[([0-9]+\\.[0-9]+\\.[0-9]+)\\]")
        set(SYMULATOR_VERSION "${CMAKE_MATCH_1}")
        break()
    endif()
endforeach()
if(NOT SYMULATOR_VERSION)
    message(FATAL_ERROR
        "Cannot determine project version.\n"
        "Add a '## [X.Y.Z] — YYYY-MM-DD' entry to CHANGELOG.md first.")
endif()

project(symulator VERSION "${SYMULATOR_VERSION}" LANGUAGES CXX)

# ── Git hooks — automatic setup ─────────────────────────────────────────────
# Point git at the tracked hooks directory (scripts/git-hooks/).
# This runs every time cmake is configured, so the setting is always in place
# without any manual step from the contributor.
# GUI clients (VS Code, GitHub Desktop, TortoiseGit) that call the system git
# binary will also respect core.hooksPath.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
    execute_process(
        COMMAND git config core.hooksPath scripts/git-hooks
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        RESULT_VARIABLE _hooks_result
        OUTPUT_QUIET
        ERROR_QUIET
    )
    if(_hooks_result EQUAL 0)
        message(STATUS "Git hooks configured: scripts/git-hooks/")
    else()
        message(WARNING
            "Could not set git hooks path automatically.\n"
            "Run once manually: git config core.hooksPath scripts/git-hooks")
    endif()
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── vcpkg toolchain is passed via -DCMAKE_TOOLCHAIN_FILE ───────────────────
# cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug
#       -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake
#       -B build

# ── Dependencies ────────────────────────────────────────────────────────────
find_package(nlohmann_json CONFIG REQUIRED)
find_package(flatbuffers CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)

# Qt is only required for client-side components; skip on a headless server build.
option(BUILD_CLIENT    "Build operator client"  ON)
option(BUILD_EDITOR    "Build scenario editor"  ON)

if(BUILD_CLIENT OR BUILD_EDITOR)
    find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets Network)
    qt_standard_project_setup()
endif()
if(BUILD_EDITOR)
    find_package(Qt6 REQUIRED COMPONENTS Sql)
endif()

# ── Sub-components ──────────────────────────────────────────────────────────
add_subdirectory(engine)
add_subdirectory(server)

if(BUILD_CLIENT OR BUILD_EDITOR)
    add_subdirectory(libtrackview)
endif()
if(BUILD_CLIENT)
    add_subdirectory(client)
endif()
if(BUILD_EDITOR)
    add_subdirectory(editor)
endif()

# ── Tests ───────────────────────────────────────────────────────────────────
enable_testing()
add_subdirectory(tests)
