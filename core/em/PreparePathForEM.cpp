// Validates EMSolverInput pointers, path validity, minimum node count, and
// transmission semantic completeness before entering the EM solver pipeline.

#include "PreparePathForEM.h"

namespace rt {

/// <summary>
/// Performs minimum pre-checks on a path before entering the EM solver pipeline.
/// Validates that config, scene, and path pointers are non-null, the path is marked
/// valid and has at least 2 nodes, and all transmission interaction nodes carry
/// complete semantic data (medium IDs, intersection side). On failure, transmission
/// completeness flags are set on the input for diagnostic use.
/// </summary>
/// <param name="input">Solver input bundle to validate (mutates transmission_semantic_complete fields).</param>
/// <returns>true if the path passes all pre-checks and can enter the EM pipeline.</returns>
bool PreparePathForEM(EMSolverInput& input)  // v10.2 B6修复: 非const引用，合法修改诊断标志
{
    // Null-pointer guard: all three core references must be valid
    if (input.config == nullptr || input.scene == nullptr || input.path == nullptr)
    {
        return false;
    }
    // Path must be marked valid by the search engine
    if (!input.path->valid)
    {
        return false;
    }
    // At minimum, a path needs a Tx node and an Rx node
    if (input.path->nodes.size() < 2)
    {
        return false;
    }

    // Scan all interaction nodes to verify transmission semantic completeness.
    // Each transmission node must have:
    //  - transmission_semantic_complete == true
    //  - valid medium_in_id and medium_out_id (non-negative, different from each other)
    bool transmissionSemanticComplete = true;
    int firstTransmissionMediumIn = -1;
    int firstTransmissionMediumOut = -1;
    bool firstTransmissionCaptured = false;

    for (const PathNode& node : input.path->nodes)
    {
        if (node.interaction_type != InteractionType::Transmission)
        {
            continue; // only transmission nodes need semantic check
        }

        if (!node.transmission_semantic_complete)
        {
            transmissionSemanticComplete = false;
            break;
        }

        if (node.medium_in_id < 0 || node.medium_out_id < 0)
        {
            transmissionSemanticComplete = false;
            break;
        }

        // Same medium on both sides indicates no boundary crossing
        if (node.medium_in_id == node.medium_out_id)
        {
            transmissionSemanticComplete = false;
            break;
        }

        // Capture the first valid transmission's medium pair for result metadata
        if (!firstTransmissionCaptured)
        {
            firstTransmissionMediumIn = node.medium_in_id;
            firstTransmissionMediumOut = node.medium_out_id;
            firstTransmissionCaptured = true;
        }
    }

    // v10.2 B6修复: 直接写入，无需const_cast
    input.transmission_semantic_complete = transmissionSemanticComplete;
    input.first_transmission_medium_in_id = firstTransmissionMediumIn;
    input.first_transmission_medium_out_id = firstTransmissionMediumOut;

    // Block path if any transmission node lacks complete semantics
    if (!transmissionSemanticComplete)
    {
        return false;
    }

    return true;
}

} // namespace rt
