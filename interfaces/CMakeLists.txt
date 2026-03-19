cmake_minimum_required(VERSION 3.15)

set(OBC_GS_INTERFACE_LIB_NAME obc-gs-interface)
get_filename_component(DIR_ONE_ABOVE ../ ABSOLUTE)

set(INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}/obc_gs_interface/common
  ${CMAKE_CURRENT_SOURCE_DIR}/obc_gs_interface/commands/inc
  ${CMAKE_CURRENT_SOURCE_DIR}/obc_gs_interface/telemetry
  ${CMAKE_CURRENT_SOURCE_DIR}/obc_gs_interface/aes128
  ${CMAKE_CURRENT_SOURCE_DIR}/obc_gs_interface/ax25
  ${CMAKE_CURRENT_SOURCE_DIR}/obc_gs_interface/fec

  ${CMAKE_CURRENT_SOURCE_DIR}/data_pack_unpack
)

set(SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/data_pack_unpack/data_pack_utils.c
  ${CMAKE_CURRENT_SOURCE_DIR}/data_pack_unpack/data_unpack_utils.c

  ${CMAKE_CURRENT_SOURCE_DIR}/obc_gs_interface/commands/src/obc_gs_command_pack.c
  ${CMAKE_CURRENT_SOURCE_DIR}/obc_gs_interface/commands/src/obc_gs_command_unpack.c
  ${CMAKE_CURRENT_SOURCE_DIR}/obc_gs_interface/telemetry/obc_gs_telemetry_pack.c
  ${CMAKE_CURRENT_SOURCE_DIR}/obc_gs_interface/telemetry/obc_gs_telemetry_unpack.c

  ${CMAKE_CURRENT_SOURCE_DIR}/obc_gs_interface/commands/src/obc_gs_commands_response_pack.c
  ${CMAKE_CURRENT_SOURCE_DIR}/obc_gs_interface/commands/src/obc_gs_commands_response_unpack.c

  ${CMAKE_CURRENT_SOURCE_DIR}/obc_gs_interface/aes128/obc_gs_aes128.c
  ${CMAKE_CURRENT_SOURCE_DIR}/obc_gs_interface/ax25/obc_gs_ax25.c
  ${CMAKE_CURRENT_SOURCE_DIR}/obc_gs_interface/fec/obc_gs_fec.c

  ${CMAKE_CURRENT_SOURCE_DIR}/obc_gs_interface/common/obc_gs_crc.c
)
if(${CMAKE_BUILD_TYPE} MATCHES GS)
  list(APPEND INCLUDE_DIRS ${DIR_ONE_ABOVE}/libs/libcorrect/include)
  list(APPEND SOURCES ${DIR_ONE_ABOVE}/gs/backend/common/gs_heap.c)
endif()


if(${CMAKE_BUILD_TYPE} MATCHES GS)
  add_library(${OBC_GS_INTERFACE_LIB_NAME} SHARED ${SOURCES})
else()
  add_library(${OBC_GS_INTERFACE_LIB_NAME} STATIC ${SOURCES})
endif()

target_include_directories(${OBC_GS_INTERFACE_LIB_NAME} PUBLIC ${INCLUDE_DIRS})

target_link_libraries(${OBC_GS_INTERFACE_LIB_NAME} PRIVATE
    tiny-aes
    lib-correct
)
