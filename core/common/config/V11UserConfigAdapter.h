#pragma once

#include "AppConfig.h"

#include <string>

namespace rt {

struct V11UserConfigDecodeResult {
    bool recognized = false;
    bool succeeded = false;
    AppConfig config;
    std::string error_message;
};

V11UserConfigDecodeResult DecodeV11UserConfigFromJsonFile(const std::string& filePath);

} // namespace rt
