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
# Le SDK est attendu dans un dossier "SDK/" relatif à la racine du projet
set(XPLM_SDK_PATH "${CMAKE_CURRENT_SOURCE_DIR}/SDK" CACHE PATH "Chemin vers le SDK X-Plane")

# Vérifie que le SDK existe
if(NOT EXISTS "${XPLM_SDK_PATH}/CHeaders/XPLM/XPLMPlugin.h")
    message(FATAL_ERROR "Le SDK X-Plane est introuvable dans : ${XPLM_SDK_PATH}. Placez-le dans un dossier SDK/ à la racine du projet.")
endif()

# --- Inclusion des headers ---
include_directories(
    ${XPLM_SDK_PATH}/CHeaders/XPLM
    ${XPLM_SDK_PATH}/CHeaders/Widgets
    ${XPLM_SDK_PATH}/CHeaders/Wrappers
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
    find_library(XPLM_LIB XPLM_64 HINTS ${XPLM_SDK_PATH}/Libraries/Win)
    if(NOT XPLM_LIB)
        message(FATAL_ERROR "XPLM_64 library not found in ${XPLM_SDK_PATH}/Libraries/Win. Ensure the SDK is correctly installed.")
    endif()
    target_link_libraries(${PLUGIN_NAME} ${XPLM_LIB})
elseif(APPLE)
    # macOS: Génère un .xpl (Bundle)
    add_library(${PLUGIN_NAME} SHARED ${SOURCES})
    set_target_properties(${PLUGIN_NAME} PROPERTIES
        SUFFIX ".xpl"
        OUTPUT_NAME "mac_x64/${PLUGIN_NAME}"
        MACOSX_BUNDLE TRUE
    )
    find_library(XPLM_FRAMEWORK XPLM HINTS ${XPLM_SDK_PATH}/Libraries/Mac)
    if(NOT XPLM_FRAMEWORK)
        message(FATAL_ERROR "XPLM framework not found in ${XPLM_SDK_PATH}/Libraries/Mac. Ensure the SDK is correctly installed.")
    endif()
    target_link_libraries(${PLUGIN_NAME} ${XPLM_FRAMEWORK} CoreFoundation.framework OpenGL.framework)
elseif(UNIX AND NOT APPLE)
    # Linux: Génère un .xpl (so)
    add_library(${PLUGIN_NAME} SHARED ${SOURCES})
    set_target_properties(${PLUGIN_NAME} PROPERTIES
        SUFFIX ".xpl"
        OUTPUT_NAME "lin_x64/${PLUGIN_NAME}"
    )
    find_library(XPLM_LIB XPLM_64 HINTS ${XPLM_SDK_PATH}/Libraries/Lin)
    if(NOT XPLM_LIB)
        message(FATAL_ERROR "XPLM_64 library not found in ${XPLM_SDK_PATH}/Libraries/Lin. Ensure the SDK is correctly installed.")
    endif()
    target_link_libraries(${PLUGIN_NAME} ${XPLM_LIB})
endif()

# --- Options de compilation ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)  # Pour les DLLs/so