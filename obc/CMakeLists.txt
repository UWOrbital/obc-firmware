cmake_minimum_required(VERSION 3.15)

if(APPLE)
    find_library(COREFOUNDATION CoreFoundation)
    find_library(IOKIT IOKit)
endif()

# Set default values for firmware build options
include(${CMAKE_SOURCE_DIR}/cmake/obc_build_options.cmake)

include(${CMAKE_CURRENT_SOURCE_DIR}/shared/hal/cmake/board_lib_defs.cmake)

# Load setup-specific definitions
include(${CMAKE_SOURCE_DIR}/obc/shared/config/peripheral_config_definitions.cmake)

set(LD_DIR_PATH "${CMAKE_CURRENT_SOURCE_DIR}/shared/hal/common/ld")
set(APP_LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/shared/hal/common/ld/app_link.ld")
set(BL_LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/shared/hal/common/ld/bl_link.ld")

if (DEBUG MATCHES 1)
    set(TARGET_OPTIMIZATION_FLAGS "-Og" "-g" "-gdwarf-3" "-gstrict-dwarf")
else()
    set(TARGET_OPTIMIZATION_FLAGS "-O2")
endif()

# Define the bootloader bypass option
option(ENABLE_BL_BYPASS "Write directly to 0x0 and overwrite the bootloader" ON)

# Set the custom address based on the option
if(ENABLE_BL_BYPASS)
    set(CUSTOM_START_ADDRESS 0)
else()
    set(CUSTOM_START_ADDRESS 0x00040000)
endif()

# Pass the address to the linker
add_link_options(-Wl,--defsym,CUSTOM_START_ADDRESS=${CUSTOM_START_ADDRESS})

# Select main for build
if(${CMAKE_BUILD_TYPE} MATCHES OBC)
    set(OUT_FILE_NAME OBC-firmware.out)
    set(BIN_FILE_NAME OBC-firmware.bin)
    set(CRC_FILE_NAME OBC-firmware-crc.bin)
    add_compile_options(-Werror) # TODO: Move this back to download_arm_toolchain once all examples warnings are fixed
    add_executable(${OUT_FILE_NAME} app/app_main.c)
elseif(${CMAKE_BUILD_TYPE} MATCHES Examples)
    set(OUT_FILE_NAME OBC-firmware-${EXAMPLE_TYPE}.out)
    set(BIN_FILE_NAME OBC-firmware-${EXAMPLE_TYPE}.bin)
	if (${EXAMPLE_TYPE} MATCHES DMA_SPI)
		add_executable(${OUT_FILE_NAME} examples/dma_spi_demo/main.c)
	elseif(${EXAMPLE_TYPE} MATCHES FRAM_PERSIST)
		add_executable(${OUT_FILE_NAME} examples/test_app_fram_persist/main.c)
	elseif(${EXAMPLE_TYPE} MATCHES FRAM_SPI)
		add_executable(${OUT_FILE_NAME} examples/test_app_fram_spi/main.c)
	elseif(${EXAMPLE_TYPE} MATCHES LM75BD)
		add_executable(${OUT_FILE_NAME} examples/test_app_lm75bd/main.c)
	elseif(${EXAMPLE_TYPE} MATCHES MPU6050)
		add_executable(${OUT_FILE_NAME} examples/test_app_mpu6050/main.c examples/test_app_mpu6050/source/mpu6050.c)
		target_include_directories (${OUT_FILE_NAME} PRIVATE examples/test_app_mpu6050/include)
	elseif(${EXAMPLE_TYPE} MATCHES RE_SD)
		add_executable(${OUT_FILE_NAME} examples/test_app_reliance_edge_sd/main.c)
	elseif(${EXAMPLE_TYPE} MATCHES RTC)
		add_executable(${OUT_FILE_NAME} examples/test_app_rtc/main.c)
	elseif(${EXAMPLE_TYPE} MATCHES UART_RX)
		add_executable(${OUT_FILE_NAME} examples/test_app_uart_rx/main.c)
	elseif(${EXAMPLE_TYPE} MATCHES UART_TX)
		add_executable(${OUT_FILE_NAME} examples/test_app_uart_tx/main.c)
	elseif(${EXAMPLE_TYPE} MATCHES VN100)
		add_executable(${OUT_FILE_NAME} examples/vn100_demo/test_binary_reading/main.c)
    elseif(${EXAMPLE_TYPE} MATCHES ADC)
		add_executable(${OUT_FILE_NAME} examples/test_app_adc/main.c)
    elseif(${EXAMPLE_TYPE} MATCHES CC1120_SPI)
        add_executable(${OUT_FILE_NAME} examples/test_app_cc1120_spi/main.c examples/test_app_cc1120_spi/cc1120_spi_tests.c)
        target_include_directories (${OUT_FILE_NAME} PRIVATE examples/test_app_cc1120_spi)
    elseif(${EXAMPLE_TYPE} MATCHES RS)
        add_executable(${OUT_FILE_NAME} examples/test_app_rs/main.c)
    elseif(${EXAMPLE_TYPE} MATCHES ARDUCAM)
        add_executable(${OUT_FILE_NAME} examples/test_app_arducam/main.c)

  # ADD MORE EXAMPLES ABOVE THIS COMMENT
	else ()
		message (FATAL_ERROR "Invalid example type: ${EXAMPLE_TYPE} ")
	endif()
else()
	message (FATAL_ERROR "Invalid build type: ${CMAKE_BUILD_TYPE}")
endif()

add_executable(OBC-bl.out)
add_executable(debug-tool.out)

# Mapping the board for metadata
if (${BOARD_TYPE} MATCHES RM46_LAUNCHPAD)
    set(BOARD_TYPE_TO_INT_MAP 0)
elseif (${BOARD_TYPE} MATCHES OBC_REVISION_1)
    set(BOARD_TYPE_TO_INT_MAP 1)
elseif (${BOARD_TYPE} MATCHES OBC_REVISION_2)
    set(BOARD_TYPE_TO_INT_MAP 2)
# Add OBC_REVISION_3 or other boards above
else()
    message (FATAL_ERROR "BOARD NOT YET ADDED TO INT MAP FOR METADATA")
endif()

# Shared
add_subdirectory(shared/config)
add_subdirectory(shared/hal)
add_subdirectory(shared/metadata)
add_subdirectory(shared/obc_errors)
add_subdirectory(shared/commands)
add_subdirectory(shared/logging)

# Application
target_link_options(${OUT_FILE_NAME} PRIVATE
    -specs=nosys.specs
    -Wl,-Map=OBC-firmware.map -T${APP_LINKER_SCRIPT}
    -Wl,--print-memory-usage
    -Wl,-L${LD_DIR_PATH}
)

target_compile_options(${OUT_FILE_NAME} PRIVATE ${TARGET_OPTIMIZATION_FLAGS})

target_compile_definitions(${OUT_FILE_NAME} PRIVATE
    ${BOARD_TYPE}
    BOARD_ID=${BOARD_TYPE_TO_INT_MAP}
    LOG_DEFAULT_OUTPUT_LOCATION=${LOG_DEFAULT_OUTPUT_LOCATION}
    LOG_DEFAULT_LEVEL=${LOG_DEFAULT_LEVEL}
    LOG_DATE_TIME
    DEBUG=${DEBUG}
    OBC_ACTIVE_POLICY=${OBC_ACTIVE_POLICY}
    COMMS_PHY=${COMMS_PHY}
    OBC_UART_BAUD_RATE=${OBC_UART_BAUD_RATE}
    CSDC_DEMO_ENABLED=${CSDC_DEMO_ENABLED}
    ENABLE_TASK_STATS_COLLECTOR=${ENABLE_TASK_STATS_COLLECTOR}
    ENABLE_BL_BYPASS=${ENABLE_BL_BYPASS}
    ${PERIPHERAL_CONFIG}
)

# Determine the root of the repository
execute_process(
    COMMAND git rev-parse --show-toplevel
    OUTPUT_VARIABLE REPO_ROOT
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Set the SOURCE_PATH using the detected REPO_ROOT
add_compile_definitions(SOURCE_PATH="${REPO_ROOT}/")

target_include_directories(${OUT_FILE_NAME} PUBLIC
    ${CMAKE_BINARY_DIR}/toolchain/arm-none-eabi/include
)

add_subdirectory(app/hal)
add_subdirectory(app/modules)
add_subdirectory(app/rtos)
add_subdirectory(app/sys)
add_subdirectory(app/drivers)
add_subdirectory(app/reliance_edge)
target_link_libraries(${OUT_FILE_NAME} PRIVATE
    tiny-aes
    lib-correct
    obc-gs-interface
    attitude-control
    attitude-determination
    onboard-env-modelling
    ${HAL_LIB_OPTIMIZE}
    $<TARGET_OBJECTS:${HAL_LIB_NO_OPTIMIZE}>
)
target_link_libraries(${OUT_FILE_NAME} PRIVATE
    ${FREERTOS_LIB_OPTIMIZE}
    $<TARGET_OBJECTS:${FREERTOS_LIB_NO_OPTIMIZE}>
)

# Generate scheduler config
set(SCHEDULER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/app/rtos/gen_scheduler_config.py)
set(SCHEDULER_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/app/rtos/obc_scheduler_config.c)
set(SCHEDULER_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/app/rtos/obc_scheduler_config.h)
add_custom_target(generate-scheduler-config ALL
    COMMAND python3 ${SCHEDULER_SCRIPT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/app/rtos
    COMMENT "Generating OBC scheduler configuration"
)

add_dependencies(${OUT_FILE_NAME} generate-scheduler-config)

# Generate .bin for the app
string(CONCAT commentStr "Generating " ${BIN_FILE_NAME})
add_custom_command(
    OUTPUT ${BIN_FILE_NAME}
    COMMAND toolchain/bin/arm-none-eabi-objcopy -O binary --only-section=.metadata
                                                                     --only-section=.intvecs
                                                                     --only-section=.kernelTEXT
                                                                     --only-section=.text
                                                                     --only-section=.rodata
                                                                     --only-section=.ARM
                                                                     --only-section=.preinit_array
                                                                     --only-section=.init_array
                                                                     --only-section=.fini_array
                                                                     --only-section=.data
                                                                     --only-section=.crc
                                                                     ${OUT_FILE_NAME} ${BIN_FILE_NAME}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS ${OUT_FILE_NAME}
    COMMENT commentStr
)



add_custom_target(
    OBC_firmware-crc_bin
    ALL
    DEPENDS ${BIN_FILE_NAME}
)

# Find Python 3 interpreter
find_package(Python3 REQUIRED COMPONENTS Interpreter)

if(${CMAKE_BUILD_TYPE} MATCHES OBC)
	add_custom_target(firmware_crc ALL
			COMMAND "${Python3_EXECUTABLE}"
			"${CMAKE_SOURCE_DIR}/obc/tools/python/append_crc.py"
			"${CMAKE_BINARY_DIR}/${BIN_FILE_NAME}"
			"${CMAKE_BINARY_DIR}/${CRC_FILE_NAME}"
			DEPENDS "${CMAKE_BINARY_DIR}/${BIN_FILE_NAME}"
			COMMENT "Generating ${CRC_FILE_NAME} with CRC32 appended"
			VERBATIM
	)

endif()




# Bootloader
target_link_options(OBC-bl.out PRIVATE
    -specs=nosys.specs
    -Wl,-Map=OBC-bl.map -T${BL_LINKER_SCRIPT}
    -Wl,--print-memory-usage
    -Wl,-L${LD_DIR_PATH}
)

target_compile_options(OBC-bl.out PRIVATE ${TARGET_OPTIMIZATION_FLAGS})

target_compile_definitions(OBC-bl.out PRIVATE
    ${BOARD_TYPE}
    BOARD_ID=${BOARD_TYPE_TO_INT_MAP}
    LOG_DEFAULT_OUTPUT_LOCATION=${LOG_DEFAULT_OUTPUT_LOCATION}
    LOG_DEFAULT_LEVEL=${LOG_DEFAULT_LEVEL}
    DEBUG=${DEBUG}
    OBC_ACTIVE_POLICY=${OBC_ACTIVE_POLICY}
    COMMS_PHY=${COMMS_PHY}
    OBC_UART_BAUD_RATE=${OBC_UART_BAUD_RATE}
    CSDC_DEMO_ENABLED=${CSDC_DEMO_ENABLED}
    ENABLE_TASK_STATS_COLLECTOR=${ENABLE_TASK_STATS_COLLECTOR}
    ENABLE_BL_BYPASS=${ENABLE_BL_BYPASS}
    NO_FREERTOS
    ${PERIPHERAL_CONFIG}
)

add_subdirectory(bl)

target_link_libraries(OBC-bl.out PRIVATE
    ${HAL_LIB_OPTIMIZE}
    $<TARGET_OBJECTS:${HAL_LIB_NO_OPTIMIZE}>
    f021_api_archive
)

# Tools
target_link_options(debug-tool.out PRIVATE
    -specs=nosys.specs
    -Wl,-Map=debug-tool.map -T${APP_LINKER_SCRIPT}
    -Wl,-L${LD_DIR_PATH}
)

target_compile_options(debug-tool.out PRIVATE ${TARGET_OPTIMIZATION_FLAGS})

target_compile_definitions(debug-tool.out PRIVATE
    ${BOARD_TYPE}
    BOARD_ID=${BOARD_TYPE_TO_INT_MAP}
    LOG_DEFAULT_OUTPUT_LOCATION=${LOG_DEFAULT_OUTPUT_LOCATION}
    LOG_DEFAULT_LEVEL=${LOG_DEFAULT_LEVEL}
    LOG_DATE_TIME
    DEBUG=${DEBUG}
    OBC_ACTIVE_POLICY=${OBC_ACTIVE_POLICY}
    COMMS_PHY=${COMMS_PHY}
    OBC_UART_BAUD_RATE=${OBC_UART_BAUD_RATE}
    CSDC_DEMO_ENABLED=${CSDC_DEMO_ENABLED}
    ENABLE_TASK_STATS_COLLECTOR=${ENABLE_TASK_STATS_COLLECTOR}
    ${PERIPHERAL_CONFIG}
)

add_subdirectory(app/tools/interface_debug_tool)

target_link_libraries(debug-tool.out PRIVATE
    tiny-aes
    lib-correct
    obc-gs-interface
    attitude-control
    attitude-determination
    onboard-env-modelling
    ${HAL_LIB_OPTIMIZE}
    $<TARGET_OBJECTS:${HAL_LIB_NO_OPTIMIZE}>
)
target_link_libraries(debug-tool.out PRIVATE
    ${FREERTOS_LIB_OPTIMIZE}
    $<TARGET_OBJECTS:${FREERTOS_LIB_NO_OPTIMIZE}>
)

# Generate file with CMake metadata
file(WRITE ${CMAKE_BINARY_DIR}/OBC-metadata.bin
${BOARD_TYPE_TO_INT_MAP} "\n" )
