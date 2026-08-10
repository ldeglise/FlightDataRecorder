#pragma once

#include <vector>
#include <string>
#include <ctime>
#include <fstream>
#include <chrono>
#include <cmath>
#include <algorithm>
#include "DataRefManager.hpp"

struct FlightPoint {
    double longitude;
    double latitude;
    double elevation_msl;      // Altitude barométrique (mètres)
    float true_heading;        // Cap vrai (degrés)
    float magnetic_heading;    // Cap magnétique (degrés)
    float ias;                 // Vitesse indiquée (en knots, valeur brute X-Plane)
    float gs;                  // Vitesse sol (en m/s, valeur brute X-Plane)
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
    bool firstPointWritten = false; // Pour gérer les virgules entre les Features
    std::string currentFilePath;
    std::ofstream currentFile;

    bool isTakeoffDetected(DataRefManager& manager);
    bool isLandingDetected(DataRefManager& manager);
    std::string generateTimestamp() const;
    std::string convertZuluTime(float zulu_time_sec) const;
    void flushWriteBuffer();
    
    // Utilitaire pour arrondir à 6 décimales (précision ~10 cm)
    static double roundTo6Decimals(double value) {
        return std::round(value * 1000000.0) / 1000000.0;
    }
    
    // Applique une moyenne mobile pour lisser les coordonnées
    // windowSize : nombre de points à prendre en compte pour le lissage
    static std::vector<std::pair<double, double>> applyMovingAverage(
        const std::vector<std::pair<double, double>>& buffer, 
        int windowSize = 3) {
        
        if (buffer.empty() || windowSize <= 1) {
            return buffer;
        }
        
        std::vector<std::pair<double, double>> smoothed;
        smoothed.reserve(buffer.size());
        
        // Conserver les premiers points inchangés (pas assez de données pour la moyenne)
        int halfWindow = windowSize / 2;
        for (int i = 0; i < std::min(halfWindow, static_cast<int>(buffer.size())); ++i) {
            smoothed.push_back(buffer[i]);
        }
        
        // Appliquer la moyenne mobile
        for (size_t i = halfWindow; i < buffer.size() - halfWindow; ++i) {
            double sumLon = 0.0;
            double sumLat = 0.0;
            int count = 0;
            
            for (int j = -halfWindow; j <= halfWindow; ++j) {
                size_t idx = i + j;
                if (idx < buffer.size()) {
                    sumLon += buffer[idx].first;
                    sumLat += buffer[idx].second;
                    count++;
                }
            }
            
            if (count > 0) {
                smoothed.emplace_back(sumLon / count, sumLat / count);
            }
        }
        
        // Conserver les derniers points inchangés
        for (size_t i = std::max(static_cast<size_t>(buffer.size() - halfWindow), static_cast<size_t>(halfWindow)); 
             i < buffer.size(); ++i) {
            // Éviter les doublons
            if (smoothed.size() <= i) {
                smoothed.push_back(buffer[i]);
            }
        }
        
        // Si le lissage a réduit la taille, compléter avec les originaux
        if (smoothed.size() < buffer.size()) {
            for (size_t i = smoothed.size(); i < buffer.size(); ++i) {
                smoothed.push_back(buffer[i]);
            }
        }
        
        return smoothed;
    }
    
    // Conversions de vitesse
    static float knotsToKmh(float knots) { return knots * 1.852f; }
    static float msToKmh(float ms) { return ms * 3.6f; }
};
