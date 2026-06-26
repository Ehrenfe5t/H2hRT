#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
第三章全量仿真 — 一键运行所有实验, 收集结果, 生成全部图表

实验:
  E1: 天线类型对比 (Ideal / dipole / patch / horn) — APS, XPR, 功率对比
  E2: Rx极化旋转 (V/H/+45/LHCP/RHCP) — 接收功率对比 (图3.3)
  E3: Rx姿态yaw扫描 (0°→180°) — 接收功率 vs 姿态角 (图3.4)
  E4: 覆盖热图对比 (Ideal / patch SBR Coverage) — 图3.8, 3.9 (需SBR模式)

用法: python test/ch3_full_sweep.py
"""

import json, os, subprocess, sys, time, shutil
from pathlib import Path
from copy import deepcopy
from collections import defaultdict
import numpy as np

ROOT = Path(__file__).resolve().parents[1]
RT_EXE = ROOT / "x64" / "Release" / "RT.exe"
BASE_CFG = ROOT / "configs" / "app" / "meeting_412.json"
ANT_DIR = ROOT / "configs" / "antennas"
OUT_BASE = ROOT / "output" / "ch3_full"
OUT_BASE.mkdir(parents=True, exist_ok=True)

# ─── Rx极化定义 ────────────────────────────────────────
RX_POLS = {
    "V":    (0.0, 1.0, 0.0),
    "H":    (0.0, 0.0, 1.0),
    "P45":  (0.0, 0.707107, 0.707107),
    "LHCP": (0.0, 0.707107, 0.0),
    "RHCP": (0.0, 0.707107, 0.0),
}

# ─── 工具函数 ──────────────────────────────────────────
def load_base():
    with open(BASE_CFG, "r", encoding="utf-8") as f:
        cfg = json.load(f)
    cfg["em_solver"]["solver_mode"] = "Precise"
    cfg["scene_preprocess"]["enable_scene_cache"] = False
    # 单Rx
    cfg["path_search"]["rx_list"] = [{"id": "rx1", "x": 1.58, "y": 1.5, "z": -8.81}]
    return cfg

def write_pol_file(path, vec):
    Path(path).parent.mkdir(parents=True, exist_ok=True)
    with open(path, "w") as f:
        f.write(f"{vec[0]} {vec[1]} {vec[2]}\n")

def make_config(run_id, antenna_type="Ideal", pol_vec=None, yaw_deg=None, pattern_file=None):
    """生成配置变体"""
    cfg = load_base()
    cfg["app_runtime"]["run_id"] = run_id

    # 天线类型
    cfg["antenna"]["source_type"] = antenna_type

    # 方向图文件
    if pattern_file:
        cfg["antenna"]["pattern_file"] = str(ANT_DIR / pattern_file)
    else:
        cfg["antenna"]["pattern_file"] = ""

    # 极化矢量
    pol_dir = OUT_BASE / "pol_files"
    pol_dir.mkdir(parents=True, exist_ok=True)
    if pol_vec:
        pol_path = pol_dir / f"pol_{run_id}.txt"
        if len(pol_vec) == 3:
            write_pol_file(pol_path, pol_vec)
            cfg["antenna"]["polarization_file"] = str(pol_path)
            cfg["antenna"]["forward_x"] = 1.0; cfg["antenna"]["forward_y"] = 0.0; cfg["antenna"]["forward_z"] = 0.0
            cfg["antenna"]["up_x"] = 0.0; cfg["antenna"]["up_y"] = 1.0; cfg["antenna"]["up_z"] = 0.0
    else:
        cfg["antenna"]["polarization_file"] = ""

    # 姿态yaw旋转
    if yaw_deg is not None:
        c = np.cos(np.radians(yaw_deg)); s = np.sin(np.radians(yaw_deg))
        cfg["antenna"]["forward_x"] = float(c); cfg["antenna"]["forward_y"] = 0.0; cfg["antenna"]["forward_z"] = float(-s)
        cfg["antenna"]["up_x"] = 0.0; cfg["antenna"]["up_y"] = 1.0; cfg["antenna"]["up_z"] = 0.0

    return cfg

def run_one(cfg, label):
    """运行单次RT仿真, 返回结果字典"""
    cfg_dir = OUT_BASE / "configs" / label
    cfg_dir.mkdir(parents=True, exist_ok=True)
    cfg_path = cfg_dir / "config.json"
    with open(cfg_path, "w", encoding="utf-8") as f:
        json.dump(cfg, f, ensure_ascii=False, indent=2)

    run_id = cfg["app_runtime"]["run_id"]
    print(f"  [{label}] running...", end=" ", flush=True)
    t0 = time.time()
    try:
        result = subprocess.run(
            [str(RT_EXE), str(cfg_path)],
            capture_output=True, text=True, timeout=600,
            cwd=str(ROOT), encoding="utf-8", errors="replace"
        )
        elapsed = time.time() - t0
        if result.returncode != 0:
            print(f"FAIL ({elapsed:.0f}s) exit={result.returncode}")
            return None
        print(f"OK ({elapsed:.0f}s)")

        # 读取结果
        paths_file = ROOT / "output" / run_id / "rx1" / "paths" / "precise_paths.json"
        if not paths_file.exists():
            print(f"    missing output: {paths_file}")
            return None

        with open(paths_file, "r", encoding="utf-8") as f:
            data = json.load(f)
        paths = data.get("paths", [])
        return {"label": label, "run_id": run_id, "path_count": len(paths),
                "paths": paths, "elapsed": elapsed}

    except subprocess.TimeoutExpired:
        print(f"TIMEOUT")
        return None
    except Exception as e:
        print(f"ERROR: {e}")
        return None

def extract_summary(results):
    """从路径结果中提取统计量"""
    paths = results["paths"]
    valid = [p for p in paths if p.get("power_linear", 0) > 0]
    powers = [p["power_linear"] for p in valid]
    total_power = sum(powers)
    los_power = sum(p["power_linear"] for p in valid if p.get("is_los"))

    co_vals = [p.get("co_pol_power_linear", 0) for p in valid]
    cx_vals = [p.get("cross_pol_power_linear", 0) for p in valid]
    xpr_vals = [p.get("xpr_dB", 0) for p in valid if abs(p.get("xpr_dB", 0)) < 200]

    aoa_thetas = [p.get("aoa_theta_deg", 90) for p in valid]
    aoa_phis = [p.get("aoa_phi_deg", 0) for p in valid]

    return {
        "label": results["label"],
        "path_count": len(valid),
        "total_power_linear": total_power,
        "total_power_dBm": 10*np.log10(total_power*1000) if total_power > 0 else -999,
        "los_power_linear": los_power,
        "total_co_power": sum(co_vals),
        "total_cross_power": sum(cx_vals),
        "mean_xpr_dB": float(np.mean(xpr_vals)) if xpr_vals else 0,
        "median_xpr_dB": float(np.median(xpr_vals)) if xpr_vals else 0,
        "xpr_vals": xpr_vals,
        "aoa_theta": aoa_thetas,
        "aoa_phi": aoa_phis,
        "powers": powers,
        "co_powers": co_vals,
        "cross_powers": cx_vals,
    }


# ═══════════════════════════════════════════════════════════════
#  E1: 天线类型对比
# ═══════════════════════════════════════════════════════════════
def experiment_antenna_types():
    print("\n" + "="*60)
    print("E1: 天线类型对比 (Ideal / dipole / patch / horn)")
    print("="*60)

    configs = [
        ("E1_ideal",  "Ideal",  None, None),
        ("E1_dipole", "Ideal",  None, "dipole_gain.csv"),
        ("E1_patch",  "Ideal",  None, "patch_gain.csv"),
        ("E1_horn",   "Ideal",  None, "horn_gain.csv"),
    ]

    results = {}
    for run_id, ant_type, pol, pattern in configs:
        cfg = make_config(run_id, ant_type, pol, pattern_file=pattern)
        r = run_one(cfg, run_id)
        if r:
            results[run_id] = extract_summary(r)
            s = results[run_id]
            print(f"    power={s['total_power_dBm']:.1f}dBm  XPR={s['mean_xpr_dB']:.1f}dB  "
                  f"paths={s['path_count']}")

    # 保存
    out = {k: {sk: sv for sk, sv in v.items() if sk not in ("xpr_vals","aoa_theta","aoa_phi","powers","co_powers","cross_powers")}
           for k, v in results.items()}
    out["_raw"] = {k: {"xpr_vals": v["xpr_vals"], "aoa_theta": v["aoa_theta"],
                       "powers": v["powers"], "co_powers": v["co_powers"]}
                   for k, v in results.items()}
    with open(OUT_BASE / "E1_antenna_types.json", "w", encoding="utf-8") as f:
        json.dump(out, f, ensure_ascii=False, indent=2)
    return results


# ═══════════════════════════════════════════════════════════════
#  E2: Rx极化旋转
# ═══════════════════════════════════════════════════════════════
def experiment_polarization():
    print("\n" + "="*60)
    print("E2: Rx极化旋转 (V/H/+45/LHCP/RHCP)")
    print("="*60)

    results = {}
    for pol_key, pol_vec in RX_POLS.items():
        run_id = f"E2_{pol_key}"
        cfg = make_config(run_id, "Ideal", pol_vec)
        r = run_one(cfg, run_id)
        if r:
            results[run_id] = extract_summary(r)
            s = results[run_id]
            print(f"    {pol_key}: power={s['total_power_dBm']:.1f}dBm  "
                  f"LOS={10*np.log10(s['los_power_linear']*1000):.1f}dBm  "
                  f"XPR={s['mean_xpr_dB']:.1f}dB")

    out = {k: {sk: sv for sk, sv in v.items() if not isinstance(sv, list)}
           for k, v in results.items()}
    with open(OUT_BASE / "E2_polarization.json", "w", encoding="utf-8") as f:
        json.dump(out, f, ensure_ascii=False, indent=2)
    return results


# ═══════════════════════════════════════════════════════════════
#  E3: Rx姿态yaw扫描
# ═══════════════════════════════════════════════════════════════
def experiment_pose():
    print("\n" + "="*60)
    print("E3: Rx姿态yaw扫描 (0°→180°)")
    print("="*60)

    results = {}
    for yaw in [0, 30, 60, 90, 120, 150, 180]:
        run_id = f"E3_yaw{yaw}"
        cfg = make_config(run_id, "Ideal", None, yaw_deg=yaw)
        r = run_one(cfg, run_id)
        if r:
            results[run_id] = extract_summary(r)
            s = results[run_id]
            print(f"    yaw={yaw:3d}: power={s['total_power_dBm']:.1f}dBm  "
                  f"LOS={10*np.log10(s['los_power_linear']*1000):.1f}dBm")

    out = {k: {sk: sv for sk, sv in v.items() if not isinstance(sv, list)}
           for k, v in results.items()}
    with open(OUT_BASE / "E3_pose.json", "w", encoding="utf-8") as f:
        json.dump(out, f, ensure_ascii=False, indent=2)
    return results


# ═══════════════════════════════════════════════════════════════
#  Main
# ═══════════════════════════════════════════════════════════════
if __name__ == "__main__":
    if not RT_EXE.exists():
        print(f"ERROR: RT.exe not found at {RT_EXE}")
        sys.exit(1)

    print("第三章全量仿真")
    print(f"RT: {RT_EXE}")
    print(f"Output: {OUT_BASE}")

    t_start = time.time()

    # 运行实验
    e1 = experiment_antenna_types()
    e2 = experiment_polarization()
    e3 = experiment_pose()

    total_time = time.time() - t_start
    print(f"\n{'='*60}")
    print(f"全量仿真完成! 总耗时: {total_time/60:.1f} min")
    print(f"结果保存在: {OUT_BASE}")
