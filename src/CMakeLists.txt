glob_cpp_source_file(cmo_src PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib")
add_library(cmo ${cmo_src})

glob_cpp_source_file(algo_src PATH "${CMAKE_CURRENT_SOURCE_DIR}/algo")
add_library(algo ${algo_src})
