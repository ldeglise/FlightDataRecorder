#include "DataRefManager.hpp"
#include <stdexcept>
#include <cstring>
#include <string.h>

DataRefManager::DataRefManager() = default;
DataRefManager::~DataRefManager() = default;

XPLMDataRef DataRefManager::getDataRef(const std::string& datarefName) {
    auto it = dataRefs.find(datarefName);
    if (it != dataRefs.end()) {
        return it->second;
    }
    XPLMDataRef ref = XPLMFindDataRef(datarefName.c_str());
    if (!ref) {
        throw std::runtime_error("DataRef not found: " + datarefName);
    }
    dataRefs[datarefName] = ref;
    return ref;
}

float DataRefManager::getFloatDataRef(const std::string& datarefName) {
    XPLMDataRef ref = getDataRef(datarefName);
    return XPLMGetDataf(ref);
}

int DataRefManager::getIntDataRef(const std::string& datarefName) {
    XPLMDataRef ref = getDataRef(datarefName);
    return XPLMGetDatai(ref);
}

PositionData DataRefManager::getPositionData() {
    PositionData pos;
    
    // Récupérer les DataRefs pour latitude, longitude et elevation
    XPLMDataRef latRef = getDataRef("sim/flightmodel/position/latitude");
    XPLMDataRef lonRef = getDataRef("sim/flightmodel/position/longitude");
    XPLMDataRef elevRef = getDataRef("sim/flightmodel/position/elevation");
    
    // Lire les valeurs de manière atomique (simultanée)
    pos.latitude = XPLMGetDataf(latRef);
    pos.longitude = XPLMGetDataf(lonRef);
    pos.elevation_msl = XPLMGetDataf(elevRef);
    
    return pos;
}

std::string DataRefManager::getStringDataRef(const std::string& datarefName) {
    XPLMDataRef ref = getDataRef(datarefName);
    char buffer[256];
    // XPLMGetDatavf attend un float* mais les strings sont stockés comme char*
    // On utilise un buffer float temporaire puis on copie les bytes
    float floatBuffer[64]; // 64 floats = 256 bytes
    XPLMGetDatavf(ref, floatBuffer, 0, 64);
    // Copier les bytes du float buffer vers le char buffer
    memcpy(buffer, floatBuffer, 256);
    return std::string(buffer);
}
