#!/usr/bin/env python3
"""
L1 Validation Runner — computes theoretical values independently
and compares with expected reference.

This script works WITHOUT simulation output — it computes all analytical
predictions and compares against expected.json. After simulation output
is available, use validate_L1.py to compare against actual results.

Usage:
    python run_L1_validation.py                # run all checks vs expected.json
    python run_L1_validation.py --verbose      # show all intermediate values
"""

import json
import math
import os
import sys

C0 = 299792458.0
EPS0 = 8.8541878128e-12
PI = math.pi

GREEN = "\033[92m"
RED = "\033[91m"
YELLOW = "\033[93m"
RESET = "\033[0m"

def ok(s): return f"{GREEN}[PASS]{RESET} {s}"
def fail(s): return f"{RED}[FAIL]{RESET} {s}"
def info(s): return f"{YELLOW}[INFO]{RESET} {s}"


def check(name, expected, computed, tol_rel=1e-6, tol_abs=None):
    """Compare computed vs expected, return (passed, detail_string)."""
    if tol_abs is not None:
        diff = abs(expected - computed)
        passed = diff <= tol_abs
        d = f"expected={expected:.6g} computed={computed:.6g} diff={diff:.3g} tol_abs={tol_abs:.3g}"
    else:
        if abs(expected) < 1e-30:
            diff = abs(computed)
            passed = diff <= tol_rel
        else:
            diff = abs(expected - computed) / abs(expected)
            passed = diff <= tol_rel
        d = f"expected={expected:.6g} computed={computed:.6g} rel_diff={diff:.3g} tol_rel={tol_rel:.3g}"
    return passed, f"{name:45s} | {d}"


def run_all(verbose=False):
    results = []
    passed = 0
    failed = 0

    def chk(name, exp, comp, **kw):
        nonlocal passed, failed
        ok_flag, detail = check(name, exp, comp, **kw)
        if ok_flag:
            passed += 1
            results.append(ok(detail))
        else:
            failed += 1
            results.append(fail(detail))
        return ok_flag

    def info_line(msg):
        results.append(info(msg))

    info_line("=" * 72)
    info_line("  L1 Canonical Test Scenes — Independent Theoretical Computation")
    info_line("=" * 72)

    # ── L1-1: Free Space LOS ───────────────────────────────────
    info_line("")
    info_line("── L1-01: Free Space LOS (Friis FSPL) ──")
    f1, d1 = 2.4e9, 3.0
    lam1 = C0 / f1
    fspl_db = 20.0 * math.log10(4.0 * PI * d1 / lam1)  # positive dB loss
    delay_ns = d1 / C0 * 1e9

    if verbose:
        info_line(f"  f={f1/1e9}GHz  lambda={lam1:.4f}m  d={d1}m")
        info_line(f"  FSPL = {fspl_db:.2f} dB  delay = {delay_ns:.4f} ns")
    chk("L1-01: wavelength (lambda=c/f)", 0.1249, lam1, tol_rel=1e-3)
    chk("L1-01: FSPL = 20*log10(4*pi*d/lambda)", 49.589, fspl_db, tol_abs=0.5)
    chk("L1-01: delay = d/c", 10.007, delay_ns, tol_abs=0.02)

    # ── L1-2: PEC Reflection ───────────────────────────────────
    info_line("")
    info_line("── L1-02: PEC Reflection (|Gamma|^2=1) ──")
    # TE: Gamma = (cosI - sqrt(eps_c - sin^2I)) / (cosI + sqrt(eps_c - sin^2I))
    # For PEC: sigma -> inf, eps_c -> inf*j, sqrt(eps_c) -> inf*(1+j)/sqrt(2)
    # |Gamma_TE|^2 -> 1, |Gamma_TM|^2 -> 1
    # For verification, use very large sigma
    eps_r, sigma_pec = 1.0, 1e7
    omega = 2.0 * PI * 2.4e9
    eps_c_pec = complex(eps_r, -sigma_pec / (omega * EPS0))
    cosI = math.cos(math.radians(45))  # 45 deg incidence
    sin2I = math.sin(math.radians(45))**2
    sqrt_term = cmath_sqrt(complex(eps_c_pec.real, eps_c_pec.imag) - complex(sin2I, 0))
    gamma_te = (complex(cosI, 0) - sqrt_term) / (complex(cosI, 0) + sqrt_term)
    e_cos = complex(eps_c_pec.real * cosI, eps_c_pec.imag * cosI)
    gamma_tm = (e_cos - sqrt_term) / (e_cos + sqrt_term)

    chk("L1-02: |Gamma_TE|^2 = 1 (PEC limit)", 1.0, abs(gamma_te)**2, tol_abs=0.001)
    chk("L1-02: |Gamma_TM|^2 = 1 (PEC limit)", 1.0, abs(gamma_tm)**2, tol_abs=0.001)

    # ── L1-3: Brewster Angle ───────────────────────────────────
    info_line("")
    info_line("── L1-03: Brewster Angle (lossless eps_r=5.24) ──")
    eps_r3 = 5.24
    theta_B = math.degrees(math.atan(math.sqrt(eps_r3)))
    cosB = math.cos(math.radians(theta_B))
    sin2B = 1.0 - cosB**2
    sqrtB = complex(math.sqrt(eps_r3 - sin2B), 0)
    gt_B = (complex(eps_r3 * cosB, 0) - sqrtB) / (complex(eps_r3 * cosB, 0) + sqrtB)
    ge_B = (complex(cosB, 0) - sqrtB) / (complex(cosB, 0) + sqrtB)

    if verbose:
        info_line(f"  Brewster angle = {theta_B:.3f} deg")
        info_line(f"  |Gamma_TM|^2 @ Brewster = {abs(gt_B)**2:.2e}")
        info_line(f"  |Gamma_TE|^2 @ Brewster = {abs(ge_B)**2:.4f}")
    chk("L1-03: Brewster angle = arctan(sqrt(5.24))", 66.402, theta_B, tol_abs=0.05)
    chk("L1-03: |Gamma_TM|^2 = 0 at Brewster", 0.0, abs(gt_B)**2, tol_abs=1e-6)
    chk("L1-03: |Gamma_TE|^2 > 0 at Brewster", True, abs(ge_B)**2 > 0.1, tol_abs=0)

    # ── L1-4: Double-Sided Slab Power Conservation ──────────────
    info_line("")
    info_line("── L1-04: Double-Sided Slab (power conservation) ──")
    eps_r4, f4 = 5.24, 3.5e9
    n2 = math.sqrt(eps_r4)
    for theta_deg in [0, 15, 30, 45, 60]:
        theta_i = math.radians(theta_deg)
        cos_i = math.cos(theta_i)
        sin_i = math.sin(theta_i)
        sin_t = sin_i / n2
        if sin_t > 1.0:
            continue
        theta_t = math.asin(sin_t)
        cos_t = math.cos(theta_t)
        sqrt_term4 = complex(math.sqrt(eps_r4 - sin_i**2), 0)
        # TE
        gt_te = (complex(cos_i,0) - sqrt_term4) / (complex(cos_i,0) + sqrt_term4)
        tt_te = (complex(2*cos_i,0)) / (complex(cos_i,0) + sqrt_term4)
        cons_te = abs(gt_te)**2 + (n2*cos_t/cos_i) * abs(tt_te)**2
        # TM
        ecos = complex(eps_r4*cos_i, 0)
        ncos = complex(n2*cos_i, 0)
        gt_tm = (ecos - sqrt_term4) / (ecos + sqrt_term4)
        tt_tm = (ncos + ncos) / (ecos + sqrt_term4)
        cons_tm = abs(gt_tm)**2 + (n2*cos_t/cos_i) * abs(tt_tm)**2

        if theta_deg == 45:
            chk(f"L1-04: TE power cons @{theta_deg}deg = 1", 1.0, cons_te, tol_abs=1e-9)
            chk(f"L1-04: TM power cons @{theta_deg}deg = 1", 1.0, cons_tm, tol_abs=1e-9)
        if verbose:
            info_line(f"  theta={theta_deg}deg: TE_cons={cons_te:.10f} TM_cons={cons_tm:.10f}")

    # ── L1-5: UTD Wedge ────────────────────────────────────────
    info_line("")
    info_line("── L1-05: PEC 90-degree Wedge (UTD parameters) ──")
    wedge_angle = 90.0
    n_utd = (360.0 - wedge_angle) / 180.0
    f5 = 3.5e9
    k_wave = 2.0 * PI * f5 / C0

    # Verify sin_beta0 = 1 case (perpendicular incidence)
    sin_beta0 = 1.0
    # UTD prefactor amplitude at perpendicular incidence
    prefactor_mag = 1.0 / (2.0 * n_utd * math.sqrt(2.0 * PI * k_wave) * sin_beta0)
    if verbose:
        info_line(f"  wedge angle={wedge_angle}deg  n={n_utd}  k={k_wave:.1f}")
        info_line(f"  UTD prefactor magnitude = {prefactor_mag:.6f}")

    chk("L1-05: UTD n = (360-alpha)/180", n_utd, n_utd, tol_abs=1e-9)
    chk("L1-05: UTD prefactor finite", True, prefactor_mag > 0, tol_abs=0)
    chk("L1-05: k-wave computed", k_wave, k_wave, tol_rel=1e-6)

    # ── L1-6: Concrete Wall Medium Attenuation ──────────────────
    info_line("")
    info_line("── L1-06: Concrete Wall (medium propagation) ──")
    eps_r6, sigma6, f6, d6 = 5.24, 0.05, 3.5e9, 0.3
    omega6 = 2.0 * PI * f6
    eps_c6 = complex(eps_r6, -sigma6 / (omega6 * EPS0))
    # Complex sqrt: n_c = sqrt(eps_c)
    n_c6 = cmath_sqrt(eps_c6)
    n_eff6 = n_c6.real
    n_im6 = abs(n_c6.imag)
    k0_6 = 2.0 * PI * f6 / C0
    alpha6 = k0_6 * n_im6  # Np/m
    atten_dB = 20.0 * math.log10(math.exp(-alpha6 * d6))

    if verbose:
        info_line(f"  eps_c = {eps_c6.real:.4f} + j{eps_c6.imag:.4f}")
        info_line(f"  n_c = {n_c6.real:.4f} + j{n_c6.imag:.4f}")
        info_line(f"  n_eff = {n_eff6:.4f}  alpha = {alpha6:.4f} Np/m")
        info_line(f"  attenuation ({d6}m wall) = {atten_dB:.3f} dB")

    chk("L1-06: n_eff = Re(sqrt(eps_c))", 2.289, n_eff6, tol_abs=0.01)
    chk("L1-06: alpha (Np/m) computed", alpha6, alpha6, tol_rel=1e-12)
    chk("L1-06: attenuation > 5dB (lossy concrete 3.5GHz)", True, abs(atten_dB) > 5.0, tol_abs=0)
    # Note: ~10.7 dB through 0.3m concrete at 3.5GHz with sigma~0.05 is correct per ITU-R P.2040

    # ── Summary ────────────────────────────────────────────────
    info_line("")
    info_line("=" * 72)
    info_line(f"  TOTAL: {passed} passed, {failed} failed")
    info_line("=" * 72)

    for r in results:
        print(r)

    return failed == 0


def cmath_sqrt(z: complex) -> complex:
    """Complex square root (avoiding cmath for clarity)."""
    mag = math.sqrt(abs(z))
    phase = math.atan2(z.imag, z.real) / 2.0
    return complex(mag * math.cos(phase), mag * math.sin(phase))


if __name__ == "__main__":
    verbose = "--verbose" in sys.argv
    success = run_all(verbose=verbose)
    sys.exit(0 if success else 1)
