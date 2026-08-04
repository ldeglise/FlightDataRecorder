cmake_minimum_required(VERSION 3.10)
project(FlightDataRecorder LANGUAGES CXX)

# --- Configuration des macros de compatibilité SDK ---
add_definitions(
    -DXPLM200=1    # Compatibilité X-Plane 10.50+
    -DXPLM210=1    # Compatibilité X-Plane 11.00+
    -DXPLM300=1    # Compatibilité X-Plane 11.50+ et 12+
    -DXPLM_USE_64_BIT=1  # Obligatoire pour X-Plane 11/12 (64-bit)
)

# --- Répertoire des headers du SDK X-Plane ---
# Spécifiez le chemin via -DXPLM_SDK_PATH=/chemin/vers/SDK lors de la configuration
# Exemple: cmake .. -DXPLM_SDK_PATH=/opt/XPLMSDK
set(XPLM_SDK_PATH "" CACHE PATH "Chemin vers le SDK X-Plane (obligatoire)")

if(NOT XPLM_SDK_PATH)
    message(FATAL_ERROR "Le chemin du SDK X-Plane (XPLM_SDK_PATH) est requis. Spécifiez-le avec -DXPLM_SDK_PATH=/chemin/vers/SDK")
endif()

# --- Inclusion des headers ---
include_directories(
    ${XPLM_SDK_PATH}/CHeaders
    ${XPLM_SDK_PATH}/Libraries
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# --- Fichiers sources ---
set(SOURCES
    main.cpp
    src/DataRefManager.cpp
    src/GeoJSONWriter.cpp
    src/FlightDataCollector.cpp
)

# --- Nom du plugin (conforme au SDK) ---
set(PLUGIN_NAME "FlightDataRecorder")

# --- Configuration spécifique à la plateforme ---
if(WIN32)
    # Windows: Génère un .xpl (DLL)
    add_library(${PLUGIN_NAME} SHARED ${SOURCES})
    set_target_properties(${PLUGIN_NAME} PROPERTIES
        SUFFIX ".xpl"
        OUTPUT_NAME "win_x64/${PLUGIN_NAME}"
    )
    target_link_libraries(${PLUGIN_NAME} XPLM_64)
elseif(APPLE)
    # macOS: Génère un .xpl (Bundle)
    add_library(${PLUGIN_NAME} SHARED ${SOURCES})
    set_target_properties(${PLUGIN_NAME} PROPERTIES
        SUFFIX ".xpl"
        OUTPUT_NAME "mac_x64/${PLUGIN_NAME}"
        MACOSX_BUNDLE TRUE
    )
    find_library(XPLM_FRAMEWORK XPLM HINTS ${XPLM_SDK_PATH}/Mac)
    target_link_libraries(${PLUGIN_NAME} ${XPLM_FRAMEWORK} CoreFoundation.framework OpenGL.framework)
elseif(UNIX AND NOT APPLE)
    # Linux: Génère un .xpl (so)
    add_library(${PLUGIN_NAME} SHARED ${SOURCES})
    set_target_properties(${PLUGIN_NAME} PROPERTIES
        SUFFIX ".xpl"
        OUTPUT_NAME "lin_x64/${PLUGIN_NAME}"
    )
    # Cherche la bibliothèque XPLM_64 dans le SDK
    find_library(XPLM_LIB XPLM_64 HINTS ${XPLM_SDK_PATH}/Linux ${XPLM_SDK_PATH}/Libraries/Linux)
    if(NOT XPLM_LIB)
        message(WARNING "XPLM_64 library not found. You may need to set XPLM_LIB manually or ensure the SDK is correctly installed.")
    else()
        target_link_libraries(${PLUGIN_NAME} ${XPLM_LIB})
    endif()
endif()

# --- Options de compilation ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)  # Pour les DLLs/so