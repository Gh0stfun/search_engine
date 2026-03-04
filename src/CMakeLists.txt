set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_executable(search_engine
        main.cpp
        converter_json.cpp
        inverted_index.cpp
        search_server.cpp
)

target_include_directories(search_engine PRIVATE ../include)