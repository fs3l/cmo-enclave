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

# whether to enable sgx
option(ENABLE_SGX "Enable SGX" OFF)

# this is the directory for our custom CMake modules.
# list(APPEND x y) command is used to append value y to the list x.
# the following command tell cmake where to find our custom modules.
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
# include() command is used to load custom modules.
include(FileUtils)

if(ENABLE_SGX)
  # sgx config
  set(SGX_SDK /opt/intel/sgxsdk)
  set(SGX_MODE HW CACHE STRING "SGX mode: SIM or HW")
  set(SGX_BUILD DEBUG CACHE STRING "SGX build: DEBUG, PRERELEASE OR RELEASE")
  include(SGXConfig)
endif()

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

glob_cpp_source_file(algo_src PATH src/algo)
add_library(algo ${algo_src})

glob_cpp_source_file(pao_src PATH src/pao_lib)
add_library(pao ${pao_src})
glob_cpp_source_file(pao_algo_src PATH src/pao_algo)
add_library(pao_algo ${pao_algo_src})

# the algo library should be linked with cmo library
# there is only one target library
target_link_libraries(pao_algo algo cmo pao)

if(ENABLE_SGX)
  # build enclave version of algo library
  add_sgx_enclave_library(algo_enclave
    EDL_FILE sgx/edl/algo.edl
    LDS_FILE sgx/lds/algo.lds
    PRIVATE_KEY sgx/private.pem
    CONFIG_FILE sgx/config.xml
    SOURCES ${algo_src} ${cmo_src} sgx/src/enclave/algo.cpp)
endif()

# add unit test
enable_testing()
# load CMakeLists.txt in the test directory.
add_subdirectory(test)
add_subdirectory(pao_test)
