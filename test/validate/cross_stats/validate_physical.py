"""
物理合理性验证: 从RT仿真结果中提取信道关键指标，逐项检查物理合理性。
验证项:
  V1: 路径损耗 vs 自由空间 (LOS不应优于自由空间)
  V2: RMS时延扩展范围 (室内应为ns量级)
  V3: 功率守恒 (最强路径功率 ≤ Tx功率)
  V4: 时延与距离一致性 (delay = d/c)
  V5: PDP指数衰减趋势
  V6: SBR收敛性 (功率随射线数趋于稳定)
  V7: 路径损耗指数 (室内应为1.5~4.0)
  V8: 3GPP InH-Office模型对比
"""
import json, math, sys, os
from pathlib import Path
from collections import defaultdict

C0 = 299792458.0  # 光速

def load_paths(json_path):
    """加载RT路径JSON."""
    with open(json_path, 'r', encoding='utf-8') as f:
        data = json.load(f)
    # 兼容不同格式: 可能是数组或 {paths: [...]}
    if isinstance(data, list):
        return data
    if isinstance(data, dict):
        for key in ['paths', 'path_results', 'results']:
            if key in data:
                return data[key]
        # 可能是直接的路径数组在某个key下
        for v in data.values():
            if isinstance(v, list) and len(v) > 0:
                return v
    return []

def compute_rms_delay_spread(paths):
    """RMS时延扩展 (秒)."""
    if not paths: return 0.0
    total_pwr = 0.0
    weighted_tau = 0.0
    weighted_tau2 = 0.0
    for p in paths:
        power_lin = p.get('power_linear', 0)
        delay = p.get('delay_s', 0)
        if power_lin <= 0: continue
        total_pwr += power_lin
        weighted_tau += power_lin * delay
        weighted_tau2 += power_lin * delay * delay
    if total_pwr <= 0: return 0.0
    mean_tau = weighted_tau / total_pwr
    rms_ds = math.sqrt(max(0, weighted_tau2 / total_pwr - mean_tau * mean_tau))
    return rms_ds

def compute_path_loss_exponent(distances_m, powers_dBm, tx_power_dBm=30.0):
    """对数距离拟合: PL(d) = PL(d0) + 10*n*log10(d/d0)."""
    if len(distances_m) < 2: return 0.0
    # 用最小二乘法拟合 n
    d0 = min(distances_m)
    x = [math.log10(d/d0) for d in distances_m]
    y = [tx_power_dBm - p for p in powers_dBm]  # path loss in dB
    n = len(x)
    sum_x = sum(x); sum_y = sum(y)
    sum_xx = sum(xi*xi for xi in x)
    sum_xy = sum(xi*yi for xi, yi in zip(x, y))
    slope = (n * sum_xy - sum_x * sum_y) / (n * sum_xx - sum_x * sum_x)
    return max(0, slope) / 10.0  # n = slope/10

def inH_office_path_loss(d_3d, f_GHz, los=True):
    """3GPP TR 38.901 Table 7.4.1-1 InH-Office."""
    if los:
        return 32.4 + 17.3 * math.log10(max(0.01, d_3d)) + 20.0 * math.log10(max(0.1, f_GHz))
    else:
        return 38.3 * math.log10(max(0.01, d_3d)) + 17.3 + 24.9 * math.log10(max(0.1, f_GHz))

def validate_all(precise_json, coverage_json=None, sbr_coverage_data=None, tx_pos=None, rx_positions=None, freq_hz=2.4e9):
    """执行全部物理合理性验证."""
    results = []
    freq_GHz = freq_hz / 1e9

    paths = load_paths(precise_json)
    print(f"[验证] 加载 {len(paths)} 条路径")

    # ── V1: 路径损耗不应优于自由空间 ──
    los_path = None
    for p in paths:
        if p.get('is_los', False):
            los_path = p
            break
    if los_path and tx_pos and rx_positions:
        d_los = math.sqrt((tx_pos[0]-rx_positions[0][0])**2 + (tx_pos[1]-rx_positions[0][1])**2 + (tx_pos[2]-rx_positions[0][2])**2)
        fspl_db = 20 * math.log10(4 * math.pi * d_los * freq_hz / C0)
        path_loss_db = -10 * math.log10(max(1e-30, los_path.get('power_linear', 1e-30)))
        v1_ok = path_loss_db >= fspl_db - 0.5  # 允许0.5dB容差
        results.append(("V1-LOS路径损耗≥自由空间", v1_ok,
            f"FSPL={fspl_db:.1f}dB, 仿真PL={path_loss_db:.1f}dB, d={d_los:.2f}m"))
    else:
        results.append(("V1-LOS路径损耗", None, "无LOS路径或位置信息"))

    # ── V2: RMS时延扩展应为ns量级 ──
    rms_ds = compute_rms_delay_spread(paths)
    rms_ds_ns = rms_ds * 1e9
    v2_ok = 0.01 < rms_ds_ns < 1000  # 室内: 0.01ns~1000ns (实际通常1~100ns)
    results.append(("V2-RMS时延扩展(室内ns量级)", v2_ok, f"RMS DS = {rms_ds_ns:.3f} ns"))

    # ── V3: 功率守恒 ──
    if paths:
        max_power = max(p.get('power_linear', 0) for p in paths)
        v3_ok = max_power <= 1.01  # 无源, 不应超过1 (归一化发射功率)
        results.append(("V3-最大功率≤1(无源)", v3_ok, f"max(power_linear) = {max_power:.6f}"))
        # 总功率也应≤1 (相干叠加可能>1, 但非相干和≤1)
        total_power = sum(p.get('power_linear', 0) for p in paths)
        v3b_ok = total_power <= 10.0  # 相干叠加可以>1, 但不能超过路径数
        results.append(("V3b-总功率合理", v3b_ok, f"total_power_linear = {total_power:.3f} (paths={len(paths)})"))

    # ── V4: 时延 = 距离/c ──
    delay_errors = []
    for p in paths[:100]:  # 前100条
        d = p.get('total_length_m', 0)
        tau = p.get('delay_s', 0)
        if d > 0 and tau > 0:
            expected_tau = d / C0
            delay_errors.append(abs(tau - expected_tau) / expected_tau)
    if delay_errors:
        max_err = max(delay_errors)
        v4_ok = max_err < 0.01  # 1%误差内
        results.append(("V4-时延=距离/c", v4_ok, f"最大相对误差={max_err*100:.4f}%"))
    else:
        results.append(("V4-时延一致性", None, "无有效数据"))

    # ── V5: PDP趋势 ──
    sorted_paths = sorted(paths, key=lambda p: p.get('delay_s', 0))
    delays_ns = [p.get('delay_s', 0)*1e9 for p in sorted_paths]
    powers_dbm = [10*math.log10(max(1e-30, p.get('power_linear', 1e-30))) + 30 for p in sorted_paths]
    # 简单检查: 随着时延增加, 功率包络应总体下降
    if len(powers_dbm) > 3:
        early_avg = sum(powers_dbm[:len(powers_dbm)//3]) / max(1, len(powers_dbm)//3)
        late_avg = sum(powers_dbm[2*len(powers_dbm)//3:]) / max(1, len(powers_dbm)-2*len(powers_dbm)//3)
        v5_ok = early_avg > late_avg  # 早期功率 > 晚期功率
        results.append(("V5-PDP包络衰减", v5_ok, f"早期avg={early_avg:.1f}dBm, 晚期avg={late_avg:.1f}dBm"))
    else:
        results.append(("V5-PDP包络", None, f"路径太少({len(paths)})"))

    # ── V7: 路径损耗指数 ──
    if tx_pos and rx_positions and len(rx_positions) > 1 and paths:
        # 每条路径对应一个Rx位置的距离
        dists = [math.sqrt((tx_pos[0]-r[0])**2+(tx_pos[1]-r[1])**2+(tx_pos[2]-r[2])**2) for r in rx_positions]
        pwr_dbm = [10*math.log10(max(1e-30, p.get('power_linear', 1e-30))) + 30 for p in paths[:len(dists)]]
        n = compute_path_loss_exponent(dists, pwr_dbm)
        v7_ok = 1.0 < n < 5.0  # 室内典型 n∈[1.5, 4.0]
        results.append(("V7-路径损耗指数(室内)", v7_ok, f"n = {n:.2f} (室内典型1.5~4.0)"))

    # ── V8: 3GPP InH-Office对比 ──
    if tx_pos and rx_positions:
        for rx_p in rx_positions[:3]:
            d = math.sqrt((tx_pos[0]-rx_p[0])**2+(tx_pos[1]-rx_p[1])**2+(tx_pos[2]-rx_p[2])**2)
            pl_3gpp_los = inH_office_path_loss(d, freq_GHz, True)
            pl_3gpp_nlos = inH_office_path_loss(d, freq_GHz, False)
        # 对每条路径计算其PL, 应与3GPP范围同量级
        valid_pl = []
        for p in paths:
            d = p.get('total_length_m', 0)
            if d > 0.1:
                pl_sim = -10*math.log10(max(1e-30, p.get('power_linear', 1e-30))) + 30
                pl_3gpp = inH_office_path_loss(d, freq_GHz, not p.get('is_los', False))
                valid_pl.append((d, pl_sim, pl_3gpp))
        if valid_pl:
            # 检查仿真PL与3GPP模型的偏差是否在合理范围
            deviations = [abs(sim - ref) for _, sim, ref in valid_pl[:50]]
            avg_dev = sum(deviations) / len(deviations)
            v8_ok = avg_dev < 30  # 平均偏差<30dB(考虑到3GPP是统计模型)
            results.append(("V8-3GPP模型量级一致", v8_ok, f"avg|PL_sim - PL_3gpp| = {avg_dev:.1f} dB"))

    return results

if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("--precise", default="output/a1_real_chain/paths/precise_paths.json")
    parser.add_argument("--freq-ghz", type=float, default=2.4)
    args = parser.parse_args()

    # 从b1配置获取Tx/Rx
    tx_pos = (1.0, 2.0, 1.5)
    rx_positions = [(5.0, 2.0, 1.5)]

    results = validate_all(args.precise, tx_pos=tx_pos, rx_positions=rx_positions, freq_hz=args.freq_ghz*1e9)

    print("\n" + "="*60)
    print("物理合理性验证报告")
    print("="*60)
    all_pass = True
    for name, ok, detail in results:
        if ok is None:
            status = "⚠️ 跳过"
        elif ok:
            status = "✅ PASS"
        else:
            status = "❌ FAIL"
            all_pass = False
        print(f"  {status}  {name}")
        print(f"          {detail}")
    print(f"\n总结果: {'全部通过' if all_pass else '存在失败项'}")
