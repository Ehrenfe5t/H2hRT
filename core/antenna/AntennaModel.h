#pragma once
#include "../scene/Face.h"
#include "AntennaPattern.h"
#include <string>
#include <vector>

namespace rt {

struct AntennaModel {
    std::string antenna_id;
    std::string source_type = "Ideal";
    bool is_tx = true;
    bool is_ideal = true;
    double frequency_hz = 0.0;
    Point3 position;
    Vec3 forward = MakeVec3(1,0,0);
    Vec3 right = MakeVec3(0,1,0);
    Vec3 up = MakeVec3(0,0,1);
    Vec3 polarization_vector = MakeVec3(1,0,0);
    double reference_gain_linear = 1.0;
    double phase_center_offset_m = 0.0;
    std::string pattern_file;
    std::string polarization_file;
    std::string custom_metadata;
    AntennaPattern pattern;
};

struct AntennaResponse {
    bool valid = false;
    std::string antenna_id;
    std::string source_type = "Ideal";
    double gain_linear = 1.0;
    double gain_db = 0.0;
    double polarization_alignment = 1.0;
    Vec3 effective_polarization;
};

struct AntennaArrayModel {
    std::string array_id;
    std::vector<AntennaModel> elements;
    bool valid = false;
};

} // namespace rt
