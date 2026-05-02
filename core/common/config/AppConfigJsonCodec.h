// File purpose:
// - Declare the formal JSON codec used by module 1 for reading and writing AppConfig.
//
// Main responsibilities:
// - Parse JSON files into AppConfig via a structured property tree.
// - Serialize AppConfig back to JSON for configuration snapshots.
// - Keep JSON mapping logic centralized so loader and snapshot writer stay consistent.

#pragma once

#include "AppConfig.h"

#include <string>

namespace rt {

/// <summary>
/// Structured JSON decode result for AppConfig.
/// </summary>
struct AppConfigJsonDecodeResult {
    bool succeeded = false;
    AppConfig config;
    std::string error_message;
};

/// <summary>
/// Decode AppConfig from a JSON file using the formal module-1 JSON path.
/// </summary>
/// <param name="filePath">Input JSON config file path.</param>
/// <returns>Decode result containing success state, config object and error message.</returns>
AppConfigJsonDecodeResult DecodeAppConfigFromJsonFile(const std::string& filePath);

/// <summary>
/// Encode AppConfig into JSON text.
/// </summary>
/// <param name="config">Configuration object to serialize.</param>
/// <returns>Formatted JSON string.</returns>
std::string EncodeAppConfigToJsonString(const AppConfig& config);

} // namespace rt
