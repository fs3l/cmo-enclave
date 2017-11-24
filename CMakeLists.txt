cmake_minimum_required(VERSION 3.5.0)
project("cmo-enclave")

# set output directories.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_TEST_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/test)

# this is the directory for our custom CMake modules.
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(FileUtils)

# enable c++11.
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 11)

# build src
include_directories("${PROJECT_SOURCE_DIR}/include")

glob_cpp_source_file(cmo_src PATH src/lib)
add_library(cmo ${cmo_src})
list(APPEND linked_libraries cmo)

glob_cpp_source_file(algo_src PATH src/algo)
add_library(algo ${algo_src})

# add unit test
enable_testing()
add_subdirectory(test)
