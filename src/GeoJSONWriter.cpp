// =============================================================================
// GeoJSONWriter.cpp
// =============================================================================
#include "GeoJSONWriter.hpp"
#include <XPLMUtilities.h>

GeoJSONWriter::GeoJSONWriter(const std::string& filename)
    : mFilename(filename), mIsOpen(false) {}

GeoJSONWriter::~GeoJSONWriter() { Cleanup(); }

bool GeoJSONWriter::Initialize() {
    mFile.open(mFilename, std::ios::out | std::ios::trunc);
    if (!mFile.is_open()) {
        std::string msg = "GeoJSONWriter: Impossible d'ouvrir le fichier: " + mFilename + "\n";
        XPLMDebugString(msg.c_str());
        return false;
    }
    mIsOpen = true;
    // Écrit l'en-tête GeoJSON
    mFile << "{\n  \"type\": \"FeatureCollection\",\n  \"features\": [\n";
    return true;
}

void GeoJSONWriter::Cleanup() {
    if (mIsOpen) {
        // Ferme le tableau et le fichier
        mFile << "  ]\n}\n";
        mFile.close();
        mIsOpen = false;
    }
}

void GeoJSONWriter::Write(const std::vector<FlightDataPoint>& dataPoints) {
    if (!mIsOpen) return;

    for (size_t i = 0; i < dataPoints.size(); ++i) {
        const auto& point = dataPoints[i];
        mFile << "    {\n"
              << "      \"type\": \"Feature\",\n"
              << "      \"geometry\": {\n"
              << "        \"type\": \"Point\",\n"
              << "        \"coordinates\": [" << point.longitude << ", " << point.latitude << "]\n"
              << "      },\n"
              << "      \"properties\": {\n"
              << "        \"altitude_msl\": " << point.altitude_msl << ",\n"
              << "        \"heading\": " << point.heading << ",\n"
              << "        \"ias\": " << point.ias << ",\n"
              << "        \"gs\": " << point.gs << ",\n"
              << "        \"timestamp\": " << point.timestamp << "\n"
              << "      }\n"
              << "    }";
        if (i < dataPoints.size() - 1) {
            mFile << ",";
        }
        mFile << "\n";
    }
    mFile.flush(); // Force l'écriture sur le disque
}