// Per-path electromagnetic result: the finalized output of propagation and
// interaction processing, carrying delay, phase, complex amplitude, power,
// polarization, and metadata for a single geometric ray path.

#pragma once

#include "../path/GeometricPath.h"
#include "FieldAccumulator.h"

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
    double incident_power_linear = 0.0; ///< Power at Rx before Rx gain/polarization filtering.
    double sampling_weight = 1.0;       ///< SBR launch-power fraction represented by the path.
    int candidate_support_count = 1;    ///< Number of launch rays that discovered this physical topology.
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
    // v7 H10: 离开角AoD和到达角AoA
    double aod_theta_deg = 0.0;          ///< Angle of Departure zenith [deg], 0=zenith(up), 90=horizon.
    double aod_phi_deg = 0.0;            ///< Angle of Departure azimuth [deg], 0=+X, 90=+Y.
    double aoa_theta_deg = 0.0;          ///< Angle of Arrival zenith [deg].
    double aoa_phi_deg = 0.0;            ///< Angle of Arrival azimuth [deg].
    // v9 B-8: 极化统计
    double co_pol_power_linear = 0.0;    ///< Co-polarized received power.
    double cross_pol_power_linear = 0.0; ///< Cross-polarized received power.
    double xpr_dB = 0.0;                  ///< Cross-Polarization Ratio = 10*log10(co/cross).
    // v11.1: Tx power fields for absolute received power semantics
    double tx_power_dBm = 0.0;           ///< Transmit power in dBm (from antenna.json tx.power_dBm).
    double power_dBm = 0.0;              ///< Received power in dBm = 10*log10(power_linear * 1000).
    ComplexVec3 incident_electric_field_world_v_per_m; ///< Physical RMS incident E-field at Rx [V/m].
    Complex channel_coefficient;         ///< Dimensionless received power-wave / sqrt(Tx power W).
    std::vector<NodeFieldTrace> node_field_trace; ///< Node-by-node complex polarization evolution.
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
