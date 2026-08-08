#include "DataRefManager.hpp"
#include <stdexcept>
#include <cstring>

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

std::string DataRefManager::getStringDataRef(const std::string& datarefName) {
    XPLMDataRef ref = getDataRef(datarefName);
    char buffer[256];
    // XPLMGetDatavf attend un float*, mais les strings sont stockés comme char*
    // On utilise reinterpret_cast pour contourner le typage strict
    XPLMGetDatavf(ref, reinterpret_cast<float*>(buffer), 0, 256);
    return std::string(buffer);
}
