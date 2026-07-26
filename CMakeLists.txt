cmake_minimum_required(VERSION 3.15)
project(FlightDataRecorder LANGUAGES C CXX)

# ============================================================================
# Platform Detection
# ============================================================================

if(WIN32)
    set(PLATFORM_NAME "win_x64")
    set(BUILD_TYPE_DEFAULT "Release")
    set(NATIVE_LIB_SUFFIX ".dll")
    set(XPLANE_SDK_LIB_DIR "${CMAKE_SOURCE_DIR}/SDK/Libraries/Win")
elseif(APPLE)
    set(PLATFORM_NAME "mac_x64")
    set(BUILD_TYPE_DEFAULT "Release")
    set(NATIVE_LIB_SUFFIX ".dylib")
    set(XPLANE_SDK_LIB_DIR "${CMAKE_SOURCE_DIR}/SDK/Libraries/Mac")
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

# X-Plane requires specific definitions (per Laminar Research docs)
add_compile_definitions(
    XPLM200
    XPLM210
    XPLM300
    XPLM301
)

# Platform-specific definitions (REQUIRED by SDK headers)
if(APPLE)
    add_compile_definitions(
        APL=1
        __APPLE__
    )
    # Find required frameworks
    find_library(COCOA_LIBRARY Cocoa)
    find_library(OPENGL_LIBRARY OpenGL)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
elseif(UNIX AND NOT APPLE)
    add_compile_definitions(
        LIN=1
        __linux__
    )
elseif(WIN32)
    add_compile_definitions(
        IBM=1
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
# Platform-Specific Settings - Linking (per Laminar Research)
# ============================================================================

if(WIN32)
    # Windows: DLL settings
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS TRUE
    )
    
    # Link against Windows system libraries
    target_link_libraries(${PROJECT_NAME} PRIVATE
        winmm
        gdi32
    )
    
    # Link against X-Plane SDK libraries (64-bit)
    # Per Laminar: "Link against: XPWidgets_64.lib, XPLM_64.lib"
    target_link_libraries(${PROJECT_NAME} PRIVATE
        "${XPLANE_SDK_LIB_DIR}/XPLM_64.lib"
        "${XPLANE_SDK_LIB_DIR}/XPWidgets_64.lib"
    )
    
    # MSVC Runtime settings per Laminar:
    # "set the runtime to multi-threaded DLL, not multi-threaded"
    if(MSVC)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
    endif()

elseif(APPLE)
    # macOS: Link against X-Plane SDK frameworks
    # Per Laminar: "add XPLM.framework and XPWidgets.framework from the SDK"
    
    # Find X-Plane SDK frameworks
    find_library(XPLM_FRAMEWORK XPLM PATHS "${XPLANE_SDK_LIB_DIR}" NO_DEFAULT_PATH)
    find_library(XPWIDGETS_FRAMEWORK XPWidgets PATHS "${XPLANE_SDK_LIB_DIR}" NO_DEFAULT_PATH)
    
    # Link against X-Plane frameworks
    if(XPLM_FRAMEWORK)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${XPLM_FRAMEWORK})
    else()
        message(WARNING "XPLM.framework not found in ${XPLANE_SDK_LIB_DIR}")
    endif()
    
    if(XPWIDGETS_FRAMEWORK)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${XPWIDGETS_FRAMEWORK})
    else()
        message(WARNING "XPWidgets.framework not found in ${XPLANE_SDK_LIB_DIR}")
    endif()
    
    # Link against required system frameworks
    if(COCOA_LIBRARY)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${COCOA_LIBRARY})
    endif()
    if(OPENGL_LIBRARY)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${OPENGL_LIBRARY})
    endif()
    if(COREFOUNDATION_LIBRARY)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${COREFOUNDATION_LIBRARY})
    endif()

elseif(UNIX)
    # Linux: Link against standard libraries
    target_link_libraries(${PROJECT_NAME} PRIVATE
        dl
        pthread
        m
    )
    
    # Per Laminar Research: "pass -shared -rdynamic -nodefaultlibs -undefined_warning"
    # Note: -shared is implied by add_library(SHARED ...)
    # For modern GNU ld, use --warn-unresolved-symbols instead of -undefined_warning
    target_link_options(${PROJECT_NAME} PRIVATE
        -rdynamic
        -nodefaultlibs
        -Wl,--warn-unresolved-symbols
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
        # Per Laminar: "use -fvisibility=hidden for GCC4+"
        -fvisibility=hidden
        -fvisibility-inlines-hidden
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
message(STATUS "X-Plane SDK Lib Dir: ${XPLANE_SDK_LIB_DIR}")
message(STATUS "=============================================================================")
message(STATUS "")
