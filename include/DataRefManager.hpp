// =============================================================================
// DataRefManager.hpp
// Gère l'accès aux DataRefs X-Plane.
// =============================================================================
#pragma once
#include <XPLMDataAccess.h>
#include <string>
#include <unordered_map>

class DataRefManager {
public:
    DataRefManager();
    ~DataRefManager();

    bool Initialize();
    void Cleanup();

    // Récupère la valeur d'une DataRef (float)
    float GetDataRefValue(const std::string& refName) const;

    // Enregistre une DataRef pour une utilisation ultérieure
    bool RegisterDataRef(const std::string& refName);

private:
    std::unordered_map<std::string, XPLMDataRef> mDataRefs;
};