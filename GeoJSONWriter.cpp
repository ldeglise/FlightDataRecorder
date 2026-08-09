#include "GeoJSONWriter.hpp"
#include <fstream>
#include <iomanip>

void GeoJSONWriter::writeGeoJSON(const std::string& filePath,
                                const FlightMetadata& metadata,
                                const std::vector<FlightPoint>& points) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filePath);
    }

    // Début du GeoJSON (RFC 7946)
    file << "{\n";
    file << "  \"type\": \"FeatureCollection\",\n";

    // Métadonnées (hors norme RFC 7946, mais autorisé par le CDC)
    file << "  \"metadata\": {\n";
    file << "    \"aircraft_icao\": \"" << metadata.aircraft_icao << "\",\n";
    file << "    \"aircraft_model\": \"" << metadata.aircraft_model << "\",\n";
    file << "    \"num_engines\": " << metadata.num_engines << ",\n";
    file << "    \"total_power\": " << metadata.total_power << ",\n";
    file << "    \"takeoff_time\": \"" << metadata.takeoff_time << "\"\n";
    file << "  },\n";

    // Features (conforme RFC 7946)
    file << "  \"features\": [\n";

    // 1. Ajouter les points (Point)
    for (size_t i = 0; i < points.size(); ++i) {
        const auto& p = points[i];
        file << "    {\n";
        file << "      \"type\": \"Feature\",\n";
        file << "      \"geometry\": {\n";
        file << "        \"type\": \"Point\",\n";
        file << "        \"coordinates\": [" << p.longitude << ", " << p.latitude << ", " << p.elevation_msl << "]\n";
        file << "      },\n";
        file << "      \"properties\": {\n";
        file << "        \"timestamp\": \"" << p.timestamp << "\",\n";
        file << "        \"elevation_msl\": " << p.elevation_msl << ",\n";
        file << "        \"true_heading\": " << p.true_heading << ",\n";
        file << "        \"magnetic_heading\": " << p.magnetic_heading << ",\n";
        file << "        \"ias\": " << p.ias << ",\n";
        file << "        \"gs\": " << p.gs << ",\n";
        file << "        \"zulu_time_sec\": " << p.zulu_time_sec << "\n";
        file << "      }\n";
        file << "    }";
        if (i < points.size() - 1 || points.size() >= 2) {
            file << ",\n";
        } else {
            file << "\n";
        }
    }

    // 2. Ajouter la trace (LineString) si au moins 2 points
    if (points.size() >= 2) {
        file << "    {\n";
        file << "      \"type\": \"Feature\",\n";
        file << "      \"geometry\": {\n";
        file << "        \"type\": \"LineString\",\n";
        file << "        \"coordinates\": [\n";
        for (size_t i = 0; i < points.size(); ++i) {
            const auto& p = points[i];
            file << "          [" << p.longitude << ", " << p.latitude << ", " << p.elevation_msl << "]";
            if (i < points.size() - 1) {
                file << ",";
            }
            file << "\n";
        }
        file << "        ]\n";
        file << "      }\n";
        file << "    }\n";
    }

    // Fin du GeoJSON
    file << "  ]\n";
    file << "}\n";
    file.close();
}
