cmake_minimum_required(VERSION 3.15)

include(cmake/board_lib_defs.cmake)

set(RM46_LAUNCHPAD_HAL_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/common/rti_weak.c
    ${CMAKE_CURRENT_SOURCE_DIR}/launchpad/source/adc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/launchpad/source/can.c
    ${CMAKE_CURRENT_SOURCE_DIR}/launchpad/source/dabort.s
    ${CMAKE_CURRENT_SOURCE_DIR}/launchpad/source/errata_SSWF021_45.c
    ${CMAKE_CURRENT_SOURCE_DIR}/launchpad/source/esm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/launchpad/source/gio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/launchpad/source/het.c
    ${CMAKE_CURRENT_SOURCE_DIR}/launchpad/source/i2c.c
    ${CMAKE_CURRENT_SOURCE_DIR}/launchpad/source/notification.c
    ${CMAKE_CURRENT_SOURCE_DIR}/launchpad/source/pinmux.c
    ${CMAKE_CURRENT_SOURCE_DIR}/launchpad/source/sci.c
    ${CMAKE_CURRENT_SOURCE_DIR}/launchpad/source/spi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/launchpad/source/sys_core.s
    ${CMAKE_CURRENT_SOURCE_DIR}/launchpad/source/sys_dma.c
    ${CMAKE_CURRENT_SOURCE_DIR}/launchpad/source/sys_mpu.s
    ${CMAKE_CURRENT_SOURCE_DIR}/launchpad/source/sys_pcr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/launchpad/source/sys_phantom.c
    ${CMAKE_CURRENT_SOURCE_DIR}/launchpad/source/sys_pmm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/launchpad/source/sys_pmu.s
    ${CMAKE_CURRENT_SOURCE_DIR}/launchpad/source/sys_vim.c
    ${CMAKE_CURRENT_SOURCE_DIR}/launchpad/source/system.c
)

set(RM46_LAUNCHPAD_NO_OPTIMIZE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/launchpad/source/sys_selftest.c
    ${CMAKE_CURRENT_SOURCE_DIR}/launchpad/source/sys_startup.c
)

set(OBC_REV1_HAL_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/common/rti_weak.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev1/source/adc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev1/source/can.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev1/source/dabort.s
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev1/source/errata_SSWF021_45.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev1/source/esm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev1/source/gio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev1/source/het.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev1/source/i2c.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev1/source/notification.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev1/source/pinmux.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev1/source/sci.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev1/source/spi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev1/source/sys_core.s
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev1/source/sys_dma.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev1/source/sys_mpu.s
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev1/source/sys_pcr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev1/source/sys_phantom.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev1/source/sys_pmm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev1/source/sys_pmu.s
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev1/source/sys_vim.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev1/source/system.c
)

set(OBC_REV1_NO_OPTIMIZE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev1/source/sys_selftest.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev1/source/sys_startup.c
)

set(OBC_REV2_HAL_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/common/rti_weak.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev2/source/adc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev2/source/can.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev2/source/dabort.s
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev2/source/errata_SSWF021_45.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev2/source/esm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev2/source/gio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev2/source/het.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev2/source/i2c.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev2/source/notification.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev2/source/pinmux.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev2/source/sci.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev2/source/spi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev2/source/sys_core.s
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev2/source/sys_dma.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev2/source/sys_mpu.s
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev2/source/sys_pcr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev2/source/sys_phantom.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev2/source/sys_pmm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev2/source/sys_pmu.s
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev2/source/sys_vim.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev2/source/system.c
)

set(OBC_REV2_NO_OPTIMIZE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev2/source/sys_selftest.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev2/source/sys_startup.c
)

set(OBC_REV4_HAL_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/common/rti_weak.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev4/source/adc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev4/source/can.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev4/source/dabort.s
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev4/source/errata_SSWF021_45.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev4/source/esm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev4/source/gio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev4/source/het.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev4/source/i2c.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev4/source/notification.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev4/source/pinmux.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev4/source/sci.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev4/source/spi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev4/source/sys_core.s
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev4/source/sys_dma.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev4/source/sys_mpu.s
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev4/source/sys_pcr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev4/source/sys_phantom.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev4/source/sys_pmm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev4/source/sys_pmu.s
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev4/source/sys_vim.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev4/source/system.c
)

set(OBC_REV4_NO_OPTIMIZE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev4/source/sys_selftest.c
    ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev4/source/sys_startup.c
)

set(FREERTOS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/common/os_required_fns.c
    ${CMAKE_CURRENT_SOURCE_DIR}/freertos/source/os_assert.c
    ${CMAKE_CURRENT_SOURCE_DIR}/freertos/source/os_croutine.c
    ${CMAKE_CURRENT_SOURCE_DIR}/freertos/source/os_event_groups.c
    ${CMAKE_CURRENT_SOURCE_DIR}/freertos/source/os_heap.c
    ${CMAKE_CURRENT_SOURCE_DIR}/freertos/source/os_list.c
    ${CMAKE_CURRENT_SOURCE_DIR}/freertos/source/os_port.c
    ${CMAKE_CURRENT_SOURCE_DIR}/freertos/source/os_portasm.s
    ${CMAKE_CURRENT_SOURCE_DIR}/freertos/source/os_queue.c
    ${CMAKE_CURRENT_SOURCE_DIR}/freertos/source/os_tasks.c
    ${CMAKE_CURRENT_SOURCE_DIR}/freertos/source/os_timer.c
)

set(FREERTOS_NO_OPTIMIZE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/freertos/source/os_mpu_wrappers.c
)

set(FREERTOS_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/freertos/include)

if (${BOARD_TYPE} MATCHES RM46_LAUNCHPAD)
    set(HAL_SOURCES ${RM46_LAUNCHPAD_HAL_SOURCES})
    set(HAL_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/launchpad/include)
    set(NO_OPTIMIZE_SOURCES ${RM46_LAUNCHPAD_NO_OPTIMIZE_SOURCES})
elseif(${BOARD_TYPE} MATCHES OBC_REVISION_1)
    set(HAL_SOURCES ${OBC_REV1_HAL_SOURCES})
    set(HAL_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev1/include)
    set(NO_OPTIMIZE_SOURCES ${OBC_REV1_NO_OPTIMIZE_SOURCES})
elseif(${BOARD_TYPE} MATCHES OBC_REVISION_2)
    set(HAL_SOURCES ${OBC_REV2_HAL_SOURCES})
    set(HAL_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev2/include)
    set(NO_OPTIMIZE_SOURCES ${OBC_REV2_NO_OPTIMIZE_SOURCES})
elseif(${BOARD_TYPE} MATCHES OBC_REVISION_4)
    set(HAL_SOURCES ${OBC_REV4_HAL_SOURCES})
    set(HAL_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/obc_rev1/include)
    set(NO_OPTIMIZE_SOURCES ${OBC_REV1_NO_OPTIMIZE_SOURCES})
else()
    message(FATAL_ERROR "Invalid board type: ${BOARD_TYPE}")
endif()

add_library(${HAL_LIB_OPTIMIZE} STATIC)
target_include_directories(${HAL_LIB_OPTIMIZE} PUBLIC ${FREERTOS_INCLUDE_DIRS} ${HAL_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/common)
add_library(${FREERTOS_LIB_OPTIMIZE} STATIC)
target_include_directories(${FREERTOS_LIB_OPTIMIZE} PUBLIC ${FREERTOS_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/common)
target_link_libraries(${FREERTOS_LIB_OPTIMIZE} PUBLIC ${HAL_LIB_OPTIMIZE})

target_sources(${HAL_LIB_OPTIMIZE} PUBLIC ${HAL_SOURCES})
target_sources(${FREERTOS_LIB_OPTIMIZE} PUBLIC ${FREERTOS_SOURCES})

if (DEBUG MATCHES 1)
    target_compile_options(${HAL_LIB_OPTIMIZE} PRIVATE -O0)
    target_compile_options(${FREERTOS_LIB_OPTIMIZE} PRIVATE -O0)
else()
    target_compile_options(${HAL_LIB_OPTIMIZE} PRIVATE -O2)
    target_compile_options(${FREERTOS_LIB_OPTIMIZE} PRIVATE -O2)
endif()

# Higher optimization levels on some hal files will break firmware for some reason
# Compile them with no optimizations
add_library(${HAL_LIB_NO_OPTIMIZE} OBJECT)
target_include_directories(${HAL_LIB_NO_OPTIMIZE} PUBLIC ${FREERTOS_INCLUDE_DIRS} ${HAL_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/common)
add_library(${FREERTOS_LIB_NO_OPTIMIZE} OBJECT)
target_include_directories(${FREERTOS_LIB_NO_OPTIMIZE} PUBLIC ${FREERTOS_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/common)

target_sources(${HAL_LIB_NO_OPTIMIZE} PUBLIC ${NO_OPTIMIZE_SOURCES})
target_compile_definitions(${HAL_LIB_NO_OPTIMIZE} PRIVATE 
    ENABLE_TASK_STATS_COLLECTOR=${ENABLE_TASK_STATS_COLLECTOR}
)
target_sources(${FREERTOS_LIB_NO_OPTIMIZE} PUBLIC ${FREERTOS_NO_OPTIMIZE_SOURCES})
target_compile_definitions(${FREERTOS_LIB_NO_OPTIMIZE} PRIVATE 
    ENABLE_TASK_STATS_COLLECTOR=${ENABLE_TASK_STATS_COLLECTOR}
)

if (DEBUG MATCHES 1)
    target_compile_options(${HAL_LIB_NO_OPTIMIZE} PRIVATE -O0 -g -gdwarf-3 -gstrict-dwarf)
else()
    target_compile_options(${HAL_LIB_NO_OPTIMIZE} PRIVATE -O0)
endif()
