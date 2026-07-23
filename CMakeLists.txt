cmake_minimum_required(VERSION 3.10)

project(FlightDataRecorder)

# Output directories configuration
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/release)

# For other build types (RelWithDebInfo, MinSizeRel) - default to release
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/bin/release)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_BINARY_DIR}/bin/release)

# Create output directories
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin/debug)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin/release)

# Add executable
add_executable(${PROJECT_NAME} src/main.cpp)