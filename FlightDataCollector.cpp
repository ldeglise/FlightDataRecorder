#include "FlightDataCollector.hpp"

#include <filesystem>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <chrono>

namespace fs = std::filesystem;

FlightDataCollector::FlightDataCollector() = default;

FlightDataCollector::~FlightDataCollector() {
    // Fermer le fichier si encore ouvert
    if (currentFile.is_open()) {
        // Écrire les points restants dans le buffer
        flushWriteBuffer();
        
        // Écrire la LineString finale à partir du buffer
        if (!lineStringBuffer.empty() && lineStringBuffer.size() >= 2) {
            currentFile << ",\n";
            currentFile << "    {\n";
            currentFile << "      \"type\": \"Feature\",\n";
            currentFile << "      \"geometry\": {\n";
            currentFile << "        \"type\": \"LineString\",\n";
            currentFile << "        \"coordinates\": [\n";
            for (size_t i = 0; i < lineStringBuffer.size(); ++i) {
                const auto& coord = lineStringBuffer[i];
                currentFile << "          [" << coord.first << ", " << coord.second << "]";
                if (i < lineStringBuffer.size() - 1) {
                    currentFile << ",";
                }
                currentFile << "\n";
            }
            currentFile << "        ]\n";
            currentFile << "      }\n";
            currentFile << "    }\n";
        }
        currentFile << "  ]\n";
        currentFile << "}\n";
        currentFile.flush();
        currentFile.close();
    }
}

void FlightDataCollector::flushWriteBuffer() {
    if (!currentFile.is_open() || writeBuffer.empty()) {
        return;
    }

    for (size_t i = 0; i < writeBuffer.size(); ++i) {
        const auto& point = writeBuffer[i];
        
        // Ajouter une virgule avant chaque point sauf le premier
        if (i > 0) {
            currentFile << ",\n";
        }
        
        currentFile << "    {\n";
        currentFile << "      \"type\": \"Feature\",\n";
        currentFile << "      \"geometry\": {\n";
        currentFile << "        \"type\": \"Point\",\n";
        currentFile << "        \"coordinates\": [" << point.longitude << ", " << point.latitude << "]\n";
        currentFile << "      },\n";
        currentFile << "      \"properties\": {\n";
        currentFile << "        \"timestamp\": \"" << point.timestamp << "\",\n";
        currentFile << "        \"elevation_msl\": " << point.elevation_msl << ",\n";
        currentFile << "        \"true_heading\": " << point.true_heading << ",\n";
        currentFile << "        \"magnetic_heading\": " << point.magnetic_heading << ",\n";
        currentFile << "        \"ias\": " << point.ias << ",\n";
        currentFile << "        \"gs\": " << point.gs << ",\n";
        currentFile << "        \"zulu_time_sec\": " << point.zulu_time_sec << ",\n";
        currentFile << "        \"zulu_time\": \"" << point.zulu_time << "\"\n";
        currentFile << "      }\n";
        currentFile << "    }";
    }
    currentFile.flush();
    writeBuffer.clear();
}

std::string FlightDataCollector::generateTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    
    std::tm tm_struct;
    #ifdef _WIN32
        gmtime_s(&tm_struct, &in_time_t);
    #else
        std::tm* tm_ptr = gmtime(&in_time_t);
        if (tm_ptr) {
            tm_struct = *tm_ptr;
        } else {
            tm_struct = std::tm();
        }
    #endif
    
    std::ostringstream oss;
    oss << std::setfill('0')
        << std::setw(4) << (tm_struct.tm_year + 1900) << "-"
        << std::setw(2) << (tm_struct.tm_mon + 1) << "-"
        << std::setw(2) << tm_struct.tm_mday << "T"
        << std::setw(2) << tm_struct.tm_hour << ":"
        << std::setw(2) << tm_struct.tm_min << ":"
        << std::setw(2) << tm_struct.tm_sec << "Z";
    return oss.str();
}

// Convertit zulu_time_sec (secondes depuis 00:00) en HH:mm:ss
std::string FlightDataCollector::convertZuluTime(float zulu_time_sec) const {
    int totalSeconds = static_cast<int>(zulu_time_sec);
    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int seconds = totalSeconds % 60;
    
    std::ostringstream oss;
    oss << std::setfill('0')
        << std::setw(2) << hours << ":"
        << std::setw(2) << minutes << ":"
        << std::setw(2) << seconds;
    return oss.str();
}

bool FlightDataCollector::isTakeoffDetected(DataRefManager& manager) {
    float agl = manager.getFloatDataRef("sim/flightmodel/position/y_agl");
    
    // Détection universelle : AGL > 10 mètres pendant 10 secondes
    if (agl > 10.0f) {
        takeoffCounter++;
        return takeoffCounter >= 10;
    } else {
        takeoffCounter = 0;
        return false;
    }
}

bool FlightDataCollector::isLandingDetected(DataRefManager& manager) {
    float agl = manager.getFloatDataRef("sim/flightmodel/position/y_agl");
    
    // Détection d'atterrissage : AGL < 1 mètre pendant 20 secondes
    if (agl < 1.0f) {
        landingCounter++;
        return landingCounter >= 20;
    } else {
        landingCounter = 0;
        return false;
    }
}

void FlightDataCollector::collectData(DataRefManager& manager) {
    // Lire les métadonnées moteur (seed = num_engines, checksum = total_power en watts)
    try {
        metadata.seed = manager.getIntDataRef("sim/aircraft/engine/acf_num_engines");
    } catch (...) {
        // Garder la valeur précédente si erreur
    }
    
    try {
        if (metadata.seed > 0) {
            float pmax = manager.getFloatDataRef("sim/aircraft/engine/acf_pmax");
            metadata.checksum = pmax * metadata.seed; // Puissance totale en watts
        }
    } catch (...) {
        // Garder la valeur précédente si erreur
    }

    // 1. Détecter le décollage
    if (!flightActive && isTakeoffDetected(manager)) {
        flightActive = true;
        takeoffCounter = 0;
        writeCounter = 0;
        writeBuffer.clear();
        lineStringBuffer.clear();
        
        // Prendre le timestamp IMMEDIATEMENT à la première détection
        metadata.takeoff_time = generateTimestamp();
        
        // Générer le nom de fichier
        std::string dir = getOutputDirectory();
        std::string timestamp = metadata.takeoff_time;
        std::string safeTimestamp = timestamp;
        std::replace(safeTimestamp.begin(), safeTimestamp.end(), ':', '-');
        currentFilePath = dir + "/" + safeTimestamp + ".geojson";
        
        // Ouvrir le fichier et écrire l'en-tête
        currentFile.open(currentFilePath, std::ios::out | std::ios::trunc);
        if (!currentFile.is_open()) {
            // Erreur d'ouverture, désactiver l'enregistrement
            flightActive = false;
            return;
        }
        
        // Écrire l'en-tête GeoJSON
        currentFile << "{\n";
        currentFile << "  \"type\": \"FeatureCollection\",\n";
        currentFile << "  \"metadata\": {\n";
        currentFile << "    \"seed\": " << metadata.seed << ",\n";
        currentFile << "    \"checksum\": " << metadata.checksum << ",\n";
        currentFile << "    \"takeoff_time\": \"" << metadata.takeoff_time << "\"\n";
        currentFile << "  },\n";
        currentFile << "  \"features\": [\n";
        
        currentFile.flush();
    }

    // 2. Gestion de l'état en vol
    if (flightActive) {
        // Détecter un atterrissage
        if (isLandingDetected(manager)) {
            // Écrire les points restants dans le buffer
            flushWriteBuffer();
            
            // Fermer le fichier, vol terminé
            flightActive = false;
            landingCounter = 0;
            writeCounter = 0;
            
            // Écrire la LineString finale à partir du buffer
            if (currentFile.is_open() && lineStringBuffer.size() >= 2) {
                currentFile << ",\n";
                currentFile << "    {\n";
                currentFile << "      \"type\": \"Feature\",\n";
                currentFile << "      \"geometry\": {\n";
                currentFile << "        \"type\": \"LineString\",\n";
                currentFile << "        \"coordinates\": [\n";
                for (size_t i = 0; i < lineStringBuffer.size(); ++i) {
                    const auto& coord = lineStringBuffer[i];
                    currentFile << "          [" << coord.first << ", " << coord.second << "]";
                    if (i < lineStringBuffer.size() - 1) {
                        currentFile << ",";
                    }
                    currentFile << "\n";
                }
                currentFile << "        ]\n";
                currentFile << "      }\n";
                currentFile << "    }\n";
            }
            
            currentFile << "  ]\n";
            currentFile << "}\n";
            currentFile.flush();
            currentFile.close();
            
            currentFilePath.clear();
            lineStringBuffer.clear();
            writeBuffer.clear();
            return;
        }

        // 3. Collecter les données
        PositionData posData = manager.getPositionData();
        FlightPoint point;
        point.longitude = posData.longitude;
        point.latitude = posData.latitude;
        point.elevation_msl = posData.elevation_msl;
        point.true_heading = manager.getFloatDataRef("sim/flightmodel/position/psi");
        point.magnetic_heading = manager.getFloatDataRef("sim/flightmodel/position/mag_psi");
        point.ias = manager.getFloatDataRef("sim/flightmodel/position/indicated_airspeed");
        point.gs = manager.getFloatDataRef("sim/flightmodel/position/groundspeed");
        point.zulu_time_sec = manager.getFloatDataRef("sim/time/zulu_time_sec");
        point.zulu_time = convertZuluTime(point.zulu_time_sec);
        point.local_date_days = manager.getIntDataRef("sim/time/local_date_days");
        point.timestamp = generateTimestamp();

        // Ajouter au buffer pour la LineString (coordonnées uniquement)
        lineStringBuffer.emplace_back(point.longitude, point.latitude);
        
        // Ajouter au buffer d'écriture (toutes les données)
        writeBuffer.push_back(point);
        writeCounter++;
        
        // Écrire toutes les 10 secondes
        if (writeCounter >= 10) {
            flushWriteBuffer();
            writeCounter = 0;
        }
    }
}

std::string FlightDataCollector::getOutputDirectory() const {
    fs::path home;
    #ifdef _WIN32
        home = fs::path(getenv("USERPROFILE"));
    #elif __APPLE__
        home = fs::path(getenv("HOME"));
    #else // Linux
        home = fs::path(getenv("HOME"));
    #endif
    fs::path dir = home / "FlightDataRecorder";
    if (!fs::exists(dir)) {
        fs::create_directories(dir);
    }
    return dir.string();
}
