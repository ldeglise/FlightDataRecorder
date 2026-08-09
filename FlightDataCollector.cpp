#include "FlightDataCollector.hpp"

#include <filesystem>
#include <random>
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
        // Écrire la LineString finale à partir du buffer
        if (!pointsBuffer.empty() && pointsBuffer.size() >= 2) {
            currentFile << ",\n";
            currentFile << "    {\n";
            currentFile << "      \"type\": \"Feature\",\n";
            currentFile << "      \"geometry\": {\n";
            currentFile << "        \"type\": \"LineString\",\n";
            currentFile << "        \"coordinates\": [\n";
            for (size_t i = 0; i < pointsBuffer.size(); ++i) {
                const auto& p = pointsBuffer[i];
                currentFile << "          [" << p.longitude << ", " << p.latitude << ", " << p.elevation_msl << "]";
                if (i < pointsBuffer.size() - 1) {
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

// Helper pour lire une string dataref avec fallback
std::string FlightDataCollector::tryGetAircraftString(DataRefManager& manager, const std::string& primary, const std::string& fallback) const {
    try {
        std::string value = manager.getStringDataRef(primary);
        if (!value.empty() && value != " " && value.find_first_not_of(" ") != std::string::npos) {
            return value;
        }
    } catch (...) {
        // Dataref non trouvée, essayer le fallback
    }
    
    try {
        std::string value = manager.getStringDataRef(fallback);
        if (!value.empty() && value != " " && value.find_first_not_of(" ") != std::string::npos) {
            return value;
        }
    } catch (...) {
        // Retourner vide si les deux échouent
    }
    
    return "";
}

bool FlightDataCollector::isTakeoffDetected(DataRefManager& manager) {
    float agl = manager.getFloatDataRef("sim/flightmodel/position/y_agl");
    float agl_feet = DataRefManager::metersToFeet(agl);
    int onground = manager.getIntDataRef("sim/flightmodel/position/onground");
    
    // Détection universelle : AGL > 10 pieds ET onground == 0 pendant 3 secondes
    if (agl_feet > 10.0f && onground == 0) {
        takeoffCounter++;
        return takeoffCounter >= 3;
    } else {
        takeoffCounter = 0;
        return false;
    }
}

bool FlightDataCollector::isLandingDetected(DataRefManager& manager) {
    float ias = manager.getFloatDataRef("sim/flightmodel/position/indicated_airspeed");
    int onground = manager.getIntDataRef("sim/flightmodel/position/onground");
    
    // Détection d'atterrissage : onground == 1 ET vitesse très faible
    if (onground == 1 && ias < 20.0f) {
        landingCounter++;
        return landingCounter >= 5;
    } else {
        landingCounter = 0;
        return false;
    }
}

bool FlightDataCollector::isFlightEnded() const {
    // Vérifie si le vol est vraiment terminé (2 minutes au sol sans redécollage)
    auto now = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::minutes>(now - groundContactTime);
    
    // Si on est au sol depuis plus de 2 minutes, considérer le vol comme terminé
    return elapsed.count() >= 2;
}

void FlightDataCollector::collectData(DataRefManager& manager) {
    // Monitorer les métadonnées avion à chaque callback
    metadata.aircraft_icao = tryGetAircraftString(manager,
        "sim/aircraft/view/acf_ICAO",
        "sim/aircraft/engine/acf_ICAO"
    );
    
    metadata.aircraft_model = tryGetAircraftString(manager,
        "sim/aircraft/view/acf_ui_name",
        "sim/aircraft/view/acf_descrip"
    );
    
    try {
        metadata.num_engines = manager.getIntDataRef("sim/aircraft/engine/acf_num_engines");
    } catch (...) {
        // Garder la valeur précédente si erreur
    }
    
    try {
        if (metadata.num_engines > 0) {
            float pmax = manager.getFloatDataRef("sim/aircraft/engine/acf_pmax");
            metadata.total_power = pmax * metadata.num_engines;
        }
    } catch (...) {
        // Garder la valeur précédente si erreur
    }

    int onground = manager.getIntDataRef("sim/flightmodel/position/onground");

    // 1. Détecter le décollage (initial ou après une pause)
    if (!flightActive && isTakeoffDetected(manager)) {
        flightActive = true;
        flightPaused = false;
        takeoffCounter = 0;
        
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
        currentFile << "    \"aircraft_icao\": \"" << metadata.aircraft_icao << "\",\n";
        currentFile << "    \"aircraft_model\": \"" << metadata.aircraft_model << "\",\n";
        currentFile << "    \"num_engines\": " << metadata.num_engines << ",\n";
        currentFile << "    \"total_power\": " << metadata.total_power << ",\n";
        currentFile << "    \"takeoff_time\": \"" << metadata.takeoff_time << "\"\n";
        currentFile << "  },\n";
        currentFile << "  \"features\": [\n";
        
        currentFile.flush();
        
        // Vider le buffer de points
        pointsBuffer.clear();
    }

    // 2. Gestion de l'état en vol
    if (flightActive) {
        // Détecter un atterrissage
        if (isLandingDetected(manager)) {
            // Premier atterrissage détecté
            if (!flightPaused) {
                flightPaused = true;
                groundContactTime = std::chrono::system_clock::now();
            }
            
            // Vérifier si le vol est vraiment terminé (2 min au sol)
            if (isFlightEnded()) {
                // Fermer le fichier, vol terminé
                flightActive = false;
                landingCounter = 0;
                
                // Écrire la LineString finale à partir du buffer
                if (currentFile.is_open() && pointsBuffer.size() >= 2) {
                    currentFile << ",\n";
                    currentFile << "    {\n";
                    currentFile << "      \"type\": \"Feature\",\n";
                    currentFile << "      \"geometry\": {\n";
                    currentFile << "        \"type\": \"LineString\",\n";
                    currentFile << "        \"coordinates\": [\n";
                    for (size_t i = 0; i < pointsBuffer.size(); ++i) {
                        const auto& p = pointsBuffer[i];
                        currentFile << "          [" << p.longitude << ", " << p.latitude << ", " << p.elevation_msl << "]";
                        if (i < pointsBuffer.size() - 1) {
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
            }
            // Sinon, on reste en pause (atterrissage temporaire)
            return; // Ne pas écrire de points pendant la pause
        }
        
        // Si on était en pause et qu'on redécolle
        if (flightPaused && isTakeoffDetected(manager)) {
            flightPaused = false;
            takeoffCounter = 0;
            return; // Le point sera écrit au prochain callback
        }

        // 3. Écrire les données (si pas en pause)
        if (!flightPaused) {
            FlightPoint point;
            point.longitude = manager.getFloatDataRef("sim/flightmodel/position/longitude");
            point.latitude = manager.getFloatDataRef("sim/flightmodel/position/latitude");
            point.elevation_msl = manager.getFloatDataRef("sim/flightmodel/position/elevation");
            point.true_heading = manager.getFloatDataRef("sim/flightmodel/position/psi");
            point.magnetic_heading = manager.getFloatDataRef("sim/flightmodel/position/mag_psi");
            
            // X-Plane fournit les vitesses en km/h - stocker sans conversion
            point.ias = manager.getFloatDataRef("sim/flightmodel/position/indicated_airspeed");
            point.gs = manager.getFloatDataRef("sim/flightmodel/position/groundspeed");
            
            point.zulu_time_sec = manager.getFloatDataRef("sim/time/zulu_time_sec");
            point.zulu_time = convertZuluTime(point.zulu_time_sec); // Conversion en HH:mm:ss
            point.local_date_days = manager.getIntDataRef("sim/time/local_date_days");
            point.timestamp = generateTimestamp();

            // Stocker le point dans le buffer pour la LineString finale
            pointsBuffer.push_back(point);

            // Écrire UNIQUEMENT le Point Feature (pas de LineString pendant le vol)
            if (currentFile.is_open()) {
                // Ajouter une virgule si ce n'est pas le premier point
                if (!pointsBuffer.empty() && pointsBuffer.size() > 1) {
                    currentFile << ",\n";
                }
                
                currentFile << "    {\n";
                currentFile << "      \"type\": \"Feature\",\n";
                currentFile << "      \"geometry\": {\n";
                currentFile << "        \"type\": \"Point\",\n";
                currentFile << "        \"coordinates\": [" << point.longitude << ", " << point.latitude << ", " << point.elevation_msl << "]\n";
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
                
                currentFile.flush();
            }
        }
    }
}

std::string FlightDataCollector::generateRandomPrefix() const {
    static const char alphanum[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 25);
    std::string prefix;
    for (int i = 0; i < 4; ++i) {
        prefix += alphanum[dis(gen)];
    }
    return prefix;
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
