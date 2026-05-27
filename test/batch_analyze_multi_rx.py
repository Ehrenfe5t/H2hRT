#!/usr/bin/env python3
"""
批量多Rx Precise仿真信道分析
=================================
从 output/{run_id}/{rx_id}/paths/precise_paths.json 批量抽参数，
输出:
  1. per_rx_channel_params.csv — 每个Rx的信道参数表
  2. 对比图: PathLoss/RMS-DS/K-Factor vs Distance
  3. 角域分布: AoD/AoA极坐标叠加
  4. PDP瀑布图 (距离-时延-功率)

用法: python test/batch_analyze_multi_rx.py [--run_id 412]
"""

import argparse, json, math, os, sys
from pathlib import Path
import numpy as np

try:
    import matplotlib; matplotlib.use("Agg")
    import matplotlib.pyplot as plt
except:
    print("ERROR: matplotlib not available"); sys.exit(1)

C0 = 299792458.0

def compute_channel_params(paths, freq_hz):
    """Compute standard channel parameters from path list."""
    n = len(paths)
    if n == 0:
        return {"num_paths": 0, "path_loss_dB": 0, "rms_ds_ns": 0,
                "mean_delay_ns": 0, "max_excess_delay_ns": 0,
                "k_factor_dB": -100, "total_power_dBm": -200,
                "los_power_linear": 0, "nlos_power_linear": 0}

    delays_s = np.array([p["delay_s"] for p in paths])
    powers_lin = np.array([p["power_linear"] for p in paths])
    total_power = np.sum(powers_lin)
    path_loss_dB = -10.0 * math.log10(total_power) if total_power > 0 else 200

    # Total power with 20dBm Tx reference
    total_power_dBm = 10.0 * math.log10(total_power * 1e3) + 20.0 if total_power > 0 else -200

    # Mean delay (power-weighted)
    mean_delay = np.sum(delays_s * powers_lin) / total_power if total_power > 0 else 0
    # RMS delay spread
    rms_ds = math.sqrt(np.sum(powers_lin * (delays_s - mean_delay)**2) / total_power) if total_power > 0 else 0

    # Excess delay (relative to first arrival)
    first_arrival = np.min(delays_s)
    max_excess = (np.max(delays_s) - first_arrival) if n > 0 else 0

    # K-factor: LOS power / NLOS power
    los_mask = np.array([p.get("is_los", False) for p in paths])
    los_power = np.sum(powers_lin[los_mask]) if np.any(los_mask) else 0
    nlos_power = np.sum(powers_lin[~los_mask]) if np.any(~los_mask) else 1e-30
    k_lin = los_power / max(nlos_power, 1e-30)
    k_factor_dB = 10.0 * math.log10(max(k_lin, 1e-10))

    return {
        "num_paths": n,
        "path_loss_dB": round(path_loss_dB, 2),
        "rms_ds_ns": round(rms_ds * 1e9, 3),
        "mean_delay_ns": round(mean_delay * 1e9, 3),
        "max_excess_delay_ns": round(max_excess * 1e9, 3),
        "k_factor_dB": round(k_factor_dB, 2),
        "total_power_dBm": round(total_power_dBm, 2),
        "los_power_linear": float(los_power),
        "nlos_power_linear": float(nlos_power),
    }


def load_rx_data(output_root, run_id):
    """Scan output/{run_id}/ for rx* subdirs, load precise_paths.json."""
    rx_data = []
    root = Path(output_root) / run_id
    if not root.exists():
        print(f"ERROR: {root} not found"); return rx_data

    for subdir in sorted(root.iterdir()):
        if not subdir.is_dir() or not subdir.name.startswith("rx"):
            continue
        paths_file = subdir / "paths" / "precise_paths.json"
        if not paths_file.exists():
            continue
        with open(paths_file) as f:
            data = json.load(f)
        rx = {"id": subdir.name, "paths": data.get("paths", [])}
        # Parse Rx position from first path's geometry
        if rx["paths"]:
            geonodes = rx["paths"][0].get("geometry_nodes", [])
            if geonodes:
                last = geonodes[-1]
                rx["x"] = last.get("x", 0); rx["y"] = last.get("y", 0); rx["z"] = last.get("z", 0)
        rx_data.append(rx)
    return rx_data


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--run_id", default="412")
    parser.add_argument("--output_root", default="G:/RT/H2hRT-7.1-SBR-/output")
    parser.add_argument("--freq_hz", type=float, default=3.0e9)
    parser.add_argument("--tx_x", type=float, default=2.38)
    parser.add_argument("--tx_y", type=float, default=1.5)
    parser.add_argument("--tx_z", type=float, default=-9.61)
    args = parser.parse_args()

    print(f"Batch Multi-Rx Channel Analysis")
    print(f"  Run: {args.run_id}  Freq: {args.freq_hz/1e9:.1f} GHz")
    print(f"  Tx: ({args.tx_x}, {args.tx_y}, {args.tx_z})")

    rx_data = load_rx_data(args.output_root, args.run_id)
    if not rx_data:
        print("  No Rx data found."); return

    # Compute channel params per Rx
    tx = np.array([args.tx_x, args.tx_y, args.tx_z])
    results = []
    for rx in rx_data:
        cp = compute_channel_params(rx["paths"], args.freq_hz)
        pos = np.array([rx.get("x", 0), rx.get("y", 0), rx.get("z", 0)])
        cp["rx_id"] = rx["id"]
        cp["distance_m"] = round(float(np.linalg.norm(pos - tx)), 2)
        cp["rx_x"] = pos[0]; cp["rx_y"] = pos[1]; cp["rx_z"] = pos[2]
        results.append(cp)
        print(f"  {rx['id']:6s}  d={cp['distance_m']:6.2f}m  "
              f"paths={cp['num_paths']:4d}  PL={cp['path_loss_dB']:6.1f}dB  "
              f"RMS={cp['rms_ds_ns']:6.2f}ns  K={cp['k_factor_dB']:+5.1f}dB")

    # ── Save CSV ──
    out_dir = Path(args.output_root) / args.run_id / "analysis"
    out_dir.mkdir(parents=True, exist_ok=True)
    csv_path = out_dir / "per_rx_channel_params.csv"
    import csv
    keys = ["rx_id", "distance_m", "rx_x", "rx_y", "rx_z",
            "num_paths", "path_loss_dB", "rms_ds_ns", "mean_delay_ns",
            "max_excess_delay_ns", "k_factor_dB", "total_power_dBm"]
    with open(csv_path, "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=keys, extrasaction="ignore")
        w.writeheader(); w.writerows(results)
    print(f"\n  CSV -> {csv_path}")

    # ── Plot 1: Path Loss vs Distance ──
    dists = [r["distance_m"] for r in results]
    pl = [r["path_loss_dB"] for r in results]
    rms = [r["rms_ds_ns"] for r in results]
    kf = [r["k_factor_dB"] for r in results]
    paths_n = [r["num_paths"] for r in results]
    ids = [r["rx_id"] for r in results]

    fig, axes = plt.subplots(2, 2, figsize=(14, 10))

    # PL vs distance
    ax = axes[0, 0]
    ax.scatter(dists, pl, c="blue", s=40)
    # Friis reference: PL_fs = 20*log10(4πd/λ)  (free-space path loss)
    lam = C0 / args.freq_hz
    d_ref = np.linspace(min(dists)*0.8, max(dists)*1.2, 100)
    pl_fs = 20 * np.log10(4 * math.pi * d_ref / lam)
    ax.plot(d_ref, pl_fs, "r--", alpha=0.5, label="Friis FSPL")
    for i, rid in enumerate(ids):
        ax.annotate(rid, (dists[i], pl[i]), fontsize=7, alpha=0.7)
    ax.set_xlabel("Distance [m]"); ax.set_ylabel("Path Loss [dB]")
    ax.set_title("Path Loss vs Distance"); ax.legend(); ax.grid(True, alpha=0.3)

    # RMS delay spread vs distance
    ax = axes[0, 1]
    ax.scatter(dists, rms, c="green", s=40)
    for i, rid in enumerate(ids):
        ax.annotate(rid, (dists[i], rms[i]), fontsize=7, alpha=0.7)
    ax.set_xlabel("Distance [m]"); ax.set_ylabel("RMS Delay Spread [ns]")
    ax.set_title("RMS Delay Spread vs Distance"); ax.grid(True, alpha=0.3)

    # K-factor vs distance
    ax = axes[1, 0]
    ax.scatter(dists, kf, c="orange", s=40)
    ax.axhline(y=0, color="gray", linestyle="--", alpha=0.5)
    for i, rid in enumerate(ids):
        ax.annotate(rid, (dists[i], kf[i]), fontsize=7, alpha=0.7)
    ax.set_xlabel("Distance [m]"); ax.set_ylabel("K-Factor [dB]")
    ax.set_title("K-Factor (LOS/NLOS) vs Distance"); ax.grid(True, alpha=0.3)

    # Path count vs distance
    ax = axes[1, 1]
    ax.scatter(dists, paths_n, c="purple", s=40)
    for i, rid in enumerate(ids):
        ax.annotate(rid, (dists[i], paths_n[i]), fontsize=7, alpha=0.7)
    ax.set_xlabel("Distance [m]"); ax.set_ylabel("Number of Valid Paths")
    ax.set_title("Path Count vs Distance"); ax.grid(True, alpha=0.3)

    plt.tight_layout()
    png_path = out_dir / "channel_params_vs_distance.png"
    fig.savefig(str(png_path), dpi=150); plt.close(fig)
    print(f"  Plot -> {png_path}")

    # ── Plot 2: PDP waterfall ──
    fig, ax = plt.subplots(figsize=(14, 6))
    for rx in rx_data:
        paths = rx["paths"]
        if not paths: continue
        delays = np.array([p["delay_s"] for p in paths]) * 1e9  # ns
        powers = np.array([p["power_linear"] for p in paths])
        pwr_dB = 10 * np.log10(powers + 1e-30)
        d = results[len(ax.collections)]["distance_m"] if len(ax.collections) < len(results) else 0
        # Use distance as y-offset
        y_base = next((r["distance_m"] for r in results if r["rx_id"] == rx["id"]), 0)
        ax.scatter(delays, [y_base]*len(delays), c=pwr_dB, cmap="jet",
                   s=10 + 100*np.sqrt(powers/np.max(powers+1e-30)),
                   alpha=0.7, edgecolors="none", vmin=-120, vmax=-40)
    ax.set_xlabel("Delay [ns]"); ax.set_ylabel("Tx-Rx Distance [m]")
    ax.set_title("PDP Waterfall (Distance-Delay-Power)"); ax.grid(True, alpha=0.2)
    png_path = out_dir / "pdp_waterfall.png"
    fig.savefig(str(png_path), dpi=150); plt.close(fig)
    print(f"  Plot -> {png_path}")

    # ── Plot 3: Received Power by Rx ID (numeric ascending) ──
    # Extract numeric index from rx_id (e.g. "rx10" → 10) and sort
    import re
    def rx_sort_key(r):
        m = re.search(r"(\d+)", r["rx_id"])
        return int(m.group(1)) if m else 0

    results_sorted = sorted(results, key=rx_sort_key)
    rx_labels = [r["rx_id"] for r in results_sorted]
    rx_powers = [r["total_power_dBm"] for r in results_sorted]
    rx_pl = [r["path_loss_dB"] for r in results_sorted]
    rx_dist = [r["distance_m"] for r in results_sorted]

    fig, ax1 = plt.subplots(figsize=(14, 5))
    color_pwr = "tab:blue"
    ax1.plot(rx_labels, rx_powers, "o-", color=color_pwr, linewidth=2, markersize=8, label="Received Power")
    ax1.set_xlabel("Rx ID"); ax1.set_ylabel("Received Power [dBm]", color=color_pwr)
    ax1.tick_params(axis="y", labelcolor=color_pwr)
    ax1.grid(True, alpha=0.3)

    ax2 = ax1.twinx()
    color_pl = "tab:red"
    ax2.plot(rx_labels, rx_pl, "s--", color=color_pl, linewidth=1.5, markersize=6, alpha=0.6, label="Path Loss")
    ax2.set_ylabel("Path Loss [dB]", color=color_pl)
    ax2.tick_params(axis="y", labelcolor=color_pl)

    ax1.set_title("Received Power & Path Loss by Rx (sorted by Rx ID)")
    ax1.set_xticks(range(len(rx_labels)))
    ax1.set_xticklabels(rx_labels, rotation=45, ha="right", fontsize=8)

    lines1, labels1 = ax1.get_legend_handles_labels()
    lines2, labels2 = ax2.get_legend_handles_labels()
    ax1.legend(lines1 + lines2, labels1 + labels2, loc="upper right")

    plt.tight_layout()
    png_path = out_dir / "rx_power_by_id.png"
    fig.savefig(str(png_path), dpi=150); plt.close(fig)
    print(f"  Plot -> {png_path}")

    print(f"\nDone -> {out_dir}/")


if __name__ == "__main__":
    main()
