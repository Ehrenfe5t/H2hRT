#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
第三章 信道分析绘图脚本 (图3.6-3.9)

图3.6: APS (角功率谱) 对比 — 全向天线 vs 贴片天线, AoA极坐标图
图3.7: XPR (交叉极化比) 分布 — 所有路径的XPR CDF曲线, 三种天线对比
图3.8: 覆盖热图对比 — 全向 vs 贴片, Z-X截面
图3.9: 方向性覆盖半径量化

用法:
  python test/antenna_channel_analysis.py --paths <precise_paths.json> --mode aps
  python test/antenna_channel_analysis.py --paths <precise_paths.json> --mode xpr
  python test/antenna_channel_analysis.py --coverage-dir <sbr_output> --mode coverage
  python test/antenna_channel_analysis.py --mode all  # 自动查找最新数据
"""

import json, sys, argparse, glob as globmod
from pathlib import Path
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.colors import LogNorm
from collections import defaultdict

ROOT = Path(__file__).resolve().parents[1]
OUT_DIR = ROOT / "document" / "v9" / "figures"
OUT_DIR.mkdir(parents=True, exist_ok=True)

# ─── 常量 ──────────────────────────────────────────────
ANTENNA_STYLES = {
    "Ideal":    {"label": "全向天线 (0dBi)", "color": "#888888", "ls": "--"},
    "dipole":   {"label": "半波偶极子 (2.15dBi)", "color": "#2d9cdb", "ls": "-"},
    "patch":    {"label": "贴片天线 (6dBi)", "color": "#e67e22", "ls": "-"},
    "horn":     {"label": "喇叭天线 (15dBi)", "color": "#27ae60", "ls": "-."},
}


# ═══════════════════════════════════════════════════════════════
#  图3.6: APS (Angular Power Spectrum) 极坐标图
# ═══════════════════════════════════════════════════════════════
def plot_aps(paths_files: dict, title="图3.6"):
    """
    paths_files: {label: filepath} — 不同天线配置的 precise_paths.json
    绘制 AoA zenith角功率分布极坐标图
    """
    fig, axes = plt.subplots(1, len(paths_files), figsize=(6*len(paths_files), 5.5),
                             subplot_kw={"projection": "polar"})
    if len(paths_files) == 1:
        axes = [axes]

    for ax, (label, fpath) in zip(axes, paths_files.items()):
        if not Path(fpath).exists():
            ax.set_title(f"{label}\n(缺数据)", fontsize=10)
            continue

        with open(fpath, "r", encoding="utf-8") as f:
            data = json.load(f)
        paths = data.get("paths", [])

        # AoA binning: 每5°一个bin
        bins = np.arange(0, 181, 5)
        power_by_theta = defaultdict(float)

        for p in paths:
            if not p.get("valid", True):
                continue
            theta = p.get("aoa_theta_deg", 90)
            power = p.get("power_linear", 0)
            # 映射到 [0, 180]
            if theta > 180:
                theta = 360 - theta
            if theta > 180:
                theta = 180
            bin_idx = int(np.clip(theta // 5, 0, len(bins) - 1))
            power_by_theta[bin_idx] += power

        theta_centers = bins[:-1] + 2.5
        powers = np.array([power_by_theta.get(i, 0) for i in range(len(bins))])
        total = powers.sum()
        if total > 0:
            powers_db = 10 * np.log10(powers / total + 1e-30)
        else:
            powers_db = np.zeros_like(powers) - 40

        # 回环绘制 (0→180→0)
        theta_plot = np.concatenate([theta_centers, theta_centers[::-1]])
        power_plot = np.concatenate([powers_db, powers_db[::-1]])
        theta_rad = np.radians(theta_plot)

        ax.fill(theta_rad, np.maximum(power_plot, -30), alpha=0.2, color="#2d9cdb")
        ax.plot(theta_rad, np.maximum(power_plot, -30), color="#2d9cdb", linewidth=1.5)
        ax.set_title(f"{label}\n({len(paths)} 路径)", fontsize=10)
        ax.set_theta_zero_location("N")
        ax.set_theta_direction(-1)
        ax.set_ylim(-30, 0)
        ax.set_yticks([-30, -20, -10, 0])
        ax.set_ylabel("归一化功率 (dB)", fontsize=8)

    plt.suptitle(f"{title} 角功率谱 (APS) — AoA Zenith分布", fontsize=13, fontweight="bold")
    plt.tight_layout(rect=[0, 0, 1, 0.93])
    out = OUT_DIR / "fig3_6_aps.png"
    fig.savefig(out, dpi=200, bbox_inches="tight")
    print(f"图3.6 已保存: {out}")
    plt.close(fig)


# ═══════════════════════════════════════════════════════════════
#  图3.7: XPR (Cross-Polarization Ratio) CDF
# ═══════════════════════════════════════════════════════════════
def plot_xpr_cdf(paths_files: dict, title="图3.7"):
    """
    绘制 XPR 的 CDF 曲线, 对比不同天线类型
    """
    fig, ax = plt.subplots(figsize=(9, 6))

    for label, fpath in paths_files.items():
        if not Path(fpath).exists():
            continue

        with open(fpath, "r", encoding="utf-8") as f:
            data = json.load(f)
        paths = data.get("paths", [])

        xprs = []
        for p in paths:
            xpr = p.get("xpr_dB", None)
            # 跳过缺值和无穷大 (300dB)
            if xpr is not None and abs(xpr) < 200:
                xprs.append(xpr)

        if not xprs:
            print(f"  跳过 {label}: 无有效XPR数据 (需重编译后才有 co/cross-pol)")
            # 尝试用 polarization_vector 推断
            continue

        xprs = np.sort(xprs)
        cdf = np.arange(1, len(xprs) + 1) / len(xprs)

        style = ANTENNA_STYLES.get(label, {"color": "#333", "ls": "-", "label": label})
        ax.plot(xprs, cdf, color=style["color"], linestyle=style["ls"],
                linewidth=2, label=style.get("label", label))

    ax.set_xlabel("XPR (dB)", fontsize=12)
    ax.set_ylabel("CDF", fontsize=12)
    ax.set_title(f"{title} 交叉极化比 (XPR) 累积分布", fontsize=13, fontweight="bold")
    ax.legend(fontsize=10, loc="lower right")
    ax.grid(True, alpha=0.3)
    ax.set_xlim(-5, 40)
    ax.set_ylim(0, 1.02)

    plt.tight_layout()
    out = OUT_DIR / "fig3_7_xpr_cdf.png"
    fig.savefig(out, dpi=200, bbox_inches="tight")
    print(f"图3.7 已保存: {out}")
    plt.close(fig)


# ═══════════════════════════════════════════════════════════════
#  图3.8: 覆盖热图对比 (Z-X 截面)
# ═══════════════════════════════════════════════════════════════
def plot_coverage_heatmap(coverage_data: dict, title="图3.8"):
    """
    coverage_data: {label: list_of_points} 其中每个点为 (x, z, power_dBm)
    绘制 Z-X 截面功率覆盖热图
    """
    fig, axes = plt.subplots(1, len(coverage_data), figsize=(7*len(coverage_data), 6))
    if len(coverage_data) == 1:
        axes = [axes]

    for ax, (label, points) in zip(axes, coverage_data.items()):
        if not points:
            ax.set_title(f"{label}\n(缺数据)")
            continue

        pts = np.array(points)
        x, z, pwr = pts[:, 0], pts[:, 1], pts[:, 2]

        # 生成网格
        grid_size = 100
        x_grid = np.linspace(x.min(), x.max(), grid_size)
        z_grid = np.linspace(z.min(), z.max(), grid_size)
        X, Z = np.meshgrid(x_grid, z_grid)

        # 简单插值: 最近邻
        from scipy.interpolate import griddata
        P = griddata((x, z), pwr, (X, Z), method="nearest")

        im = ax.pcolormesh(X, Z, P, cmap="jet", shading="auto")
        ax.set_xlabel("X (m)")
        ax.set_ylabel("Z (m)")
        ax.set_title(label, fontsize=10)
        ax.set_aspect("equal")
        plt.colorbar(im, ax=ax, label="接收功率 (dBm)", shrink=0.8)

        # 标记Tx位置
        ax.scatter([2.38], [-9.61], marker="*", color="white", s=150,
                  edgecolors="black", linewidths=1, zorder=5)
        ax.annotate("Tx", (2.38, -9.61), color="white", fontsize=9,
                   ha="center", va="bottom")

    plt.suptitle(f"{title} 接收功率覆盖热图 — Z-X截面", fontsize=13, fontweight="bold")
    plt.tight_layout(rect=[0, 0, 1, 0.93])
    out = OUT_DIR / "fig3_8_coverage_heatmap.png"
    fig.savefig(out, dpi=200, bbox_inches="tight")
    print(f"图3.8 已保存: {out}")
    plt.close(fig)


# ═══════════════════════════════════════════════════════════════
#  图3.9: 方向性覆盖半径量化
# ═══════════════════════════════════════════════════════════════
def plot_directional_coverage(beam_results: dict, title="图3.9"):
    """
    beam_results: {direction_label: (angles, radii_m)}
    绘制不同主瓣指向下的覆盖半径极坐标图
    """
    fig, ax = plt.subplots(figsize=(7, 7), subplot_kw={"projection": "polar"})

    colors = ["#2d9cdb", "#e67e22", "#27ae60", "#e74c3c"]
    for i, (label, (angles, radii)) in enumerate(beam_results.items()):
        color = colors[i % len(colors)]
        ang_rad = np.radians(angles)
        ax.fill(ang_rad, radii, alpha=0.15, color=color)
        ax.plot(ang_rad, radii, color=color, linewidth=2, label=label)
        # 标注最大覆盖方向
        max_idx = np.argmax(radii)
        ax.annotate(f"{radii[max_idx]:.1f}m",
                    xy=(ang_rad[max_idx], radii[max_idx]),
                    fontsize=8, color=color)

    ax.set_title(f"{title} 方向性覆盖半径", fontsize=13, fontweight="bold", pad=20)
    ax.set_theta_zero_location("N")
    ax.set_theta_direction(-1)
    ax.legend(fontsize=9, loc="upper right", bbox_to_anchor=(1.3, 1.0))
    ax.set_ylabel("覆盖半径 (m)", fontsize=9)

    out = OUT_DIR / "fig3_9_cover_radius.png"
    fig.savefig(out, dpi=200, bbox_inches="tight")
    print(f"图3.9 已保存: {out}")
    plt.close(fig)


# ═══════════════════════════════════════════════════════════════
#  Synthetic data generators (for when RT data not yet available)
# ═══════════════════════════════════════════════════════════════
def generate_synthetic_aps():
    """生成合成APS数据, 模拟全向 vs 贴片天线效果"""
    np.random.seed(42)
    n_paths = 300

    # 全向: 均匀角度分布
    omni_theta = np.random.uniform(0, 180, n_paths)
    omni_power = np.random.exponential(1e-5, n_paths)

    # 贴片: 主瓣集中在60°-120° (水平方向)
    patch_theta = np.random.normal(90, 25, n_paths)
    patch_theta = np.clip(patch_theta, 0, 180)
    patch_power = np.random.exponential(2e-5, n_paths)  # 更高功率 (有增益)

    return {
        "全向天线 (0dBi)": (omni_theta, omni_power),
        "贴片天线 (6dBi)": (patch_theta, patch_power),
    }

def generate_synthetic_xpr():
    """生成合成XPR数据"""
    np.random.seed(42)
    return {
        "Ideal": np.random.normal(25, 8, 200),
        "dipole": np.random.normal(22, 6, 200),
        "patch": np.random.normal(18, 5, 200),
    }

def plot_aps_synthetic():
    """使用合成数据绘制APS (当无RT数据时)"""
    data = generate_synthetic_aps()
    fig, axes = plt.subplots(1, 2, figsize=(13, 5.5),
                             subplot_kw={"projection": "polar"})

    for ax, (label, (theta, power)) in zip(axes, data.items()):
        # Bin by theta
        bins = np.arange(0, 181, 10)
        power_binned = np.zeros(len(bins))
        for t, p in zip(theta, power):
            bi = int(np.clip(t // 10, 0, len(bins) - 1))
            power_binned[bi] += p

        theta_centers = bins + 5
        power_db = 10 * np.log10(power_binned / power_binned.sum() + 1e-30)

        theta_plot = np.concatenate([theta_centers, theta_centers[::-1]])
        power_plot = np.concatenate([power_db, power_db[::-1]])
        theta_rad = np.radians(theta_plot)

        color = "#2d9cdb" if "全向" in label else "#e67e22"
        ax.fill(theta_rad, np.maximum(power_plot, -25), alpha=0.2, color=color)
        ax.plot(theta_rad, np.maximum(power_plot, -25), color=color, linewidth=1.5)
        ax.set_title(label, fontsize=11)
        ax.set_theta_zero_location("N")
        ax.set_theta_direction(-1)
        ax.set_ylim(-25, 0)
        ax.set_yticks([-25, -20, -15, -10, -5, 0])

    plt.suptitle("图3.6 角功率谱 (APS) — AoA Zenith分布 [合成数据预览]",
                 fontsize=13, fontweight="bold")
    plt.tight_layout(rect=[0, 0, 1, 0.93])
    out = OUT_DIR / "fig3_6_aps.png"
    fig.savefig(out, dpi=200, bbox_inches="tight")
    print(f"图3.6 (合成数据) 已保存: {out}")
    plt.close(fig)

def plot_xpr_synthetic():
    """使用合成数据绘制XPR CDF"""
    data = generate_synthetic_xpr()
    fig, ax = plt.subplots(figsize=(9, 6))

    for label, xprs in data.items():
        style = ANTENNA_STYLES.get(label, {"color": "#333", "ls": "-", "label": label})
        xprs_sorted = np.sort(xprs)
        cdf = np.arange(1, len(xprs_sorted) + 1) / len(xprs_sorted)
        ax.plot(xprs_sorted, cdf, color=style["color"], linestyle=style["ls"],
                linewidth=2, label=style.get("label", label))

    ax.set_xlabel("XPR (dB)", fontsize=12)
    ax.set_ylabel("CDF", fontsize=12)
    ax.set_title("图3.7 交叉极化比 (XPR) 累积分布 [合成数据预览]",
                 fontsize=13, fontweight="bold")
    ax.legend(fontsize=10)
    ax.grid(True, alpha=0.3)
    ax.set_xlim(-5, 40); ax.set_ylim(0, 1.02)

    plt.tight_layout()
    out = OUT_DIR / "fig3_7_xpr_cdf.png"
    fig.savefig(out, dpi=200, bbox_inches="tight")
    print(f"图3.7 (合成数据) 已保存: {out}")
    plt.close(fig)


# ═══════════════════════════════════════════════════════════════
#  Utility: find latest output
# ═══════════════════════════════════════════════════════════════
def find_output_data():
    """自动查找最新的RT输出数据"""
    result = {"paths": {}, "coverage": {}}

    # 查找 precise_paths.json
    for run_dir in ["412", "meeting"]:
        for rx_dir in Path(ROOT / "output" / run_dir).glob("rx*"):
            pf = rx_dir / "paths" / "precise_paths.json"
            if pf.exists():
                label = Path(pf).parent.parent.parent.name
                result["paths"][f"{label}/rx1"] = str(pf)
                break

    return result


# ═══════════════════════════════════════════════════════════════
#  Main
# ═══════════════════════════════════════════════════════════════
if __name__ == "__main__":
    plt.rcParams.update({
        "font.family": "sans-serif",
        "font.sans-serif": ["SimHei", "Microsoft YaHei", "DejaVu Sans"],
        "axes.unicode_minus": False,
        "figure.dpi": 150,
    })

    parser = argparse.ArgumentParser(description="第三章信道分析绘图")
    parser.add_argument("--paths", nargs="*", help="precise_paths.json 文件列表")
    parser.add_argument("--mode", choices=["aps", "xpr", "coverage", "directional", "all"],
                       default="all")
    parser.add_argument("--synthetic", action="store_true",
                       help="使用合成数据 (无RT输出时)")
    args = parser.parse_args()

    print("=== 第三章 信道分析绘图 ===\n")

    if args.synthetic or (not args.paths and not find_output_data().get("paths")):
        print("使用合成数据生成预览图 (实际仿真数据需RT重编译后获取)")
        if args.mode in ("aps", "all"):
            plot_aps_synthetic()
        if args.mode in ("xpr", "all"):
            plot_xpr_synthetic()
        if args.mode in ("coverage", "directional", "all"):
            print("图3.8/3.9 需要SBR覆盖数据, 请先运行Coverage模式仿真")
    else:
        paths_files = {}
        if args.paths:
            for i, fp in enumerate(args.paths):
                paths_files[f"配置{i+1}"] = fp
        else:
            data = find_output_data()
            paths_files = data.get("paths", {})

        if not paths_files:
            print("未找到 precise_paths.json, 使用 --synthetic 生成合成数据预览")
            sys.exit(1)

        if args.mode in ("aps", "all"):
            plot_aps(paths_files)
        if args.mode in ("xpr", "all"):
            plot_xpr_cdf(paths_files)
        if args.mode in ("coverage", "all"):
            print("覆盖热图需 SBR Coverage 输出数据, 请指定 --coverage-dir")

    print("\n[Done] 绘图完成")
