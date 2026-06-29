#pragma once
#include "../common/config/AppConfig.h"
#include "../common/material/MaterialDatabase.h"
#include "../antenna/AntennaModel.h"
#include "../path/GeometricPath.h"
#include "../scene/Scene.h"

namespace rt {

struct EMSolverInput {
    const AppConfig* config = nullptr;
    const Scene* scene = nullptr;
    const GeometricPath* path = nullptr;
    const AntennaModel* tx_antenna = nullptr;
    const AntennaModel* rx_antenna = nullptr;
    const MaterialDatabase* material_db = nullptr;
    // v11.1: Tx power for absolute receive power scaling in InitializeTxField
    double tx_power_dBm = 0.0;
    bool transmission_semantic_complete = true;
    int first_transmission_medium_in_id = -1;
    int first_transmission_medium_out_id = -1;
};

} // namespace rt
