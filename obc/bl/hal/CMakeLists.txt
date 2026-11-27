cmake_minimum_required(VERSION 3.15)

set(INCLUDES

)

set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/bl_intvecs.S
)

target_include_directories(OBC-bl.out PUBLIC ${INCLUDES})
target_sources(OBC-bl.out PUBLIC ${SOURCES})
