find_package(Threads REQUIRED)

add_library(engine STATIC
    src/placeholder.cpp
    src/fleet_registry.cpp
    src/train_sim.cpp
)

target_include_directories(engine PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(engine PUBLIC
    nlohmann_json::nlohmann_json
    flatbuffers::flatbuffers
    Threads::Threads
)

target_compile_features(engine PUBLIC cxx_std_20)
