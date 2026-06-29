#pragma once

#include "../common/material/MaterialDatabase.h"
#include "../path/GeometricPath.h"
#include "../query/SceneQuery.h"
#include "../scene/Scene.h"

#include <string>
#include <vector>

namespace rt {

struct SbrPathRefineStats {
    long long input_candidates = 0;
    long long topology_groups = 0;
    long long refined_paths = 0;
    long long rejected_paths = 0;
};

std::vector<int> BuildSbrSurfacePatchIds(const Scene& scene);

bool RefineSbrPathGeometry(
    GeometricPath& path,
    const Scene& scene,
    const SceneQuery* query,
    const MaterialDatabase* materialDb,
    double frequencyHz,
    const std::vector<int>& faceToPatch,
    std::string* reason);

SbrPathRefineStats RefineSbrCandidatesForEm(
    std::vector<GeometricPath>& paths,
    const Scene& scene,
    const SceneQuery& query,
    const MaterialDatabase* materialDb,
    double frequencyHz);

} // namespace rt
