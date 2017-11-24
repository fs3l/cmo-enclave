glob_cpp_source_file(coda_src PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib")
add_library(coda ${coda_src})

glob_cpp_source_file(algo_src PATH "${CMAKE_CURRENT_SOURCE_DIR}/algo")
add_library(algo ${algo_src})
