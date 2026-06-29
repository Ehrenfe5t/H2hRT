"""Generate traceable analytic antenna benchmark patterns for V11 experiments."""

from __future__ import annotations

import csv
import math
from pathlib import Path


OUT = Path(__file__).resolve().parent
THETA = list(range(0, 181, 2))
PHI = list(range(0, 361, 5))


def db(value: float) -> float:
    return 10.0 * math.log10(max(value, 1e-12))


def directional_gain(theta_deg: float, peak_dbi: float, exponent: float, floor_dbi: float) -> float:
    theta = math.radians(theta_deg)
    main = 10.0 ** (peak_dbi / 10.0) * max(math.cos(theta), 0.0) ** exponent
    return db(max(main, 10.0 ** (floor_dbi / 10.0)))


def dipole_gain(theta_deg: float) -> float:
    theta = math.radians(theta_deg)
    s = math.sin(theta)
    if abs(s) < 1e-12:
        return -40.0
    field = math.cos(0.5 * math.pi * math.cos(theta)) / s
    return db(1.640922376984585 * field * field)


def write_gain(name: str, evaluator) -> None:
    with (OUT / f"{name}_gain.csv").open("w", newline="", encoding="utf-8") as f:
        writer = csv.writer(f, lineterminator="\n")
        writer.writerow(["theta_deg", "phi_deg", "gain_dBi"])
        for theta in THETA:
            for phi in PHI:
                writer.writerow([theta, phi, f"{evaluator(theta):.6f}"])


def jones_components(model: str, theta_deg: float, phi_deg: float) -> tuple[float, float, float, float]:
    theta = math.radians(theta_deg)
    phi = math.radians(phi_deg)
    if model == "dipole":
        cross_fraction = 0.0
    elif model == "patch":
        cross_fraction = 0.001 + 0.08 * math.sin(theta) ** 2 * (0.25 + 0.75 * math.sin(2.0 * phi) ** 2)
    elif model == "horn":
        cross_fraction = 0.0003 + 0.025 * math.sin(theta) ** 2 * (0.2 + 0.8 * math.sin(2.0 * phi) ** 2)
    else:
        raise ValueError(model)
    cross_fraction = min(max(cross_fraction, 0.0), 0.25)
    co = math.sqrt(1.0 - cross_fraction)
    cross = math.sqrt(cross_fraction)
    phase = 0.5 * math.pi * math.sin(phi) * math.sin(theta)
    return co, 0.0, cross * math.cos(phase), cross * math.sin(phase)


def write_jones(name: str) -> None:
    with (OUT / f"{name}_jones.csv").open("w", newline="", encoding="utf-8") as f:
        writer = csv.writer(f, lineterminator="\n")
        writer.writerow(["theta_deg", "phi_deg", "Etheta_real", "Etheta_imag", "Ephi_real", "Ephi_imag"])
        for theta in THETA:
            for phi in PHI:
                values = jones_components(name, theta, phi)
                writer.writerow([theta, phi, *(f"{value:.8f}" for value in values)])


def spherical_average(evaluator) -> float:
    dtheta = math.radians(THETA[1] - THETA[0])
    dphi = math.radians(PHI[1] - PHI[0])
    integral = 0.0
    for theta in THETA:
        for phi in PHI[:-1]:
            gain = 10.0 ** (evaluator(theta) / 10.0)
            integral += gain * math.sin(math.radians(theta)) * dtheta * dphi
    return integral / (4.0 * math.pi)


def half_power_beamwidth(evaluator) -> float:
    peak = evaluator(0.0)
    inside = [theta for theta in THETA if theta <= 90 and evaluator(theta) >= peak - 3.0]
    return 2.0 * max(inside)


def main() -> None:
    models = {
        "dipole": dipole_gain,
        "patch": lambda theta: directional_gain(theta, 6.0, 2.75, -30.0),
        "horn": lambda theta: directional_gain(theta, 15.0, 22.0, -50.0),
    }
    for name, evaluator in models.items():
        write_gain(name, evaluator)
        write_jones(name)
        average = spherical_average(evaluator)
        if average > 1.0001:
            raise RuntimeError(f"{name}: non-passive spherical average gain {average}")
        if name != "dipole":
            print(f"{name}: peak={evaluator(0):.2f} dBi, HPBW={half_power_beamwidth(evaluator):.1f} deg, efficiency={average:.3f}")
        else:
            print(f"{name}: peak={max(evaluator(t) for t in THETA):.2f} dBi, efficiency={average:.3f}")


if __name__ == "__main__":
    main()

