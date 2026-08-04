cmake_minimum_required(VERSION 3.10)
project(FlightDataRecorder LANGUAGES CXX)

# --- Configuration des macros de compatibilité SDK ---
add_definitions(
    -DXPLM200=1    # Compatibilité X-Plane 10.50+
    -DXPLM210=1    # Compatibilité X-Plane 11.00+
    -DXPLM300=1    # Compatibilité X-Plane 11.50+ et 12+
    -DXPLM_USE_64_BIT=1  # Obligatoire pour X-Plane 11/12 (64-bit)
)

# --- Vérification du SDK X-Plane ---
# Le SDK doit être placé dans un dossier "SDK/" à la racine du projet.
# Structure attendue:
#   SDK/
#   ├── CHeaders/
#   │   ├── XPLM/
#   │   │   ├── XPLMPlugin.h
#   │   │   ├── XPLMDebug.h
#   │   │   └── ...
#   │   ├── Widgets/
#   │   └── Wrappers/
#   └── Libraries/
#       ├── Lin/      (libXPLM_64.so)
#       ├── Mac/      (XPLM.framework)
#       └── Win/      (XPLM_64.dll)

set(SDK_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/SDK")

# Vérifie que les headers XPLM existent
if(NOT EXISTS "${SDK_ROOT}/CHeaders/XPLM/XPLMPlugin.h")
    message(FATAL_ERROR 
        "Le SDK X-Plane est introuvable !\n"
        "Placez le dossier SDK (contenant CHeaders/ et Libraries/) à la racine du projet.\n"
        "Structure attendue: ${SDK_ROOT}/CHeaders/XPLM/XPLMPlugin.h"
    )
endif()

# --- Inclusion des headers ---
include_directories(
    ${SDK_ROOT}/CHeaders/XPLM
    ${SDK_ROOT}/CHeaders/Widgets
    ${SDK_ROOT}/CHeaders/Wrappers
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# --- Fichiers sources ---
set(SOURCES
    main.cpp
    src/DataRefManager.cpp
    src/GeoJSONWriter.cpp
    src/FlightDataCollector.cpp
)

# --- Nom du plugin ---
set(PLUGIN_NAME "FlightDataRecorder")

# --- Configuration spécifique à la plateforme ---
if(WIN32)
    # Windows: Génère un .xpl (DLL)
    add_library(${PLUGIN_NAME} SHARED ${SOURCES})
    set_target_properties(${PLUGIN_NAME} PROPERTIES
        SUFFIX ".xpl"
        OUTPUT_NAME "win_x64/${PLUGIN_NAME}"
    )
    # Cherche la bibliothèque Windows
    find_library(XPLM_LIB XPLM_64 HINTS "${SDK_ROOT}/Libraries/Win")
    if(NOT XPLM_LIB)
        message(FATAL_ERROR 
            "Bibliothèque XPLM_64 introuvable pour Windows !\n"
            "Vérifiez que ${SDK_ROOT}/Libraries/Win/XPLM_64.dll existe."
        )
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
    # Cherche le framework macOS
    find_library(XPLM_FRAMEWORK XPLM HINTS "${SDK_ROOT}/Libraries/Mac")
    if(NOT XPLM_FRAMEWORK)
        message(FATAL_ERROR 
            "Framework XPLM introuvable pour macOS !\n"
            "Vérifiez que ${SDK_ROOT}/Libraries/Mac/XPLM.framework existe."
        )
    endif()
    target_link_libraries(${PLUGIN_NAME} ${XPLM_FRAMEWORK} CoreFoundation.framework OpenGL.framework)

elseif(UNIX AND NOT APPLE)
    # Linux: Génère un .xpl (so)
    add_library(${PLUGIN_NAME} SHARED ${SOURCES})
    set_target_properties(${PLUGIN_NAME} PROPERTIES
        SUFFIX ".xpl"
        OUTPUT_NAME "lin_x64/${PLUGIN_NAME}"
    )
    # Cherche la bibliothèque Linux
    find_library(XPLM_LIB XPLM_64 HINTS "${SDK_ROOT}/Libraries/Lin")
    if(NOT XPLM_LIB)
        message(FATAL_ERROR 
            "Bibliothèque XPLM_64 introuvable pour Linux !\n"
            "Vérifiez que ${SDK_ROOT}/Libraries/Lin/libXPLM_64.so existe."
        )
    endif()
    target_link_libraries(${PLUGIN_NAME} ${XPLM_LIB})
endif()

# --- Options de compilation ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)  # Pour les DLLs/so
