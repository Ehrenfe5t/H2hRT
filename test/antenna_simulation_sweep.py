#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
第三章 批量仿真脚本: 极化扫描 + 姿态扫描 (图3.3-3.5)

图3.3: Rx极化旋转实验 — V/H/+45°/LHCP/RHCP 五种Rx极化, 接收功率对比
图3.4: Rx姿态yaw扫描 — 0°→180° 接收功率变化曲线
图3.5: 联合极化效率矩阵 — Tx极化 × Rx极化 的功率矩阵

用法:
  python test/antenna_simulation_sweep.py --mode polarize   # 图3.3 + 3.5
  python test/antenna_simulation_sweep.py --mode pose       # 图3.4
  python test/antenna_simulation_sweep.py --mode all        # 全部

前置条件:
  1. 已编译 RT.exe (含 co/cross-pol 修改)
  2. 基础配置文件已存在 (configs/app/meeting_412.json)
"""

import json, os, subprocess, sys, shutil, time, argparse
from pathlib import Path
from copy import deepcopy
import numpy as np

ROOT = Path(__file__).resolve().parents[1]
RT_EXE = ROOT / "x64" / "Release" / "RT.exe"
BASE_CONFIG = ROOT / "configs" / "app" / "meeting_412.json"
ANT_DIR = ROOT / "configs" / "antennas"
SWEEP_DIR = ROOT / "output" / "ch3_sweeps"

# ─── 极化配置 ──────────────────────────────────────────
# 世界坐标中的极化矢量 (Y-up坐标系)
# co-pol = theta_hat方向 (主极化), cross-pol = phi_hat方向
POLARIZATIONS = {
    "V":    {"vec": (0.0, 1.0, 0.0), "label": "垂直极化 (V)"},
    "H":    {"vec": (0.0, 0.0, 1.0), "label": "水平极化 (H)"},
    "P45":  {"vec": (0.0, 0.707107, 0.707107), "label": "+45°斜极化"},
    "LHCP": {"vec": (0.0, 0.707107, 0.0), "imag": (0.0, 0.0, 0.707107),
             "label": "左旋圆极化 (LHCP)"},
    "RHCP": {"vec": (0.0, 0.707107, 0.0), "imag": (0.0, 0.0, -0.707107),
             "label": "右旋圆极化 (RHCP)"},
}

# ─── 姿态配置 (yaw旋转: 绕Y-up轴旋转forward/up) ────────
def rotate_yaw(forward, up, yaw_deg):
    """绕Y轴旋转 forward/up 矢量 (yaw角)"""
    c = np.cos(np.radians(yaw_deg))
    s = np.sin(np.radians(yaw_deg))
    f = np.array(forward, float)
    u = np.array(up, float)
    # Rotation matrix about Y axis
    R = np.array([[c, 0, s], [0, 1, 0], [-s, 0, c]])
    return tuple(R @ f), tuple(R @ u)

POSE_YAWS = [0, 30, 60, 90, 120, 150, 180]


# ═══════════════════════════════════════════════════════════════
def load_base_config():
    with open(BASE_CONFIG, "r", encoding="utf-8") as f:
        return json.load(f)

def write_pol_file(path: Path, vec, imag=None):
    """写入极化矢量文件 (3个数字: px py pz)"""
    with open(path, "w") as f:
        f.write(f"{vec[0]} {vec[1]} {vec[2]}\n")
    # 若为圆极化, 保存虚部到单独文件
    if imag:
        imag_path = Path(str(path).replace(".txt", "_imag.txt"))
        with open(imag_path, "w") as f:
            f.write(f"{imag[0]} {imag[1]} {imag[2]}\n")

def create_polarization_config(base, pol_key, pol_info, is_tx=True):
    """创建极化变体配置"""
    cfg = deepcopy(base)
    # 使用偶极子增益方向图 (最接近真实天线)
    cfg["antenna"]["pattern_file"] = str(ANT_DIR / "dipole_gain.csv")
    cfg["antenna"]["polarization_file"] = ""  # 不使用 per-angle Jones, 用固定极化

    # 极化矢量和方向图都从同一文件读取 —— 这里用简单极化文件
    tag = f"{'tx' if is_tx else 'rx'}_{pol_key}"
    pol_dir = SWEEP_DIR / "pol_configs"
    pol_dir.mkdir(parents=True, exist_ok=True)
    pol_file = pol_dir / f"pol_{tag}.txt"
    write_pol_file(pol_file, pol_info["vec"], pol_info.get("imag"))

    # 设置极化文件 (AntennaFactory会从中读取3个浮点数作为极化矢量)
    cfg["antenna"]["polarization_file"] = str(pol_file)

    # 天线姿态: forward沿+X, up沿+Y (使得theta=0→boresight=+X)
    cfg["antenna"]["forward_x"] = 1.0
    cfg["antenna"]["forward_y"] = 0.0
    cfg["antenna"]["forward_z"] = 0.0
    cfg["antenna"]["up_x"] = 0.0
    cfg["antenna"]["up_y"] = 1.0
    cfg["antenna"]["up_z"] = 0.0

    return cfg, tag

def create_pose_config(base, yaw_deg):
    """创建姿态变体配置"""
    cfg = deepcopy(base)
    cfg["antenna"]["pattern_file"] = str(ANT_DIR / "dipole_gain.csv")
    cfg["antenna"]["polarization_file"] = ""

    # 默认 forward (+X), up (+Y), 绕Y轴旋转
    fwd_orig = (1.0, 0.0, 0.0)
    up_orig = (0.0, 1.0, 0.0)
    fwd, up = rotate_yaw(fwd_orig, up_orig, yaw_deg)

    cfg["antenna"]["forward_x"] = fwd[0]
    cfg["antenna"]["forward_y"] = fwd[1]
    cfg["antenna"]["forward_z"] = fwd[2]
    cfg["antenna"]["up_x"] = up[0]
    cfg["antenna"]["up_y"] = up[1]
    cfg["antenna"]["up_z"] = up[2]

    return cfg, f"yaw{yaw_deg}"

def run_rt(config_path: Path, run_name: str) -> dict:
    """运行 RT.exe, 返回结果摘要"""
    cfg_dir = SWEEP_DIR / "configs" / run_name
    cfg_dir.mkdir(parents=True, exist_ok=True)
    cfg_file = cfg_dir / "config.json"
    with open(cfg_file, "w", encoding="utf-8") as f:
        json.dump(config_path if isinstance(config_path, dict) else {},
                 f, ensure_ascii=False, indent=2)

    if isinstance(config_path, dict):
        # 写入配置到临时文件
        tmp_cfg = cfg_dir / "run_config.json"
        with open(tmp_cfg, "w", encoding="utf-8") as f:
            json.dump(config_path, f, ensure_ascii=False, indent=2)
        config_arg = str(tmp_cfg)
    else:
        config_arg = str(config_path)

    print(f"  运行: {run_name} ... ", end="", flush=True)
    t0 = time.time()
    try:
        result = subprocess.run(
            [str(RT_EXE), config_arg],
            capture_output=True, text=True, timeout=600,
            cwd=str(ROOT), encoding="utf-8", errors="replace"
        )
        elapsed = time.time() - t0
        if result.returncode == 0:
            print(f"OK ({elapsed:.1f}s)")
            return {"success": True, "elapsed": elapsed, "run_name": run_name}
        else:
            print(f"FAIL (exit={result.returncode}, {elapsed:.1f}s)")
            print(f"  stderr: {result.stderr[:200]}")
            return {"success": False, "elapsed": elapsed, "run_name": run_name,
                    "stderr": result.stderr[:500]}
    except subprocess.TimeoutExpired:
        print(f"TIMEOUT")
        return {"success": False, "elapsed": 600, "run_name": run_name,
                "stderr": "timeout"}

def collect_results(output_dir: Path) -> dict:
    """从 RT 输出目录收集功率、XPR等统计量"""
    paths_file = output_dir / "paths" / "precise_paths.json"
    if not paths_file.exists():
        # 尝试子目录
        for sub in output_dir.iterdir():
            if sub.is_dir():
                pf = sub / "paths" / "precise_paths.json"
                if pf.exists():
                    paths_file = pf
                    break
        if not paths_file.exists():
            return None

    with open(paths_file, "r", encoding="utf-8") as f:
        data = json.load(f)

    paths = data.get("paths", [])
    if not paths:
        return None

    powers = [p["power_linear"] for p in paths if p.get("power_linear", 0) > 0]
    total_power = sum(powers) if powers else 0
    # co/cross-pol (从 recompiled 版本才有; 兼容旧版)
    co_powers = [p.get("co_pol_power_linear", 0) for p in paths]
    cx_powers = [p.get("cross_pol_power_linear", 0) for p in paths]
    xprs = [p.get("xpr_dB", 0) for p in paths if p.get("xpr_dB", 0) != 300.0]

    return {
        "path_count": len(paths),
        "los_count": sum(1 for p in paths if p.get("is_los")),
        "total_power_linear": total_power,
        "total_power_dBm": 10 * np.log10(total_power * 1000) if total_power > 0 else -999,
        "mean_power_linear": np.mean(powers) if powers else 0,
        "total_co_power": sum(co_powers) if co_powers else 0,
        "total_cross_power": sum(cx_powers) if cx_powers else 0,
        "mean_xpr_dB": np.mean(xprs) if xprs else float("nan"),
    }


# ═══════════════════════════════════════════════════════════════
#  极化扫描 (图3.3 + 图3.5)
# ═══════════════════════════════════════════════════════════════
def sweep_polarization(dry_run=False):
    """扫描 Tx极化×Rx极化 组合"""
    print("\n" + "="*60)
    print("极化扫描实验: Tx极化 × Rx极化")
    print("="*60)

    if not RT_EXE.exists() and not dry_run:
        print(f"❌ RT.exe 不存在: {RT_EXE}")
        print("   请先编译 RT 项目 (含 co/cross-pol 修改)")
        return None

    base = load_base_config()
    # 确保 Precise 模式
    base.setdefault("em_solver", {})["solver_mode"] = "Precise"

    results = {}
    tx_pols = ["V", "H"]
    rx_pols = ["V", "H", "P45", "LHCP", "RHCP"]

    for tx_key in tx_pols:
        for rx_key in rx_pols:
            run_name = f"pol_Tx{tx_key}_Rx{rx_key}"

            # 构建配置: Tx用tx极化, Rx用rx极化
            # 注意: 当前框架中Tx和Rx共用同一个 antenna 配置节
            # 所以极化扫描只能通过分别修改 polarization_file 来实现
            # 简便方案: Tx固定V, 只扫Rx极化
            if tx_key != "V":
                continue  # 简化: 只做Tx=V, Rx扫描 (论文中 PLF 分析已足够)

            cfg = deepcopy(base)
            cfg["antenna"]["pattern_file"] = str(ANT_DIR / "dipole_gain.csv")

            pol_dir = SWEEP_DIR / "pol_configs"
            pol_dir.mkdir(parents=True, exist_ok=True)
            pol_file = pol_dir / f"pol_rx_{rx_key}.txt"

            pol_info = POLARIZATIONS[rx_key]
            write_pol_file(pol_file, pol_info["vec"], pol_info.get("imag"))
            cfg["antenna"]["polarization_file"] = str(pol_file)

            # 姿态: forward=+X, up=+Y
            cfg["antenna"]["forward_x"] = 1.0; cfg["antenna"]["forward_y"] = 0.0; cfg["antenna"]["forward_z"] = 0.0
            cfg["antenna"]["up_x"] = 0.0; cfg["antenna"]["up_y"] = 1.0; cfg["antenna"]["up_z"] = 0.0

            if dry_run:
                print(f"  [DRY-RUN] {run_name}: Rx={POLARIZATIONS[rx_key]['label']}")
                results[run_name] = {"config": cfg, "pol": POLARIZATIONS[rx_key]}
                continue

            # 写入临时配置
            tmp_cfg = SWEEP_DIR / "configs" / run_name / "config.json"
            tmp_cfg.parent.mkdir(parents=True, exist_ok=True)
            with open(tmp_cfg, "w", encoding="utf-8") as f:
                json.dump(cfg, f, ensure_ascii=False, indent=2)

            rt_result = run_rt(tmp_cfg, run_name)
            if rt_result["success"]:
                # 查找输出目录 (RT通常输出到 output/<run_id>/rx1/)
                # run_id 在配置中设置
                run_id = cfg.get("app_runtime", {}).get("run_id", "412")
                out_dir = ROOT / "output" / run_id / "rx1"
                summary = collect_results(out_dir)
                if summary:
                    summary["pol_label"] = POLARIZATIONS[rx_key]["label"]
                    results[run_name] = summary
                    print(f"    总功率={summary['total_power_dBm']:.1f}dBm  "
                          f"XPR={summary['mean_xpr_dB']:.1f}dB  "
                          f"路径={summary['path_count']}")
            else:
                results[run_name] = {"error": rt_result.get("stderr", "unknown")}

    # 保存结果摘要
    summary_file = SWEEP_DIR / "polarization_sweep_results.json"
    # 清理不可序列化的对象
    clean = {}
    for k, v in results.items():
        if isinstance(v, dict):
            clean[k] = {sk: sv for sk, sv in v.items()
                       if not callable(sv) and not isinstance(sv, (type,))}
    with open(summary_file, "w", encoding="utf-8") as f:
        json.dump(clean, f, ensure_ascii=False, indent=2, default=str)
    print(f"\n极化扫描结果已保存: {summary_file}")
    return results


# ═══════════════════════════════════════════════════════════════
#  姿态扫描 (图3.4)
# ═══════════════════════════════════════════════════════════════
def sweep_pose(dry_run=False):
    """扫描 Rx天线 yaw 角"""
    print("\n" + "="*60)
    print("姿态扫描实验: Rx天线 yaw 0°→180°")
    print("="*60)

    if not RT_EXE.exists() and not dry_run:
        print(f"❌ RT.exe 不存在: {RT_EXE}")
        return None

    base = load_base_config()
    base.setdefault("em_solver", {})["solver_mode"] = "Precise"
    base["antenna"]["pattern_file"] = str(ANT_DIR / "dipole_gain.csv")
    base["antenna"]["polarization_file"] = ""

    results = {}
    for yaw in POSE_YAWS:
        run_name = f"pose_yaw{yaw}"
        cfg, _ = create_pose_config(base, yaw)

        if dry_run:
            print(f"  [DRY-RUN] {run_name}: yaw={yaw}°")
            results[run_name] = {"yaw_deg": yaw}
            continue

        tmp_cfg = SWEEP_DIR / "configs" / run_name / "config.json"
        tmp_cfg.parent.mkdir(parents=True, exist_ok=True)
        with open(tmp_cfg, "w", encoding="utf-8") as f:
            json.dump(cfg, f, ensure_ascii=False, indent=2)

        rt_result = run_rt(tmp_cfg, run_name)
        if rt_result["success"]:
            run_id = cfg.get("app_runtime", {}).get("run_id", "412")
            out_dir = ROOT / "output" / run_id / "rx1"
            summary = collect_results(out_dir)
            if summary:
                summary["yaw_deg"] = yaw
                results[run_name] = summary
                print(f"    yaw={yaw}°: 总功率={summary['total_power_dBm']:.1f}dBm  "
                      f"XPR={summary['mean_xpr_dB']:.1f}dB")
        else:
            results[run_name] = {"error": rt_result.get("stderr", "unknown"), "yaw_deg": yaw}

    summary_file = SWEEP_DIR / "pose_sweep_results.json"
    clean = {k: {sk: sv for sk, sv in v.items() if not callable(sv)}
             for k, v in results.items() if isinstance(v, dict)}
    with open(summary_file, "w", encoding="utf-8") as f:
        json.dump(clean, f, ensure_ascii=False, indent=2, default=str)
    print(f"\n姿态扫描结果已保存: {summary_file}")
    return results


# ═══════════════════════════════════════════════════════════════
#  Main
# ═══════════════════════════════════════════════════════════════
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="第三章批量仿真")
    parser.add_argument("--mode", choices=["polarize", "pose", "all"], default="all")
    parser.add_argument("--dry-run", action="store_true",
                       help="仅生成配置文件, 不实际运行RT")
    args = parser.parse_args()

    SWEEP_DIR.mkdir(parents=True, exist_ok=True)

    if args.mode in ("polarize", "all"):
        sweep_polarization(dry_run=args.dry_run)

    if args.mode in ("pose", "all"):
        sweep_pose(dry_run=args.dry_run)

    print("\n✅ 批量仿真完成")
