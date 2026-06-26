#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Generate/check the frozen V10 chapter-3 experiment matrix.

The script is intentionally lightweight.  It does not run RT by default; it
validates source assets and can emit per-case JSON configs plus a manifest that
ties every planned figure/table to its exact configuration.
"""

from __future__ import annotations

import argparse
import copy
import json
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Dict, Iterable, List


ROOT = Path(__file__).resolve().parents[1]
FINE_TEMPLATE = ROOT / "configs" / "app" / "v10_finechannel_template.json"
COVERAGE_TEMPLATE = ROOT / "configs" / "app" / "v10_coverage_template.json"
ANT_DIR = ROOT / "configs" / "antennas"
DEFAULT_OUT = ROOT / "output" / "ch3_frozen"


@dataclass(frozen=True)
class Case:
    experiment_id: str
    case_id: str
    template: str
    purpose: str
    expected_outputs: List[str]
    mutations: Dict[str, Any] = field(default_factory=dict)


ANTENNA_FILES = {
    "dipole": ("dipole_gain.csv", "dipole_jones.csv"),
    "patch": ("patch_gain.csv", "patch_jones.csv"),
    "horn": ("horn_gain.csv", "horn_jones.csv"),
}


def nested_set(root: Dict[str, Any], dotted_key: str, value: Any) -> None:
    cur = root
    parts = dotted_key.split(".")
    for part in parts[:-1]:
        cur = cur.setdefault(part, {})
    cur[parts[-1]] = value


def load_json(path: Path) -> Dict[str, Any]:
    with path.open("r", encoding="utf-8") as f:
        return json.load(f)


def apply_common_identity(cfg: Dict[str, Any], case: Case) -> Dict[str, Any]:
    out = copy.deepcopy(cfg)
    run_id = f"v10_{case.experiment_id}_{case.case_id}"
    nested_set(out, "app_runtime.run_id", run_id)
    nested_set(out, "experiment.experiment_tag", f"v10_{case.experiment_id}")
    nested_set(out, "experiment.dataset_tag", case.case_id)
    for key, value in case.mutations.items():
        nested_set(out, key, value)
    return out


def antenna_mutations(name: str) -> Dict[str, Any]:
    if name == "ideal":
        return {
            "antenna.source_type": "Ideal",
            "antenna.pattern_file": "",
            "antenna.polarization_file": "",
        }
    gain, jones = ANTENNA_FILES[name]
    return {
        "antenna.source_type": name,
        "antenna.pattern_file": f"configs/antennas/{gain}",
        "antenna.polarization_file": f"configs/antennas/{jones}",
    }


def pose_mutations(yaw_deg: int) -> Dict[str, Any]:
    # Use precomputed values to avoid an extra numpy dependency in this utility.
    values = {
        0: (1.0, 0.0),
        30: (0.8660254038, -0.5),
        60: (0.5, -0.8660254038),
        90: (0.0, -1.0),
        120: (-0.5, -0.8660254038),
        150: (-0.8660254038, -0.5),
        180: (-1.0, 0.0),
    }
    fx, fz = values[yaw_deg]
    return {
        "antenna.forward_x": fx,
        "antenna.forward_y": 0.0,
        "antenna.forward_z": fz,
        "antenna.up_x": 0.0,
        "antenna.up_y": 0.0,
        "antenna.up_z": 1.0,
    }


def build_cases() -> List[Case]:
    cases: List[Case] = []

    for ant in ["ideal", "dipole", "patch", "horn"]:
        cases.append(Case(
            "E1_antenna_model",
            ant,
            "fine",
            "Antenna gain/Jones model comparison for APS/XPR/PDP/CIR.",
            ["paths", "cir", "pdp", "aps", "xpr_summary"],
            antenna_mutations(ant),
        ))

    for yaw in [0, 30, 60, 90, 120, 150, 180]:
        mut = antenna_mutations("patch")
        mut.update(pose_mutations(yaw))
        cases.append(Case(
            "E2_pose_yaw",
            f"yaw_{yaw:03d}",
            "fine",
            "Antenna attitude sensitivity sweep.",
            ["paths", "aps", "xpr_summary", "received_power"],
            mut,
        ))

    for rays in [50000, 100000, 200000, 500000]:
        cases.append(Case(
            "C1_ray_count_convergence",
            f"rays_{rays}",
            "fine",
            "FineChannel ray-count convergence and path-count stability.",
            ["paths", "diagnostics", "convergence_table"],
            {"sbr.ray_count": rays},
        ))

    for n in [2, 4, 8, 16]:
        cases.append(Case(
            "C2_diffraction_sampling",
            f"keller_{n}",
            "fine",
            "Keller cone sampling sensitivity for wedge-coupled diffraction.",
            ["paths", "diagnostics", "diffraction_residuals"],
            {"sbr.diffraction_rays_per_event": n},
        ))

    for radius in [0.03, 0.06, 0.10, 0.20]:
        cases.append(Case(
            "C3_rx_radius_sensitivity",
            f"rx_radius_{radius:.2f}".replace(".", "p"),
            "fine",
            "Rx sphere/tube sensitivity for missed-path risk analysis.",
            ["paths", "diagnostics", "hit_rate"],
            {"sbr.rx_sphere_radius_m": radius},
        ))

    for rays in [200000, 500000, 1000000]:
        cases.append(Case(
            "E3_coverage_heatmap",
            f"coverage_rays_{rays}",
            "coverage",
            "Large-scale coverage heatmap with simplified Rx model.",
            ["coverage_grid", "diagnostics"],
            {"sbr.ray_count": rays},
        ))

    return cases


def required_assets(cases: Iterable[Case]) -> List[Path]:
    paths = [FINE_TEMPLATE, COVERAGE_TEMPLATE]
    paths.extend([
        ROOT / "demo" / "412" / "412-6k.obj",
        ROOT / "configs" / "scenes" / "scene_material_map-412.json",
        ROOT / "demo" / "ItuMaterial.csv",
    ])
    for case in cases:
        for key, value in case.mutations.items():
            if key.endswith("_file") and value:
                paths.append(ROOT / value)
    return paths


def emit_configs(cases: Iterable[Case], out_root: Path) -> List[Dict[str, Any]]:
    fine = load_json(FINE_TEMPLATE)
    coverage = load_json(COVERAGE_TEMPLATE)
    templates = {"fine": fine, "coverage": coverage}
    manifest: List[Dict[str, Any]] = []

    cfg_root = out_root / "configs"
    cfg_root.mkdir(parents=True, exist_ok=True)
    for case in cases:
        cfg = apply_common_identity(templates[case.template], case)
        case_dir = cfg_root / case.experiment_id
        case_dir.mkdir(parents=True, exist_ok=True)
        cfg_path = case_dir / f"{case.case_id}.json"
        cfg_path.write_text(json.dumps(cfg, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
        manifest.append({
            "experiment_id": case.experiment_id,
            "case_id": case.case_id,
            "template": case.template,
            "config_path": str(cfg_path.relative_to(ROOT).as_posix()),
            "run_id": cfg["app_runtime"]["run_id"],
            "purpose": case.purpose,
            "expected_outputs": case.expected_outputs,
            "mutations": case.mutations,
        })

    manifest_path = out_root / "v10_ch3_manifest.json"
    manifest_path.write_text(json.dumps(manifest, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    return manifest


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--emit-configs", action="store_true", help="write per-case configs and manifest")
    parser.add_argument("--out-root", type=Path, default=DEFAULT_OUT, help="output root for emitted configs")
    args = parser.parse_args()

    cases = build_cases()
    missing = sorted({p for p in required_assets(cases) if not p.exists()})
    if missing:
        print("Missing required assets:")
        for path in missing:
            print(f"  - {path.relative_to(ROOT)}")
        return 2

    print(f"V10 chapter-3 matrix: {len(cases)} cases")
    for exp in sorted({c.experiment_id for c in cases}):
        print(f"  {exp}: {sum(1 for c in cases if c.experiment_id == exp)}")

    if args.emit_configs:
        manifest = emit_configs(cases, args.out_root)
        rel = (args.out_root / "v10_ch3_manifest.json").relative_to(ROOT)
        print(f"Emitted {len(manifest)} configs")
        print(f"Manifest: {rel.as_posix()}")
    else:
        print("Check-only mode: all referenced assets exist.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
