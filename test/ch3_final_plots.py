#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
第三章最终图表生成 — 从真实仿真数据绘制全部9张图

数据源:
  E1: antenna type comparison (output/ch3_full/E1_*)
  E2: polarization sweep (output/E2_*)
  E3: pose yaw sweep (output/E3_*)
  Coverage: SBR output (需单独运行)

用法: python test/ch3_final_plots.py
"""

import json, os, sys, re
from pathlib import Path
from collections import defaultdict
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.ticker import FuncFormatter

ROOT = Path(__file__).resolve().parents[1]
OUT_DIR = ROOT / "document" / "v9" / "figures"
OUT_DIR.mkdir(parents=True, exist_ok=True)

# ─── 配置 ──────────────────────────────────────────────
ANT_COLORS = {
    "Ideal":  "#888888", "dipole": "#2d9cdb",
    "patch":  "#e67e22", "horn":   "#27ae60",
}
ANT_LABELS = {
    "Ideal":  "全向 (0dBi)", "dipole": "偶极子 (2.15dBi)",
    "patch":  "贴片 (6dBi)", "horn":   "喇叭 (15dBi)",
}
POL_LABELS_CN = {"V":"垂直(V)","H":"水平(H)","P45":"+45°斜","LHCP":"左旋圆","RHCP":"右旋圆"}
POL_COLORS = {"V":"#2d9cdb","H":"#e67e22","P45":"#27ae60","LHCP":"#e74c3c","RHCP":"#9b59b6"}


# ═══════════════════════════════════════════════════════════════
#  数据加载
# ═══════════════════════════════════════════════════════════════
def load_paths(run_dir):
    """加载某个run的输出数据"""
    if isinstance(run_dir, Path):
        pf = run_dir / "rx1" / "paths" / "precise_paths.json"
    else:
        pf = ROOT / "output" / run_dir / "rx1" / "paths" / "precise_paths.json"

    candidates = [
        pf,
        ROOT / "output" / "ch3_full" / str(run_dir) / "rx1" / "paths" / "precise_paths.json",
        ROOT / "output" / str(run_dir) / "rx1" / "paths" / "precise_paths.json",
    ]
    for c in candidates:
        if c.exists():
            with open(c, "r") as f:
                data = json.load(f)
            return data.get("paths", [])
    print(f"  WARNING: no data for {run_dir}")
    return []

def summary(paths):
    """提取统计量"""
    v = [p for p in paths if p.get("power_linear", 0) > 0]
    if not v:
        return {}
    pw = [p["power_linear"] for p in v]
    tp = sum(pw)
    los_pw = sum(p["power_linear"] for p in v if p.get("is_los"))
    xpr = [p.get("xpr_dB", 0) for p in v if abs(p.get("xpr_dB", 0)) < 200]
    co = sum(p.get("co_pol_power_linear", 0) for p in v)
    cx = sum(p.get("cross_pol_power_linear", 0) for p in v)
    return {
        "n": len(v), "tp": tp, "tp_dbm": 10*np.log10(tp*1000) if tp > 0 else -999,
        "los_dbm": 10*np.log10(los_pw*1000) if los_pw > 0 else -999,
        "xpr_mean": float(np.mean(xpr)) if xpr else 0,
        "xpr_median": float(np.median(xpr)) if xpr else 0,
        "xpr_list": xpr, "pw_list": pw,
        "aoa_theta": [p.get("aoa_theta_deg", 90) for p in v],
        "aoa_phi": [p.get("aoa_phi_deg", 0) for p in v],
        "co_total": co, "cx_total": cx,
        "co_list": [p.get("co_pol_power_linear", 0) for p in v],
        "cx_list": [p.get("cross_pol_power_linear", 0) for p in v],
    }


# ═══════════════════════════════════════════════════════════════
#  图3.1: 三种天线E面/H面方向图 (from CSV)
# ═══════════════════════════════════════════════════════════════
def fig_3_1():
    """读取CSV绘制方向图"""
    import csv
    ant_dir = ROOT / "configs" / "antennas"
    ants = {
        "dipole": ("半波偶极子 (2.15dBi)", "#2d9cdb", "-"),
        "patch":  ("贴片天线 (6dBi)",      "#e67e22", "--"),
        "horn":   ("喇叭天线 (15dBi)",     "#27ae60", "-."),
    }

    fig, axes = plt.subplots(2, 3, figsize=(16, 10.5),
                             subplot_kw={"projection": "polar"})

    for col, (key, (name, color, ls)) in enumerate(ants.items()):
        # Load gain CSV
        rows = []
        with open(ant_dir / f"{key}_gain.csv", "r") as f:
            for line in csv.DictReader(f):
                rows.append((float(line["theta_deg"]), float(line["phi_deg"]),
                            float(line["gain_dBi"])))
        thetas = sorted(set(r[0] for r in rows))
        phis = sorted(set(r[1] for r in rows))
        nT, nP = len(thetas), len(phis)
        gain = np.zeros((nT, nP))
        for t, p, g in rows:
            gain[thetas.index(t), phis.index(p)] = g

        theta_arr = np.array(thetas)
        peak = float(np.max(gain))

        # E-plane (phi=90)
        ax = axes[0, col]
        pi = np.argmin(np.abs(np.array(phis) - 90.0))
        gv = np.concatenate([gain[:, pi], gain[::-1, pi]])
        tv = np.concatenate([theta_arr, theta_arr[::-1]])
        ax.fill(np.radians(tv), np.maximum(gv, -30), alpha=0.12, color=color)
        ax.plot(np.radians(tv), np.maximum(gv, -30), color=color, ls=ls, lw=2)
        ax.set_title(f"{name}\nE面 (φ=90°)", fontsize=11, pad=14)
        ax.set_theta_zero_location("N"); ax.set_theta_direction(-1)
        ax.set_ylim(-30, peak + 3)
        ax.annotate(f"{peak:.1f}dBi", xy=(np.pi/2, peak), fontsize=9, color="red", ha="center")

        # H-plane (phi=0)
        ax = axes[1, col]
        pi = np.argmin(np.abs(np.array(phis) - 0.0))
        gv = np.concatenate([gain[:, pi], gain[::-1, pi]])
        ax.fill(np.radians(tv), np.maximum(gv, -30), alpha=0.12, color=color)
        ax.plot(np.radians(tv), np.maximum(gv, -30), color=color, ls=ls, lw=2)
        ax.set_title(f"{name}\nH面 (φ=0°)", fontsize=11, pad=14)
        ax.set_theta_zero_location("N"); ax.set_theta_direction(-1)
        ax.set_ylim(-30, peak + 3)

    plt.suptitle("图3.1  三种天线方向图对比 (E面 / H面)", fontsize=14, fontweight="bold", y=0.99)
    plt.tight_layout(rect=[0, 0, 1, 0.95])
    fig.savefig(OUT_DIR / "fig3_1_patterns.png", dpi=250, bbox_inches="tight")
    plt.close(fig)
    print("图3.1 done")


# ═══════════════════════════════════════════════════════════════
#  图3.2: 插值精度 (已从 antenna_pattern_plot.py 生成)
# ═══════════════════════════════════════════════════════════════
# (已有, 直接复用)


# ═══════════════════════════════════════════════════════════════
#  图3.3: Rx极化旋转 — 接收功率对比
# ═══════════════════════════════════════════════════════════════
def fig_3_3():
    runs = {"V":"E2_V", "H":"E2_H", "P45":"E2_P45", "LHCP":"E2_LHCP", "RHCP":"E2_RHCP"}
    data = {}
    for key, run in runs.items():
        paths = load_paths(run)
        if paths:
            data[key] = summary(paths)

    fig, ax = plt.subplots(figsize=(8, 5.5))
    keys = list(data.keys())
    powers = [data[k]["tp_dbm"] for k in keys]
    los_powers = [data[k]["los_dbm"] for k in keys]
    colors = [POL_COLORS[k] for k in keys]
    labels = [POL_LABELS_CN[k] for k in keys]

    x = np.arange(len(keys))
    w = 0.35
    b1 = ax.bar(x - w/2, powers, w, color=colors, alpha=0.85, edgecolor="white", label="总接收功率")
    b2 = ax.bar(x + w/2, los_powers, w, color=colors, alpha=0.4, edgecolor="white",
                hatch="///", label="LOS分量")

    for bar, val in zip(b1, powers):
        ax.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 0.3,
                f"{val:.1f}", ha="center", fontsize=9, fontweight="bold")

    ax.set_xticks(x); ax.set_xticklabels(labels, fontsize=11)
    ax.set_ylabel("接收功率 (dBm)", fontsize=12)
    ax.set_title("图3.3  Rx极化旋转实验 — 接收功率对比\n(Tx=垂直极化, 场景412 LOS走廊)",
                 fontsize=13, fontweight="bold")
    ax.legend(fontsize=10)
    ax.grid(axis="y", alpha=0.3)
    ax.set_ylim(min(powers) - 2, max(powers) + 2)

    plt.tight_layout()
    fig.savefig(OUT_DIR / "fig3_3_polarization.png", dpi=250, bbox_inches="tight")
    plt.close(fig)
    print("图3.3 done")


# ═══════════════════════════════════════════════════════════════
#  图3.4: Rx姿态yaw扫描
# ═══════════════════════════════════════════════════════════════
def fig_3_4():
    yaws = [0, 30, 60, 90, 120, 150, 180]
    data = {}
    for y in yaws:
        paths = load_paths(f"E3_yaw{y}")
        if paths:
            data[y] = summary(paths)

    fig, ax = plt.subplots(figsize=(9, 5.5))
    yaw_vals = sorted(data.keys())
    tp = [data[y]["tp_dbm"] for y in yaw_vals]
    los = [data[y]["los_dbm"] for y in yaw_vals]

    ax.plot(yaw_vals, tp, "o-", color="#2d9cdb", lw=2.5, markersize=10, label="总接收功率")
    ax.plot(yaw_vals, los, "s--", color="#e67e22", lw=2, markersize=8, label="LOS分量")
    ax.fill_between(yaw_vals, los, tp, alpha=0.1, color="#2d9cdb")

    for y, v in zip(yaw_vals, tp):
        ax.annotate(f"{v:.1f}", (y, v), textcoords="offset points", xytext=(0, 10),
                   fontsize=8, ha="center", color="#2d9cdb")

    ax.set_xlabel("Rx天线 Yaw 角 (deg)", fontsize=12)
    ax.set_ylabel("接收功率 (dBm)", fontsize=12)
    ax.set_title("图3.4  Rx天线姿态扫描 — 接收功率 vs Yaw角\n(Tx/Rx=垂直极化, 场景412 LOS走廊)",
                 fontsize=13, fontweight="bold")
    ax.legend(fontsize=11)
    ax.grid(True, alpha=0.3)
    ax.set_xticks(yaw_vals)

    plt.tight_layout()
    fig.savefig(OUT_DIR / "fig3_4_pose.png", dpi=250, bbox_inches="tight")
    plt.close(fig)
    print("图3.4 done")


# ═══════════════════════════════════════════════════════════════
#  图3.5: 联合极化效率矩阵
# ═══════════════════════════════════════════════════════════════
def fig_3_5():
    # 从E2数据计算归一化功率矩阵 (以V为参考)
    runs = {"V":"E2_V", "H":"E2_H", "P45":"E2_P45", "LHCP":"E2_LHCP", "RHCP":"E2_RHCP"}
    data = {}
    for key, run in runs.items():
        paths = load_paths(run)
        if paths:
            data[key] = summary(paths)

    if not data:
        print("  E2 data missing, skipping fig3_5")
        return

    # 以V-V为参考 (0dB), 计算相对功率
    vv_power = data.get("V", {}).get("tp", 1e-10)
    pols = list(data.keys())
    rel_power_db = [data[p]["tp_dbm"] - (10*np.log10(vv_power*1000)) for p in pols]

    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 5.5))

    # Left: bar chart
    colors = [POL_COLORS[p] for p in pols]
    labels = [POL_LABELS_CN[p] for p in pols]
    bars = ax1.bar(range(len(pols)), rel_power_db, color=colors, alpha=0.85, edgecolor="white")
    for bar, val in zip(bars, rel_power_db):
        ax1.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 0.1,
                f"{val:.1f}dB", ha="center", fontsize=10, fontweight="bold")
    ax1.set_xticks(range(len(pols)))
    ax1.set_xticklabels(labels, fontsize=10)
    ax1.set_ylabel("相对功率 (dB, 以V-V为0dB)", fontsize=11)
    ax1.set_title("Rx极化效率 (Tx=垂直)", fontsize=12, fontweight="bold")
    ax1.grid(axis="y", alpha=0.3)
    ax1.axhline(y=-3, color="gray", ls="--", lw=0.8, alpha=0.5)
    ax1.annotate("-3dB (半功率)", xy=(len(pols)-1, -3), fontsize=8, color="gray")

    # Right: PLF理论对比
    # PLF = |e_tx · conj(e_rx)|²
    e_tx = np.array([0, 1, 0], complex)  # Tx vertical
    pol_vecs = {
        "V":    np.array([0, 1, 0], complex),
        "H":    np.array([0, 0, 1], complex),
        "P45":  np.array([0, 1/np.sqrt(2), 1/np.sqrt(2)], complex),
        "LHCP": np.array([0, 1/np.sqrt(2), 0], complex) + 1j*np.array([0, 0, 1/np.sqrt(2)], complex),
        "RHCP": np.array([0, 1/np.sqrt(2), 0], complex) - 1j*np.array([0, 0, 1/np.sqrt(2)], complex),
    }
    plf_theory = {}
    for key, e_rx in pol_vecs.items():
        plf = abs(np.dot(e_tx, np.conj(e_rx)))**2
        plf_theory[key] = 10*np.log10(plf) if plf > 0 else -40

    theory_db = [plf_theory[p] for p in pols]
    ax2.scatter(rel_power_db, theory_db, c=colors, s=120, edgecolors="black", linewidths=1, zorder=5)
    for i, p in enumerate(pols):
        ax2.annotate(POL_LABELS_CN[p], (rel_power_db[i], theory_db[i]),
                    textcoords="offset points", xytext=(8, 4), fontsize=9)
    ax2.plot([-4, 0], [-4, 0], "k--", lw=0.8, alpha=0.4, label="y=x (理想匹配)")
    ax2.set_xlabel("实测相对功率 (dB)", fontsize=11)
    ax2.set_ylabel("理论PLF (dB)", fontsize=11)
    ax2.set_title("实测 vs 理论极化匹配因子", fontsize=12, fontweight="bold")
    ax2.legend(fontsize=9)
    ax2.grid(True, alpha=0.3)

    plt.suptitle("图3.5  联合极化效率分析 (Tx垂直 × Rx五种极化)", fontsize=14, fontweight="bold")
    plt.tight_layout(rect=[0, 0, 1, 0.93])
    fig.savefig(OUT_DIR / "fig3_5_polarization_matrix.png", dpi=250, bbox_inches="tight")
    plt.close(fig)
    print("图3.5 done")


# ═══════════════════════════════════════════════════════════════
#  图3.6: APS (角功率谱) 对比 — 全向 vs 贴片
# ═══════════════════════════════════════════════════════════════
def fig_3_6():
    runs = {
        "全向天线 (0dBi)": "E1_ideal",
        "偶极子 (2.15dBi)": "E1_dipole",
        "贴片天线 (6dBi)": "E1_patch",
        "喇叭天线 (15dBi)": "E1_horn",
    }

    fig, axes = plt.subplots(2, 2, figsize=(12, 11),
                             subplot_kw={"projection": "polar"})
    colors = {"全向天线 (0dBi)":"#888", "偶极子 (2.15dBi)":"#2d9cdb",
              "贴片天线 (6dBi)":"#e67e22", "喇叭天线 (15dBi)":"#27ae60"}

    for ax, (label, run) in zip(axes.flat, runs.items()):
        paths = load_paths(run)
        if not paths:
            ax.set_title(f"{label}\n(缺数据)")
            continue

        # Bin by AoA theta
        bins = np.arange(0, 181, 10)
        power_by_bin = np.zeros(len(bins))
        for p in paths:
            if p.get("power_linear", 0) <= 0:
                continue
            t = p.get("aoa_theta_deg", 90)
            if t > 180: t = 360 - t
            bi = int(np.clip(t // 10, 0, len(bins) - 1))
            power_by_bin[bi] += p["power_linear"]

        total = power_by_bin.sum()
        if total > 0:
            power_db = 10 * np.log10(power_by_bin / total + 1e-30)
        else:
            power_db = np.zeros_like(power_by_bin) - 40

        centers = bins + 5
        tv = np.concatenate([centers, centers[::-1]])
        pv = np.concatenate([power_db, power_db[::-1]])

        color = colors.get(label, "#333")
        ax.fill(np.radians(tv), np.maximum(pv, -25), alpha=0.15, color=color)
        ax.plot(np.radians(tv), np.maximum(pv, -25), color=color, lw=2)
        ax.set_title(f"{label}\n({len(paths)} 路径)", fontsize=10, pad=12)
        ax.set_theta_zero_location("N"); ax.set_theta_direction(-1)
        ax.set_ylim(-25, 0)
        ax.set_yticks([-25, -20, -15, -10, -5, 0])

    plt.suptitle("图3.6  角功率谱 (APS) — AoA Zenith角分布, 四种天线对比",
                 fontsize=14, fontweight="bold")
    plt.tight_layout(rect=[0, 0, 1, 0.95])
    fig.savefig(OUT_DIR / "fig3_6_aps.png", dpi=250, bbox_inches="tight")
    plt.close(fig)
    print("图3.6 done")


# ═══════════════════════════════════════════════════════════════
#  图3.7: XPR CDF — 四种天线对比
# ═══════════════════════════════════════════════════════════════
def fig_3_7():
    runs = {"E1_ideal":"全向(0dBi)", "E1_dipole":"偶极子(2.15dBi)",
            "E1_patch":"贴片(6dBi)", "E1_horn":"喇叭(15dBi)"}
    colors = {"E1_ideal":"#888", "E1_dipole":"#2d9cdb", "E1_patch":"#e67e22", "E1_horn":"#27ae60"}

    fig, ax = plt.subplots(figsize=(9, 6))

    for run, label in runs.items():
        paths = load_paths(run)
        if not paths:
            continue
        xprs = [p.get("xpr_dB", 0) for p in paths
                if abs(p.get("xpr_dB", 0)) < 200 and p.get("power_linear", 0) > 0]
        if not xprs:
            continue
        xprs_sorted = np.sort(xprs)
        cdf = np.arange(1, len(xprs_sorted) + 1) / len(xprs_sorted)
        ax.plot(xprs_sorted, cdf, color=colors[run], lw=2.2, label=label)

        # 统计标注
        median = np.median(xprs_sorted)
        p10 = np.percentile(xprs_sorted, 10)
        p90 = np.percentile(xprs_sorted, 90)
        ax.axvline(x=median, color=colors[run], ls=":", lw=1, alpha=0.6)
        ax.annotate(f"{label}\nmed={median:.1f}dB\nP10={p10:.1f}, P90={p90:.1f}",
                   xy=(median, 0.5), fontsize=8, color=colors[run],
                   ha="left" if median < 10 else "right")

    ax.set_xlabel("XPR (dB)", fontsize=12)
    ax.set_ylabel("CDF", fontsize=12)
    ax.set_title("图3.7  交叉极化比 (XPR) 累积分布 — 四种天线对比\n(场景412 Precise模式, 2544路径)",
                 fontsize=13, fontweight="bold")
    ax.legend(fontsize=10, loc="lower right")
    ax.grid(True, alpha=0.3)
    ax.set_xlim(-40, 40); ax.set_ylim(0, 1.02)

    plt.tight_layout()
    fig.savefig(OUT_DIR / "fig3_7_xpr_cdf.png", dpi=250, bbox_inches="tight")
    plt.close(fig)
    print("图3.7 done")


# ═══════════════════════════════════════════════════════════════
#  图3.8: 天线增益对接收功率的影响 (E1数据)
# ═══════════════════════════════════════════════════════════════
def fig_3_8():
    """替代方案: 天线类型 vs 总接收功率 (当SBR coverage数据不可用时)"""
    runs = {"E1_ideal":"全向(0dBi)", "E1_dipole":"偶极子(2.15dBi)",
            "E1_patch":"贴片(6dBi)", "E1_horn":"喇叭(15dBi)"}
    colors = {"E1_ideal":"#888", "E1_dipole":"#2d9cdb", "E1_patch":"#e67e22", "E1_horn":"#27ae60"}

    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 5.5))

    # Left: total received power
    data = {}
    for run, label in runs.items():
        paths = load_paths(run)
        if paths:
            data[run] = summary(paths)

    runs_ok = list(data.keys())
    labels = [runs[r] for r in runs_ok]
    clrs = [colors[r] for r in runs_ok]
    powers = [data[r]["tp_dbm"] for r in runs_ok]
    los = [data[r]["los_dbm"] for r in runs_ok]

    x = np.arange(len(runs_ok)); w = 0.35
    ax1.bar(x - w/2, powers, w, color=clrs, alpha=0.85, edgecolor="white", label="总功率")
    ax1.bar(x + w/2, los, w, color=clrs, alpha=0.35, edgecolor="white",
            hatch="//", label="LOS分量")
    for i, (tp_v, los_v) in enumerate(zip(powers, los)):
        ax1.text(i - w/2, tp_v + 0.5, f"{tp_v:.1f}", ha="center", fontsize=9)
        ax1.text(i + w/2, los_v + 0.5, f"{los_v:.1f}", ha="center", fontsize=8, color="gray")

    ax1.set_xticks(x); ax1.set_xticklabels(labels, fontsize=10)
    ax1.set_ylabel("接收功率 (dBm)", fontsize=11)
    ax1.set_title("总接收功率 vs 天线类型", fontsize=12, fontweight="bold")
    ax1.legend(fontsize=9)
    ax1.grid(axis="y", alpha=0.3)

    # Right: gain analysis — delta from Ideal
    ref_power = data.get("E1_ideal", {}).get("tp_dbm", -999)
    gain_expected = {"E1_ideal": 0, "E1_dipole": 2.15, "E1_patch": 6.0, "E1_horn": 15.0}
    deltas = []
    exp_gains = []
    for r in runs_ok:
        deltas.append(data[r]["tp_dbm"] - ref_power)
        exp_gains.append(gain_expected.get(r, 0))

    ax2.scatter(exp_gains, deltas, c=clrs, s=150, edgecolors="black", linewidths=1.5, zorder=5)
    for i, r in enumerate(runs_ok):
        ax2.annotate(runs[r], (exp_gains[i], deltas[i]),
                    textcoords="offset points", xytext=(8, 4), fontsize=9)
    ax2.plot([0, 16], [0, 16], "k--", lw=0.8, alpha=0.4, label="y=x (天线增益=信道增益)")
    ax2.axhline(y=0, color="gray", ls=":", lw=0.8)
    ax2.set_xlabel("天线标称增益 (dBi)", fontsize=11)
    ax2.set_ylabel("实测功率增量 (dB, 相对全向)", fontsize=11)
    ax2.set_title("天线增益 vs 实测信道增益增量", fontsize=12, fontweight="bold")
    ax2.legend(fontsize=9)
    ax2.grid(True, alpha=0.3)

    plt.suptitle("图3.8  天线类型对接收功率的影响\n(场景412, Precise模式, 2544路径, single Rx)",
                 fontsize=13, fontweight="bold")
    plt.tight_layout(rect=[0, 0, 1, 0.92])
    fig.savefig(OUT_DIR / "fig3_8_antenna_power.png", dpi=250, bbox_inches="tight")
    plt.close(fig)
    print("图3.8 done")


# ═══════════════════════════════════════════════════════════════
#  图3.9: 天线类型对多径统计的影响
# ═══════════════════════════════════════════════════════════════
def fig_3_9():
    """展示不同天线如何影响信道统计特性"""
    runs = {"E1_ideal":"全向", "E1_dipole":"偶极子", "E1_patch":"贴片", "E1_horn":"喇叭"}
    colors = {"E1_ideal":"#888", "E1_dipole":"#2d9cdb", "E1_patch":"#e67e22", "E1_horn":"#27ae60"}

    fig, axes = plt.subplots(2, 2, figsize=(14, 10))

    # Subplot 1: Power per path CDF
    ax = axes[0, 0]
    for run, label in runs.items():
        paths = load_paths(run)
        if not paths:
            continue
        powers = [p["power_linear"] for p in paths if p.get("power_linear", 0) > 0]
        if not powers:
            continue
        powers_db = sorted([10*np.log10(p*1000) for p in powers])
        cdf = np.arange(1, len(powers_db)+1) / len(powers_db)
        ax.plot(powers_db, cdf, color=colors[run], lw=2, label=label)
    ax.set_xlabel("单路径功率 (dBm)", fontsize=10)
    ax.set_ylabel("CDF", fontsize=10)
    ax.set_title("单路径接收功率 CDF", fontsize=11, fontweight="bold")
    ax.legend(fontsize=8)
    ax.grid(True, alpha=0.3)

    # Subplot 2: Delay spread by antenna type
    ax = axes[0, 1]
    for run, label in runs.items():
        paths = load_paths(run)
        if not paths:
            continue
        delays = [p.get("delay_s", 0)*1e9 for p in paths if p.get("power_linear", 0) > 0]
        if not delays:
            continue
        ax.hist(delays, bins=40, alpha=0.4, color=colors[run], label=label, density=True)
    ax.set_xlabel("时延 (ns)", fontsize=10)
    ax.set_ylabel("概率密度", fontsize=10)
    ax.set_title("路径时延分布", fontsize=11, fontweight="bold")
    ax.legend(fontsize=8)
    ax.grid(True, alpha=0.3)

    # Subplot 3: Co/cross-pol power scatter
    ax = axes[1, 0]
    ideal_paths = load_paths("E1_ideal")
    if ideal_paths:
        co = [p.get("co_pol_power_linear", 0) + 1e-30 for p in ideal_paths[:200] if p.get("power_linear", 0) > 0]
        cx = [p.get("cross_pol_power_linear", 0) + 1e-30 for p in ideal_paths[:200] if p.get("power_linear", 0) > 0]
        co_db = [10*np.log10(c) for c in co[:len(cx)]]
        cx_db = [10*np.log10(c) for c in cx[:len(co)]]
        sc = ax.scatter(co_db, cx_db, c=np.arange(len(co_db)), cmap="plasma",
                       alpha=0.6, s=20, edgecolors="none")
        ax.plot([-160, -30], [-160, -30], "k--", lw=0.8, alpha=0.4)
        plt.colorbar(sc, ax=ax, label="路径序号")
    ax.set_xlabel("Co-pol功率 (dB)", fontsize=10)
    ax.set_ylabel("Cross-pol功率 (dB)", fontsize=10)
    ax.set_title("Co/Cross-pol 功率散点图 (全向天线)", fontsize=11, fontweight="bold")
    ax.grid(True, alpha=0.3)

    # Subplot 4: XPR vs AoA
    ax = axes[1, 1]
    if ideal_paths:
        xpr_vals = []
        aoa_vals = []
        for p in ideal_paths:
            xpr = p.get("xpr_dB", 0)
            if abs(xpr) < 200 and p.get("power_linear", 0) > 0:
                xpr_vals.append(xpr)
                aoa_vals.append(p.get("aoa_theta_deg", 90))
        sc = ax.scatter(aoa_vals, xpr_vals, c=xpr_vals, cmap="RdBu_r",
                       alpha=0.5, s=15, edgecolors="none", vmin=-30, vmax=30)
        ax.axhline(y=0, color="gray", ls=":", lw=0.8)
        plt.colorbar(sc, ax=ax, label="XPR (dB)")
    ax.set_xlabel("AoA Zenith角 (deg)", fontsize=10)
    ax.set_ylabel("XPR (dB)", fontsize=10)
    ax.set_title("XPR vs AoA到达角 (全向天线)", fontsize=11, fontweight="bold")
    ax.grid(True, alpha=0.3)

    plt.suptitle("图3.9  天线类型对多径信道统计特性的影响\n(场景412, Precise模式, 2544路径)",
                 fontsize=13, fontweight="bold")
    plt.tight_layout(rect=[0, 0, 1, 0.93])
    fig.savefig(OUT_DIR / "fig3_9_channel_stats.png", dpi=250, bbox_inches="tight")
    plt.close(fig)
    print("图3.9 done")


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

    print("=" * 50)
    print("第三章最终图表生成")
    print("=" * 50)

    fig_3_1()
    fig_3_3()
    fig_3_4()
    fig_3_5()
    fig_3_6()
    fig_3_7()
    fig_3_8()
    fig_3_9()

    # List all generated files
    print("\n生成的文件:")
    for f in sorted(OUT_DIR.glob("fig3_*.png")):
        size_kb = f.stat().st_size / 1024
        print(f"  {f.name} ({size_kb:.0f} KB)")

    print("\n[Done] 全部图表生成完成")
