cmake_minimum_required(VERSION 3.15)
project(LoggerRegressionTests C)
enable_testing()

get_filename_component(FIRMWARE_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
set(LOG_MAPPING_DIR "${FIRMWARE_ROOT}/obc/shared/logging" CACHE PATH
    "Directory containing generated obc_log_file_ids.c and .h")

foreach(use_ids IN ITEMS 0 1)
    foreach(timestamp IN ITEMS none unix date)
        set(name logger-${use_ids}-${timestamp})
        add_executable(${name}
            test_logger.c
            "${FIRMWARE_ROOT}/obc/app/modules/logger/logger.c"
            "${FIRMWARE_ROOT}/obc/shared/logging/obc_logging_codec.c"
            "${FIRMWARE_ROOT}/obc/app/sys/time/obc_time_utils.c"
            "${FIRMWARE_ROOT}/interfaces/data_pack_unpack/data_pack_utils.c"
            "${FIRMWARE_ROOT}/interfaces/data_pack_unpack/data_unpack_utils.c")
        if(NOT use_ids)
            target_sources(${name} PRIVATE "${LOG_MAPPING_DIR}/obc_log_file_ids.c")
        endif()
        target_include_directories(${name} PRIVATE
            stubs
            "${LOG_MAPPING_DIR}"
            "${FIRMWARE_ROOT}/obc/app/modules/logger"
            "${FIRMWARE_ROOT}/obc/shared/logging"
            "${FIRMWARE_ROOT}/obc/shared/obc_errors"
            "${FIRMWARE_ROOT}/obc/shared/hal/freertos/include"
            "${FIRMWARE_ROOT}/obc/app/drivers/ds3232"
            "${FIRMWARE_ROOT}/obc/app/sys/time"
            "${FIRMWARE_ROOT}/obc/app/sys/print"
            "${FIRMWARE_ROOT}/interfaces/data_pack_unpack"
            "${FIRMWARE_ROOT}/interfaces/obc_gs_interface/common")
        target_compile_features(${name} PRIVATE c_std_11)
        target_compile_options(${name} PRIVATE -UNDEBUG)
        target_compile_definitions(${name} PRIVATE
            LOG_USE_FILE_IDS=${use_ids} OBC_LOG_FILE_ID=7
            SOURCE_PATH="${FIRMWARE_ROOT}/")
        if(timestamp STREQUAL "unix")
            target_compile_definitions(${name} PRIVATE LOG_UNIX)
        elseif(timestamp STREQUAL "date")
            target_compile_definitions(${name} PRIVATE LOG_DATE_TIME)
        endif()
        add_test(NAME ${name} COMMAND ${name})
    endforeach()
endforeach()
