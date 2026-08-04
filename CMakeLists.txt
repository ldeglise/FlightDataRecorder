cmake_minimum_required(VERSION 3.10)
project(FlightDataRecorder LANGUAGES CXX)

# --- Définition de la plateforme pour le SDK X-Plane (DOIT être avant les includes) ---
# Le SDK X-Plane utilise APL, IBM, LIN pour définir PLUGIN_API et XPLM_API
if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DIBM=1")
elif(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DAPL=1")
elif(UNIX AND NOT APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLIN=1")
endif()

# --- Configuration des macros de compatibilité SDK ---
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DXPLM200=1 -DXPLM210=1 -DXPLM300=1 -DXPLM_USE_64_BIT=1")

# --- Vérification du SDK X-Plane ---
# Le SDK doit être placé dans un dossier "SDK/" à la racine du projet.
# Structure attendue:
#   SDK/
#   ├── CHeaders/
#   │   ├── XPLM/
#   │   │   ├── XPLMPlugin.h
#   │   │   ├── XPLMDefs.h
#   │   │   ├── XPLMUtilities.h
#   │   │   └── ...
#   │   ├── Widgets/
#   │   └── Wrappers/
#   └── Libraries/
#       ├── Lin/      (XPLM_64.so)
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
    target_compile_options(${PLUGIN_NAME} PRIVATE -DIBM=1)
    # Cherche la bibliothèque Windows (nom réel: XPLM_64.dll)
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
    target_compile_options(${PLUGIN_NAME} PRIVATE -DAPL=1)
    # Cherche la bibliothèque macOS (nom réel: XPLM_64)
    find_library(XPLM_LIB XPLM_64 HINTS "${SDK_ROOT}/Libraries/Mac")
    if(NOT XPLM_LIB)
        message(FATAL_ERROR 
            "Bibliothèque XPLM_64 introuvable pour macOS !\n"
            "Vérifiez que ${SDK_ROOT}/Libraries/Mac/XPLM_64 existe."
        )
    endif()
    target_link_libraries(${PLUGIN_NAME} ${XPLM_LIB} CoreFoundation.framework OpenGL.framework)

elseif(UNIX AND NOT APPLE)
    # Linux: Génère un .xpl (so)
    add_library(${PLUGIN_NAME} SHARED ${SOURCES})
    set_target_properties(${PLUGIN_NAME} PROPERTIES
        SUFFIX ".xpl"
        OUTPUT_NAME "lin_x64/${PLUGIN_NAME}"
    )
    target_compile_options(${PLUGIN_NAME} PRIVATE -DLIN=1)
    # Cherche la bibliothèque Linux (nom réel: XPLM_64.so, sans préfixe 'lib')
    # Désactive le préfixe automatique 'lib' pour find_library
    set(CMAKE_FIND_LIBRARY_PREFIXES "" "lib")
    find_library(XPLM_LIB XPLM_64 HINTS "${SDK_ROOT}/Libraries/Lin")
    set(CMAKE_FIND_LIBRARY_PREFIXES "lib")
    if(NOT XPLM_LIB)
        message(WARNING 
            "Bibliothèque XPLM_64 introuvable pour Linux !\n"
            "Vérifiez que ${SDK_ROOT}/Libraries/Lin/XPLM_64.so existe.\n"
            "La compilation des sources continuera, mais le lien final échouera."
        )
    else()
        target_link_libraries(${PLUGIN_NAME} ${XPLM_LIB})
    endif()
endif()

# --- Options de compilation ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)  # Pour les DLLs/so
