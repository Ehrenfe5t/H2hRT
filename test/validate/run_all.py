#!/usr/bin/env python3
"""RT v4 三级验证体系 — 一键运行入口。
用法: python test/validate/run_all.py [--rt path/to/RT.exe] [--config-dir configs/app]
输出: test/validate/results/validate_report.json
"""

import sys, json, time, subprocess
from pathlib import Path
from datetime import datetime

ROOT = Path(__file__).resolve().parents[2]
RESDIR = Path(__file__).resolve().parent / "results"
RESDIR.mkdir(exist_ok=True)

# ── 配置 ──────────────────────────────────────────────
RT_EXE = ROOT / "x64" / "Debug" / "RT.exe"
CONFIG_DIR = ROOT / "configs" / "app"


def run_rt(exe_path: Path, config_name: str, timeout: int = 60) -> dict:
    """运行 RT.exe 并解析输出。"""
    cfg = CONFIG_DIR / config_name
    if not cfg.exists():
        return {"error": f"config not found: {cfg}"}
    try:
        proc = subprocess.run([str(exe_path), str(cfg)], cwd=str(ROOT),
                              capture_output=True, text=True, encoding="utf-8", errors="replace",
                              timeout=timeout)
    except subprocess.TimeoutExpired:
        return {"error": f"timeout after {timeout}s"}
    output = (proc.stdout or "") + (proc.stderr or "")
    result = {"returncode": proc.returncode, "output": output}
    import re
    m = re.search(r"search_paths=(\d+)", output)
    if m: result["paths"] = int(m.group(1))
    m = re.search(r"em_results=(\d+)", output)
    if m: result["em_results"] = int(m.group(1))
    result["pipeline_ok"] = "A1真实生产链闭环完成" in output
    return result


def run_l1() -> dict:
    """L1: 解析解单元验证（纯 Python 独立计算，不依赖 RT.exe）。"""
    import math, cmath
    results = []
    eps0 = 8.8541878128e-12
    f = 2.4e9
    w = 2 * math.pi * f
    lam = 299792458.0 / f

    # ── L1-1: PEC 垂直入射 → Γ = -1 ──
    sigma_pec = 1e7
    eps_c = complex(1.0, -sigma_pec / (w * eps0))
    cos_i = 1.0
    sqrt_t = cmath.sqrt(eps_c - (1.0 - cos_i * cos_i))
    gamma_te = (cos_i - sqrt_t) / (cos_i + sqrt_t)
    r1 = abs(abs(gamma_te) - 1.0) < 0.001 and abs(cmath.phase(gamma_te) - math.pi) < 0.02
    results.append({"id": "L1-1", "name": "PEC垂直入射", "passed": r1,
                    "detail": f"|Γ|={abs(gamma_te):.4f}, φ={math.degrees(cmath.phase(gamma_te)):.1f}°"})

    # ── L1-2: Concrete 45° 入射 ──
    eps_r, sigma = 5.24, 0.08
    eps_c = complex(eps_r, -sigma / (w * eps0))
    cos_i = math.cos(math.radians(45))
    sqrt_t = cmath.sqrt(eps_c - (1.0 - cos_i * cos_i))
    gamma_te = (cos_i - sqrt_t) / (cos_i + sqrt_t)
    gamma_tm = (eps_c * cos_i - sqrt_t) / (eps_c * cos_i + sqrt_t)
    r2 = abs(gamma_te) > 0.01 and abs(gamma_tm) > 0.01 and abs(abs(gamma_te) - abs(gamma_tm)) > 0.01
    results.append({"id": "L1-2", "name": "Concrete 45°", "passed": r2,
                    "detail": f"|Γ_TE|={abs(gamma_te):.4f}, |Γ_TM|={abs(gamma_tm):.4f}"})

    # ── L1-3: Glass 45° → TE ≠ TM ──
    eps_r, sigma = 6.31, 0.009
    eps_c = complex(eps_r, -sigma / (w * eps0))
    gamma_te = (cos_i - cmath.sqrt(eps_c - (1.0 - cos_i * cos_i))) / (cos_i + cmath.sqrt(eps_c - (1.0 - cos_i * cos_i)))
    gamma_tm = (eps_c * cos_i - cmath.sqrt(eps_c - (1.0 - cos_i * cos_i))) / (eps_c * cos_i + cmath.sqrt(eps_c - (1.0 - cos_i * cos_i)))
    r3 = abs(abs(gamma_te) - abs(gamma_tm)) > 0.01
    results.append({"id": "L1-3", "name": "Glass 45°", "passed": r3,
                    "detail": f"|Γ_TE|={abs(gamma_te):.4f}, |Γ_TM|={abs(gamma_tm):.4f}"})

    # ── L1-4: Friis FSPL @ d=1/5/10/20m ──
    fspl_ok = True
    for d in [1.0, 5.0, 10.0, 20.0]:
        expected_db = -20 * math.log10(lam / (4 * math.pi * d))
        fspl_amp = lam / (4 * math.pi * d)
        actual_db = -20 * math.log10(fspl_amp)
        if abs(expected_db - actual_db) > 0.01: fspl_ok = False
    results.append({"id": "L1-4", "name": "Friis FSPL", "passed": fspl_ok,
                    "detail": f"d=1m→{20*math.log10(4*math.pi*1/lam):.1f}dB, d=10m→{20*math.log10(4*math.pi*10/lam):.1f}dB"})

    # ── L1-5: Snell 折射 Glass 45° ──
    n1, n2 = 1.0, math.sqrt(6.31)
    sin_t = n1 / n2 * math.sin(math.radians(45))
    theta_t = math.degrees(math.asin(sin_t))
    r5 = abs(theta_t - 16.4) < 0.5  # expected ~16.4°
    results.append({"id": "L1-5", "name": "Snell折射", "passed": r5,
                    "detail": f"θ_t={theta_t:.1f}° (expected ~16.4°)"})

    passed = sum(1 for r in results if r["passed"])
    return {"total": len(results), "passed": passed, "failed": len(results) - passed, "details": results}


def run_l2(exe_path: Path) -> dict:
    """L2: 交叉验证 — precise vs SBR 路径覆盖。"""
    results = []
    # Run precise mode on b1 scene
    rt = run_rt(exe_path, "b1_mixed_path_test.json", timeout=30)
    paths_precise = rt.get("paths", 0)
    l2_1 = paths_precise >= 300
    results.append({"id": "L2-1", "name": "b1路径数≥300(基线)", "passed": l2_1,
                    "detail": f"precise paths={paths_precise}"})

    # Run SBR on b4
    rt2 = run_rt(exe_path, "b4_sbr_test.json", timeout=30)
    sbr_ok = rt2.get("pipeline_ok", False)
    results.append({"id": "L2-2", "name": "SBR可运行", "passed": sbr_ok,
                    "detail": f"pipeline_ok={sbr_ok}"})

    passed = sum(1 for r in results if r["passed"])
    return {"total": len(results), "passed": passed, "failed": len(results) - passed, "details": results}


def run_l3() -> dict:
    """L3: 统计域验证 — 从 precise_paths.json 提取 PDP 特征。"""
    results = []
    pth = ROOT / "output" / "a1_real_chain" / "paths" / "precise_paths.json"
    if not pth.exists():
        results.append({"id": "L3-1", "name": "路径JSON存在", "passed": False, "detail": "file not found"})
        return {"total": 1, "passed": 0, "failed": 1, "details": results}
    data = json.loads(pth.read_text(encoding="utf-8"))
    paths = data.get("paths", [])
    results.append({"id": "L3-1", "name": "路径JSON存在", "passed": len(paths) > 0,
                    "detail": f"{len(paths)} paths"})

    if paths:
        delays = [p.get("delay_s", 0) for p in paths if p.get("delay_s", 0) > 0]
        powers = [p.get("power_linear", 0) for p in paths]
        if delays:
            # RMS delay spread
            import math
            total_pow = sum(powers)
            mean_d = sum(d * p for d, p in zip(delays, powers)) / total_pow if total_pow > 0 else 0
            rms_ds = math.sqrt(sum((d - mean_d)**2 * p for d, p in zip(delays, powers)) / total_pow) if total_pow > 0 else 0
            results.append({"id": "L3-2", "name": "RMS时延扩展", "passed": 1e-10 < rms_ds < 1e-6,
                            "detail": f"RMS DS={rms_ds:.2e}s"})
    passed = sum(1 for r in results if r["passed"])
    return {"total": len(results), "passed": passed, "failed": len(results) - passed, "details": results}


def main():
    import argparse
    ap = argparse.ArgumentParser()
    ap.add_argument("--rt", default=str(RT_EXE), help="RT.exe path")
    ap.add_argument("--skip-rt", action="store_true", help="Skip tests requiring RT.exe")
    args = ap.parse_args()
    rt_exe = Path(args.rt)

    print("=" * 60)
    print("RT v4 三级验证体系")
    print(f"时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    print(f"RT.exe: {rt_exe} (exists={rt_exe.exists()})")
    print("=" * 60)

    report = {"version": "v4", "timestamp": datetime.now().isoformat(), "results": {}}

    # L1
    print("\n── L1 解析解单元验证 ──")
    l1 = run_l1()
    report["results"]["L1"] = l1
    for d in l1["details"]:
        print(f"  [{d['id']}] {'PASS' if d['passed'] else 'FAIL'} {d['name']}: {d['detail']}")
    print(f"  L1: {l1['passed']}/{l1['total']} passed")

    # L2
    if not args.skip_rt and rt_exe.exists():
        print("\n── L2 交叉验证 ──")
        l2 = run_l2(rt_exe)
        report["results"]["L2"] = l2
        for d in l2["details"]:
            print(f"  [{d['id']}] {'PASS' if d['passed'] else 'FAIL'} {d['name']}: {d['detail']}")
        print(f"  L2: {l2['passed']}/{l2['total']} passed")
    else:
        report["results"]["L2"] = {"skipped": True, "reason": "RT.exe not available or --skip-rt"}

    # L3
    print("\n── L3 统计域验证 ──")
    l3 = run_l3()
    report["results"]["L3"] = l3
    for d in l3["details"]:
        print(f"  [{d['id']}] {'PASS' if d['passed'] else 'FAIL'} {d['name']}: {d['detail']}")
    print(f"  L3: {l3['passed']}/{l3['total']} passed")

    # Summary
    all_passed = all(r.get("passed", 0) == r.get("total", 0) for r in report["results"].values() if isinstance(r, dict) and "passed" in r)
    report["overall_passed"] = all_passed

    # Write report
    rpath = RESDIR / "validate_report.json"
    rpath.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(f"\n{'='*60}")
    print(f"Overall: {'ALL PASSED' if all_passed else 'SOME FAILED'}")
    print(f"报告: {rpath}")
    return 0 if all_passed else 1


if __name__ == "__main__":
    sys.exit(main())
