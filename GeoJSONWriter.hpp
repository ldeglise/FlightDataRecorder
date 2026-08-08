#pragma once

#include <string>
#include <vector>
#include "FlightDataCollector.hpp"

class GeoJSONWriter {
public:
    void writeGeoJSON(const std::string& filePath,
                     const FlightMetadata& metadata,
                     const std::vector<FlightPoint>& points);
};
