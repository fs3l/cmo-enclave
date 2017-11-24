include (CMakeParseArguments)

function(glob_file ret)
    set(options RECURSIVE)
    set(multiValueArgs PATH SUFFIX)
    cmake_parse_arguments(GLOB_FILE "${options}" "" "${multiValueArgs}" ${ARGN} )
    set(_ret)
    if(${GLOB_FILE_RECURSIVE})
        set(_operation GLOB_RECURSE)
    else()
        set(_operation GLOB)
    endif()
    foreach(_path ${GLOB_FILE_PATH})
        foreach(_suffix ${GLOB_FILE_SUFFIX})
            file(${_operation} _files "${_path}/*${_suffix}")
            list(APPEND _ret ${_files})
        endforeach()
    endforeach()
    set(${ret} ${_ret} PARENT_SCOPE)
endfunction()

set(CPP_SOURCE_FILE_SUFFIX
    ".h" ".hh" ".hpp"
    ".cpp" ".cxx" ".cc" ".c"
)

function(glob_cpp_source_file ret)
    glob_file(_ret SUFFIX ${CPP_SOURCE_FILE_SUFFIX} ${ARGN})
    set(${ret} ${_ret} PARENT_SCOPE)
endfunction()
