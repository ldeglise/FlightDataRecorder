#include "FlightDataCollector.hpp"
#include "GeoJSONWriter.hpp"
#include <filesystem>
#include <random>
#include <algorithm>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;

FlightDataCollector::FlightDataCollector() = default;
FlightDataCollector::~FlightDataCollector() = default;

std::string FlightDataCollector::generateTimestamp(float zulu_time_sec, int local_date_days) const {
    // Calculer l'heure (HH:mm:SS)
    int hours = static_cast<int>(zulu_time_sec) / 3600;
    int minutes = (static_cast<int>(zulu_time_sec) % 3600) / 60;
    int seconds = static_cast<int>(zulu_time_sec) % 60;

    // Calculer le mois et le jour à partir de local_date_days
    int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int month = 1;
    int day = local_date_days;
    int year = 2026; // Année par défaut

    // Ajustement pour les années bissextiles
    bool is_leap_year = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    if (is_leap_year) {
        days_in_month[1] = 29; // Février a 29 jours
    }

    for (int m = 0; m < 12; ++m) {
        if (day > days_in_month[m]) {
            day -= days_in_month[m];
            month++;
        } else {
            break;
        }
    }

    // Formater en ISO 8601 : YYYY-MM-DDTHH:mm:SSZ
    std::ostringstream oss;
    oss << std::setfill('0')
        << std::setw(4) << year << "-"
        << std::setw(2) << month << "-"
        << std::setw(2) << day << "T"
        << std::setw(2) << hours << ":"
        << std::setw(2) << minutes << ":"
        << std::setw(2) << seconds << "Z";
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
        metadata.takeoff_time = generateTimestamp(
            manager.getFloatDataRef("sim/time/zulu_time_sec"),
            manager.getIntDataRef("sim/time/local_date_days")
        );
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
        point.timestamp = generateTimestamp(point.zulu_time_sec, point.local_date_days);

        buffer.push_back(point);
        writeCounter++;

        // 3. Détecter l'atterrissage
        if (isLandingDetected(manager)) {
            flightActive = false;
            landingCounter = 0;
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
