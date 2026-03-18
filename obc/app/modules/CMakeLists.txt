cmake_minimum_required(VERSION 3.15)

set(INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/alarm_mgr
    ${CMAKE_CURRENT_SOURCE_DIR}/altitude_ctrl_mgr
    ${CMAKE_CURRENT_SOURCE_DIR}/camera_mgr
    ${CMAKE_CURRENT_SOURCE_DIR}/command_mgr
    ${CMAKE_CURRENT_SOURCE_DIR}/comms_link_mgr
    ${CMAKE_CURRENT_SOURCE_DIR}/digital_watchdog_mgr
    ${CMAKE_CURRENT_SOURCE_DIR}/eps_mgr
    ${CMAKE_CURRENT_SOURCE_DIR}/gnc_mgr
    ${CMAKE_CURRENT_SOURCE_DIR}/health_collector
    ${CMAKE_CURRENT_SOURCE_DIR}/state_mgr
    ${CMAKE_CURRENT_SOURCE_DIR}/telemetry_mgr
    ${CMAKE_CURRENT_SOURCE_DIR}/timekeeper
    ${CMAKE_CURRENT_SOURCE_DIR}/task_stats_collector
    ${CMAKE_CURRENT_SOURCE_DIR}/logger
)

set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/alarm_mgr/alarm_handler.c

    ${CMAKE_CURRENT_SOURCE_DIR}/camera_mgr/payload_manager.c
    ${CMAKE_CURRENT_SOURCE_DIR}/camera_mgr/camera_control.c

    ${CMAKE_CURRENT_SOURCE_DIR}/command_mgr/command_manager.c
    ${CMAKE_CURRENT_SOURCE_DIR}/command_mgr/command_callbacks.c

    ${CMAKE_CURRENT_SOURCE_DIR}/comms_link_mgr/comms_manager.c
    ${CMAKE_CURRENT_SOURCE_DIR}/comms_link_mgr/downlink_encoder.c
    ${CMAKE_CURRENT_SOURCE_DIR}/comms_link_mgr/uplink_decoder.c
    ${CMAKE_CURRENT_SOURCE_DIR}/comms_link_mgr/cc1120_txrx.c

    ${CMAKE_CURRENT_SOURCE_DIR}/eps_mgr/eps_manager.c

    ${CMAKE_CURRENT_SOURCE_DIR}/gnc_mgr/gnc_manager.c

    ${CMAKE_CURRENT_SOURCE_DIR}/health_collector/health_collector.c

    ${CMAKE_CURRENT_SOURCE_DIR}/state_mgr/state_mgr.c

    ${CMAKE_CURRENT_SOURCE_DIR}/digital_watchdog_mgr/digital_watchdog_mgr.c

    ${CMAKE_CURRENT_SOURCE_DIR}/telemetry_mgr/telemetry_manager.c
    ${CMAKE_CURRENT_SOURCE_DIR}/telemetry_mgr/telemetry_fs_utils.c

    ${CMAKE_CURRENT_SOURCE_DIR}/timekeeper/timekeeper.c
    ${CMAKE_CURRENT_SOURCE_DIR}/task_stats_collector/task_stats_collector.c
    ${CMAKE_CURRENT_SOURCE_DIR}/task_stats_collector/runtime_stats.c
    ${CMAKE_CURRENT_SOURCE_DIR}/logger/logger.c

)

target_include_directories(${OUT_FILE_NAME} PUBLIC ${INCLUDES})
target_sources(${OUT_FILE_NAME} PUBLIC ${SOURCES})

target_include_directories(debug-tool.out PUBLIC ${INCLUDES})
target_sources(debug-tool.out PUBLIC ${SOURCES})
