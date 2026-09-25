cmake_minimum_required(VERSION 3.15)

# Optimize for code size
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os")

set(BL_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/bl_main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/bl_uart.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/bl_command_callbacks.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/bl_time.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/bl_logging.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/bl_utils.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/bl_verification.c

    ${CMAKE_SOURCE_DIR}/interfaces/obc_gs_interface/commands/src/obc_gs_command_pack.c
    ${CMAKE_SOURCE_DIR}/interfaces/obc_gs_interface/commands/src/obc_gs_commands_response_pack.c
    ${CMAKE_SOURCE_DIR}/interfaces/obc_gs_interface/commands/src/obc_gs_command_unpack.c
    ${CMAKE_SOURCE_DIR}/interfaces/data_pack_unpack/data_pack_utils.c
    ${CMAKE_SOURCE_DIR}/interfaces/data_pack_unpack/data_unpack_utils.c
    ${CMAKE_SOURCE_DIR}/interfaces/obc_gs_interface/common/obc_gs_crc.c
)

set(BL_INCLUDES
    ${CMAKE_BINARY_DIR}/toolchain/arm-none-eabi/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include

    ${CMAKE_SOURCE_DIR}/obc/app/sys/utils
    ${CMAKE_SOURCE_DIR}/interfaces/obc_gs_interface/common
    ${CMAKE_SOURCE_DIR}/interfaces/obc_gs_interface/commands/inc
    ${CMAKE_SOURCE_DIR}/interfaces/data_pack_unpack
)

target_include_directories(OBC-bl.out PUBLIC ${BL_INCLUDES})
target_sources(OBC-bl.out PUBLIC ${BL_SOURCES})

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/hal)
