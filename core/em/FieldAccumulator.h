// Per-path field state accumulator that tracks complex amplitude, phase, delay,
// power, medium attenuation, and polarization through propagation and interactions.

#pragma once

#include "../path/InteractionType.h"
#include "../scene/Face.h"

namespace rt {

/// <summary>
/// Per-path field state accumulator -- the working state that propagates through
/// free-space segments and interaction nodes before being finalized into EMPathResult.
/// </summary>
struct FieldAccumulator {
    double frequency_hz = 0.0;           ///< Carrier frequency used for this path.
    double wavelength_m = 0.0;           ///< Free-space wavelength = c0 / frequency_hz.
    double total_length_m = 0.0;         ///< Cumulative geometric path length (sum of all segment lengths).
    double delay_s = 0.0;                ///< Cumulative propagation delay = total_length_m / c0.
    double phase_rad = 0.0;              ///< Cumulative phase accumulation (negative from propagation, plus interaction phase shifts).
    double amplitude_real = 1.0;         ///< Real part of the complex field amplitude (starts at 1.0).
    double amplitude_imag = 0.0;         ///< Imaginary part of the complex field amplitude (starts at 0.0).
    double power_linear = 1.0;           ///< Current linear power = |amplitude|^2.
    double free_space_amplitude_scale = 1.0;  ///< FSPL amplitude scale, applied once at FinalizeAtReceiver (not per segment).
    double free_space_power_scale = 1.0;      ///< FSPL power scale = (amplitude_scale)^2.
    double last_segment_length_m = 0.0;  ///< Length of the most recent propagation segment (used in diffraction distance parameter L).
    std::string tx_antenna_id;           ///< Transmit antenna identifier.
    std::string tx_antenna_source_type;  ///< Transmit antenna source type (e.g. "ideal", "pattern").
    std::string rx_antenna_id;           ///< Receive antenna identifier.
    std::string rx_antenna_source_type;  ///< Receive antenna source type.
    int current_medium_id = -1;          ///< ID of the medium the ray is currently traveling in (-1 = free space).
    int last_transmission_medium_in_id = -1;   ///< Medium ID on the incident side of the last transmission.
    int last_transmission_medium_out_id = -1;  ///< Medium ID on the transmitted side of the last transmission.
    double current_attenuation_np_per_m = 0.0; ///< Attenuation constant (Np/m) of the current medium (0 = lossless).
    double media_attenuation_np = 0.0;   ///< Cumulative exponential attenuation in nepers = sum(alpha_i * d_i).
    bool transmission_semantic_consumed = false; ///< Whether a transmission interaction has already been applied on this path.
    Vec3 polarization_vector;            ///< Current polarization direction (unit vector in 3D).
    bool valid = false;                  ///< Set to true after successful InitializeTxField; false aborts the path.
};

} // namespace rt
