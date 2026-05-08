// Per-path electromagnetic result: the finalized output of propagation and
// interaction processing, carrying delay, phase, complex amplitude, power,
// polarization, and metadata for a single geometric ray path.

#pragma once

#include "../path/GeometricPath.h"

#include <string>
#include <vector>

namespace rt {

/// <summary>
/// Final per-path EM result produced by FinalizeAtReceiver, containing the
/// total accumulated delay, complex amplitude, power, polarization, and
/// metadata needed for CIR/PDP/APS aggregation.
/// </summary>
struct EMPathResult {
    int path_id = -1;                   ///< Unique path identifier from the search engine.
    bool valid = false;                 ///< Whether this result carries valid EM data.
    double total_length_m = 0.0;        ///< Total geometric path length from Tx to Rx.
    double delay_s = 0.0;               ///< Total propagation delay = total_length_m / c0.
    double phase_rad = 0.0;             ///< Cumulative phase at the receiver (includes interaction phase shifts).
    double amplitude_real = 0.0;        ///< Real part of the complex received field amplitude.
    double amplitude_imag = 0.0;        ///< Imaginary part of the complex received field amplitude.
    double power_linear = 0.0;          ///< Received linear power = |amplitude|^2.
    double free_space_amplitude_scale = 0.0; ///< FSPL amplitude factor = wavelength / (4 * pi * d).
    double free_space_power_scale = 0.0;     ///< FSPL power factor = (amplitude_scale)^2.
    double wavelength_m = 0.0;          ///< Free-space wavelength used for this path.
    double polarization_magnitude = 0.0; ///< Magnitude of the polarization vector.
    double free_space_loss_db = 0.0;    ///< Free-space path loss in dB = -10*log10(power_scale).
    std::string tx_antenna_id;          ///< Transmit antenna identifier.
    std::string tx_antenna_source_type; ///< Transmit antenna source type.
    std::string rx_antenna_id;          ///< Receive antenna identifier.
    std::string rx_antenna_source_type; ///< Receive antenna source type.
    Vec3 polarization_vector;           ///< 极化方向实部 (Jones矢量实分量).
    Vec3 polarization_imag;            ///< v5 D6-A: 极化方向虚部 (Jones矢量虚分量).
    bool is_los = false;                ///< Whether this is a line-of-sight path.
    uint64_t source_path_signature = 0; ///< Hash/signature of the source geometric path.
    std::string source_tag = "unknown"; ///< Label indicating the path origin (e.g. "search_engine_real_output").
    bool contains_transmission = false; ///< Whether the path includes any transmission interactions.
    bool transmission_semantic_consumed = false; ///< Whether a transmission interaction was applied.
    int first_transmission_medium_in_id = -1;    ///< Medium ID on the incident side of the first transmission.
    int first_transmission_medium_out_id = -1;   ///< Medium ID on the transmitted side of the first transmission.
    int last_transmission_medium_in_id = -1;     ///< Medium ID on the incident side of the last transmission.
    int last_transmission_medium_out_id = -1;    ///< Medium ID on the transmitted side of the last transmission.
};

/// <summary>
/// Collection of per-path EM results produced by the solver, with metadata
/// about the source (search engine, input path count, valid count).
/// </summary>
struct EMPathResultSet {
    std::vector<EMPathResult> results;  ///< Ordered list of per-path results.
    bool from_search_engine = false;    ///< Whether these results originate from the search engine.
    int input_path_count = 0;           ///< Total number of paths fed into the EM solver.
    int valid_result_count = 0;         ///< Number of paths that produced valid results.
    std::string source_tag = "unknown"; ///< Label indicating the source pipeline stage.
};

} // namespace rt
