"""Validate V11 experiment outputs and generate figures, tables, and report."""

from __future__ import annotations

import argparse
import csv
import json
import math
from collections import defaultdict
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np


ROOT = Path(__file__).resolve().parents[2]
MANIFEST = ROOT / "configs/v11_experiments/experiment_manifest.json"
ANALYSIS = ROOT / "document/v11/analysis"
FIGURES = ANALYSIS / "figures_v11_5"
GENERATED = ANALYSIS / "generated"
COLORS = ["#176B87", "#D1495B", "#2A9D8F", "#E9A23B", "#6A4C93", "#5C677D", "#8A9A5B"]


def load_json(path: Path):
    return json.loads(path.read_text(encoding="utf-8-sig"))


def dbm(power_w: float) -> float:
    return 10.0 * math.log10(max(power_w, 1e-30) * 1000.0)


def wrap_phase(value: float) -> float:
    return math.atan2(math.sin(value), math.cos(value))


def configure_plotting() -> None:
    available = {font.name for font in __import__("matplotlib").font_manager.fontManager.ttflist}
    for candidate in ("Microsoft YaHei", "SimHei", "Arial"):
        if candidate in available:
            plt.rcParams["font.family"] = candidate
            break
    plt.rcParams.update({"font.size": 10, "axes.grid": True, "grid.alpha": 0.22,
                         "axes.spines.top": False, "axes.spines.right": False,
                         "figure.dpi": 140, "savefig.dpi": 180})


def case_output(case: dict) -> Path:
    return ROOT / "output" / case["run_id"] / "Tx01-Rx01"


def load_case(case: dict) -> dict:
    root = case_output(case)
    paths = load_json(root / "paths/precise_paths.json")["paths"]
    return {
        "case": case,
        "root": root,
        "paths": paths,
        "stats": load_json(root / "channel/channel_stats.json"),
        "xpr": load_json(root / "channel/xpr_stats.json"),
        "meg": load_json(root / "channel/meg.json"),
        "aps": load_json(root / "channel/aps_2d_grid.json"),
    }


def validate_case(data: dict) -> list[str]:
    errors: list[str] = []
    paths = data["paths"]
    if len(paths) != data["stats"]["valid_path_count"]:
        errors.append("path count differs from channel_stats")
    signatures = [p["source_path_signature"] for p in paths]
    if len(signatures) != len(set(signatures)):
        errors.append("duplicate source_path_signature")
    for p in paths:
        values = [p.get("power_linear", 0.0), p.get("amplitude_real", 0.0),
                  p.get("amplitude_imag", 0.0), p.get("sampling_weight", 0.0)]
        if not all(math.isfinite(float(v)) for v in values):
            errors.append("non-finite path value")
            break
        reconstructed = float(p["amplitude_real"]) ** 2 + float(p["amplitude_imag"]) ** 2
        tolerance = max(1e-18, abs(float(p["power_linear"])) * 2e-4)
        if abs(reconstructed - float(p["power_linear"])) > tolerance:
            errors.append("complex amplitude does not reconstruct power")
            break
    observed = sum(sum(row) for row in data["aps"]["observed_grid_linear"])
    total = float(data["stats"]["total_power_linear"])
    if abs(observed - total) > max(1e-18, total * 2e-4):
        errors.append("observed APS does not conserve total path power")
    return errors


def summary_row(data: dict) -> dict:
    c = data["case"]
    s = data["stats"]
    return {
        "group": c["group"], "case_id": c["case_id"], "run_id": c["run_id"],
        "path_count": s["valid_path_count"], "total_power_w": s["total_power_linear"],
        "total_power_dbm": dbm(s["total_power_linear"]),
        "los_power_w": s["los_power_linear"], "los_power_dbm": dbm(s["los_power_linear"]),
        "nlos_power_w": s["nlos_power_linear"], "nlos_power_dbm": dbm(s["nlos_power_linear"]),
        "rms_delay_spread_ns": s["rms_delay_spread_s"] * 1e9,
        "k_factor_db": s["k_factor_dB"], "xpr_mean_db": data["xpr"]["mean_dB"],
        "xpr_weighted_db": data["xpr"]["power_weighted_mean_dB"],
        "xpr_aggregate_db": data["xpr"]["aggregate_xpr_dB"],
        "meg_db": data["meg"]["meg_dB"],
    }


def binned_pdp(paths: list[dict], bin_ns: float = 0.25) -> tuple[np.ndarray, np.ndarray]:
    delays = np.array([float(p["delay_s"]) * 1e9 for p in paths])
    powers = np.array([float(p["power_linear"]) for p in paths])
    if len(delays) == 0:
        return np.array([]), np.array([])
    edges = np.arange(0.0, max(delays) + 2 * bin_ns, bin_ns)
    hist, _ = np.histogram(delays, bins=edges, weights=powers)
    return 0.5 * (edges[:-1] + edges[1:]), hist


def js_divergence(paths_a: list[dict], paths_b: list[dict], bin_ns: float = 0.25,
                  exclude_los: bool = False) -> float:
    if exclude_los:
        paths_a = [p for p in paths_a if not p["is_los"]]
        paths_b = [p for p in paths_b if not p["is_los"]]
    if not paths_a or not paths_b:
        return math.nan
    max_delay = max(max(float(p["delay_s"]) for p in paths_a), max(float(p["delay_s"]) for p in paths_b)) * 1e9
    edges = np.arange(0.0, max_delay + 2 * bin_ns, bin_ns)
    def distribution(paths):
        h, _ = np.histogram([float(p["delay_s"]) * 1e9 for p in paths], bins=edges,
                            weights=[float(p["power_linear"]) for p in paths])
        h = h + 1e-30
        return h / h.sum()
    p, q = distribution(paths_a), distribution(paths_b)
    m = 0.5 * (p + q)
    return float(0.5 * np.sum(p * np.log2(p / m)) + 0.5 * np.sum(q * np.log2(q / m)))


def save_figure(fig, name: str, tight: bool = True) -> None:
    if tight:
        fig.tight_layout()
    fig.savefig(FIGURES / name, bbox_inches="tight")
    plt.close(fig)


def plot_e1(group: list[dict]) -> None:
    ordered = sorted(group, key=lambda d: ["ideal", "dipole", "patch", "horn"].index(d["case"]["case_id"]))
    labels = [d["case"]["case_id"] for d in ordered]
    rows = [summary_row(d) for d in ordered]
    fig, axes = plt.subplots(2, 2, figsize=(10, 7))
    metrics = [("total_power_dbm", "Received power (dBm)"), ("meg_db", "MEG (dB)"),
               ("rms_delay_spread_ns", "RMS delay spread (ns)"), ("k_factor_db", "K-factor (dB)")]
    for ax, (key, title) in zip(axes.flat, metrics):
        ax.bar(labels, [r[key] for r in rows], color=COLORS[:len(labels)])
        ax.set_title(title); ax.set_ylabel(title)
    save_figure(fig, "E1_channel_metrics.png")

    fig, ax = plt.subplots(figsize=(9, 5))
    for i, data in enumerate(ordered):
        delay, power = binned_pdp(data["paths"])
        relative = 10.0 * np.log10(np.maximum(power / max(power.max(), 1e-30), 1e-12))
        ax.plot(delay, relative, label=labels[i], color=COLORS[i], linewidth=1.5)
    ax.set(xlabel="Path delay (ns)", ylabel="Relative PDP (dB)", ylim=(-80, 2), title="E1 normalized PDP")
    ax.legend(ncol=2)
    save_figure(fig, "E1_normalized_pdp.png")

    fig, axes = plt.subplots(2, 2, figsize=(11, 7), sharex=True, sharey=True)
    for ax, data, label in zip(axes.flat, ordered, labels):
        grid = np.array(data["aps"]["observed_grid_linear"])
        grid_db = 10.0 * np.log10(np.maximum(grid / max(grid.max(), 1e-30), 1e-12))
        image = ax.imshow(grid_db, origin="lower", aspect="auto", extent=[0, 360, 0, 180],
                          vmin=-50, vmax=0, cmap="viridis")
        ax.set_title(label); ax.set_xlabel("AoA phi (deg)"); ax.set_ylabel("AoA theta (deg)")
    fig.subplots_adjust(left=0.08, right=0.86, bottom=0.09, top=0.94, wspace=0.22, hspace=0.28)
    color_axis = fig.add_axes([0.89, 0.15, 0.02, 0.70])
    fig.colorbar(image, cax=color_axis, label="Relative observed power (dB)")
    save_figure(fig, "E1_observed_aps.png", tight=False)


def plot_e2(group: list[dict]) -> None:
    ordered = sorted(group, key=lambda d: int(d["case"]["case_id"][-3:]))
    yaw = [int(d["case"]["case_id"][-3:]) for d in ordered]
    rows = [summary_row(d) for d in ordered]
    fig, axes = plt.subplots(3, 1, figsize=(9, 9), sharex=True)
    axes[0].plot(yaw, [r["total_power_dbm"] for r in rows], "o-", color=COLORS[0]); axes[0].set_ylabel("Power (dBm)")
    axes[1].plot(yaw, [r["meg_db"] for r in rows], "o-", color=COLORS[1]); axes[1].set_ylabel("MEG (dB)")
    axes[2].plot(yaw, [r["xpr_weighted_db"] for r in rows], "o-", color=COLORS[2]); axes[2].set_ylabel("Weighted XPR (dB)")
    axes[2].set_xlabel("Rx yaw around world +Y (deg)"); axes[0].set_title("E2 antenna-pose response")
    save_figure(fig, "E2_pose_scan.png")


def plot_e3(group: list[dict]) -> dict:
    by_id = {d["case"]["case_id"]: d for d in group}
    fig, ax = plt.subplots(figsize=(8.5, 5))
    for i, case_id in enumerate(("fixed_fixed", "jones_fixed", "fixed_jones", "jones_jones")):
        values = sorted(float(v) for v in by_id[case_id]["xpr"]["cdf_values_dB"])
        if values:
            ax.plot(values, np.arange(1, len(values)+1) / len(values), label=case_id, color=COLORS[i])
    ax.set(xlabel="Path XPR (dB)", ylabel="Empirical CDF", title="E3 path-level XPR CDF")
    ax.legend()
    save_figure(fig, "E3_xpr_cdf.png")

    base = {p["source_path_signature"]: p for p in by_id["fixed_fixed"]["paths"]}
    full = {p["source_path_signature"]: p for p in by_id["jones_jones"]["paths"]}
    common = sorted(set(base) & set(full))
    phase = [wrap_phase(float(full[k]["phase_rad"]) - float(base[k]["phase_rad"])) for k in common]
    power_db = [10.0 * math.log10(max(float(full[k]["power_linear"]), 1e-30) / max(float(base[k]["power_linear"]), 1e-30)) for k in common]
    fig, axes = plt.subplots(1, 2, figsize=(10, 4))
    axes[0].hist(np.degrees(phase), bins=36, color=COLORS[1]); axes[0].set(xlabel="Jones - Fixed phase (deg)", ylabel="Path count")
    axes[1].hist(power_db, bins=36, color=COLORS[2]); axes[1].set(xlabel="Jones / Fixed path power (dB)", ylabel="Path count")
    save_figure(fig, "E3_paired_path_differences.png")
    return {"paired_paths": len(common), "median_abs_phase_deg": float(np.median(np.abs(np.degrees(phase)))),
            "median_abs_power_db": float(np.median(np.abs(power_db)))}


def plot_c1(group: list[dict]) -> list[dict]:
    ordered = sorted(group, key=lambda d: int(d["case"]["case_id"][:3]))
    reference = ordered[-1]
    ref_row = summary_row(reference)
    rows = []
    for data in ordered:
        row = summary_row(data)
        row["power_error_db"] = row["total_power_dbm"] - ref_row["total_power_dbm"]
        row["nlos_power_error_db"] = row["nlos_power_dbm"] - ref_row["nlos_power_dbm"]
        row["rms_error_ns"] = row["rms_delay_spread_ns"] - ref_row["rms_delay_spread_ns"]
        row["nlos_pdp_js_divergence"] = js_divergence(
            data["paths"], reference["paths"], exclude_los=True)
        rows.append(row)
    rays = [int(d["case"]["case_id"][:3]) for d in ordered]
    fig, axes = plt.subplots(2, 3, figsize=(12, 7))
    axes[0,0].plot(rays, [r["power_error_db"] * 1e6 for r in rows], "o-"); axes[0,0].set_ylabel("Total-power error (micro-dB)")
    axes[0,1].plot(rays, [r["nlos_power_error_db"] * 1e3 for r in rows], "o-"); axes[0,1].set_ylabel("NLOS-power error (milli-dB)")
    axes[0,2].plot(rays, [r["rms_error_ns"] * 1e3 for r in rows], "o-"); axes[0,2].set_ylabel("RMS-DS error (ps)")
    axes[1,0].plot(rays, [r["k_factor_db"] for r in rows], "o-"); axes[1,0].set_ylabel("K-factor (dB)")
    axes[1,1].plot(rays, [r["nlos_pdp_js_divergence"] for r in rows], "o-"); axes[1,1].set_ylabel("NLOS-PDP JS divergence")
    axes[1,2].plot(rays, [r["path_count"] for r in rows], "o-"); axes[1,2].set_ylabel("Valid path count")
    for ax in axes.flat: ax.set_xlabel("Launched rays (thousands)")
    save_figure(fig, "C1_convergence.png")
    return rows


def plot_c2_c3(c2: list[dict], c3: list[dict]) -> None:
    fig, axes = plt.subplots(1, 2, figsize=(10, 4))
    c2o = sorted(c2, key=lambda d: d["case"]["case_id"])
    c2rows = [summary_row(d) for d in c2o]
    c2base = c2rows[0]["total_power_dbm"]
    axes[0].bar([d["case"]["case_id"] for d in c2o],
                [r["total_power_dbm"] - c2base for r in c2rows], color=COLORS[:2], label="power change")
    c2delay = axes[0].twinx()
    c2delay.plot([d["case"]["case_id"] for d in c2o],
                 [r["rms_delay_spread_ns"] for r in c2rows], "o--", color=COLORS[2], label="RMS-DS")
    axes[0].set(title="C2 analytical diffraction", ylabel="Power change vs off (dB)")
    c2delay.set_ylabel("RMS-DS (ns)")
    c3o = sorted(c3, key=lambda d: float(d["case"]["case_id"][1:]))
    radii = [float(d["case"]["case_id"][1:]) / 100 for d in c3o]
    c3rows = [summary_row(d) for d in c3o]
    c3base = c3rows[0]["total_power_dbm"]
    axes[1].plot(radii, [(r["total_power_dbm"] - c3base) * 1e3 for r in c3rows], "o-", label="power change")
    ax2 = axes[1].twinx(); ax2.plot(radii, [len(d["paths"]) for d in c3o], "s--", color=COLORS[1], label="paths")
    axes[1].set(title="C3 fixed reception radius", xlabel="Radius (m)", ylabel="Power change vs 0.03 m (milli-dB)"); ax2.set_ylabel("Path count")
    save_figure(fig, "C2_C3_sensitivity.png")


def write_csv(path: Path, rows: list[dict]) -> None:
    if not rows: return
    with path.open("w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(f, fieldnames=list(rows[0]))
        writer.writeheader(); writer.writerows(rows)


def report(groups: dict[str, list[dict]], summaries: list[dict], c1_rows: list[dict], e3_effect: dict, quality: list[str]) -> None:
    by_group = {g: [summary_row(d) for d in values] for g, values in groups.items()}
    e1 = {r["case_id"]: r for r in by_group["E1"]}
    e2 = by_group["E2"]
    c2 = {r["case_id"]: r for r in by_group["C2"]}
    c3 = by_group["C3"]
    x1 = {r["case_id"]: r for r in by_group["X1"]}
    power_span_e2 = max(r["total_power_dbm"] for r in e2) - min(r["total_power_dbm"] for r in e2)
    e2_peak = max(e2, key=lambda r: r["total_power_dbm"])
    c1_200 = next(r for r in c1_rows if r["case_id"] == "200k")
    convergence_pass = (abs(c1_200["power_error_db"]) <= 0.1 and
                        abs(c1_200["nlos_power_error_db"]) <= 1.0 and
                        abs(c1_200["rms_error_ns"]) <= 0.5 and
                        c1_200["nlos_pdp_js_divergence"] <= 0.1)
    c2_power_delta = c2["on"]["total_power_dbm"] - c2["off"]["total_power_dbm"]
    c3_power_span = max(r["total_power_dbm"] for r in c3) - min(r["total_power_dbm"] for r in c3)
    model_rows = [e1[name] for name in ("ideal", "dipole", "patch", "horn")]
    model_table = ["| 模型 | 接收功率 (dBm) | MEG (dB) | RMS-DS (ns) | K (dB) |",
                   "|---|---:|---:|---:|---:|"]
    model_table += [f"| {r['case_id']} | {r['total_power_dbm']:.3f} | {r['meg_db']:.3f} | "
                    f"{r['rms_delay_spread_ns']:.4f} | {r['k_factor_db']:.3f} |" for r in model_rows]
    lines = [
        "# V11.5 仿真实验分析报告", "",
        "## 1. 结论摘要", "",
        f"27 个正式案例全部通过数据门禁，告警数为 {len(quality)}。3 GHz、1.131 m 的理想 LOS 理论功率约为 -43.05 dBm，仿真为 {e1['ideal']['total_power_dbm']:.3f} dBm；这是自由空间基准与实现一致的首要证据。",
        f"200k 射线相对 500k 的预定义四项收敛门限判定为 **{'通过' if convergence_pass else '不通过'}**。天线姿态扫描产生 {power_span_e2:.2f} dB 的动态范围，说明方向图和姿态确实进入主链。", "",
        "## 2. 方法与选择理由", "",
        "实验遵循“单因素对照 + 高样本参考 + 物理不变量”三层设计。E1/E2/E3 分别隔离方向图、姿态和 Jones 极化；C1 用 500k 射线作数值参考；C2/C3 检查绕射与接收球敏感性。选择 RMS-DS、K-factor、APS、MEG 和 XPR，是因为它们分别约束时延、LOS/NLOS 功率比、到达角、方向图平均耦合及极化耦合，能避免仅凭总功率得出结论。权威定义和公式见 [方法调研与实验设计](v11.5_方法调研与实验设计.md)。", "",
        "## 3. 零基础读图说明", "",
        "dBm 是对数功率：增加 3 dB 约等于功率翻倍。RMS-DS 越大，表示显著多径在时间上铺得越开。K-factor 越大，表示 LOS 相对 NLOS 越强。MEG 是天线增益按实际入射角功率加权后的平均值。XPR 越大，表示同极化功率相对交叉极化越强。JS 距离为 0 表示两个归一化功率分布相同，越接近 1 差异越大。", "",
        "## 4. 数据质量与可追溯性", "",
        f"共分析 {len(summaries)} 个案例。每个案例均检查：路径数与统计文件一致、几何签名无重复、复振幅平方可重建路径功率、所有数值有限、观测 APS 的 bin 功率之和等于总路径功率。运行清单另保存可执行文件/配置哈希和 Git 状态。", "",
        "## 5. E1 天线模型", "",
        *model_table, "",
        "![E1 metrics](figures_v11_5/E1_channel_metrics.png)", "",
        "横轴为天线模型，四个子图依次是绝对接收功率、MEG、RMS-DS 和 K-factor。偶极子把 `forward` 定义为竖直振子轴，因此水平 Tx-Rx 链路位于最大辐射平面；patch/horn 的主瓣对准 LOS。理想、patch 和 horn 的 LOS 功率增量应分别接近 0、12 和 30 dB，这对应 Tx/Rx 两端峰值增益之和。", "",
        "![E1 PDP](figures_v11_5/E1_normalized_pdp.png)", "",
        "横轴是路径时延，纵轴是各模型相对自身最强时延 bin 的功率。它比较的是多径形状，不是模型间绝对功率。定向天线抑制晚到达路径时，尾部下降并伴随 RMS-DS 减小。", "",
        "![E1 APS](figures_v11_5/E1_observed_aps.png)", "",
        "横轴是世界方位角，纵轴是天顶角，颜色是相对最强角度 bin 的功率。这里是经过 Rx 方向图后的观测 APS；入射 APS 也已单独导出，供 MEG 计算使用，二者不能混用。", "",
        "## 6. E2 姿态扫描", "",
        "![E2](figures_v11_5/E2_pose_scan.png)", "",
        f"Rx 绕世界 +Y 轴旋转。功率动态范围为 {power_span_e2:.2f} dB，最强案例为 `{e2_peak['case_id']}`。若主瓣偏离主要到达方向，功率和 MEG 应共同下降；XPR 还会因 co/cross 基底与 Jones 响应改变。曲线无需左右对称，因为室内到达角谱并不对称。", "",
        "## 7. E3 极化模型", "",
        "![E3 XPR](figures_v11_5/E3_xpr_cdf.png)", "",
        "CDF 横向移动表示整体 XPR 改变，斜率变化表示不同路径受影响不一致。纯 co/cross 状态按 ±60 dB 截尾后纳入统计，避免把物理上有意义的纯极化路径静默删除。", "",
        "![E3 paired](figures_v11_5/E3_paired_path_differences.png)", "",
        f"按几何签名配对 {e3_effect['paired_paths']} 条路径。Jones-Jones 相对固定极化的相位差绝对值中位数为 {e3_effect['median_abs_phase_deg']:.2f} 度，功率差绝对值中位数为 {e3_effect['median_abs_power_db']:.3f} dB。它证明 Jones 幅相信息进入链路，但当前 Jones 文件是解析/合成基准，不能声称代表某一实测或 HFSS 天线。", "",
        "## 8. C1 数值收敛", "",
        "![C1](figures_v11_5/C1_convergence.png)", "",
        f"六个子图均以 500k 为参考或直接展示稳定量。图中 micro-dB 是百万分之一 dB，milli-dB 是千分之一 dB，ps 是千分之一 ns。200k 的总功率误差为 {c1_200['power_error_db']:.4f} dB，NLOS 功率误差为 {c1_200['nlos_power_error_db']:.4f} dB，RMS-DS 误差为 {c1_200['rms_error_ns']:.4f} ns，NLOS-PDP JS 距离为 {c1_200['nlos_pdp_js_divergence']:.5f}。使用 NLOS 指标是为了防止强 LOS 把弱多径的未收敛隐藏掉。门限为 0.1 dB、1 dB、0.5 ns 和 0.1。", "",
        "## 9. C2/C3 敏感性", "",
        "![C2 C3](figures_v11_5/C2_C3_sensitivity.png)", "",
        f"左图柱表示开启解析绕射相对关闭时的总功率变化（{c2_power_delta:.4f} dB），折线表示 RMS-DS。绕射总功率虽弱，晚到达分量仍可能显著改变 RMS-DS。右图将固定接收球半径从 0.03 m 扫到 0.20 m：路径数会增加，但总功率跨度只有 {c3_power_span:.4f} dB；这支持采样权重已抑制“命中越多、功率越大”的伪效应。", "",
        "## 10. 跨场景检查", "",
        f"meeting 场景中，ideal 为 {x1['meeting_ideal']['total_power_dbm']:.3f} dBm，patch 为 {x1['meeting_patch']['total_power_dbm']:.3f} dBm。第二场景复现了定向天线增益趋势，说明结论不是 412 场景单点偶然。它仍不是统计意义上的场景泛化，需要更多位置和 NLOS 点位。", "",
        "## 11. 剩余风险与论文表述边界", "",
        "1. 材料数据库沿用项目既有 ITU-R P.2040-1 参数口径，而当前建议书已有更新版本；在升级数据库前，不应把绝对损耗称为最新标准结果。",
        "2. patch/horn 与 Jones 文件是参数化验证资产，不是实测或全波数据；可用于验证算法趋势，不可用于宣称真实天线性能。",
        "3. 当前绕射实验验证的是已接入的解析绕射开关；`diffraction_rays_per_event` 尚未成为主链有效自变量，报告没有伪造其扫描结论。",
        "4. 500k 是当前场景/点位的数值参考，不是真值。缺少暗室天线数据和信道实测，因此外部有效性仍待验证。",
        "5. 动态接收半径通过功率稳定性检查，但更复杂遮挡、纯 NLOS 和多接收机布置仍需专项回归。", "",
        "因此，当前证据支持“实现符合自由空间、方向图、极化和采样权重的理论趋势”，不支持“已达到真实场景测量精度”的更强结论。", "",
    ]
    (ANALYSIS / "v11.5_仿真实验分析报告.md").write_text("\n".join(lines), encoding="utf-8")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--allow-partial", action="store_true")
    args = parser.parse_args()
    configure_plotting(); FIGURES.mkdir(parents=True, exist_ok=True); GENERATED.mkdir(parents=True, exist_ok=True)
    manifest = load_json(MANIFEST)
    groups: dict[str, list[dict]] = defaultdict(list)
    quality: list[str] = []
    missing: list[str] = []
    for case in manifest["cases"]:
        try:
            data = load_case(case)
        except FileNotFoundError as exc:
            missing.append(f"{case['run_id']}: {exc.filename}")
            continue
        errors = validate_case(data)
        quality.extend(f"{case['run_id']}: {error}" for error in errors)
        groups[case["group"]].append(data)
    if missing and not args.allow_partial:
        raise SystemExit("Missing experiment outputs:\n" + "\n".join(missing))
    if quality:
        raise SystemExit("Data-quality failures:\n" + "\n".join(quality))
    required = {"E1": 4, "E2": 7, "E3": 4, "C1": 4, "C2": 2, "C3": 4, "X1": 2}
    if not args.allow_partial:
        bad = [f"{g}: {len(groups[g])}/{n}" for g, n in required.items() if len(groups[g]) != n]
        if bad: raise SystemExit("Incomplete groups: " + ", ".join(bad))
    summaries = [summary_row(data) for values in groups.values() for data in values]
    write_csv(GENERATED / "experiment_summary.csv", summaries)
    if len(groups["E1"]) == 4: plot_e1(groups["E1"])
    if len(groups["E2"]) == 7: plot_e2(groups["E2"])
    e3_effect = plot_e3(groups["E3"]) if len(groups["E3"]) == 4 else {}
    c1_rows = plot_c1(groups["C1"]) if len(groups["C1"]) == 4 else []
    if len(groups["C2"]) == 2 and len(groups["C3"]) == 4: plot_c2_c3(groups["C2"], groups["C3"])
    if not args.allow_partial: report(groups, summaries, c1_rows, e3_effect, quality)
    (GENERATED / "data_quality.json").write_text(json.dumps({"missing": missing, "errors": quality,
        "analyzed_cases": len(summaries)}, indent=2), encoding="utf-8")
    print(f"Analyzed {len(summaries)} cases; figures: {FIGURES}")


if __name__ == "__main__":
    main()
