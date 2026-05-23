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

# ── Compiler warnings ───────────────────────────────────────────────────────
# -Wno-unused-parameter: virtual overrides routinely ignore some parameters.
add_compile_options(-Wall -Wextra -Wpedantic -Wno-unused-parameter)

# ── vcpkg toolchain is passed via -DCMAKE_TOOLCHAIN_FILE ───────────────────
# cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug
#       -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake
#       -B build

# ── Dependencies ────────────────────────────────────────────────────────────
find_package(nlohmann_json CONFIG REQUIRED)

# flatbuffers: vcpkg installs as "flatbuffers", Ubuntu/Debian apt as "FlatBuffers".
# Both export the same target (flatbuffers::flatbuffers); try vcpkg name first.
find_package(flatbuffers CONFIG QUIET)
if(NOT flatbuffers_FOUND)
    find_package(FlatBuffers CONFIG REQUIRED)
endif()

# ── Protocol schemas (FlatBuffers) ─────────────────────────────────────────
set(SYMULATOR_PROTO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/proto")
set(SYMULATOR_PROTO_SCHEMAS
    "${SYMULATOR_PROTO_DIR}/common.fbs"
    "${SYMULATOR_PROTO_DIR}/session.fbs"
    "${SYMULATOR_PROTO_DIR}/commands.fbs"
    "${SYMULATOR_PROTO_DIR}/events.fbs"
    "${SYMULATOR_PROTO_DIR}/snapshot.fbs"
    "${SYMULATOR_PROTO_DIR}/ownership.fbs"
    "${SYMULATOR_PROTO_DIR}/chat.fbs"
    "${SYMULATOR_PROTO_DIR}/voice.fbs"
    "${SYMULATOR_PROTO_DIR}/bilateral.fbs"
)

# Generated FlatBuffers headers live in the source tree so they are committed,
# reviewed in PRs, and available without running flatc on first checkout.
# Re-run scripts/autogens/generate_proto_headers.sh after editing any .fbs file.
set(SYMULATOR_PROTO_GENERATED_DIR "${CMAKE_SOURCE_DIR}/autogens/proto")
set(SYMULATOR_PROTO_GENERATED_HEADERS
    "${SYMULATOR_PROTO_GENERATED_DIR}/common_generated.h"
    "${SYMULATOR_PROTO_GENERATED_DIR}/session_generated.h"
    "${SYMULATOR_PROTO_GENERATED_DIR}/commands_generated.h"
    "${SYMULATOR_PROTO_GENERATED_DIR}/events_generated.h"
    "${SYMULATOR_PROTO_GENERATED_DIR}/snapshot_generated.h"
    "${SYMULATOR_PROTO_GENERATED_DIR}/ownership_generated.h"
    "${SYMULATOR_PROTO_GENERATED_DIR}/chat_generated.h"
    "${SYMULATOR_PROTO_GENERATED_DIR}/voice_generated.h"
    "${SYMULATOR_PROTO_GENERATED_DIR}/bilateral_generated.h"
)

if(TARGET flatbuffers::flatc)
    set(SYMULATOR_FLATC_EXECUTABLE "$<TARGET_FILE:flatbuffers::flatc>")
elseif(DEFINED FLATBUFFERS_FLATC_EXECUTABLE AND FLATBUFFERS_FLATC_EXECUTABLE)
    set(SYMULATOR_FLATC_EXECUTABLE "${FLATBUFFERS_FLATC_EXECUTABLE}")
else()
    find_program(SYMULATOR_FLATC_EXECUTABLE NAMES flatc REQUIRED)
endif()

add_custom_command(
    OUTPUT ${SYMULATOR_PROTO_GENERATED_HEADERS}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${SYMULATOR_PROTO_GENERATED_DIR}"
    COMMAND ${SYMULATOR_FLATC_EXECUTABLE}
            --cpp
            --gen-object-api
            -I "${SYMULATOR_PROTO_DIR}"
            -o "${SYMULATOR_PROTO_GENERATED_DIR}"
            ${SYMULATOR_PROTO_SCHEMAS}
    DEPENDS ${SYMULATOR_PROTO_SCHEMAS}
    COMMENT "Generating FlatBuffers headers from proto/*.fbs"
    VERBATIM
)

add_custom_target(generate_proto_headers DEPENDS ${SYMULATOR_PROTO_GENERATED_HEADERS})

set(SYMULATOR_PROTO_VALIDATE_DIR "${CMAKE_BINARY_DIR}/generated/proto-validate")
add_custom_target(validate_proto_schemas
    COMMAND ${CMAKE_COMMAND} -E make_directory "${SYMULATOR_PROTO_VALIDATE_DIR}"
    COMMAND ${SYMULATOR_FLATC_EXECUTABLE}
            --cpp
            --gen-object-api
            -I "${SYMULATOR_PROTO_DIR}"
            -o "${SYMULATOR_PROTO_VALIDATE_DIR}"
            ${SYMULATOR_PROTO_SCHEMAS}
    DEPENDS ${SYMULATOR_PROTO_SCHEMAS}
    COMMENT "Validating FlatBuffers schemas in proto/*.fbs"
    VERBATIM
)

add_library(symulator_proto INTERFACE)
target_include_directories(symulator_proto INTERFACE "${SYMULATOR_PROTO_GENERATED_DIR}")
target_link_libraries(symulator_proto INTERFACE flatbuffers::flatbuffers)
add_dependencies(symulator_proto generate_proto_headers)

find_package(GTest CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)

# asio: try vcpkg first, fall back to system libasio-dev or 3rdParty/asio
find_package(asio CONFIG QUIET)
if(NOT asio_FOUND)
    find_path(ASIO_INCLUDE_DIR NAMES asio.hpp REQUIRED
        PATHS
            /usr/include
            /usr/local/include
            "${CMAKE_SOURCE_DIR}/3rdParty/asio/include")
    add_library(asio::asio INTERFACE IMPORTED GLOBAL)
    set_target_properties(asio::asio PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${ASIO_INCLUDE_DIR}"
        INTERFACE_COMPILE_DEFINITIONS "ASIO_STANDALONE")
    message(STATUS "Found system standalone asio: ${ASIO_INCLUDE_DIR}")
endif()

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
add_subdirectory(srk)
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
