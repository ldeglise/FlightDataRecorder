#define XPLM430
#define XPLM420
#define XPLM411
#define XPLM400

#include <stdio.h>
#include <string.h>
#include <string>
#include <algorithm>
#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"
#include "XPLMUtilities.h"
#include "DataRefManager.hpp"
#include "FlightDataCollector.hpp"
#include "GeoJSONWriter.hpp"

// Instances globales
static DataRefManager dataRefManager;
static FlightDataCollector flightDataCollector;
static GeoJSONWriter geoJSONWriter;
static int callbackCounter = 0;
static std::string currentFilePath; // Pour suivre le fichier en cours

static float FlightDataRecorderCallback(
    float inElapsedSinceLastCall,
    float inElapsedTimeSinceLastFlightLoop,
    int inCounter,
    void *inRefcon) {

    try {
        // 1. Collecter les données
        flightDataCollector.collectData(dataRefManager);

        // 2. Écrire sur disque toutes les 20s (si en vol)
        if (flightDataCollector.isFlightActive()) {
            if (callbackCounter % 20 == 0) {
                // Générer le nom de fichier (1x au décollage)
                if (currentFilePath.empty()) {
                    std::string dir = flightDataCollector.getOutputDirectory();
                    std::string prefix = flightDataCollector.generateRandomPrefix();
                    std::string timestamp = flightDataCollector.getMetadata().takeoff_time;
                    // Remplacer les ":" par "-" pour le nom de fichier
                    std::string safeTimestamp = timestamp;
                    std::replace(safeTimestamp.begin(), safeTimestamp.end(), ':', '-');
                    currentFilePath = dir + "/" + prefix + "_" + safeTimestamp + ".geojson";
                }

                // Écrire le GeoJSON avec les données tamponnées
                geoJSONWriter.writeGeoJSON(
                    currentFilePath,
                    flightDataCollector.getMetadata(),
                    flightDataCollector.getBuffer()
                );
                // Vider le tampon
                flightDataCollector.clearBuffer();
            }
            callbackCounter++;
        } else {
            // Si le vol est terminé (atterrissage), réinitialiser
            if (!currentFilePath.empty()) {
                // Écrire les dernières données (si tampon non vide)
                if (!flightDataCollector.getBuffer().empty()) {
                    geoJSONWriter.writeGeoJSON(
                        currentFilePath,
                        flightDataCollector.getMetadata(),
                        flightDataCollector.getBuffer()
                    );
                }
                currentFilePath.clear();
                callbackCounter = 0;
            }
        }
    } catch (const std::exception& e) {
        XPLMDebugString("FlightDataRecorder: Error - ");
        XPLMDebugString(e.what());
        XPLMDebugString("\n");
    }

    return 1.0; // Rappel toutes les 1s
}

PLUGIN_API int XPluginStart(
    char *outName,
    char *outSig,
    char *outDesc) {
    strcpy(outName, "Flight Data Recorder");
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
    // Fermer proprement le fichier en cours
    if (!currentFilePath.empty() && flightDataCollector.isFlightActive()) {
        geoJSONWriter.writeGeoJSON(
            currentFilePath,
            flightDataCollector.getMetadata(),
            flightDataCollector.getBuffer()
        );
    }
}

PLUGIN_API void XPluginDisable(void) {
    // Fermer le fichier en cours
    if (!currentFilePath.empty() && flightDataCollector.isFlightActive()) {
        geoJSONWriter.writeGeoJSON(
            currentFilePath,
            flightDataCollector.getMetadata(),
            flightDataCollector.getBuffer()
        );
    }
}

PLUGIN_API int XPluginEnable(void) {
    return 1;
}

PLUGIN_API void XPluginReceiveMessage(
    XPLMPluginID inFromWho,
    int inMessage,
    void *inParam) {
}
