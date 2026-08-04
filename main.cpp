// =============================================================================
// Includes SDK X-Plane
// =============================================================================
#include <XPLMDefs.h>     // Doit être inclus en premier pour définir PLUGIN_API
#include <XPLMPlugin.h>
#include <XPLMProcessing.h>
#include <XPLMUtilities.h> // Pour XPLMDebugString
#include <string>
#include <cstring>        // Pour strncpy

// =============================================================================
// Includes des classes métiers
// =============================================================================
#include "DataRefManager.hpp"
#include "GeoJSONWriter.hpp"
#include "FlightDataCollector.hpp"

// =============================================================================
// Constantes du plugin
// =============================================================================
static const char* PLUGIN_NAME         = "Flight Data Recorder";
static const char* PLUGIN_SIGNATURE    = "xplane.flight_data_recorder";
static const char* PLUGIN_DESCRIPTION  = "To do";

// =============================================================================
// Variables globales (UNIQUEMENT pour le SDK)
// =============================================================================
static bool gFlightLoopRegistered = false;
static DataRefManager*     gDataRefManager     = nullptr;
static GeoJSONWriter*      gGeoJSONWriter      = nullptr;
static FlightDataCollector* gFlightDataCollector = nullptr;

// =============================================================================
// Prototypes des fonctions OBLIGATOIRES (SDK X-Plane)
// =============================================================================
PLUGIN_API int  XPluginStart(char* outName, char* outSig, char* outDesc);
PLUGIN_API void XPluginDisable(void);
PLUGIN_API int  XPluginEnable(void);
PLUGIN_API void XPluginStop(void);
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void* inParam);

// =============================================================================
// Callback de la boucle de vol (délégué à FlightDataCollector)
// =============================================================================
float FlightLoopCallback(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter, void* inRefcon) {
    if (gFlightDataCollector) {
        gFlightDataCollector->Update(inElapsedSinceLastCall);
    }
    return 1.0f; // Intervalle de rappel (1 seconde)
}

// =============================================================================
// XPluginStart : Initialisation du plugin
// =============================================================================
PLUGIN_API int XPluginStart(char* outName, char* outSig, char* outDesc) {
    strncpy(outName, PLUGIN_NAME, 255);
    strncpy(outSig, PLUGIN_SIGNATURE, 255);
    strncpy(outDesc, PLUGIN_DESCRIPTION, 255);

    // Instanciation des classes métiers
    gDataRefManager     = new (std::nothrow) DataRefManager();
    gGeoJSONWriter      = new (std::nothrow) GeoJSONWriter("flight_data.geojson");
    gFlightDataCollector = new (std::nothrow) FlightDataCollector(gDataRefManager, gGeoJSONWriter);

    if (!gDataRefManager || !gGeoJSONWriter || !gFlightDataCollector) {
        XPLMDebugString("Flight Data Recorder: Erreur d'allocation mémoire.\n");
        return 0;
    }

    if (!gDataRefManager->Initialize() || !gGeoJSONWriter->Initialize() || !gFlightDataCollector->Initialize()) {
        XPLMDebugString("Flight Data Recorder: Échec de l'initialisation.\n");
        return 0;
    }

    XPLMRegisterFlightLoopCallback(FlightLoopCallback, 1.0f, nullptr);
    gFlightLoopRegistered = true;

    XPLMDebugString("Flight Data Recorder: Plugin initialisé.\n");
    return 1;
}

// =============================================================================
// XPluginEnable
// =============================================================================
PLUGIN_API int XPluginEnable(void) {
    if (!gFlightLoopRegistered) {
        XPLMRegisterFlightLoopCallback(FlightLoopCallback, 1.0f, nullptr);
        gFlightLoopRegistered = true;
    }
    return 1;
}

// =============================================================================
// XPluginDisable
// =============================================================================
PLUGIN_API void XPluginDisable(void) {
    if (gFlightLoopRegistered) {
        XPLMUnregisterFlightLoopCallback(FlightLoopCallback, nullptr);
        gFlightLoopRegistered = false;
    }
}

// =============================================================================
// XPluginStop : Nettoyage des ressources
// =============================================================================
PLUGIN_API void XPluginStop(void) {
    XPluginDisable();

    if (gFlightDataCollector) {
        gFlightDataCollector->Cleanup();
        delete gFlightDataCollector;
        gFlightDataCollector = nullptr;
    }
    if (gGeoJSONWriter) {
        gGeoJSONWriter->Cleanup();
        delete gGeoJSONWriter;
        gGeoJSONWriter = nullptr;
    }
    if (gDataRefManager) {
        gDataRefManager->Cleanup();
        delete gDataRefManager;
        gDataRefManager = nullptr;
    }

    XPLMDebugString("Flight Data Recorder: Plugin arrêté.\n");
}

// =============================================================================
// XPluginReceiveMessage
// =============================================================================
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void* inParam) {
    // Réservé pour une future utilisation
}