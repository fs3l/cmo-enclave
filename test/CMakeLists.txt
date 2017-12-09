find_package(Boost COMPONENTS unit_test_framework REQUIRED)

file(GLOB test_srcs RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")
foreach(test_src ${test_srcs})
  get_filename_component(test_name ${test_src} NAME_WE)

  # add normal executable
  add_executable(${test_name} ${test_src})
  target_link_libraries(${test_name}
    algo cmo ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
  set_target_properties(${test_name}
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_TEST_OUTPUT_DIRECTORY})
  add_test(
    NAME ${test_name}
    COMMAND "${CMAKE_TEST_OUTPUT_DIRECTORY}/${test_name}"
    WORKING_DIRECTORY "${CMAKE_TEST_OUTPUT_DIRECTORY}"
    )

  # add sgx executable
  if(ENABLE_SGX AND NOT ${test_name} MATCHES "cmo_.*")
    add_sgx_executable(${test_name}_enclave
      EDL_FILE ${PROJECT_SOURCE_DIR}/sgx/edl/algo.edl
      SOURCES ${test_src}
              ${PROJECT_SOURCE_DIR}/sgx/src/app/app.cpp
              ${PROJECT_SOURCE_DIR}/src/lib/utils.cpp
      )
    target_link_libraries(${test_name}_enclave
      ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
    set_target_properties(${test_name}_enclave
      PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_TEST_OUTPUT_DIRECTORY})
    target_include_directories(${test_name}_enclave
      PUBLIC "${PROJECT_SOURCE_DIR}/sgx/include" )
    add_test(
      NAME ${test_name}_enclave
      COMMAND "${CMAKE_TEST_OUTPUT_DIRECTORY}/${test_name}_enclave"
      WORKING_DIRECTORY "${CMAKE_TEST_OUTPUT_DIRECTORY}"
      )
  endif()
endforeach()
