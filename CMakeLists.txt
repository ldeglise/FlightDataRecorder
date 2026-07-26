cmake_minimum_required(VERSION 3.15)
project(FlightDataRecorder LANGUAGES C CXX)

# ============================================================================
# Platform Detection
# ============================================================================

if(WIN32)
    set(PLATFORM_NAME "win_x64")
    set(BUILD_TYPE_DEFAULT "Release")
    set(NATIVE_LIB_SUFFIX ".dll")
elseif(APPLE)
    set(PLATFORM_NAME "mac_x64")
    set(BUILD_TYPE_DEFAULT "Release")
    set(NATIVE_LIB_SUFFIX ".dylib")
elseif(UNIX)
    set(PLATFORM_NAME "lin_x64")
    set(BUILD_TYPE_DEFAULT "Release")
    set(NATIVE_LIB_SUFFIX ".so")
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE ${BUILD_TYPE_DEFAULT})
endif()

# ============================================================================
# Output Directories
# ============================================================================

# Create platform-specific output directory
set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/plugins/FlightDataRecorder/${PLATFORM_NAME}")

# Ensure output directory exists
file(MAKE_DIRECTORY ${OUTPUT_DIR})

# Set library output directory
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})

# ============================================================================
# Include Directories
# ============================================================================

# Add main directories - compiler will search recursively in these
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/SDK
    ${CMAKE_SOURCE_DIR}/SDK/CHeaders
    ${CMAKE_SOURCE_DIR}/SDK/CHeaders/Widgets
    ${CMAKE_SOURCE_DIR}/SDK/CHeaders/Wrappers
    ${CMAKE_SOURCE_DIR}/SDK/CHeaders/XPLM
)

# ============================================================================
# Source Files
# ============================================================================

# Main source file (adjust as needed)
set(SOURCES
    ${CMAKE_SOURCE_DIR}/src/main.cpp
)

# Find all source files recursively in src/
file(GLOB_RECURSE SOURCE_FILES 
    ${CMAKE_SOURCE_DIR}/src/*.cpp
    ${CMAKE_SOURCE_DIR}/src/*.c
    ${CMAKE_SOURCE_DIR}/src/*.h
)

# Combine sources
list(APPEND SOURCES ${SOURCE_FILES})

# Remove duplicates
list(REMOVE_DUPLICATES SOURCES)

# ============================================================================
# X-Plane SDK Configuration
# ============================================================================

# X-Plane requires specific definitions
add_compile_definitions(
    XPLM200
    XPLM210
    XPLM300
    XPLM301
)

# Platform-specific definitions
if(APPLE)
    add_compile_definitions(
        APL
        __APPLE__
    )
    # Find Cocoa framework for macOS
    find_library(COCOA_LIBRARY Cocoa)
    find_library(CARBON_LIBRARY Carbon)
    find_library(IOKIT_LIBRARY IOKit)
elseif(UNIX AND NOT APPLE)
    add_compile_definitions(
        LIN
        __linux__
    )
elseif(WIN32)
    add_compile_definitions(
        IBM
        _WIN32
        _WINDOWS
        _WIN32_WINNT=0x0601
    )
endif()

# ============================================================================
# Library Type Configuration
# ============================================================================

# Create the shared library - CMake will add native suffix (.so, .dll, .dylib)
add_library(${PROJECT_NAME} SHARED ${SOURCES})

# Set output name without prefix
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "FlightDataRecorder"
    PREFIX ""
)

# Define files for rename command
set(BASE_NAME "FlightDataRecorder")
set(SOURCE_LIB "${OUTPUT_DIR}/${BASE_NAME}${NATIVE_LIB_SUFFIX}")
set(TARGET_LIB "${OUTPUT_DIR}/${BASE_NAME}.xpl")

# ============================================================================
# Platform-Specific Settings
# ============================================================================

if(WIN32)
    # Windows: DLL settings
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS TRUE
    )
    
    # Link against Windows libraries
    target_link_libraries(${PROJECT_NAME} PRIVATE
        winmm
        gdi32
    )

elseif(APPLE)
    # macOS: Link against frameworks
    if(COCOA_LIBRARY)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${COCOA_LIBRARY})
    endif()
    if(CARBON_LIBRARY)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${CARBON_LIBRARY})
    endif()
    if(IOKIT_LIBRARY)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${IOKIT_LIBRARY})
    endif()

elseif(UNIX)
    # Linux: Link against standard libraries
    target_link_libraries(${PROJECT_NAME} PRIVATE
        dl
        pthread
        m
    )
endif()

# Post-build: Rename native library to .xpl and clean up
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove -f "${OUTPUT_DIR}/${BASE_NAME}"
    COMMAND ${CMAKE_COMMAND} -E remove -f "${TARGET_LIB}"
    COMMAND ${CMAKE_COMMAND} -E rename "${SOURCE_LIB}" "${TARGET_LIB}"
    COMMENT "Renaming ${NATIVE_LIB_SUFFIX} to .xpl for X-Plane"
)

# ============================================================================
# Compiler Flags
# ============================================================================

# C++ standard (X-Plane SDK typically uses C++11 or later)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Common compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall
        -Wextra
        -Wno-unused-parameter
        -Wno-unused-variable
        -fPIC
    )
endif()

if(MSVC)
    add_compile_options(
        /W4
        /D_CRT_SECURE_NO_WARNINGS
    )
    # Disable specific warnings that X-Plane SDK might trigger
    add_compile_options(
        /wd4100  # unreferenced parameter
        /wd4189  # local variable is initialized but not referenced
        /wd4996  # function may be unsafe
        /wd4251  # class needs to have dll-interface
    )
endif()

# ============================================================================
# Print Configuration Summary
# ============================================================================

message(STATUS "")
message(STATUS "=============================================================================")
message(STATUS "FlightDataRecorder - X-Plane Plugin Configuration")
message(STATUS "=============================================================================")
message(STATUS "Platform: ${PLATFORM_NAME}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Output Directory: ${OUTPUT_DIR}")
message(STATUS "Output File: ${TARGET_LIB}")
message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "=============================================================================")
message(STATUS "")
