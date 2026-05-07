add_library(engine STATIC
    src/placeholder.cpp
)

target_include_directories(engine PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(engine PUBLIC
    nlohmann_json::nlohmann_json
    flatbuffers::flatbuffers
)

target_compile_features(engine PUBLIC cxx_std_20)
