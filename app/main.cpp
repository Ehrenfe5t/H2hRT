// Console entry point for the RT raytracing pipeline.
// Parses an optional config-file path from the command line, delegates all work to RtPipeline,
// and returns a unified exit code for scripting / CI use.

#include "RtPipeline.h"

#include <exception>
#include <iostream>
#include <string>

/// <summary>
/// Application entry point. Parses an optional config-file path from argv[1],
/// delegates to RtPipeline::Run, and returns a unified exit code.
/// </summary>
/// <param name="argc">Number of command-line arguments.</param>
/// <param name="argv">Command-line argument array; argv[1], if present, is the config file path.</param>
/// <returns>Process exit code; 0 = success, nonzero = failure.</returns>
int main(int argc, char* argv[])
{
    try
    {
        // Resolve config path: CLI arg first, otherwise fall back to the minimal default.
        const std::string configPath = (argc > 1) ? argv[1] : "configs/app/meeting_v3.json";

        // Bootstrap the full pipeline (config, validation, batches, A1 chain, SBR).
        rt::RtPipeline pipeline;
        const rt::PipelineRunResult result = pipeline.Run(configPath);

        if (!result.succeeded)
        {
            std::cerr << "RT pipeline failed." << std::endl;
        }

        return result.exit_code;
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Unhandled exception: " << ex.what() << std::endl;
        return -1;
    }
}
