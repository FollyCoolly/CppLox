cmake_minimum_required(VERSION 3.10)
project(cpplox)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-stdlib=libc++)

add_executable(cpplox
    main.cpp
    chunk.cpp
    debug.cpp
    vm.cpp
    compiler.cpp
    scanner.cpp
    parser.cpp
    value.cpp
)

target_include_directories(cpplox PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(cpplox PRIVATE c++ c++abi) 