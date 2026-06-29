"""Generate portable, controlled V11 experiment configurations."""

from __future__ import annotations

import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
BASE = Path(__file__).resolve().parent
FREQUENCY_HZ = 3.0e9

SCENES = {
    "412": {
        "obj_file": "../../../Scene&Material/412-6k.obj",
        "material_binding_file": "../../../Scene&Material/material_map-412.json",
        "material_database_file": "../../../Scene&Material/ItuMaterial.csv",
    },
    "meeting": {
        "obj_file": "../../../Scene&Material/meeting.obj",
        "material_binding_file": "../../../Scene&Material/material_map-meeting.json",
        "material_database_file": "../../../Scene&Material/ItuMaterial.csv",
    },
}

POSITIONS = {
    "412": ([2.38, 1.5, -9.61], [1.58, 1.5, -8.81]),
    "meeting": ([10.0, 1.5, -10.0], [7.0, 1.5, -12.0]),
}

MODELS = {
    "ideal": ("", "", "analytic isotropic reference"),
    "dipole": ("../../antennas/dipole_gain.csv", "../../antennas/dipole_jones.csv", "analytic parametric model"),
    "patch": ("../../antennas/patch_gain.csv", "../../antennas/patch_jones.csv", "synthetic parametric model"),
    "horn": ("../../antennas/horn_gain.csv", "../../antennas/horn_jones.csv", "synthetic parametric model"),
}


def unit(v: list[float]) -> list[float]:
    n = math.sqrt(sum(x * x for x in v))
    return [x / n for x in v]


def aligned_pose(scene: str) -> tuple[list[float], list[float], list[float]]:
    tx, rx = POSITIONS[scene]
    tx_forward = unit([rx[i] - tx[i] for i in range(3)])
    rx_forward = [-x for x in tx_forward]
    return tx_forward, rx_forward, [0.0, 1.0, 0.0]


def rotate_y(v: list[float], degrees: float) -> list[float]:
    a = math.radians(degrees)
    c, s = math.cos(a), math.sin(a)
    return [c * v[0] + s * v[2], v[1], -s * v[0] + c * v[2]]


def antenna(model: str, scene: str, tx_jones: bool = False, rx_jones: bool = False,
            rx_yaw_deg: float = 0.0) -> dict:
    gain, jones, source = MODELS[model]
    tx, rx = POSITIONS[scene]
    tx_forward, rx_forward, up = aligned_pose(scene)
    rx_forward = rotate_y(rx_forward, rx_yaw_deg)
    tx_up, rx_up = up, up
    orientation = "forward is boresight; up is Ludwig-3 co-pol reference"
    if model == "dipole":
        tx_up, rx_up = tx_forward, rx_forward
        tx_forward = [0.0, 1.0, 0.0]
        rx_forward = [0.0, 1.0, 0.0]
        orientation = "forward is the vertical dipole axis (pattern theta=0); up lies in the link plane"
    return {
        "schema_version": "v11.5",
        "coordinate_system": f"right-handed Y-up world; {orientation}",
        "model_id": model,
        "model_source": source,
        "tx_list": [{
            "id": "Tx01", "position_m": tx, "power_dBm": 0.0,
            "pattern_file": gain, "polarization_file": jones if tx_jones else "",
            "forward": tx_forward, "up": tx_up,
        }],
        "rx_list": [{
            "id": "Rx01", "position_m": rx,
            "pattern_file": gain, "polarization_file": jones if rx_jones else "",
            "forward": rx_forward, "up": rx_up,
        }],
    }


def config(run_id: str, antenna_name: str, group: str, scene: str = "412",
           ray_count: int = 200_000, radius: float = 0.06,
           dynamic_radius: bool = True, max_diffraction: int = 1) -> dict:
    return {
        "schema_version": "v11.5",
        "run_id": run_id,
        "experiment_group": group,
        "antenna_file": antenna_name,
        "scene": SCENES[scene],
        "simulation": {"enable_p2p": True, "enable_coverage": False},
        "p2p": {
            "tx_ids": ["Tx01"], "rx_ids": ["Rx01"],
            "ray_count": ray_count, "max_depth": 8,
            "max_reflection": 3, "max_transmission": 2,
            "max_diffraction": max_diffraction,
            "power_threshold_dB": -180.0,
            "rx_sphere_radius_m": radius,
            "enable_dynamic_rx_radius": dynamic_radius,
            "ray_tube_radius_scale": 0.5,
            "ray_tube_min_radius_m": radius if dynamic_radius else 0.0,
            "ray_tube_max_radius_m": 0.30 if dynamic_radius else 0.0,
            "enable_path_dedup": True,
            "enable_path_similarity_pruning": False,
            "enable_path_residual_filter": False,
        },
        "em": {
            "frequency_list_hz": [FREQUENCY_HZ],
            "aps_theta_bins": 36, "aps_phi_bins": 72, "compute_meg": True,
        },
    }


def write_json(path: Path, data: dict) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(data, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")


def add_case(manifest: list[dict], group: str, case_id: str, folder: str,
             antenna_data: dict, config_data: dict, purpose: str) -> None:
    directory = BASE / folder
    antenna_name = f"antenna_{case_id}.json"
    config_name = f"config_{case_id}.json"
    config_data["antenna_file"] = antenna_name
    write_json(directory / antenna_name, antenna_data)
    write_json(directory / config_name, config_data)
    manifest.append({
        "group": group, "case_id": case_id, "run_id": config_data["run_id"],
        "config": f"configs/v11_experiments/{folder}/{config_name}",
        "purpose": purpose,
    })


def main() -> None:
    manifest: list[dict] = []

    for model in ("ideal", "dipole", "patch", "horn"):
        case = model
        add_case(manifest, "E1", case, "e1_antenna_model",
                 antenna(model, "412"),
                 config(f"v11_e1_{case}_412", "", "E1"),
                 "Isolate gain-pattern effects with fixed co-polarization.")

    for yaw in (0, 30, 60, 90, 120, 150, 180):
        case = f"yaw{yaw:03d}"
        add_case(manifest, "E2", case, "e2_pose_yaw",
                 antenna("patch", "412", True, True, yaw),
                 config(f"v11_e2_{case}_412", "", "E2"),
                 "Rotate Rx around world +Y while all other variables remain fixed.")

    for tx_jones, rx_jones, case in (
        (False, False, "fixed_fixed"), (True, False, "jones_fixed"),
        (False, True, "fixed_jones"), (True, True, "jones_jones"),
    ):
        add_case(manifest, "E3", case, "e3_polarization",
                 antenna("patch", "412", tx_jones, rx_jones),
                 config(f"v11_e3_{case}_412", "", "E3"),
                 "Pair paths by signature and isolate Tx/Rx Jones polarization effects.")

    for rays, case in ((50_000, "050k"), (100_000, "100k"),
                       (200_000, "200k"), (500_000, "500k")):
        add_case(manifest, "C1", case, "c1_ray_convergence",
                 antenna("ideal", "412"),
                 config(f"v11_c1_{case}_412", "", "C1", ray_count=rays),
                 "Test convergence of weighted channel metrics against the 500k reference.")

    for enabled, case in ((False, "off"), (True, "on")):
        add_case(manifest, "C2", case, "c2_diffraction",
                 antenna("ideal", "412"),
                 config(f"v11_c2_diff_{case}_412", "", "C2", max_diffraction=int(enabled)),
                 "Compare the current analytical diffraction branch off versus on.")

    for radius, case in ((0.03, "r003"), (0.06, "r006"), (0.10, "r010"), (0.20, "r020")):
        add_case(manifest, "C3", case, "c3_rx_radius",
                 antenna("ideal", "412"),
                 config(f"v11_c3_{case}_412", "", "C3", radius=radius, dynamic_radius=False),
                 "Fixed reception-sphere sensitivity with dynamic radius disabled.")

    for model in ("ideal", "patch"):
        case = f"meeting_{model}"
        add_case(manifest, "X1", case, "x1_cross_scene",
                 antenna(model, "meeting", model == "patch", model == "patch"),
                 config(f"v11_x1_{case}", "", "X1", scene="meeting"),
                 "Cross-scene check of the principal ideal-versus-patch trend.")

    write_json(BASE / "experiment_manifest.json", {
        "schema_version": "v11.5",
        "primary_scene": "412-6k",
        "frequency_hz": FREQUENCY_HZ,
        "case_count": len(manifest),
        "cases": manifest,
    })
    print(f"Generated {len(manifest)} experiment cases under {BASE}")


if __name__ == "__main__":
    main()
