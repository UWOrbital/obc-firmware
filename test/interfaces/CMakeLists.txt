set(TEST_BINARY obc-interface-tests)

set(TEST_DEPENDENCIES

)

set(TEST_MOCKS
    ${CMAKE_SOURCE_DIR}/test/mocks/mock_heap.c
)

set(TEST_SOURCES
    ${CMAKE_SOURCE_DIR}/test/interfaces/main.cpp
    ${CMAKE_SOURCE_DIR}/test/interfaces/test_pack_unpack_utils.cpp
    ${CMAKE_SOURCE_DIR}/test/interfaces/test_command_pack_unpack.cpp
    ${CMAKE_SOURCE_DIR}/test/interfaces/test_telemetry_pack_unpack.cpp
    ${CMAKE_SOURCE_DIR}/test/interfaces/test_obc_ax25.cpp
    ${CMAKE_SOURCE_DIR}/test/interfaces/test_obc_fec.cpp
    ${CMAKE_SOURCE_DIR}/test/interfaces/test_command_response_pack_unpack.cpp
    ${CMAKE_SOURCE_DIR}/test/interfaces/test_encode_decode_pipeline.cpp
)

set(TEST_SOURCES ${TEST_SOURCES} ${TEST_DEPENDENCIES} ${TEST_MOCKS})

add_executable(${TEST_BINARY} ${TEST_SOURCES})

target_include_directories(${TEST_BINARY}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/obc/shared/logging
)

target_link_libraries(${TEST_BINARY}
    PRIVATE
    GTest::GTest
    lib-correct
    obc-gs-interface
)

add_test(${TEST_BINARY} ${TEST_BINARY})
