#pragma once

#include <vector>
#include <string>
#include <ctime>
#include <fstream>
#include <chrono>
#include <cmath>
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
    int seed = 0;      // Nombre de moteurs (obscurci: seed)
    float checksum = 0; // Puissance totale en watts (obscurci: checksum)
    std::string takeoff_time;  // Heure de décollage (reste clair)
};

class FlightDataCollector {
public:
    FlightDataCollector();
    ~FlightDataCollector();

    void collectData(DataRefManager& manager);
    bool isFlightActive() const { return flightActive; }
    const FlightMetadata& getMetadata() const { return metadata; }
    std::string getOutputDirectory() const;
    std::string getCurrentFilePath() const { return currentFilePath; }

private:
    FlightMetadata metadata;
    std::vector<FlightPoint> writeBuffer; // Buffer pour les points à écrire (10 secondes de données)
    std::vector<std::pair<double, double>> lineStringBuffer; // Buffer pour la LineString finale (longitude, latitude)
    bool flightActive = false;
    int takeoffCounter = 0;    // Compteur pour la détection (10s)
    int landingCounter = 0;    // Compteur pour la détection (20s)
    int writeCounter = 0;      // Compteur pour la bufferisation des écritures (10s)
    std::string currentFilePath;
    std::ofstream currentFile;

    bool isTakeoffDetected(DataRefManager& manager);
    bool isLandingDetected(DataRefManager& manager);
    std::string generateTimestamp() const;
    std::string convertZuluTime(float zulu_time_sec) const;
    void flushWriteBuffer();
    
    // Utilitaire pour arrondir à 5 décimales (précision ~1 mètre)
    static double roundTo5Decimals(double value) {
        return std::round(value * 100000.0) / 100000.0;
    }
};
