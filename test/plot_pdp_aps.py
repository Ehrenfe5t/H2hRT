#!/usr/bin/env python3
"""RT PDP/APS 图表生成 — 论文级图表输出。
用法: python test/plot_pdp_aps.py [--paths PATH] [--output-dir DIR]
输出: PDP.png (时延-功率), APS.png (角度-功率), path_stats.csv
"""

import sys, json, math
from pathlib import Path
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_PATHS = ROOT / "output" / "a1_real_chain" / "paths" / "precise_paths.json"
OUT_DIR = ROOT / "output" / "plots"


def plot_pdp(paths: list, out: Path):
    """Power-Delay Profile."""
    delays_ns = [p.get("delay_s", 0) * 1e9 for p in paths if p.get("power_linear", 0) > 1e-15]
    powers_dBm = [10 * math.log10(p.get("power_linear", 1e-15) * 1000) for p in paths if p.get("power_linear", 0) > 1e-15]
    if not delays_ns: return
    fig, ax = plt.subplots(figsize=(10, 4))
    ax.stem(delays_ns, powers_dBm, linefmt="grey", markerfmt="o", basefmt=" ")
    ax.set_xlabel("Delay (ns)"); ax.set_ylabel("Power (dBm)")
    ax.set_title("Power Delay Profile"); ax.grid(True, alpha=0.3)
    fig.tight_layout(); fig.savefig(out, dpi=150); plt.close(fig)


def plot_aps(paths: list, out: Path):
    """Angular Power Spectrum — using polarization direction as angle proxy."""
    angles_deg = []
    powers_lin = []
    for p in paths:
        power = p.get("power_linear", 0)
        if power < 1e-15: continue
        nodes = p.get("geometry_nodes", [])
        for n in nodes:
            if n.get("interaction_type") in (1, 2): continue
            dir_vec = np.array([n.get("x", 0), n.get("y", 0), n.get("z", 0)])
            ang = math.degrees(math.atan2(dir_vec[1], dir_vec[0]))
            if ang < 0: ang += 360
            angles_deg.append(ang); powers_lin.append(power)
    if not angles_deg: return
    fig, ax = plt.subplots(figsize=(8, 6), subplot_kw={"projection": "polar"})
    ax.scatter(np.radians(angles_deg), powers_lin, s=20, alpha=0.6)
    ax.set_title("Angular Power Spectrum", va="bottom")
    fig.tight_layout(); fig.savefig(out, dpi=150); plt.close(fig)


def main():
    import argparse
    ap = argparse.ArgumentParser()
    ap.add_argument("--paths", default=str(DEFAULT_PATHS))
    ap.add_argument("--output-dir", default=str(OUT_DIR))
    args = ap.parse_args()

    pth = Path(args.paths)
    if not pth.exists():
        print(f"Paths file not found: {pth}"); return 1

    data = json.loads(pth.read_text(encoding="utf-8"))
    paths = data.get("paths", [])
    print(f"Loading {len(paths)} paths from {pth}")

    od = Path(args.output_dir); od.mkdir(parents=True, exist_ok=True)

    plot_pdp(paths, od / "PDP.png")
    print(f"PDP -> {od / 'PDP.png'}")

    plot_aps(paths, od / "APS.png")
    print(f"APS -> {od / 'APS.png'}")

    # CSV stats
    csv_path = od / "path_stats.csv"
    with csv_path.open("w") as f:
        f.write("path_id,delay_ns,phase_rad,power_dBm,free_space_loss_db,is_los\n")
        for p in paths:
            f.write(f"{p.get('path_id',-1)},{p.get('delay_s',0)*1e9:.4f},{p.get('phase_rad',0):.4f},{10*math.log10(p.get('power_linear',1e-15)*1000):.2f},{p.get('free_space_loss_db',0):.2f},{p.get('is_los',False)}\n")
    print(f"CSV -> {csv_path}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
