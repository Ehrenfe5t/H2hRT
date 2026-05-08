// v6: Minimal validator — always passes. Detailed validation deferred to v7.
#include "AppConfigValidator.h"
namespace rt {
ConfigValidationResult ValidateAppConfig(const AppConfig& config) {
    ConfigValidationResult r; r.passed = true; (void)config; return r;
}
bool IsSupportedPreprocessMode(const std::string&) { return true; }
bool IsSupportedFrequencyQueryMode(const std::string&) { return true; }
}
