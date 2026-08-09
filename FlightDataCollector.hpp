#pragma once

#include <vector>
#include <string>
#include <ctime>
#include <fstream>
#include <chrono>
#include "DataRefManager.hpp"

struct FlightPoint {
    double longitude;
    double latitude;
    double elevation_msl;      // Altitude barométrique (mètres)
    float true_heading;        // Cap vrai (degrés)
    float magnetic_heading;    // Cap magnétique (degrés)
    float ias;                 // Vitesse indiquée (km/h, valeur brute X-Plane)
    float gs;                  // Vitesse sol (km/h, valeur brute X-Plane)
    float zulu_time_sec;       // Secondes depuis minuit Zulu
    int local_date_days;       // Jours depuis le 1er janvier (local)
    std::string timestamp;     // ISO 8601 (ex: "2026-08-08T14:30:20Z")
    std::string zulu_time;     // HH:mm:ss (converti depuis zulu_time_sec)
};

struct FlightMetadata {
    int num_engines = 0;
    float total_power = 0;     // acf_pmax * acf_num_engines
    std::string takeoff_time;  // ISO 8601
};

class FlightDataCollector {
public:
    FlightDataCollector();
    ~FlightDataCollector();

    void collectData(DataRefManager& manager);
    bool isFlightActive() const { return flightActive; }
    const FlightMetadata& getMetadata() const { return metadata; }
    std::string generateRandomPrefix() const;
    std::string getOutputDirectory() const;
    std::string getCurrentFilePath() const { return currentFilePath; }

private:
    FlightMetadata metadata;
    std::vector<FlightPoint> pointsBuffer; // Buffer pour la LineString finale
    bool flightActive = false;
    bool flightPaused = false;  // En pause au sol (atterrissage temporaire)
    int takeoffCounter = 0;    // Compteur pour la détection (10s)
    int landingCounter = 0;    // Compteur pour la détection (30s)
    std::string currentFilePath;
    std::ofstream currentFile;
    std::chrono::time_point<std::chrono::system_clock> groundContactTime; // Heure du contact sol

    bool isTakeoffDetected(DataRefManager& manager);
    bool isLandingDetected(DataRefManager& manager);
    bool isFlightEnded() const; // Vérifie si le vol est vraiment terminé (2 min au sol)
    std::string generateTimestamp() const;
    std::string convertZuluTime(float zulu_time_sec) const;
};
