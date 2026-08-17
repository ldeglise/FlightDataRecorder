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

# Activer C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ajouter le fichier source
add_library(FlightDataRecorder MODULE 
    main.cpp
    DataRefManager.cpp
    FlightDataCollector.cpp
)

# Configuration pour Windows
if(WIN32)
    set(LIBRARY_DIR "${XPLM_SDK_PATH}/Libraries/Win")
    set_target_properties(FlightDataRecorder
        PROPERTIES
            PREFIX ""
            OUTPUT_NAME "win"
            SUFFIX ".xpl"
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/plugin/FlightDataRecorder/64"
    )
    target_compile_definitions(FlightDataRecorder PUBLIC APL=0 IBM=1 LIN=0)
    # Lier statiquement les runtimes GCC/MinGW (libgcc, libstdc++, libwinpthread)
    # afin que win.xpl soit autonome et ne depende pas de libgcc_s_seh-1.dll,
    # libstdc++-6.dll ni libwinpthread-1.dll, qui peuvent etre absentes ou d'une
    # version incompatible sur le systeme de l'utilisateur (erreur X-Plane 127).
    target_link_options(FlightDataRecorder PRIVATE -static-libgcc -static-libstdc++ -static-libwinpthread -static)
    target_link_libraries(FlightDataRecorder PRIVATE 
        "${CMAKE_SOURCE_DIR}/SDK/Libraries/Win/XPLM_64.lib"
        "${CMAKE_SOURCE_DIR}/SDK/Libraries/Win/XPWidgets_64.lib"
    )
endif()

# Configuration pour Linux
if(UNIX AND NOT APPLE)
    set(LIBRARY_DIR "${XPLM_SDK_PATH}/Libraries/Lin")  
    set_target_properties(FlightDataRecorder
        PROPERTIES
            PREFIX ""
            OUTPUT_NAME "lin"
            SUFFIX ".xpl"
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/plugin/FlightDataRecorder/64"
    )
    target_compile_definitions(FlightDataRecorder PUBLIC APL=0 IBM=0 LIN=1)
    # Lier statiquement le runtime libstdc++ pour eviter tout conflit de version
    # avec la libstdc++.so.6 embarquee par X-Plane 12 sur Linux, qui peut etre
    # plus ancienne que celle du compilateur utilise pour le build.
    target_link_options(FlightDataRecorder PRIVATE -static-libgcc -static-libstdc++)
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
            OUTPUT_NAME "mac"
            SUFFIX ".xpl"
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/plugin/FlightDataRecorder/64"
    )
    target_compile_definitions(FlightDataRecorder PUBLIC APL=1 IBM=0 LIN=0)
    target_link_libraries(FlightDataRecorder PRIVATE 
        "${CMAKE_SOURCE_DIR}/SDK/Libraries/Mac/XPLM.framework"
        "${CMAKE_SOURCE_DIR}/SDK/Libraries/Mac/XPWidgets.framework"
    )
endif()
