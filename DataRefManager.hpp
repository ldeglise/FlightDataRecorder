#pragma once

#include <string>
#include <unordered_map>
#include "XPLMDataAccess.h"

class DataRefManager {
public:
    DataRefManager();
    ~DataRefManager();

    float getFloatDataRef(const std::string& datarefName);
    int getIntDataRef(const std::string& datarefName);
    std::string getStringDataRef(const std::string& datarefName);

    // Conversions pour la détection des phases de vol (en interne uniquement)
    static float metersToFeet(float meters) { return meters * 3.28084f; }
    static float kmhToKnots(float kmh) { return kmh * 0.539957f; }

private:
    std::unordered_map<std::string, XPLMDataRef> dataRefs;
    XPLMDataRef getDataRef(const std::string& datarefName);
};
