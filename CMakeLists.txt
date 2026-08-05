cmake_minimum_required(VERSION 3.10)
project(FlightDataRecorder)

# Chemin vers le SDK X-Plane (à adapter selon votre installation)
set(XPLM_SDK_PATH "SDK/")

# Inclure les en-têtes du SDK
include_directories(
    ${XPLM_SDK_PATH}/CHeaders/XPLM
    ${XPLM_SDK_PATH}/CHeaders/Widget
)

# Ajouter le fichier source
add_library(FlightDataRecorder MODULE main.cpp)

# Configuration pour Windows
if(WIN32)
    set_target_properties(FlightDataRecorder PROPERTIES SUFFIX ".xpl")
    target_link_libraries(FlightDataRecorder PRIVATE XPLM_64)
endif()

# Configuration pour Linux
if(UNIX AND NOT APPLE)
    set_target_properties(FlightDataRecorder PROPERTIES SUFFIX ".xpl")
    target_link_libraries(FlightDataRecorder PRIVATE XPLM)
endif()

# Configuration pour macOS
if(APPLE)
    set_target_properties(FlightDataRecorder PROPERTIES SUFFIX ".xpl")
    target_link_libraries(FlightDataRecorder PRIVATE XPLM)
endif()