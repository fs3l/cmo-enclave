if(NOT DEFINED SGX_SDK OR SGX_SDK STREQUAL "")
  message(FATAL_ERROR "SGX_SDK is not set.")
endif()

if(NOT DEFINED SGX_ARCH OR SGX_ARCH STREQUAL "")
  execute_process(COMMAND getconf LONG_BIT OUTPUT_VARIABLE VAR_LONG_BIT)
  if (VAR_LONG_BIT STREQUAL 32)
    set(SGX_ARCH x86)
  elseif(CMAKE_CXX_FLAGS MATCHES -m32)
    set(SGX_ARCH x86)
  else()
    set(SGX_ARCH x64)
  endif()
endif()

if (SGX_ARCH STREQUAL x86)
  set(SGX_COMMON_CFLAGS -m32)
  set(SGX_INCLUDE_DIR ${SGX_SDK}/include)
  set(SGX_LIBRARY_PATH ${SGX_SDK}/lib)
  set(SGX_ENCLAVE_SIGNER ${SGX_SDK}/bin/x86/sgx_sign)
  set(SGX_EDGER8R ${SGX_SDK}/bin/x86/sgx_edger8r)
elseif (SGX_ARCH STREQUAL x64)
  set(SGX_COMMON_CFLAGS -m64)
  set(SGX_INCLUDE_DIR ${SGX_SDK}/include)
  set(SGX_LIBRARY_PATH ${SGX_SDK}/lib64)
  set(SGX_ENCLAVE_SIGNER ${SGX_SDK}/bin/x64/sgx_sign)
  set(SGX_EDGER8R ${SGX_SDK}/bin/x64/sgx_edger8r)
else()
  message(FATAL_ERROR "Unsupported SGX_ARCH: ${SGX_ARCH}")
endif()

if (SGX_MODE STREQUAL HW)
  set(SGX_URTS_LIB sgx_urts)
  set(SGX_TRTS_LIB sgx_trts)
  set(SGX_TSVC_LIB sgx_tservice)
  set(SGX_CRYP_LIB sgx_tcrypto)
elseif (SGX_MODE STREQUAL SIM)
  set(SGX_URTS_LIB sgx_urts_sim)
  set(SGX_TRTS_LIB sgx_trts_sim)
  set(SGX_TSVC_LIB sgx_tservice_sim)
  set(SGX_CRYP_LIB sgx_tcrypto_sim)
else()
  message(FATAL_ERROR "Unsupported SGX_MODE: ${SGX_MDOE}")
endif()

if (SGX_BUILD STREQUAL DEBUG)
  set(SGX_COMMON_CFLAGS ${SGX_COMMON_CFLAGS} -g -O0)
  set(SGX_APP_CFLAGS -DDEBUG -UNDEBUG -UEDEBUG)
elseif(SGX_BUILD STREQUAL PRERELEASE)
  set(SGX_COMMON_CFLAGS ${SGX_COMMON_CFLAGS} -O2)
  set(SGX_APP_CFLAGS -UDEBUG -DNDEBUG -DEDEBUG)
elseif(SGX_BUILD STREQUAL RELEASE)
  if(NOT SGX_MODE STREQUAL HW)
    message(FATAL_ERROR "HW mode must be set with RELEASE")
  endif()
  set(SGX_COMMON_CFLAGS ${SGX_COMMON_CFLAGS} -O2)
  set(SGX_APP_CFLAGS -UDEBUG -DNDEBUG -UEDEBUG)
else()
  message(FATAL_ERROR "Unsupported SGX_BUILD: ${SGX_BUILD}")
endif()

set(SGX_APP_CFLAGS ${SGX_COMMON_CFLAGS} ${SGX_APP_CFLAGS} -fPIC -Wno-attributes -I${SGX_INCLUDE_DIR} -DSGX_APP)
set(SGX_APP_CPPFLAGS ${SGX_APP_CFLAGS} -std=c++11)
set(SGX_APP_LDFLAGS ${SGX_COMMON_CFLAGS} -L${SGX_LIBRARY_PATH} -l${SGX_URTS_LIB} -lpthread)
set(SGX_ENCLAVE_CFLAGS ${SGX_COMMON_CFLAGS} -nostdinc -fvisibility=hidden -fpie -fstack-protector -I${SGX_INCLUDE_DIR} -I${SGX_INCLUDE_DIR}/tlibc -I${SGX_INCLUDE_DIR}/stlport -DSGX_ENCLAVE)
set(SGX_ENCLAVE_CPPFLAGS ${SGX_ENCLAVE_CFLAGS} -nostdinc++ -std=c++11)
set(SGX_ENCLAVE_LDFLAGS ${SGX_COMMON_CFLAGS} -Wl,--no-undefined -nostdlib -nodefaultlibs -nostartfiles -L${SGX_LIBRARY_PATH} -Wl,--whole-archive -l${SGX_TRTS_LIB} -Wl,--no-whole-archive -Wl,--start-group -lsgx_tstdc -lsgx_tstdcxx -l${SGX_TSVC_LIB} -l${SGX_CRYP_LIB} -Wl,--end-group -Wl,-Bstatic -Wl,-Bsymbolic -Wl,--no-undefined -Wl,-pie,-eenclave_entry -Wl,--export-dynamic -Wl,--defsym,__ImageBase=0)

message(STATUS "SGX_SDK: ${SGX_SDK}")
message(STATUS "SGX_ARCH: ${SGX_ARCH}")
message(STATUS "SGX_MODE: ${SGX_MODE}")
message(STATUS "SGX_BUILD: ${SGX_BUILD}")
message(STATUS "SGX_INCLUDE_DIR: ${SGX_INCLUDE_DIR}")
message(STATUS "SGX_LIBRARY_PATH: ${SGX_LIBRARY_PATH}")
message(STATUS "SGX_ENCLAVE_SIGNER: ${SGX_ENCLAVE_SIGNER}")
message(STATUS "SGX_EDGER8R: ${SGX_EDGER8R}")

include(CMakeParseArguments)

function(add_sgx_enclave_library target)
  set(oneValueArgs EDL_FILE LDS_FILE PRIVATE_KEY CONFIG_FILE)
  set(multiValueArgs SOURCES)
  cmake_parse_arguments(SGX "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  get_filename_component(SGX_EDL_FILE "${SGX_EDL_FILE}" ABSOLUTE)
  get_filename_component(SGX_LDS_FILE "${SGX_LDS_FILE}" ABSOLUTE)
  get_filename_component(SGX_PRIVATE_KEY "${SGX_PRIVATE_KEY}" ABSOLUTE)
  get_filename_component(SGX_CONFIG_FILE "${SGX_CONFIG_FILE}" ABSOLUTE)

  get_filename_component(SGX_EDL_NAME "${SGX_EDL_FILE}" NAME_WE)
  get_filename_component(SGX_EDL_FILENAME "${SGX_EDL_FILE}" NAME)
  get_filename_component(SGX_EDL_DIR "${SGX_EDL_FILE}" DIRECTORY)

  set(SGX_GENERATE_DIR "${CMAKE_BINARY_DIR}")
  set(SGX_TRUST_SRC "${SGX_GENERATE_DIR}/${SGX_EDL_NAME}_t.c")

  set(SGX_EDGER8R_ARGS)
  list(APPEND SGX_EDGER8R_ARGS --search-path ${SGX_INCLUDE_DIR})
  list(APPEND SGX_EDGER8R_ARGS --search-path ${SGX_EDL_DIR})
  foreach(dir ${INCLUDE_DIRECTORIES})
    list(APPEND SGX_EDGER8R_ARGS --search-path ${dir})
  endforeach()

  add_custom_command(
    OUTPUT ${SGX_TRUST_SRC}
    DEPENDS ${SGX_EDL_FILE}
    COMMAND ${SGX_EDGER8R} --trusted ${SGX_EDL_FILENAME} ${SGX_EDGER8R_ARGS}
    WORKING_DIRECTORY "${SGX_GENERATE_DIR}"
    )

  add_library(${target} SHARED ${SGX_SOURCES} ${SGX_TRUST_SRC})
  target_compile_options(${target}
    PUBLIC $<$<COMPILE_LANGUAGE:C>:${SGX_ENCLAVE_CFLAGS} -I${SGX_GENERATE_DIR} -include ${SGX_EDL_NAME}_t.h>
    PUBLIC $<$<COMPILE_LANGUAGE:CXX>:${SGX_ENCLAVE_CPPFLAGS} -I${SGX_GENERATE_DIR} -include ${SGX_EDL_NAME}_t.h>
    )
  target_link_libraries(${target} ${SGX_ENCLAVE_LDFLAGS} -Wl,--version-script=${SGX_LDS_FILE})

  add_custom_command(
    TARGET ${target}
    POST_BUILD
    DEPENDS ${SGX_PRIVATE_KEY} ${SGX_CONFIG_FILE}
    COMMAND mv $<TARGET_FILE:${target}> $<TARGET_FILE:${target}>.unsigned
    COMMAND ${SGX_ENCLAVE_SIGNER} sign -key ${SGX_PRIVATE_KEY}
                                       -enclave $<TARGET_FILE:${target}>.unsigned
                                       -out $<TARGET_FILE:${target}>
                                       -config ${SGX_CONFIG_FILE}
    )
endfunction()

function(add_sgx_executable target)
  set(oneValueArgs EDL_FILE)
  set(multiValueArgs SOURCES)
  cmake_parse_arguments(SGX "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  get_filename_component(SGX_EDL_FILE "${SGX_EDL_FILE}" ABSOLUTE)

  get_filename_component(SGX_EDL_NAME "${SGX_EDL_FILE}" NAME_WE)
  get_filename_component(SGX_EDL_FILENAME "${SGX_EDL_FILE}" NAME)
  get_filename_component(SGX_EDL_DIR "${SGX_EDL_FILE}" DIRECTORY)

  set(SGX_GENERATE_DIR "${CMAKE_BINARY_DIR}")
  set(SGX_UNTRUST_SRC "${SGX_GENERATE_DIR}/${SGX_EDL_NAME}_u.c")

  set(SGX_EDGER8R_ARGS)
  list(APPEND SGX_EDGER8R_ARGS --search-path ${SGX_INCLUDE_DIR})
  list(APPEND SGX_EDGER8R_ARGS --search-path ${SGX_EDL_DIR})
  foreach(dir ${INCLUDE_DIRECTORIES})
    list(APPEND SGX_EDGER8R_ARGS --search-path ${dir})
  endforeach()

  add_custom_command(
    OUTPUT ${SGX_UNTRUST_SRC}
    DEPENDS ${SGX_EDL_FILE}
    COMMAND ${SGX_EDGER8R} --untrusted ${SGX_EDL_FILENAME} ${SGX_EDGER8R_ARGS}
    WORKING_DIRECTORY "${SGX_GENERATE_DIR}"
    )

  add_executable(${target} ${SGX_SOURCES} ${SGX_UNTRUST_SRC})
  target_compile_options(${target}
    PUBLIC $<$<COMPILE_LANGUAGE:C>:${SGX_APP_CFLAGS} -I${SGX_GENERATE_DIR} -include ${SGX_EDL_NAME}_u.h>
    PUBLIC $<$<COMPILE_LANGUAGE:CXX>:${SGX_APP_CPPFLAGS} -I${SGX_GENERATE_DIR} -include ${SGX_EDL_NAME}_u.h>
    )
  target_link_libraries(${target} ${SGX_APP_LDFLAGS})
endfunction()
