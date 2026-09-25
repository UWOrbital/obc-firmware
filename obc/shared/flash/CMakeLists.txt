cmake_minimum_required(VERSION 3.15)

# Optimize for code size
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os")

set(SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/source/flash.c
  ${CMAKE_CURRENT_SOURCE_DIR}/F021_Flash_API/source/Fapi_UserDefinedFunctions.c
)

set(INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(f021_api STATIC IMPORTED)
set_property(TARGET f021_api PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/F021_Flash_API/F021_API_CortexR4_LE.lib)
target_include_directories(f021_api INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/F021_Flash_API/include)

add_library(f021_api_archive INTERFACE)
target_link_libraries(f021_api_archive INTERFACE "-Wl,--whole-archive" f021_api "-Wl,--no-whole-archive")

# Lib doesn't use wchar so warning can be ignored
target_link_options(f021_api_archive INTERFACE "-Wl,--no-wchar-size-warning")

target_include_directories(OBC-bl.out PUBLIC ${INCLUDES})
target_sources(OBC-bl.out PUBLIC ${SOURCES})

target_include_directories(OBC-firmware.out PUBLIC ${INCLUDES})
target_sources(OBC-firmware.out PUBLIC ${SOURCES})

target_include_directories(debug-tool.out PUBLIC ${INCLUDES})
target_sources(debug-tool.out PUBLIC ${SOURCES})
