add_executable(symulator-server
    src/main.cpp
)

target_include_directories(symulator-server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(symulator-server PRIVATE
    engine
    flatbuffers::flatbuffers
    OpenSSL::SSL
    OpenSSL::Crypto
)

target_compile_features(symulator-server PRIVATE cxx_std_20)
