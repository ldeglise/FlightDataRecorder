// =============================================================================
// GeoJSONWriter.hpp
// Écrit les données de vol au format GeoJSON (RFC 7946).
// =============================================================================
#pragma once
#include <string>
#include <vector>
#include <fstream>

struct FlightDataPoint {
    double latitude;
    double longitude;
    double altitude_msl;
    double heading;
    double ias;
    double gs;
    double timestamp;
};

class GeoJSONWriter {
public:
    explicit GeoJSONWriter(const std::string& filename);
    ~GeoJSONWriter();

    bool Initialize();
    void Cleanup();

    // Écrit un ensemble de points dans le fichier GeoJSON
    void Write(const std::vector<FlightDataPoint>& dataPoints);

private:
    std::string mFilename;
    std::ofstream mFile;
    bool mIsOpen;
};