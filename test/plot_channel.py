#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
RT 信道数据绘图 (对标 Sionna RT 风格)
=====================================
从 precise_paths.json 生成全套信道图表:
  Fig1: CIR stem + PDP stem
  Fig2: AoD + AoA 极坐标散点图
  Fig3: 功率-时延散点 + 功率 CDF

用法:
  python test/plot_channel.py [--input PATH] [--output DIR]
  默认: output/a1_real_chain/paths/precise_paths.json → output/plots/channel/
"""
import argparse, json, math, os, sys
from pathlib import Path

import numpy as np

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

# 优先使用本地工具库, 回退到手动加载
try:
    sys.path.insert(0, str(Path(__file__).resolve().parent))
    from rt_utils import (load_precise_paths, compute_cir, compute_pdp,
                          compute_aod_aoa, compute_channel_params,
                          compute_cdf, safe_db, set_sionna_style)
except ImportError:
    # 回退: 独立运行, 内联所有函数
    from rt_utils import *  # type: ignore

C0 = 299792458.0

# ═══════════ 修改此路径指向你的 precise_paths.json ═══════════
IN_DIR = Path("F:/RT/RT/output/meeting-cov-hires/paths/precise_paths.json")
OUT_DIR = Path("F:/RT/RT/output/meeting-cov-hires/plots/channel")
# ═════════════════════════════════════════════════════════════



def plot_figure1_cir_pdp(cir: dict, pdp: dict, title: str, out_path: Path):
    """Fig1: CIR stem (左) + PDP stem (右) — Sionna style."""
    fig, axes = plt.subplots(1, 2, figsize=(14, 5))

    # CIR
    ax = axes[0]
    if len(cir["taus_ns"]) > 0:
        markerline, stemlines, baseline = ax.stem(
            cir["taus_ns"], np.abs(cir["amplitudes"]),
            linefmt="C0-", markerfmt="C0o", basefmt="k-")
        plt.setp(stemlines, linewidth=1.5)
        plt.setp(markerline, markersize=6)
    ax.set_xlabel("Delay [ns]")
    ax.set_ylabel("|CIR|")
    ax.set_title(f"{title} — CIR")
    ax.grid(True, alpha=0.3)

    # PDP
    ax = axes[1]
    if len(pdp["taus_ns"]) > 0:
        mline, slines, bline = ax.stem(pdp["taus_ns"], pdp["power_dB"],
                                        linefmt="C1-", markerfmt="C1o", basefmt="k-")
        plt.setp(slines, linewidth=1.5)
        plt.setp(mline, markersize=6)
    ax.set_xlabel("Delay [ns]")
    ax.set_ylabel("Power [dB]")
    ax.set_title(f"{title} — PDP")
    ax.grid(True, alpha=0.3)

    plt.tight_layout()
    out_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(str(out_path), dpi=150)
    plt.close(fig)
    print(f"  [1] CIR/PDP  → {out_path}")


def plot_figure2_angles(angles: dict, title: str, out_path: Path):
    """Fig2: AoD (左) + AoA (右) 极坐标散点 — Sionna style."""
    if len(angles.get("aod_theta_deg", [])) == 0:
        return
    aod_t = angles["aod_theta_deg"]
    aod_p = angles["aod_phi_deg"]
    aoa_t = angles["aoa_theta_deg"]
    aoa_p = angles["aoa_phi_deg"]
    p_norm = angles["powers_norm"]
    pwr_dB = safe_db(angles.get("powers_linear", np.ones_like(p_norm)))

    fig, axes = plt.subplots(1, 2, figsize=(13, 6),
                             subplot_kw={"projection": "polar"})

    # AoD
    ax = axes[0]
    sc = ax.scatter(np.radians(aod_p), 90.0 - aod_t, s=20 + p_norm * 200,
                    c=pwr_dB, cmap="hot", alpha=0.8, edgecolors="black",
                    linewidths=0.3)
    ax.set_title(f"{title} — AoD", va="bottom", fontsize=12)
    ax.set_theta_zero_location("N")
    ax.set_theta_direction(-1)
    plt.colorbar(sc, ax=ax, label="Power [dB]", shrink=0.7)

    # AoA
    ax = axes[1]
    sc = ax.scatter(np.radians(aoa_p), 90.0 - aoa_t, s=20 + p_norm * 200,
                    c=pwr_dB, cmap="hot", alpha=0.8, edgecolors="black",
                    linewidths=0.3)
    ax.set_title(f"{title} — AoA", va="bottom", fontsize=12)
    ax.set_theta_zero_location("N")
    ax.set_theta_direction(-1)
    plt.colorbar(sc, ax=ax, label="Power [dB]", shrink=0.7)

    plt.tight_layout()
    out_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(str(out_path), dpi=150)
    plt.close(fig)
    print(f"  [2] Angle  → {out_path}")


def plot_figure3_stats(pdp: dict, title: str, out_path: Path):
    """Fig3: 功率-时延散点 (左) + 功率 CDF (右) — Sionna style."""
    fig, axes = plt.subplots(1, 2, figsize=(14, 5))

    taus = pdp["taus_ns"]
    pwrs_lin = pdp["power_linear"]
    pwr_dB = pdp["power_dB"]

    # Power-Delay scatter
    ax = axes[0]
    if len(taus) > 0:
        sizes = 20 + 180 * pwrs_lin / np.max(pwrs_lin)
        sc = ax.scatter(taus, pwr_dB, s=sizes, c=taus, cmap="plasma",
                        alpha=0.8, edgecolors="black", linewidths=0.3)
        plt.colorbar(sc, ax=ax, label="Delay [ns]")
    ax.set_xlabel("Delay [ns]")
    ax.set_ylabel("Power [dB]")
    ax.set_title(f"{title} — Power-Delay")
    ax.grid(True, alpha=0.3)

    # Power CDF
    ax = axes[1]
    if len(pwr_dB) > 0:
        s, cdf = compute_cdf(pwr_dB)
        ax.plot(s, cdf, "b-", linewidth=2)
        ax.fill_between(s, 0, cdf, alpha=0.15)
    ax.set_xlabel("Power [dB]")
    ax.set_ylabel("CDF")
    ax.set_title(f"{title} — Power CDF")
    ax.grid(True, alpha=0.3)

    plt.tight_layout()
    out_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(str(out_path), dpi=150)
    plt.close(fig)
    print(f"  [3] Stats  → {out_path}")


def print_channel_params(cp: dict, title: str):
    """打印信道参数摘要。"""
    print(f"\n  ── {title} Channel Parameters ──")
    for k in ["num_paths", "path_loss_dB", "rms_ds_ns", "mean_delay_ns",
              "max_excess_delay_ns", "k_factor_dB", "total_power_dBm"]:
        v = cp.get(k, "N/A")
        if isinstance(v, float):
            print(f"    {k}: {v:.2f}")
        else:
            print(f"    {k}: {v}")


def main():
    parser = argparse.ArgumentParser(description="RT Channel Plotting (Sionna-style)")
    parser.add_argument("--input", "-i", type=str, default=None,
                        help=f"Path to precise_paths.json (default: {IN_DIR})")
    parser.add_argument("--output", "-o", type=str, default=None,
                        help="Output directory for plots")
    parser.add_argument("--title", "-t", type=str, default="RT Channel",
                        help="Title prefix for all plots")
    args = parser.parse_args()

    in_path = Path(args.input) if args.input else IN_DIR
    out_dir = Path(args.output) if args.output else OUT_DIR
    title = args.title

    set_sionna_style()

    print(f"RT Channel Plotting (Sionna-style)")
    print(f"  Input:  {in_path}")
    print(f"  Output: {out_dir}/")

    paths = load_precise_paths(in_path)
    print(f"  Loaded {len(paths)} paths")

    if not paths:
        print("  WARNING: No paths found. Check IN_DIR at top of script.")
        return 1

    # Compute all data
    cir = compute_cir(paths)
    pdp = compute_pdp(paths)
    angles = compute_aod_aoa(paths)
    cp = compute_channel_params(paths)

    print_channel_params(cp, title)

    # Generate plots
    base = title.lower().replace(" ", "_")
    plot_figure1_cir_pdp(cir, pdp, title, out_dir / f"{base}_01_cir_pdp.png")
    plot_figure2_angles(angles, title, out_dir / f"{base}_02_angles.png")
    plot_figure3_stats(pdp, title, out_dir / f"{base}_03_stats.png")

    # Save channel params as JSON
    param_path = out_dir / f"{base}_channel_params.json"
    param_path.parent.mkdir(parents=True, exist_ok=True)
    param_path.write_text(json.dumps(cp, indent=2, ensure_ascii=False), encoding="utf-8")
    print(f"  [4] Params → {param_path}")

    print(f"\nDone → {out_dir}/")
    return 0


if __name__ == "__main__":
    sys.exit(main())
