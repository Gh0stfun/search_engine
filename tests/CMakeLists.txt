if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

if(WIN32)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

FetchContent_MakeAvailable(googletest)

enable_testing()

add_library(search_engine_lib STATIC
        ${CMAKE_SOURCE_DIR}/src/converter_json.cpp
        ${CMAKE_SOURCE_DIR}/src/inverted_index.cpp
        ${CMAKE_SOURCE_DIR}/src/search_server.cpp
)

target_include_directories(search_engine_lib PRIVATE ${CMAKE_SOURCE_DIR}/include)

add_executable(search_engine_tests
        inverted_index_tests.cpp
        search_server_tests.cpp
)

target_include_directories(search_engine_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${gtest_SOURCE_DIR}/include
)

target_link_libraries(search_engine_tests PRIVATE
        search_engine_lib
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(search_engine_tests)