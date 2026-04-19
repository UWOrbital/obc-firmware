cmake_minimum_required(VERSION 3.15)

# Optimize for code size
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os")

set(BL_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/bl_main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/bl_flash.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/bl_uart.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/bl_command_callbacks.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/bl_time.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/bl_logging.c
    ${CMAKE_CURRENT_SOURCE_DIR}/F021_Flash_API/source/Fapi_UserDefinedFunctions.c

    ${CMAKE_SOURCE_DIR}/interfaces/obc_gs_interface/commands/obc_gs_command_pack.c
    ${CMAKE_SOURCE_DIR}/interfaces/obc_gs_interface/commands/obc_gs_commands_response_pack.c
    ${CMAKE_SOURCE_DIR}/interfaces/obc_gs_interface/commands/obc_gs_command_unpack.c
    ${CMAKE_SOURCE_DIR}/interfaces/data_pack_unpack/data_pack_utils.c
    ${CMAKE_SOURCE_DIR}/interfaces/data_pack_unpack/data_unpack_utils.c
    ${CMAKE_SOURCE_DIR}/interfaces/obc_gs_interface/common/obc_gs_crc.c
)

set(BL_INCLUDES
    ${CMAKE_BINARY_DIR}/toolchain/arm-none-eabi/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include

    ${CMAKE_SOURCE_DIR}/obc/app/sys/utils
    ${CMAKE_SOURCE_DIR}/interfaces/obc_gs_interface/common
    ${CMAKE_SOURCE_DIR}/interfaces/obc_gs_interface/commands
    ${CMAKE_SOURCE_DIR}/interfaces/data_pack_unpack
)

target_include_directories(OBC-bl.out PUBLIC ${BL_INCLUDES})
target_sources(OBC-bl.out PUBLIC ${BL_SOURCES})

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/hal)

add_library(f021_api STATIC IMPORTED)
set_property(TARGET f021_api PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/F021_Flash_API/F021_API_CortexR4_LE.lib)
target_include_directories(f021_api INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/F021_Flash_API/include)

add_library(f021_api_archive INTERFACE)
target_link_libraries(f021_api_archive INTERFACE "-Wl,--whole-archive" f021_api "-Wl,--no-whole-archive")

# Lib doesn't use wchar so warning can be ignored
target_link_options(f021_api_archive INTERFACE "-Wl,--no-wchar-size-warning")
