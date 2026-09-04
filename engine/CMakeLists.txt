find_package(Threads REQUIRED)

add_library(engine STATIC
    src/placeholder.cpp
    src/fleet_registry.cpp
    src/train_sim.cpp
    src/train_fleet.cpp
    src/engine_state.cpp
    src/engine_snapshot.cpp
    src/control_system_registry.cpp
    src/topology_loader.cpp
    src/engine_loop.cpp
    src/snapshot_service.cpp
    src/spawn_resolver.cpp
    src/timetable/timetable_catalog_loader.cpp
)

target_include_directories(engine PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(engine PUBLIC
    nlohmann_json::nlohmann_json
    symulator_proto
    flatbuffers::flatbuffers
    Threads::Threads
)

target_compile_features(engine PUBLIC cxx_std_20)
