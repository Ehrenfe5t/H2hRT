#pragma once

#include "../common/config/AppConfig.h"
#include "../path/GeometricPath.h"

#include <cstdint>

namespace rt {

uint64_t BuildPathSignature(const GeometricPath& path, const AppConfig& config);

} // namespace rt
