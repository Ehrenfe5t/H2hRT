#pragma once

#include "../common/config/AppConfig.h"
#include "../path/PathState.h"

#include <cstdint>

namespace rt {

uint64_t BuildStateSignature(const PathState& state, const AppConfig& config);

} // namespace rt
