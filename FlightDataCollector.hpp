#pragma once

#include <vector>
#include <string>
#include <ctime>
#include "DataRefManager.hpp"

struct FlightPoint {
    double longitude;
    double latitude;
    double elevation_msl;      // Altitude barométrique (mètres)
    float true_heading;        // Cap vrai (degrés)
    float magnetic_heading;    // Cap magnétique (degrés)
    float ias;                 // Vitesse indiquée (km/h)
    float gs;                  // Vitesse sol (km/h)
    float zulu_time_sec;       // Secondes depuis minuit Zulu
    int local_date_days;       // Jours depuis le 1er janvier (local)
    std::string timestamp;     // ISO 8601 (ex: "2026-08-08T14:30:20Z")
};

struct FlightMetadata {
    std::string aircraft_icao;
    std::string aircraft_model;
    int num_engines = 0;
    float total_power = 0;     // acf_pmax * acf_num_engines
    std::string takeoff_time;  // ISO 8601
};

class FlightDataCollector {
public:
    FlightDataCollector();
    ~FlightDataCollector();

    void collectData(DataRefManager& manager);
    void writeToDisk(const std::string& outputDir, class GeoJSONWriter& writer);
    void clearBuffer() { buffer.clear(); }
    bool isFlightActive() const { return flightActive; }
    const FlightMetadata& getMetadata() const { return metadata; }
    const std::vector<FlightPoint>& getBuffer() const { return buffer; }
    std::string generateRandomPrefix() const;
    std::string getOutputDirectory() const;

private:
    std::vector<FlightPoint> buffer;
    FlightMetadata metadata;
    bool flightActive = false;
    int takeoffCounter = 0;    // Compteur pour la détection (3s)
    int landingCounter = 0;    // Compteur pour la détection (7s)

    bool isTakeoffDetected(DataRefManager& manager);
    bool isLandingDetected(DataRefManager& manager);
    std::string generateTimestamp() const;
    std::string tryGetAircraftString(DataRefManager& manager, const std::string& primary, const std::string& fallback) const;
};
