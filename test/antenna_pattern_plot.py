#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
第三章 图3.1 & 图3.2: 天线方向图绘制与插值精度验证

图3.1: 三种天线 (偶极子/贴片/喇叭) 的 E面/H面 极坐标方向图
图3.2: 双线性插值精度验证 — 粗网格插值 vs 解析参考值

用法: python test/antenna_pattern_plot.py
输出: document/v9/figures/fig3_1_patterns.png / fig3_2_interpolation.png
"""

import numpy as np
import matplotlib.pyplot as plt
from pathlib import Path
import csv

ROOT = Path(__file__).resolve().parents[1]
ANT_DIR = ROOT / "configs" / "antennas"
OUT_DIR = ROOT / "document" / "v9" / "figures"
OUT_DIR.mkdir(parents=True, exist_ok=True)

# ─── 天线配置 ────────────────────────────────────────────
ANTENNAS = {
    "dipole":  {"name": "半波偶极子", "color": "#2d9cdb", "peak_gain": 2.15, "ls": "-"},
    "patch":   {"name": "贴片天线 6dBi", "color": "#e67e22", "peak_gain": 6.0,  "ls": "--"},
    "horn":    {"name": "喇叭天线 15dBi", "color": "#27ae60", "peak_gain": 15.0, "ls": "-."},
}

# ─── 解析参考函数 (用于插值验证) ───────────────────────
def dipole_gain_dbi(theta_deg: float) -> float:
    """半波偶极子解析方向图: G(θ) = 1.64 * [cos(π/2·cosθ)/sinθ]²"""
    t = np.radians(theta_deg)
    if abs(np.sin(t)) < 1e-9:
        return -40.0  # null
    v = np.cos(np.pi / 2.0 * np.cos(t)) / np.sin(t)
    return max(-40.0, 10.0 * np.log10(1.64 * v * v))

def patch_gain_dbi(theta_deg: float) -> float:
    """3GPP贴片: G(θ,φ) = 6 + max{-12(θ/65°)², -30} dB (垂直面, φ无关)"""
    t = abs(theta_deg)
    if t > 90:
        t = 180 - t
    return max(-30.0, 6.0 - 12.0 * (t / 65.0) ** 2)

def horn_gain_dbi(theta_deg: float) -> float:
    """cos⁸ 喇叭: G(θ) = 15 + 10·log10(cos⁸θ) = 15 + 80·log10(cosθ)"""
    t = np.radians(min(abs(theta_deg), 89.99))
    ct = np.cos(t)
    if ct < 1e-9:
        return -40.0
    return max(-40.0, 15.0 + 80.0 * np.log10(ct))


# ═══════════════════════════════════════════════════════════════
#  图3.1: 三种天线 E面/H面 极坐标方向图
# ═══════════════════════════════════════════════════════════════
def load_gain_csv(path: Path):
    """加载 gain CSV, 返回 theta[], phi[], gainDBi[][] (theta×phi 矩阵)"""
    rows = []
    with open(path, "r") as f:
        for line in csv.DictReader(f):
            rows.append((float(line["theta_deg"]), float(line["phi_deg"]),
                         float(line["gain_dBi"])))
    thetas = sorted(set(r[0] for r in rows))
    phis = sorted(set(r[1] for r in rows))
    nT, nP = len(thetas), len(phis)
    gain = np.zeros((nT, nP))
    for t, p, g in rows:
        ti = thetas.index(t)
        pi = phis.index(p)
        gain[ti, pi] = g
    return np.array(thetas), np.array(phis), gain

def plot_pattern_3panel():
    """绘制 3×2 子图: 三种天线, 各两条phi-cut (E面/H面)"""
    fig, axes = plt.subplots(2, 3, figsize=(16, 10),
                             subplot_kw={"projection": "polar"})
    fig.suptitle("图3.1 三种天线 E面/H面 方向图对比", fontsize=14, fontweight="bold", y=0.98)

    for col, (key, info) in enumerate(ANTENNAS.items()):
        csv_path = ANT_DIR / f"{key}_gain.csv"
        thetas, phis, gain = load_gain_csv(csv_path)

        # E-plane: phi=90° cut (YZ plane, vertical polarization plane)
        ax_e = axes[0, col]
        phi_idx = np.argmin(np.abs(phis - 90.0))
        theta_vals = np.concatenate([thetas, thetas[::-1]])  # 0→180→0 for full polar
        gain_vals = np.concatenate([gain[:, phi_idx], gain[::-1, phi_idx]])
        ax_e.fill(theta_vals * np.pi / 180.0, np.maximum(gain_vals, -30),
                  alpha=0.15, color=info["color"])
        ax_e.plot(theta_vals * np.pi / 180.0, np.maximum(gain_vals, -30),
                  color=info["color"], linestyle=info["ls"], linewidth=1.8,
                  label=f'{info["name"]} ({info["peak_gain"]}dBi)')
        ax_e.set_title(f'{info["name"]} — E面 (φ=90°)', fontsize=11, pad=15)
        ax_e.set_theta_zero_location("N")
        ax_e.set_theta_direction(-1)
        ax_e.set_ylim(-30, info["peak_gain"] + 3)
        ax_e.set_yticks([-30, -20, -10, 0, info["peak_gain"]])
        ax_e.legend(loc="upper right", fontsize=8, bbox_to_anchor=(1.35, 1.0))

        # H-plane: phi=0° cut (XZ plane, horizontal polarization plane)
        ax_h = axes[1, col]
        phi_idx = np.argmin(np.abs(phis - 0.0))
        gain_vals = np.concatenate([gain[:, phi_idx], gain[::-1, phi_idx]])
        ax_h.fill(theta_vals * np.pi / 180.0, np.maximum(gain_vals, -30),
                  alpha=0.15, color=info["color"])
        ax_h.plot(theta_vals * np.pi / 180.0, np.maximum(gain_vals, -30),
                  color=info["color"], linestyle=info["ls"], linewidth=1.8)
        ax_h.set_title(f'{info["name"]} — H面 (φ=0°)', fontsize=11, pad=15)
        ax_h.set_theta_zero_location("N")
        ax_h.set_theta_direction(-1)
        ax_h.set_ylim(-30, info["peak_gain"] + 3)
        ax_h.set_yticks([-30, -20, -10, 0, info["peak_gain"]])

        # 标注主瓣方向
        ax_e.annotate("主瓣", xy=(np.pi/2, info["peak_gain"]),
                      fontsize=9, color="red", ha="center")
        ax_h.annotate("主瓣", xy=(np.pi/2, info["peak_gain"]),
                      fontsize=9, color="red", ha="center")

    plt.tight_layout(rect=[0, 0, 1, 0.95])
    out = OUT_DIR / "fig3_1_patterns.png"
    fig.savefig(out, dpi=200, bbox_inches="tight")
    print(f"图3.1 已保存: {out}")
    plt.close(fig)


# ═══════════════════════════════════════════════════════════════
#  图3.2: 方向图双线性插值精度验证
# ═══════════════════════════════════════════════════════════════
def bilinear_interp(theta, phi, thetas, phis, gain):
    """双线性插值 (与 C++ AntennaPattern::QueryGainDBi 逻辑一致)"""
    theta = max(0.0, min(180.0, theta))
    phi = phi % 360.0

    ti = 0
    for i in range(len(thetas) - 1):
        if theta >= thetas[i] and theta <= thetas[i+1]:
            ti = i; break
    ti1 = min(ti + 1, len(thetas) - 1)
    t_frac = (theta - thetas[ti]) / (thetas[ti1] - thetas[ti]) if thetas[ti1] != thetas[ti] else 0

    pi = 0
    for j in range(len(phis) - 1):
        if phi >= phis[j] and phi <= phis[j+1]:
            pi = j; break
    pi1 = (pi + 1) % len(phis)
    p_frac = (phi - phis[pi]) / (phis[pi1] - phis[pi]) if phis[pi1] != phis[pi] else 0

    nPhi = len(phis)
    g00 = gain[ti, pi];   g10 = gain[ti1, pi]
    g01 = gain[ti, pi1]; g11 = gain[ti1, pi1]
    g0 = g00 + t_frac * (g10 - g00)
    g1 = g01 + t_frac * (g11 - g01)
    return g0 + p_frac * (g1 - g0)

def plot_interpolation_accuracy():
    """验证双线性插值精度: 粗网格(5°步长) vs 解析参考(0.1°步长)"""
    fig, axes = plt.subplots(1, 3, figsize=(16, 5.5))
    fig.suptitle("图3.2 方向图双线性插值精度验证 (粗网格5° vs 解析参考0.1°)",
                 fontsize=13, fontweight="bold")

    ref_funcs = {"dipole": dipole_gain_dbi, "patch": patch_gain_dbi, "horn": horn_gain_dbi}

    for col, (key, info) in enumerate(ANTENNAS.items()):
        ax = axes[col]

        # 粗网格 (5°步长) 模拟 C++ 中加载的 CSV
        theta_coarse = np.arange(0, 181, 5, dtype=float)
        phi_coarse = np.array([0, 45, 90, 135, 180, 225, 270, 315], dtype=float)
        gain_coarse = np.zeros((len(theta_coarse), len(phi_coarse)))
        for ti, t in enumerate(theta_coarse):
            for pi, p in enumerate(phi_coarse):
                gain_coarse[ti, pi] = ref_funcs[key](t)  # 解析公式, φ无关

        # 随机测试点 (在粗网格间隙中)
        np.random.seed(42)
        n_test = 500
        test_theta = np.random.uniform(0, 180, n_test)
        test_phi = np.random.uniform(0, 360, n_test)

        ref_vals = np.array([ref_funcs[key](t) for t in test_theta])
        interp_vals = np.array([bilinear_interp(test_theta[i], test_phi[i],
                                                 theta_coarse, phi_coarse, gain_coarse)
                                for i in range(n_test)])

        errors = interp_vals - ref_vals

        # 散点图: 参考值 vs 插值误差
        sc = ax.scatter(ref_vals, errors, c=test_theta, cmap="plasma",
                       alpha=0.6, s=15, edgecolors="none")
        ax.axhline(y=0, color="gray", linestyle=":", linewidth=0.8)
        ax.axhline(y=0.5, color="red", linestyle="--", linewidth=0.8, label="±0.5dB")
        ax.axhline(y=-0.5, color="red", linestyle="--", linewidth=0.8)
        ax.set_xlabel("参考增益 (dBi)")
        ax.set_ylabel("插值误差 (dB)")
        ax.set_title(f'{info["name"]}\n均方根误差={np.sqrt(np.mean(errors**2)):.3f}dB  '
                     f'最大={np.max(np.abs(errors)):.3f}dB')
        ax.legend(fontsize=8)
        ax.grid(True, alpha=0.3)
        plt.colorbar(sc, ax=ax, label="θ (deg)")

    plt.tight_layout(rect=[0, 0, 1, 0.93])
    out = OUT_DIR / "fig3_2_interpolation.png"
    fig.savefig(out, dpi=200, bbox_inches="tight")
    print(f"图3.2 已保存: {out}")
    plt.close(fig)


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
    print("=== 第三章 天线方向图绘制 ===")
    plot_pattern_3panel()
    plot_interpolation_accuracy()
    print("完成！")
