cmake_minimum_required(VERSION 3.5.0) # set minimum version for CMake runtime.
project("cmo-enclave")                # set the project name
enable_language(C ASM)

# set output directories.
# set() command is used to assign variables.
# the following commands will set where to store the building binaries.
# here, the CMAKE_* variables are internal variables used by CMake.
# the `${}` syntax is used to get the value of the corresponding variable.
# please refer to CMake document for the purpose of each variables.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_TEST_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/test)

# this is the directory for our custom CMake modules.
# list(APPEND x y) command is used to append value y to the list x.
# the following command tell cmake where to find our custom modules.
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
# include() command is used to load custom modules.
include(FileUtils)

# enable c++11.
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 11)

# build src
# include_directories(path) command will cause -Ipath being passed
# to the compiler.
include_directories("${PROJECT_SOURCE_DIR}/include")

# glob_cpp_source_file is a custom function defined in FileUtils.cmake
# the following command will glob all cpp source files in src/lib directory
# and store the results to cmo_src variable.
glob_cpp_source_file(cmo_src PATH src/lib)
# build cmo library
add_library(cmo ${cmo_src})
# linked_libraries is a custom variable to keep track of the list of libraries,
# which will be linked by the test cases.
list(APPEND linked_libraries cmo)

glob_cpp_source_file(algo_src PATH src/algo)
add_library(algo ${algo_src})
# the algo library should be linked with cmo library
target_link_libraries(algo cmo)
list(APPEND linked_libraries algo)

# add unit test
enable_testing()
# load CMakeLists.txt in the test directory.
add_subdirectory(test)
