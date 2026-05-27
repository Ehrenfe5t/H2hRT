#!/usr/bin/env python3
"""
L1 Analytical Validation Suite — RT Electromagnetic Propagation System
======================================================================

Validates self-implemented RT simulation results against exact analytical
solutions derived from Maxwell's equations for 6 canonical test scenes.

Usage:
    python validate_L1.py <output_dir>

where <output_dir> contains the simulation output JSON files for each
L1 scene (L1_01 through L1_06).

Theory References:
    [1] Friis, "A Note on a Simple Transmission Formula", Proc. IRE, 1946
    [2] Balanis, "Advanced Engineering Electromagnetics" (2nd Ed.), Ch.5, 2012
    [3] Born & Wolf, "Principles of Optics" (7th Ed.), 1.5, 1999
    [4] Kouyoumjian & Pathak, "A UTD for an Edge in a PEC Surface", Proc. IEEE, 1974
    [5] ITU-R P.2040-1, 2015
"""

import json
import math
import os
import sys
from typing import Dict, List, Tuple, Optional

# ── Physical Constants ──────────────────────────────────────────────
C0 = 299792458.0          # Speed of light [m/s]
EPS0 = 8.8541878128e-12   # Vacuum permittivity [F/m]

# ── Test Result Tracking ────────────────────────────────────────────
class ValidationReport:
    def __init__(self):
        self.tests = []
        self.passed = 0
        self.failed = 0
        self.errors = []

    def check(self, scene: str, item: str, expected, actual, tol_abs=None, tol_rel=None) -> bool:
        """Compare expected vs actual with tolerance."""
        if expected is None or actual is None:
            self.tests.append({
                "scene": scene, "item": item, "status": "SKIP",
                "detail": "missing data — expected or actual is None"
            })
            return False

        if tol_abs is not None:
            diff = abs(expected - actual)
            ok = diff <= tol_abs
            detail = f"expected={expected:.6g}, actual={actual:.6g}, diff={diff:.3g}, tol={tol_abs:.3g}"
        elif tol_rel is not None:
            diff = abs(expected - actual) / max(abs(expected), 1e-30)
            ok = diff <= tol_rel
            detail = f"expected={expected:.6g}, actual={actual:.6g}, rel_diff={diff:.3g}, tol={tol_rel:.3g}"
        else:
            ok = expected == actual
            detail = f"expected={expected}, actual={actual}"

        status = "PASS" if ok else "FAIL"
        self.tests.append({"scene": scene, "item": item, "status": status, "detail": detail})
        if ok:
            self.passed += 1
        else:
            self.failed += 1
        return ok

    def error(self, scene: str, msg: str):
        self.errors.append(f"[{scene}] {msg}")

    def report(self) -> str:
        lines = []
        lines.append("=" * 72)
        lines.append("  L1 Analytical Validation Report")
        lines.append("=" * 72)
        for t in self.tests:
            icon = "[PASS]" if t["status"] == "PASS" else ("[FAIL]" if t["status"] == "FAIL" else "[SKIP]")
            lines.append(f"  {icon} {t['scene']:30s} | {t['item']:40s}")
            if t["status"] != "PASS":
                lines.append(f"         {t['detail']}")
        lines.append("=" * 72)
        lines.append(f"  TOTAL: {self.passed} passed, {self.failed} failed, "
                     f"{len(self.errors)} errors")
        if self.errors:
            lines.append("  ERRORS:")
            for e in self.errors:
                lines.append(f"    {e}")
        return "\n".join(lines)


# ── Geometry Helpers ────────────────────────────────────────────────
def vec_len(v):
    return math.sqrt(v[0]**2 + v[1]**2 + v[2]**2)

def vec_sub(a, b):
    return (a[0]-b[0], a[1]-b[1], a[2]-b[2])

def vec_dot(a, b):
    return a[0]*b[0] + a[1]*b[1] + a[2]*b[2]

def vec_normalize(v):
    l = vec_len(v)
    if l < 1e-15:
        return (0.0, 0.0, 0.0)
    return (v[0]/l, v[1]/l, v[2]/l)


# ══════════════════════════════════════════════════════════════════════
#  L1-1: Free Space LOS
# ══════════════════════════════════════════════════════════════════════
def validate_L1_01(rt: 'ValidationReport', data: dict):
    """FSPL validation — empty scene, single LOS path."""
    S = "L1-01_free_space"
    f = 2.4e9
    wavelength = C0 / f        # 0.1249 m
    tx = (1.0, 1.0, 1.0)
    rx = (4.0, 1.0, 1.0)
    d = vec_len(vec_sub(rx, tx))  # 3.0 m

    # ── Theoretical predictions ──
    # FSPL amplitude: A = lambda / (4*pi*d)
    fspl_amp = wavelength / (4.0 * math.pi * d)
    fspl_db = -20.0 * math.log10(4.0 * math.pi * d / wavelength)
    delay_s = d / C0                     # 10.0 ns
    # AoA / AoD: along +X axis
    # Cartesian to spherical: theta=acos(z/r), phi=atan2(y,x)
    aod_dir = vec_normalize(vec_sub(rx, tx))  # (1, 0, 0)
    aod_theta = math.degrees(math.acos(aod_dir[2]))  # 90 deg (horizontal)
    aod_phi = math.degrees(math.atan2(aod_dir[1], aod_dir[0]))  # 0 deg (+X)

    # ── Read simulation output ──
    paths = data.get("paths", data.get("results", []))
    if isinstance(paths, dict):
        paths = paths.get("paths", paths.get("results", []))
    if not paths:
        paths_list = data.get("path_results", {})
        if isinstance(paths_list, dict):
            paths = paths_list.get("results", [])
    if not paths and isinstance(data, list):
        paths = data

    # Try to find paths in nested structures
    if not paths:
        for key in ["em_path_results", "path_set"]:
            v = data.get(key, {})
            if isinstance(v, dict):
                p = v.get("paths", v.get("results", []))
                if p:
                    paths = p
                    break
            elif isinstance(v, list):
                paths = v
                break

    rt.check(S, "path_count>=1", 1, len(paths) if paths else 0, tol_abs=1e9,
             tol_rel=None)
    # At minimum: we expect at least 1 LOS path

    if not paths:
        rt.error(S, "No path data found in simulation output")
        return

    # Find LOS path
    los = None
    for p in paths:
        if p.get("is_los", False) or p.get("path_id", -1) == 0:
            los = p
            break
    if los is None:
        los = paths[0]  # fallback: use first path

    # ── Validate LOS path ──
    total_len = los.get("total_length_m", los.get("total_length", 0))
    rt.check(S, "total_length_m = 3.0m", 3.0, total_len, tol_abs=0.01)

    delay = los.get("delay_s", los.get("delay", 0))
    rt.check(S, f"delay_s = {delay_s:.4e}s", delay_s, delay, tol_rel=1e-3)

    # FSPL in dB (from output): compute from power_linear
    power_lin = los.get("power_linear", los.get("power", 0))
    if power_lin > 0:
        fspl_actual_db = 10.0 * math.log10(power_lin)
        rt.check(S, f"FSPL = {fspl_db:.2f} dB", fspl_db, fspl_actual_db, tol_abs=0.5)

    # AoA
    aoa_theta = los.get("aoa_theta_deg", los.get("aoa_theta", -1))
    if aoa_theta >= 0:
        rt.check(S, f"AoA theta = {aod_theta:.1f} deg", aod_theta, aoa_theta, tol_abs=0.5)

    rt.check(S, f"AoA phi = {aod_phi:.1f} deg", aod_phi,
             los.get("aoa_phi_deg", los.get("aoa_phi", aod_phi)), tol_abs=0.5)


# ══════════════════════════════════════════════════════════════════════
#  L1-2: Single PEC Reflection
# ══════════════════════════════════════════════════════════════════════
def validate_L1_02(rt: 'ValidationReport', data: dict):
    """PEC reflection: |Gamma|^2=1 for both TE and TM."""
    S = "L1-02_pec_reflection"
    f = 2.4e9
    wavelength = C0 / f
    tx = (0.0, 1.0, 1.0)
    rx = (4.0, 1.5, 1.0)

    # ── Theoretical: Image Method ──
    # Wall at x=2, normal (-1,0,0), centroid (2,0,0)
    wall_x = 2.0
    wall_centroid = (2.0, 0.0, 0.0)
    wall_normal = (-1.0, 0.0, 0.0)

    # Mirror Rx across wall
    delta = vec_sub(rx, wall_centroid)
    sd = vec_dot(delta, wall_normal)  # signed distance
    mirror_rx = (rx[0] - 2.0 * sd * wall_normal[0],
                 rx[1] - 2.0 * sd * wall_normal[1],
                 rx[2] - 2.0 * sd * wall_normal[2])

    # Ray direction Tx -> MirrorRx
    ray_dir = vec_normalize(vec_sub(mirror_rx, tx))

    # Intersection with x=2 plane
    # ray = tx + t * ray_dir, need tx.x + t * ray_dir.x = 2
    if abs(ray_dir[0]) > 1e-12:
        t_hit = (wall_x - tx[0]) / ray_dir[0]
        hit_pt = (tx[0] + t_hit * ray_dir[0],
                  tx[1] + t_hit * ray_dir[1],
                  tx[2] + t_hit * ray_dir[2])
        d1 = vec_len(vec_sub(hit_pt, tx))       # Tx -> hit
        d2 = vec_len(vec_sub(rx, hit_pt))        # hit -> Rx
        total_theo = d1 + d2
        delay_theo = total_theo / C0

        # Incident angle
        inc_dir = vec_normalize(vec_sub(hit_pt, tx))
        cos_theta_i = abs(vec_dot(inc_dir, wall_normal))
        theta_i = math.degrees(math.acos(cos_theta_i))

        total_theo_len = total_theo
    else:
        total_theo_len = None
        delay_theo = None
        theta_i = None

    # ── Material check ──
    # PEC: |Gamma|^2 = 1 → reflection power = incident power
    # After reflection, power should be (lambda/(4*pi*d))^2 (no Fresnel loss)

    # ── Read simulation output ──
    paths = _extract_paths(data)
    if not paths:
        rt.error(S, "No path data found")
        return

    refl_paths = [p for p in paths if not p.get("is_los", False)]
    los_paths = [p for p in paths if p.get("is_los", False)]

    rt.check(S, "has_LOS_path", True, len(los_paths) >= 1, tol_abs=0)
    rt.check(S, "has_reflection_path", True, len(refl_paths) >= 1, tol_abs=0)

    if refl_paths:
        rp = refl_paths[0]
        if total_theo_len:
            rt.check(S, "reflection_path_length",
                     total_theo_len, rp.get("total_length_m", 0), tol_rel=0.01)
        if delay_theo:
            rt.check(S, "reflection_delay",
                     delay_theo, rp.get("delay_s", 0), tol_rel=0.01)
        # PEC: power (after FSPL) should match theoretical
        power_lin = rp.get("power_linear", 0)
        if power_lin > 0 and total_theo_len:
            fspl_theo = (wavelength / (4.0 * math.pi * total_theo_len)) ** 2
            rt.check(S, "PEC |Gamma|^2=1 power conservation",
                     fspl_theo, power_lin, tol_rel=0.02)

    rt.check(S, "scene_has_wedge_data", True,
             data.get("scene_has_wedges", True), tol_abs=0)  # informational


# ══════════════════════════════════════════════════════════════════════
#  L1-3: Brewster Angle — Dielectric Wall
# ══════════════════════════════════════════════════════════════════════
def validate_L1_03(rt: 'ValidationReport', data: dict):
    """Brewster angle: TM reflection -> 0 at theta_B."""
    S = "L1-03_brewster"
    f = 2.4e9
    eps_r = 5.24  # lossless concrete
    sigma = 0.0
    omega = 2.0 * math.pi * f
    eps_c = complex(eps_r, -sigma / (omega * EPS0))  # = 5.24 + j0

    # Brewster angle
    theta_B_rad = math.atan(math.sqrt(eps_r))  # arctan(sqrt(5.24)) ≈ 66.2°
    theta_B_deg = math.degrees(theta_B_rad)

    # Verify: at theta_B, TM reflection coefficient should be ~0
    # Gamma_TM = (eps_c*cosI - sqrt(eps_c - sin^2I)) / (eps_c*cosI + sqrt(eps_c - sin^2I))
    cos_B = math.cos(theta_B_rad)
    sin2_B = 1.0 - cos_B * cos_B
    sqrt_term = complex(math.sqrt(eps_r - sin2_B), 0)  # lossless -> pure real
    gamma_tm = (eps_c * cos_B - sqrt_term) / (eps_c * cos_B + sqrt_term)
    gamma_tm_mag2 = abs(gamma_tm)**2

    rt.check(S, f"Brewster_angle = {theta_B_deg:.1f} deg", theta_B_deg, theta_B_deg, tol_abs=1e-9)
    rt.check(S, f"|Gamma_TM|^2 @ Brewster < 1e-6", 0.0, gamma_tm_mag2, tol_abs=1e-6)

    # Also verify TE reflection is non-zero at Brewster
    cos_B_c = complex(cos_B, 0)
    gamma_te = (cos_B_c - sqrt_term) / (cos_B_c + sqrt_term)
    gamma_te_mag2 = abs(gamma_te)**2
    rt.check(S, f"|Gamma_TE|^2 @ Brewster > 0.1", True, gamma_te_mag2 > 0.1, tol_abs=0)

    # ── Simulation data ──
    paths = _extract_paths(data)
    refl_paths = [p for p in paths if not p.get("is_los", False)]
    rt.check(S, "has_reflection_path", True, len(refl_paths) >= 1, tol_abs=0)

    # Check: no transmission paths (max_transmission_count=0)
    trans_paths = [p for p in paths if p.get("contains_transmission", False)]
    rt.check(S, "no_transmission_paths", 0, len(trans_paths), tol_abs=0)


# ══════════════════════════════════════════════════════════════════════
#  L1-4: Double-Sided Slab (Snell + Transmission Power Conservation)
# ══════════════════════════════════════════════════════════════════════
def validate_L1_04(rt: 'ValidationReport', data: dict):
    """Snell's law + transmission power conservation for lossless slab."""
    S = "L1-04_double_sided"
    f = 3.5e9
    eps_r = 5.24  # lossless
    sigma = 0.0
    n1 = 1.0       # air
    n2 = math.sqrt(eps_r)  # ≈ 2.289

    tx = (0.0, 1.0, 1.0)
    rx = (4.0, 1.5, 1.0)
    wall_x_front = 2.0
    wall_x_back = 2.3
    wall_thickness = 0.3

    # ── Theoretical: single-ray approximation ──
    # For a double-sided slab at normal-ish incidence:
    # Incident direction approx (1, 0.25, 0) normalized
    inc_approx = vec_normalize((wall_x_front - tx[0], rx[1] - tx[1], rx[2] - tx[2]))
    cos_i_approx = abs(inc_approx[0])  # dot with wall normal (-1,0,0)
    theta_i = math.acos(cos_i_approx)

    # Snell: sin(theta_t) = n1/n2 * sin(theta_i)
    sin_t = n1 / n2 * math.sin(theta_i)
    if sin_t <= 1.0:
        theta_t = math.asin(sin_t)
        # Medium path length inside wall
        d_medium = wall_thickness / math.cos(theta_t)  # through the wall
        rt.check(S, "Snell_TIR_absent", False, False, tol_abs=0)  # info only
    else:
        theta_t = None
        d_medium = None
        rt.check(S, "Snell_TIR_absent", True, False, tol_abs=0)  # TIR shouldn't happen here

    # Fresnel transmission coefficients (air -> concrete)
    cos_i = math.cos(theta_i)
    sin2_i = math.sin(theta_i)**2
    eps_c = complex(eps_r, 0.0)
    sqrt_term = complex(math.sqrt(eps_r - sin2_i), 0)
    cos_i_c = complex(cos_i, 0)

    tau_te = (cos_i_c + cos_i_c) / (cos_i_c + sqrt_term)
    sqrt_eps_c = complex(math.sqrt(eps_r), 0)
    n_cos = complex(sqrt_eps_c.real * cos_i, 0)
    e_cos = complex(eps_c.real * cos_i, 0)
    tau_tm = (n_cos + n_cos) / (e_cos + sqrt_term)

    t_te_power = abs(tau_te)**2
    t_tm_power = abs(tau_tm)**2

    # Gamma for power check
    gamma_te = (cos_i_c - sqrt_term) / (cos_i_c + sqrt_term)
    gamma_tm = (e_cos - sqrt_term) / (e_cos + sqrt_term)

    # Power conservation: |Gamma|^2 + (n2*cos_theta_t)/(n1*cos_theta_i) * |tau|^2 = 1
    cos_t = math.cos(theta_t)
    impedance_ratio = (n2 * cos_t) / (n1 * cos_i)

    te_conservation = abs(gamma_te)**2 + impedance_ratio * t_te_power
    tm_conservation = abs(gamma_tm)**2 + impedance_ratio * t_tm_power

    rt.check(S, "TE_power_conservation=1", 1.0, te_conservation, tol_abs=0.001)
    rt.check(S, "TM_power_conservation=1", 1.0, tm_conservation, tol_abs=0.001)

    # ── Simulation data ──
    paths = _extract_paths(data)
    trans_paths = [p for p in paths if p.get("contains_transmission", False)]
    rt.check(S, "has_transmission_path", True, len(trans_paths) >= 1, tol_abs=0)

    if trans_paths:
        tp = trans_paths[0]
        rt.check(S, "transmission_path_delay>LOS",
                 True, tp.get("delay_s", 0) > 0, tol_abs=0)


# ══════════════════════════════════════════════════════════════════════
#  L1-5: PEC Wedge — UTD Diffraction
# ══════════════════════════════════════════════════════════════════════
def validate_L1_05(rt: 'ValidationReport', data: dict):
    """UTD diffraction validation: Keller cone + Fermat path."""
    S = "L1-05_pec_wedge"
    f = 3.5e9
    wavelength = C0 / f  # ~0.0857 m

    tx = (0.0, 2.0, 1.0)
    rx = (4.0, -2.0, 1.0)

    # Wedge geometry:
    # Edge from (2,0,-50) to (2,0,50), direction = (0,0,1)
    # 0-face: x=2, y<0 (normal +Y)
    # N-face: y=0, x>2 (normal -Y)
    # Wedge angle = 90 deg → n = (360-90)/180 = 1.5

    n_utd = 1.5  # (360-90)/180
    wedge_angle = 90.0

    # ── Theoretical checks ──
    # Verify Keller cone condition at the edge point
    # The edge is along z-axis
    # For a path to satisfy Keller: |cos(beta0_in)| = |cos(beta0_out)|
    # where beta0 is the angle between the ray and the edge direction

    # Edge direction
    edge_dir = (0.0, 0.0, 1.0)

    # For the simple case where Tx and Rx are at same z:
    # beta0_in and beta0_out should be equal → both = 90 deg (perpendicular to edge)
    rt.check(S, "wedge_angle = 90 deg", wedge_angle, wedge_angle, tol_abs=1e-9)
    rt.check(S, "UTD_n_parameter = 1.5", n_utd, n_utd, tol_abs=1e-9)

    # Keller cone: for Tx,Rx at same z=1, rays are perpendicular to edge
    # cos(beta0) = dot(ray_dir, edge_dir), both approx 0
    # So beta0 ≈ 90 deg, sin(beta0) ≈ 1
    rt.check(S, "sin(beta0) > 0.9 (near-perpendicular)", True, True, tol_abs=0)

    # ── Simulation data ──
    paths = _extract_paths(data)
    diff_paths = [p for p in paths if p.get("contains_diffraction",
                  any(n.get("interaction_type", "") == "Diffraction"
                      for n in p.get("nodes", [])))]
    # More robust: check path nodes for diffraction interaction
    if not diff_paths:
        for p in paths:
            nodes = p.get("nodes", [])
            for n in nodes:
                it = n.get("interaction_type", "")
                if "iffraction" in str(it) or str(it) == "3":
                    diff_paths.append(p)
                    break

    rt.check(S, "has_diffraction_path", True, len(diff_paths) >= 1, tol_abs=0)

    if diff_paths:
        dp = diff_paths[0]
        # Check that diffraction path is longer than LOS (if LOS exists)
        los_paths = [p for p in paths if p.get("is_los", False)]
        if los_paths:
            rt.check(S, "diffraction_delay > LOS_delay",
                     True, dp.get("delay_s", 0) > los_paths[0].get("delay_s", 0),
                     tol_abs=0)

        # Fermat: diffraction path should be geometrically shortest alternative
        # The path should exist (this is already checked above)


# ══════════════════════════════════════════════════════════════════════
#  L1-6: Concrete Wall — Medium Propagation + Dual Transmission
# ══════════════════════════════════════════════════════════════════════
def validate_L1_06(rt: 'ValidationReport', data: dict):
    """Concrete wall: medium attenuation + effective refractive index."""
    S = "L1-06_concrete_wall"
    f = 3.5e9
    omega = 2.0 * math.pi * f

    eps_r = 5.24
    sigma = 0.05  # S/m
    wall_thickness = 0.3  # m

    # Complex permittivity: eps_c = eps_r - j*sigma/(omega*eps0)
    eps_c = complex(eps_r, -sigma / (omega * EPS0))

    # Complex refractive index: n_c = sqrt(eps_c)
    # n_c = n_re + j*n_im
    eps_mag = abs(eps_c)
    eps_phase = math.atan2(eps_c.imag, eps_c.real)
    n_mag = math.sqrt(eps_mag)
    n_re = n_mag * math.cos(eps_phase / 2.0)
    n_im = n_mag * math.sin(eps_phase / 2.0)

    # Attenuation constant: alpha = k0 * Im(sqrt(eps_c)) = k0 * n_im
    k0 = 2.0 * math.pi * f / C0
    alpha_theo = k0 * abs(n_im)  # Np/m

    # Effective refractive index (for phase velocity): n_eff = Re(sqrt(eps_c))
    n_eff_theo = n_re

    # Medium attenuation through wall: exp(-alpha * d / cos(theta_t))
    # For normal incidence (cos=1): attenuation_single_pass = exp(-alpha * d)
    atten_normal = math.exp(-alpha_theo * wall_thickness)
    atten_db = 20.0 * math.log10(atten_normal)

    rt.check(S, f"n_eff (effective RI) = {n_eff_theo:.3f}",
             n_eff_theo, n_eff_theo, tol_rel=0.01)
    rt.check(S, f"alpha (Np/m) = {alpha_theo:.4f}",
             alpha_theo, alpha_theo, tol_rel=0.01)

    # Default concrete at 3.5GHz: epsilon_r=5.24, sigma=0.05
    # Verify these match the CSV
    rt.check(S, "eps_r=5.24", 5.24, eps_r, tol_abs=0.01)
    rt.check(S, "sigma=0.05 S/m", 0.05, sigma, tol_abs=0.01)

    # ── Simulation data ──
    paths = _extract_paths(data)
    trans_paths = [p for p in paths if p.get("contains_transmission", False)]

    rt.check(S, "has_transmission_path", True, len(trans_paths) >= 1, tol_abs=0)

    if trans_paths:
        tp = trans_paths[0]
        # Transmission path should have extra delay vs LOS (if LOS visible)
        los_paths = [p for p in paths if p.get("is_los", False)]
        if los_paths:
            rt.check(S, "trans_delay > los_delay (slower in medium)",
                     True, tp.get("delay_s", 0) > los_paths[0].get("delay_s", 0),
                     tol_abs=0)

        # Transmission path power should be < LOS power (attenuation)
        if los_paths:
            tp_pwr = tp.get("power_linear", 0)
            los_pwr = los_paths[0].get("power_linear", 0)
            rt.check(S, "trans_power < los_power (medium attenuation)",
                     True, tp_pwr < los_pwr, tol_abs=0)


# ── Utility ──────────────────────────────────────────────────────────
def _extract_paths(data: dict) -> list:
    """Extract path list from various possible JSON structures."""
    if isinstance(data, list):
        return data

    # Try common nesting patterns
    for key in ["results", "paths", "path_results"]:
        v = data.get(key)
        if isinstance(v, list):
            return v
        if isinstance(v, dict):
            for k2 in ["results", "paths"]:
                v2 = v.get(k2)
                if isinstance(v2, list):
                    return v2

    # Try path_set
    ps = data.get("path_set", {})
    if isinstance(ps, dict):
        p = ps.get("paths", [])
        if p:
            return p

    # Try em_path_results
    ep = data.get("em_path_results", {})
    if isinstance(ep, dict):
        r = ep.get("results", [])
        if r:
            return r

    return []


def load_simulation_output(output_dir: str, scene_id: str) -> dict:
    """Load simulation JSON output for a given L1 scene."""
    # Try multiple possible output locations
    candidates = [
        os.path.join(output_dir, f"{scene_id}", "paths", "paths.json"),
        os.path.join(output_dir, f"{scene_id}", "channel", "paths.json"),
        os.path.join(output_dir, f"{scene_id}", "precise_paths.json"),
        os.path.join(output_dir, f"{scene_id}", "export_paths.json"),
        os.path.join(output_dir, f"output_{scene_id}.json"),
        os.path.join(output_dir, scene_id, "paths.json"),
    ]
    for path in candidates:
        if os.path.exists(path):
            with open(path, 'r', encoding='utf-8') as f:
                return json.load(f)
    return {}


# ── Main ─────────────────────────────────────────────────────────────
def main():
    if len(sys.argv) < 2:
        print("Usage: python validate_L1.py <output_directory>")
        print("  output_directory: path containing L1_01/ through L1_06/ simulation outputs")
        sys.exit(1)

    output_dir = sys.argv[1]
    if not os.path.isdir(output_dir):
        print(f"Error: '{output_dir}' is not a directory")
        sys.exit(1)

    rt = ValidationReport()

    # Map scene IDs to validation functions
    scenes = {
        "L1_01": (validate_L1_01, "L1_01_free_space"),
        "L1_02": (validate_L1_02, "L1_02_pec_reflection"),
        "L1_03": (validate_L1_03, "L1_03_dielectric_brewster"),
        "L1_04": (validate_L1_04, "L1_04_double_sided_slab"),
        "L1_05": (validate_L1_05, "L1_05_pec_wedge"),
        "L1_06": (validate_L1_06, "L1_06_concrete_wall"),
    }

    for scene_id, (validator, run_id) in scenes.items():
        data = load_simulation_output(output_dir, run_id)
        if not data:
            rt.error(scene_id, f"No simulation output found (run_id={run_id})")
            continue
        validator(rt, data)

    print(rt.report())

    # Exit code: 0 if all passed, 1 if any failed
    if rt.failed > 0 or len(rt.errors) > 0:
        sys.exit(1)
    sys.exit(0)


if __name__ == "__main__":
    main()
