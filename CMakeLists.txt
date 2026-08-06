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
add_library(FlightDataRecorder MODULE main.cpp)

# Configuration pour Windows
if(WIN32)
    set(LIBRARY_DIR "${XPLM_SDK_PATH}/Libraries/win")
    set_target_properties(FlightDataRecorder
        PROPERTIES
            PREFIX ""
            OUTPUT_NAME "fdr"
            SUFFIX ".xpl"
            LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_ROOT}/plugin/FlightDataRecorder/win_X64"
    )
    target_link_libraries(FlightDataRecorder PRIVATE XPLM_64 XPWidgets_64)
endif()

# Configuration pour Linux
if(UNIX AND NOT APPLE)
    set(LIBRARY_DIR "${XPLM_SDK_PATH}/Libraries/lin")
    set_target_properties(FlightDataRecorder
        PROPERTIES
            PREFIX ""
            OUTPUT_NAME "fdr"
            SUFFIX ".xpl"
            LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_ROOT}/plugin/FlightDataRecorder/lin_X64"
    )
    target_link_libraries(FlightDataRecorder PRIVATE XPLM_64 XPWidgets_64)
endif()

# Configuration pour macOS
if(APPLE)
    set(LIBRARY_DIR "${XPLM_SDK_PATH}/Libraries/mac")
    set_target_properties(FlightDataRecorder
        PROPERTIES
            PREFIX ""
            OUTPUT_NAME "fdr"
            SUFFIX ".xpl"
            LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_ROOT}/plugin/FlightDataRecorder/mac_X64"
    )
    target_link_libraries(FlightDataRecorder PRIVATE XPLM XPWidgets)
endif()