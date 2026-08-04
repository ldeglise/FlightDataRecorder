// =============================================================================
// FlightDataCollector.cpp
// =============================================================================
#include "FlightDataCollector.hpp"
#include <XPLMUtilities.h>
#include <XPLMProcessing.h> // Pour XPLMGetElapsedRealTime

FlightDataCollector::FlightDataCollector(DataRefManager* dataRefManager, GeoJSONWriter* geoJSONWriter)
    : mDataRefManager(dataRefManager), mGeoJSONWriter(geoJSONWriter) {}

FlightDataCollector::~FlightDataCollector() { Cleanup(); }

bool FlightDataCollector::Initialize() {
    if (!mDataRefManager || !mGeoJSONWriter) {
        XPLMDebugString("FlightDataCollector: Pointeurs invalides.\n");
        return false;
    }
    return true;
}

void FlightDataCollector::Cleanup() {
    // Écrit les données restantes avant de nettoyer
    if (!mFlightDataBuffer.empty()) {
        mGeoJSONWriter->Write(mFlightDataBuffer);
        mFlightDataBuffer.clear();
    }
}

void FlightDataCollector::Update(float deltaTime) {
    if (!mDataRefManager || !mGeoJSONWriter) return;

    // Récupère les données
    FlightDataPoint point;
    point.latitude    = mDataRefManager->GetDataRefValue("sim/flightmodel/position/latitude");
    point.longitude   = mDataRefManager->GetDataRefValue("sim/flightmodel/position/longitude");
    point.altitude_msl = mDataRefManager->GetDataRefValue("sim/flightmodel/position/elevation");
    point.heading     = mDataRefManager->GetDataRefValue("sim/flightmodel/position/psi");
    point.ias         = mDataRefManager->GetDataRefValue("sim/flightmodel/position/indicated_airspeed");
    point.gs          = mDataRefManager->GetDataRefValue("sim/flightmodel/position/groundspeed");
    point.timestamp   = static_cast<double>(XPLMGetElapsedTime());

    mFlightDataBuffer.push_back(point);
    mTimeSinceLastWrite += deltaTime;

    // Écrit dans le fichier toutes les 10 secondes
    if (mTimeSinceLastWrite >= WRITE_INTERVAL) {
        mGeoJSONWriter->Write(mFlightDataBuffer);
        mFlightDataBuffer.clear();
        mTimeSinceLastWrite = 0.0f;
    }
}