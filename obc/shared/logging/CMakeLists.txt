cmake_minimum_required(VERSION 3.15)

SET(INCLUDES
    ${LOG_FILE_IDS_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

SET(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_logging_codec.c
)
if(NOT LOG_USE_FILE_IDS)
    list(APPEND SOURCES "${LOG_FILE_IDS_SOURCE}")
endif()

target_include_directories(${OUT_FILE_NAME} BEFORE PUBLIC ${INCLUDES})
target_sources(${OUT_FILE_NAME} PUBLIC ${SOURCES})
add_dependencies(${OUT_FILE_NAME} generate-log-file-ids)

target_include_directories(OBC-bl.out PUBLIC ${INCLUDES})

target_include_directories(debug-tool.out BEFORE PUBLIC ${INCLUDES})
target_sources(debug-tool.out PUBLIC ${SOURCES})
add_dependencies(debug-tool.out generate-log-file-ids)
