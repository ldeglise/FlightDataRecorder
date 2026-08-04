// =============================================================================
// Définitions pour la compatibilité SDK X-Plane
// =============================================================================
#define XPLM200 1    // Compatibilité X-Plane 10.50+
#define XPLM210 1    // Compatibilité X-Plane 11.00+
#define XPLM300 1    // Compatibilité X-Plane 11.50+ et 12+
#define XPLM_USE_64_BIT 1  // Obligatoire pour X-Plane 11/12 (64-bit)

// =============================================================================
// Includes SDK X-Plane
// =============================================================================
#include <XPLMPlugin.h>
#include <XPLMProcessing.h>
#include <string>

// =============================================================================
// Forward Declarations (interfaces des classes métiers)
// =============================================================================
class DataRefManager;
class GeoJSONWriter;
class FlightDataCollector;

// =============================================================================
// Constantes du plugin
// =============================================================================
static const char* PLUGIN_NAME         = "Flight Data Recorder";
static const char* PLUGIN_SIGNATURE    = "xplane.flight_data_recorder";
static const char* PLUGIN_DESCRIPTION  = "To do";

// =============================================================================
// Variables globales (UNIQUEMENT pour le SDK)
// =============================================================================
static XPLMFlightLoopID gFlightLoopId = nullptr;
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

    gFlightLoopId = XPLMRegisterFlightLoopCallback(FlightLoopCallback, 1.0f, nullptr);
    if (!gFlightLoopId) {
        XPLMDebugString("Flight Data Recorder: Impossible de créer la boucle de vol.\n");
        return 0;
    }

    XPLMDebugString("Flight Data Recorder: Plugin initialisé.\n");
    return 1;
}

// =============================================================================
// XPluginEnable
// =============================================================================
PLUGIN_API int XPluginEnable(void) {
    if (!gFlightLoopId) {
        gFlightLoopId = XPLMRegisterFlightLoopCallback(FlightLoopCallback, 1.0f, nullptr);
        if (!gFlightLoopId) return 0;
    }
    return 1;
}

// =============================================================================
// XPluginDisable
// =============================================================================
PLUGIN_API void XPluginDisable(void) {
    if (gFlightLoopId) {
        XPLMUnregisterFlightLoopCallback(gFlightLoopId, nullptr);
        gFlightLoopId = nullptr;
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