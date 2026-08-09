#include "FlightDataCollector.hpp"
#include "GeoJSONWriter.hpp"
#include <filesystem>
#include <random>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <chrono>

namespace fs = std::filesystem;

FlightDataCollector::FlightDataCollector() = default;
FlightDataCollector::~FlightDataCollector() = default;

std::string FlightDataCollector::generateTimestamp() const {
    // Utiliser le timestamp système au lieu du simulateur
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    
    std::tm tm_struct;
    #ifdef _WIN32
        gmtime_s(&tm_struct, &in_time_t);
    #else
        // Pour Linux et macOS, utiliser gmtime (thread-safe dans ce contexte)
        std::tm* tm_ptr = gmtime(&in_time_t);
        if (tm_ptr) {
            tm_struct = *tm_ptr;
        } else {
            // Valeurs par défaut en cas d'erreur
            tm_struct = std::tm();
        }
    #endif
    
    // Formater en ISO 8601 : YYYY-MM-DDTHH:mm:SSZ
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

bool FlightDataCollector::isTakeoffDetected(DataRefManager& manager) {
    float agl = manager.getFloatDataRef("sim/flightmodel/position/y_agl");
    float ias = manager.getFloatDataRef("sim/flightmodel/position/indicated_airspeed");
    float agl_feet = DataRefManager::metersToFeet(agl);
    float ias_knots = DataRefManager::kmhToKnots(ias);

    if (agl_feet > 50.0f && ias_knots > 55.0f) {
        takeoffCounter++;
        return takeoffCounter >= 7;
    } else {
        takeoffCounter = 0;
        return false;
    }
}

bool FlightDataCollector::isLandingDetected(DataRefManager& manager) {
    float agl = manager.getFloatDataRef("sim/flightmodel/position/y_agl");
    float ias = manager.getFloatDataRef("sim/flightmodel/position/indicated_airspeed");
    float agl_feet = DataRefManager::metersToFeet(agl);
    float ias_knots = DataRefManager::kmhToKnots(ias);

    if (agl_feet < 10.0f && ias_knots < 30.0f) {
        landingCounter++;
        return landingCounter >= 7;
    } else {
        landingCounter = 0;
        return false;
    }
}

void FlightDataCollector::collectData(DataRefManager& manager) {
    // 1. Détecter le décollage
    if (!flightActive && isTakeoffDetected(manager)) {
        flightActive = true;
        takeoffCounter = 0;
        // Lire les métadonnées (1x)
        metadata.aircraft_icao = manager.getStringDataRef("sim/aircraft/view/acf_ICAO");
        metadata.aircraft_model = manager.getStringDataRef("sim/aircraft/view/acf_ui_name");
        metadata.num_engines = manager.getIntDataRef("sim/aircraft/engine/acf_num_engines");
        float pmax = manager.getFloatDataRef("sim/aircraft/engine/acf_pmax");
        metadata.total_power = pmax * metadata.num_engines;
        metadata.takeoff_time = generateTimestamp();
    }

    // 2. Si en vol, collecter les données
    if (flightActive) {
        FlightPoint point;
        point.longitude = manager.getFloatDataRef("sim/flightmodel/position/longitude");
        point.latitude = manager.getFloatDataRef("sim/flightmodel/position/latitude");
        point.elevation_msl = manager.getFloatDataRef("sim/flightmodel/position/elevation");
        point.true_heading = manager.getFloatDataRef("sim/flightmodel/position/psi");
        point.magnetic_heading = manager.getFloatDataRef("sim/flightmodel/position/mag_psi");
        point.ias = manager.getFloatDataRef("sim/flightmodel/position/indicated_airspeed");
        point.gs = manager.getFloatDataRef("sim/flightmodel/position/groundspeed");
        point.zulu_time_sec = manager.getFloatDataRef("sim/time/zulu_time_sec");
        point.local_date_days = manager.getIntDataRef("sim/time/local_date_days");
        point.timestamp = generateTimestamp();

        buffer.push_back(point);

        // 3. Détecter l'atterrissage
        if (isLandingDetected(manager)) {
            flightActive = false;
            landingCounter = 0;
        }
    }
}

std::string FlightDataCollector::generateRandomPrefix() const {
    // Plus utilisé mais gardé pour compatibilité
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
