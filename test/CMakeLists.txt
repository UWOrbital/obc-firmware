cmake_minimum_required(VERSION 3.15)

# Determine the root of the repository
execute_process(
    COMMAND git rev-parse --show-toplevel
    OUTPUT_VARIABLE REPO_ROOT
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Set the SOURCE_PATH using the detected REPO_ROOT
add_compile_definitions(SOURCE_PATH="${REPO_ROOT}/")

add_subdirectory(interfaces)
add_subdirectory(unit)

# TODO: uncomment once there's at least 1 test
# add_subdirectory(test_gs/unit)
