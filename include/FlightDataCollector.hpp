// =============================================================================
// FlightDataCollector.hpp
// Coordonne la collecte des données et l'écriture dans GeoJSON.
// =============================================================================
#pragma once
#include "DataRefManager.hpp"
#include "GeoJSONWriter.hpp"
#include <vector>

class FlightDataCollector {
public:
    FlightDataCollector(DataRefManager* dataRefManager, GeoJSONWriter* geoJSONWriter);
    ~FlightDataCollector();

    bool Initialize();
    void Cleanup();
    void Update(float deltaTime); // Appelé par la boucle de vol

private:
    DataRefManager* mDataRefManager;
    GeoJSONWriter*  mGeoJSONWriter;
    std::vector<FlightDataPoint> mFlightDataBuffer;
    float mTimeSinceLastWrite = 0.0f;
    static constexpr float WRITE_INTERVAL = 10.0f; // Écriture toutes les 10 secondes
};