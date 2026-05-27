#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
RT 功率覆盖静态热力图 (对标 Sionna RadioMap 风格)
================================================
从 sbr_coverage.json 读取全量覆盖结果, 输出选定平面的 2D 功率热力图。
支持 X=const, Y=const, Z=const 三个方向的平面切片。
无平滑处理: 每个 Rx 网格点直接显示原始功率值。
全空间覆盖: 无功率的 Rx 位置按 colorbar 最小值绘制, 与有功率区域一致。

用法:
  python test/plot_coverage_static.py [--sbr PATH] [--obj PATH] [--output DIR]
       [--plane {xy,xz,yz}] [--position VAL] [--tx X Y Z] [--rx X Y Z]

默认: output/a1_real_chain/coverage/sbr_coverage.json → output/plots/coverage/

输出文件:
  coverage_{plane}_pos{val}.png — 功率覆盖热力图
  coverage_stats.json — 覆盖统计摘要
"""
import argparse, json, os, sys, math
from pathlib import Path

import numpy as np

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.colors import Normalize

MODULE_DIR = Path(__file__).resolve().parent
REPO_ROOT = MODULE_DIR.parent

# ═══════════ 修改此路径指向你的 sbr_coverage.json ═══════════
IN_SBR = REPO_ROOT / "output" / "meeting-cov-hires" / "coverage" / "sbr_coverage.json"
OUT_DIR = REPO_ROOT / "output" / "plots" / "coverage"
# ═════════════════════════════════════════════════════════════

C0 = 299792458.0


def safe_db(x, floor_db=-200.0):
    return np.maximum(10.0 * np.log10(np.maximum(np.abs(x), 1e-30)), floor_db)


def load_coverage(sbr_path):
    with open(sbr_path, "r", encoding="utf-8") as f:
        data = json.load(f)
    records = data.get("records", [])
    n = len(records)
    rx_x = np.zeros(n); rx_y = np.zeros(n); rx_z = np.zeros(n)
    rx_pwr = np.zeros(n); rx_hit = np.zeros(n, dtype=bool)

    for i, r in enumerate(records):
        rx_x[i] = float(r.get("x", 0)); rx_y[i] = float(r.get("y", 0))
        rx_z[i] = float(r.get("z", 0))
        rx_pwr[i] = float(r.get("power_dBm", -200))
        rx_hit[i] = int(r.get("ray_hit_count", 0)) > 0
    return rx_x, rx_y, rx_z, rx_pwr, rx_hit


def build_image_grid(x_coords, y_coords, values, x_label, y_label):
    """从分散 Rx 点构建规则网格图像矩阵。无平滑/插值。"""
    ux = np.sort(np.unique(np.round(x_coords, 3)))
    uy = np.sort(np.unique(np.round(y_coords, 3)))
    nx, ny = len(ux), len(uy)
    grid = np.full((ny, nx), np.nan)
    for i in range(len(values)):
        ix = np.searchsorted(ux, round(x_coords[i], 3))
        iy = np.searchsorted(uy, round(y_coords[i], 3))
        if 0 <= ix < nx and 0 <= iy < ny:
            grid[iy, ix] = values[i]
    return ux, uy, grid


def plot_plane(rx_x, rx_y, rx_z, rx_pwr, plane, position, tx_pos, rx_pos,
               out_path, title=""):
    """在指定平面上绘制功率覆盖热力图 (无平滑)。"""
    axis_map = {
        "xy": (0, 1, "X [m]", "Y [m]", 2, rx_z),
        "xz": (0, 2, "X [m]", "Z [m]", 1, rx_y),
        "yz": (1, 2, "Y [m]", "Z [m]", 0, rx_x),
    }
    if plane not in axis_map:
        raise ValueError(f"Unknown plane: {plane}")
    i0, i1, xl, yl, ia, ax_data = axis_map[plane]
    coords = {0: rx_x, 1: rx_y, 2: rx_z}

    # 筛选切片平面附近的 Rx (全部 Rx, 不要求 hit)
    tol = max(0.05, abs(np.diff(np.unique(np.round(ax_data, 2))).mean()) * 0.6)
    if np.isnan(tol) or tol < 1e-3:
        tol = 0.08
    mask = np.abs(ax_data - position) < tol
    n_plane = int(np.sum(mask))
    if n_plane < 4:
        print(f"  WARNING: Only {n_plane} Rx in {plane.upper()} plane at pos={position:.2f}")
        return

    c0 = coords[i0][mask]; c1 = coords[i1][mask]; pp = rx_pwr[mask]
    u0, u1, grid = build_image_grid(c0, c1, pp, xl, yl)

    if np.all(np.isnan(grid)):
        print(f"  WARNING: All NaN grid in {plane.upper()} plane at pos={position:.2f}")
        return

    pwr_min = float(np.nanmin(grid)) if np.any(~np.isnan(grid)) else -200.0
    pwr_max = float(np.nanmax(grid)) if np.any(~np.isnan(grid)) else -50.0
    if pwr_max <= pwr_min:
        pwr_max = pwr_min + 10.0

    # 填充 NaN → pwr_min (全空间覆盖)
    grid_fill = np.where(np.isnan(grid), pwr_min, grid)

    fig, ax = plt.subplots(figsize=(16, 8))
    extent = [float(u0[0]), float(u0[-1]), float(u1[0]), float(u1[-1])]
    im = ax.imshow(grid_fill, origin="lower", cmap="jet", aspect="auto",
                   extent=extent, vmin=pwr_min, vmax=pwr_max,
                   interpolation="none")
    cbar = fig.colorbar(im, ax=ax, shrink=0.8, pad=0.02)
    cbar.set_label("Power [dBm]", fontsize=12)

    # Tx/Rx 标记 (映射到自由轴)
    if plane == "xy":
        ax.plot(tx_pos[0], tx_pos[1], "r^", markersize=15,
                markeredgecolor="black", markeredgewidth=2, label="TX")
        ax.plot(rx_pos[0], rx_pos[1], "gs", markersize=12,
                markeredgecolor="black", markeredgewidth=2, label="RX")
    elif plane == "xz":
        ax.plot(tx_pos[0], tx_pos[2], "r^", markersize=15,
                markeredgecolor="black", markeredgewidth=2, label="TX")
        ax.plot(rx_pos[0], rx_pos[2], "gs", markersize=12,
                markeredgecolor="black", markeredgewidth=2, label="RX")
    elif plane == "yz":
        ax.plot(tx_pos[1], tx_pos[2], "r^", markersize=15,
                markeredgecolor="black", markeredgewidth=2, label="TX")
        ax.plot(rx_pos[1], rx_pos[2], "gs", markersize=12,
                markeredgecolor="black", markeredgewidth=2, label="RX")

    ax.legend(fontsize=11, loc="upper right")
    ax.set_xlabel(xl, fontsize=12)
    ax.set_ylabel(yl, fontsize=12)
    t = f"{title + ' — ' if title else ''}{plane.upper()} plane @ pos={position:.2f}m"
    ax.set_title(t, fontsize=13)
    ax.grid(False)

    out_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(str(out_path), dpi=150)
    plt.close(fig)
    print(f"  Coverage: {out_path} ({n_plane} Rx points)")


def compute_coverage_stats(rx_pwr, rx_hit):
    """覆盖统计。"""
    n_total = len(rx_pwr)
    n_covered = int(np.sum(rx_hit))
    pct = n_covered / n_total * 100.0 if n_total > 0 else 0.0
    pwr_valid = rx_pwr[rx_hit]
    return {
        "total_rx_points": n_total,
        "covered_rx_points": n_covered,
        "coverage_pct": round(pct, 2),
        "power_min_dBm": float(np.min(pwr_valid)) if len(pwr_valid) > 0 else -200,
        "power_max_dBm": float(np.max(pwr_valid)) if len(pwr_valid) > 0 else -200,
        "power_mean_dBm": float(np.mean(pwr_valid)) if len(pwr_valid) > 0 else -200,
        "power_median_dBm": float(np.median(pwr_valid)) if len(pwr_valid) > 0 else -200,
    }


def main():
    parser = argparse.ArgumentParser(description="RT Coverage Static Heatmaps")
    parser.add_argument("--sbr", type=str, default=None,
                        help="Path to sbr_coverage.json")
    parser.add_argument("--output", "-o", type=str, default=None,
                        help="Output directory for plots")
    parser.add_argument("--plane", type=str, default="xz",
                        choices=["xy", "xz", "yz"],
                        help="Slice plane (default: xz)")
    parser.add_argument("--position", "-p", type=float, default=None,
                        help="Slice position along plane-normal axis")
    parser.add_argument("--tx", type=float, nargs=3, default=[16.0, 1.5, -12.0],
                        help="Tx position (x y z)")
    parser.add_argument("--rx", type=float, nargs=3, default=[10.0, 1.5, -10.0],
                        help="Rx position (x y z)")
    parser.add_argument("--all-planes", action="store_true",
                        help="Generate all three plane views at midpoint positions")
    parser.add_argument("--title", "-t", type=str, default="RT Coverage",
                        help="Title prefix for plots")
    args = parser.parse_args()

    sbr_path = Path(args.sbr) if args.sbr else IN_SBR
    out_dir = Path(args.output) if args.output else OUT_DIR
    tx_pos = np.array(args.tx)
    rx_pos = np.array(args.rx)

    if not sbr_path.exists():
        print(f"ERROR: SBR data not found: {sbr_path}")
        return 1

    print(f"RT Coverage Static Heatmaps (Sionna-style)")
    print(f"  SBR:   {sbr_path}")
    print(f"  Output: {out_dir}/")

    rx_x, rx_y, rx_z, rx_pwr, rx_hit = load_coverage(sbr_path)
    print(f"  Loaded {len(rx_x)} Rx points")

    # 覆盖统计
    stats = compute_coverage_stats(rx_pwr, rx_hit)
    print(f"\n  Coverage Statistics:")
    for k, v in stats.items():
        print(f"    {k}: {v}")

    base = args.title.lower().replace(" ", "_")

    if args.all_planes:
        # 三个平面的中间位置
        planes = {
            "xy": float(np.median(rx_z)),
            "xz": float(np.median(rx_y)),
            "yz": float(np.median(rx_x)),
        }
        for plane, pos in planes.items():
            fname = f"{base}_coverage_{plane}.png"
            plot_plane(rx_x, rx_y, rx_z, rx_pwr, plane, pos, tx_pos, rx_pos,
                       out_dir / fname, args.title)
    else:
        position = args.position
        if position is None:
            if args.plane == "xy":
                position = float(np.median(rx_z))
            elif args.plane == "xz":
                position = float(np.median(rx_y))
            else:
                position = float(np.median(rx_x))
        fname = f"{base}_coverage_{args.plane}.png"
        plot_plane(rx_x, rx_y, rx_z, rx_pwr, args.plane, position,
                   tx_pos, rx_pos, out_dir / fname, args.title)

    # 保存统计
    stats_path = out_dir / f"{base}_coverage_stats.json"
    stats_path.parent.mkdir(parents=True, exist_ok=True)
    stats_path.write_text(json.dumps(stats, indent=2, ensure_ascii=False), encoding="utf-8")
    print(f"\n  Stats → {stats_path}")
    print(f"Done → {out_dir}/")
    return 0


if __name__ == "__main__":
    sys.exit(main())
