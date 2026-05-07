cmake_minimum_required(VERSION 3.25)
project(symulator VERSION 0.1.0 LANGUAGES CXX)

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
