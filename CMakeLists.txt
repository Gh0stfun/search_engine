cmake_minimum_required(VERSION 3.20)
project(search_engine VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Добавляем подпроекты
add_subdirectory(src)
add_subdirectory(tests)