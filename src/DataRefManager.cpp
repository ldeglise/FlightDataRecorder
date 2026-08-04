// =============================================================================
// DataRefManager.cpp
// =============================================================================
#include "DataRefManager.hpp"
#include <XPLMUtilities.h>

DataRefManager::DataRefManager() = default;
DataRefManager::~DataRefManager() { Cleanup(); }

bool DataRefManager::Initialize() {
    // Exemple : Enregistre les DataRefs de base
    if (!RegisterDataRef("sim/flightmodel/position/latitude") ||
        !RegisterDataRef("sim/flightmodel/position/longitude") ||
        !RegisterDataRef("sim/flightmodel/position/elevation") ||
        !RegisterDataRef("sim/flightmodel/position/psi") ||
        !RegisterDataRef("sim/flightmodel/position/indicated_airspeed") ||
        !RegisterDataRef("sim/flightmodel/position/groundspeed")) {
        XPLMDebugString("DataRefManager: Échec de l'enregistrement des DataRefs.\n");
        return false;
    }
    return true;
}

void DataRefManager::Cleanup() {
    mDataRefs.clear();
}

float DataRefManager::GetDataRefValue(const std::string& refName) const {
    auto it = mDataRefs.find(refName);
    if (it == mDataRefs.end()) {
        std::string msg = "DataRefManager: DataRef non trouvée: " + refName + "\n";
        XPLMDebugString(msg.c_str());
        return 0.0f;
    }
    return XPLMGetDataf(it->second);
}

bool DataRefManager::RegisterDataRef(const std::string& refName) {
    XPLMDataRef ref = XPLMFindDataRef(refName.c_str());
    if (!ref) {
        std::string msg = "DataRefManager: DataRef introuvable: " + refName + "\n";
        XPLMDebugString(msg.c_str());
        return false;
    }
    mDataRefs[refName] = ref;
    return true;
}