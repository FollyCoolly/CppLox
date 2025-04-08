cmake_minimum_required(VERSION 3.10)
project(cpplox)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(cpplox
    main.cpp
    chunk.cpp
    memory.cpp
    debug.cpp
    value.cpp
)

target_include_directories(cpplox PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}) 