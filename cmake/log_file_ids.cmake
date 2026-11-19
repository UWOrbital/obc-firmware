# Generate one mapping for both logging modes, without changing source files.
option(LOG_USE_FILE_IDS "Store numeric logging file IDs instead of paths in firmware" OFF)
find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(LOG_FILE_IDS_DIR "${CMAKE_BINARY_DIR}/generated/logging")
set(LOG_FILE_IDS_HEADER "${LOG_FILE_IDS_DIR}/obc_log_file_ids.h")
set(LOG_FILE_IDS_SOURCE "${LOG_FILE_IDS_DIR}/obc_log_file_ids.c")
set(LOG_FILE_IDS_JSON "${LOG_FILE_IDS_DIR}/log_file_ids.json")
set(LOG_FILE_IDS_CMAKE "${LOG_FILE_IDS_DIR}/log_file_ids.cmake")
set(LOG_FILE_IDS_GENERATOR "${CMAKE_SOURCE_DIR}/scripts/gen_log_file_ids.py")

# Existing-file edits and newly added/removed files must refresh source IDs
# before CMake generates compilation commands.
file(GLOB_RECURSE LOG_FILE_ID_INPUTS CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/obc/*.c" "${CMAKE_SOURCE_DIR}/obc/*.h"
    "${CMAKE_SOURCE_DIR}/interfaces/*.c" "${CMAKE_SOURCE_DIR}/interfaces/*.h"
)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    "${LOG_FILE_IDS_GENERATOR}" ${LOG_FILE_ID_INPUTS}
)
execute_process(
    COMMAND "${Python3_EXECUTABLE}" "${LOG_FILE_IDS_GENERATOR}"
        --output-dir "${LOG_FILE_IDS_DIR}"
    RESULT_VARIABLE LOG_FILE_IDS_RESULT
    OUTPUT_VARIABLE LOG_FILE_IDS_OUTPUT
    ERROR_VARIABLE LOG_FILE_IDS_ERROR
)
if(NOT LOG_FILE_IDS_RESULT EQUAL 0)
    message(FATAL_ERROR "Log file ID generation failed:\n${LOG_FILE_IDS_OUTPUT}${LOG_FILE_IDS_ERROR}")
endif()
include("${LOG_FILE_IDS_CMAKE}")

# Also recreate deleted outputs and retain the existing dependency target.
# Unchanged outputs are not rewritten, so this does not force recompilation.
add_custom_target(generate-log-file-ids
    COMMAND "${Python3_EXECUTABLE}" "${LOG_FILE_IDS_GENERATOR}"
        --output-dir "${LOG_FILE_IDS_DIR}"
    BYPRODUCTS "${LOG_FILE_IDS_HEADER}" "${LOG_FILE_IDS_SOURCE}"
        "${LOG_FILE_IDS_JSON}" "${LOG_FILE_IDS_CMAKE}"
    VERBATIM
)

# These must reach libraries such as FreeRTOS as well as the main executable:
# their assertion declarations and call sites share the logging argument type.
add_compile_definitions(LOG_USE_FILE_IDS=$<BOOL:${LOG_USE_FILE_IDS}> SOURCE_PATH="${CMAKE_SOURCE_DIR}/")
include_directories(BEFORE "${CMAKE_SOURCE_DIR}/obc/shared/logging")
include_directories(BEFORE "${LOG_FILE_IDS_DIR}")

function(apply_log_file_ids directory)
    # Source properties are scoped to the directory that creates a target.
    # Run after all add_subdirectory calls and apply the IDs in each such scope.
    get_property(targets DIRECTORY "${directory}" PROPERTY BUILDSYSTEM_TARGETS)
    if(targets)
        list(GET targets 0 scope_target)
        set(file_id 0)
        foreach(source IN LISTS LOG_FILE_ID_SOURCES)
            set_property(SOURCE "${CMAKE_SOURCE_DIR}/${source}"
                TARGET_DIRECTORY "${scope_target}"
                APPEND PROPERTY COMPILE_DEFINITIONS "OBC_LOG_FILE_ID=${file_id}"
            )
            math(EXPR file_id "${file_id} + 1")
        endforeach()
    endif()
    get_property(subdirectories DIRECTORY "${directory}" PROPERTY SUBDIRECTORIES)
    foreach(subdirectory IN LISTS subdirectories)
        apply_log_file_ids("${subdirectory}")
    endforeach()
endfunction()
