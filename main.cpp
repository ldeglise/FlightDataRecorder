#define XPLM430
#define XPLM420
#define XPLM411
#define XPLM400
#define XPLM303
#define XPLM301
#define XPLM300
#define XPLM210
#define XPLM200

#include <stdio.h>
#include <string.h>
#include <string>
#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"
#include "XPLMUtilities.h"
#include "DataRefManager.hpp"
#include "FlightDataCollector.hpp"

// Instances globales
static DataRefManager dataRefManager;
static FlightDataCollector flightDataCollector;

static float FlightDataRecorderCallback(
    float inElapsedSinceLastCall,
    float inElapsedTimeSinceLastFlightLoop,
    int inCounter,
    void *inRefcon) {

    try {
        // Collecter les données (gère tout : détection, écriture, etc.)
        flightDataCollector.collectData(dataRefManager);
    } catch (const std::exception& e) {
        XPLMDebugString("FlightDataRecorder: Error - ");
        XPLMDebugString(e.what());
        XPLMDebugString("\n");
    }

    return 1.0; // Rappel toutes les 1 seconde
}

PLUGIN_API int XPluginStart(
    char *outName,
    char *outSig,
    char *outDesc) {
    strcpy(outName, "Flight Data Recorder v1.0");
    strcpy(outSig, "conconlolo.FlightDataRecorder");
    strcpy(outDesc, "A plugin that records flight data and writes to GeoJSON.");

    XPLMRegisterFlightLoopCallback(
        FlightDataRecorderCallback,
        1.0,  // Intervalle de 1 seconde
        NULL
    );
    return 1;
}

PLUGIN_API void XPluginStop(void) {
    XPLMUnregisterFlightLoopCallback(FlightDataRecorderCallback, NULL);
    // Fermer proprement le fichier en cours si nécessaire
    // (Le destructeur de FlightDataCollector le fera)
}

PLUGIN_API void XPluginDisable(void) {
    // Fermer le fichier en cours
    // (Le destructeur de FlightDataCollector le fera)
}

PLUGIN_API int XPluginEnable(void) {
    return 1;
}

PLUGIN_API void XPluginReceiveMessage(
    XPLMPluginID inFromWho,
    int inMessage,
    void *inParam) {
}
