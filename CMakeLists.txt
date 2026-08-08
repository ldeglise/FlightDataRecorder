cmake_minimum_required(VERSION 3.10)
project(FlightDataRecorder)

# Chemin vers le SDK X-Plane
set(XPLM_SDK_PATH "SDK/")

# Inclure les en-têtes du SDK
include_directories(
    ${XPLM_SDK_PATH}/CHeaders/XPLM
    ${XPLM_SDK_PATH}/CHeaders/Widget
    ${XPLM_SDK_PATH}/CHeaders/Wrappers
)

# Ajouter le fichier source
add_library(FlightDataRecorder MODULE 
    main.cpp
    DataRefManager.cpp
    FlightDataCollector.cpp
    GeoJSONWriter.cpp
)

# Configuration pour Windows
if(WIN32)
    set(LIBRARY_DIR "${XPLM_SDK_PATH}/Libraries/Win")
    set_target_properties(FlightDataRecorder
        PROPERTIES
            PREFIX ""
            OUTPUT_NAME "fdr"
            SUFFIX ".xpl"
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/plugin/FlightDataRecorder/win_x64"
    )
    target_compile_definitions(FlightDataRecorder PUBLIC APL=0 IBM=1 LIN=0)
    target_link_libraries(FlightDataRecorder PRIVATE 
        "${CMAKE_SOURCE_DIR}/SDK/Libraries/Win/XPLM_64.dll"
        "${CMAKE_SOURCE_DIR}/SDK/Libraries/Win/XPWidgets_64.dll"
    )
endif()

# Configuration pour Linux
if(UNIX AND NOT APPLE)
    set(LIBRARY_DIR "${XPLM_SDK_PATH}/Libraries/Lin")   
    set_target_properties(FlightDataRecorder
        PROPERTIES
            PREFIX ""
            OUTPUT_NAME "fdr"
            SUFFIX ".xpl"
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/plugin/FlightDataRecorder/lin_x64"
    )
    target_compile_definitions(FlightDataRecorder PUBLIC APL=0 IBM=0 LIN=1)
    target_link_libraries(FlightDataRecorder PRIVATE 
        "${CMAKE_SOURCE_DIR}/SDK/Libraries/Lin/XPLM_64.so"
        "${CMAKE_SOURCE_DIR}/SDK/Libraries/Lin/XPWidgets_64.so"
    )
endif()

# Configuration pour macOS
if(APPLE)
    set(LIBRARY_DIR "${XPLM_SDK_PATH}/Libraries/Mac")
    set_target_properties(FlightDataRecorder
        PROPERTIES
            PREFIX ""
            OUTPUT_NAME "fdr"
            SUFFIX ".xpl"
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/plugin/FlightDataRecorder/mac_x64"
    )
    target_compile_definitions(FlightDataRecorder PUBLIC APL=1 IBM=0 LIN=0)
    target_link_libraries(FlightDataRecorder PRIVATE 
        "${CMAKE_SOURCE_DIR}/SDK/Libraries/Mac/XPLM.framework"
        "${CMAKE_SOURCE_DIR}/SDK/Libraries/Mac/XPWidgets.framework"
    )
endif()