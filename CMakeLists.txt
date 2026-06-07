cmake_minimum_required(VERSION 3.16)
project(esp32-ambient-matrix-lamp CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(effects-core)
add_subdirectory(preview)
add_subdirectory(tests)
