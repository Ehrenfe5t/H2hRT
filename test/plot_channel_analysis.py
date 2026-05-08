"""
信道分析: 从 precise 路径数据生成 AoA/AoD, PDP, CIR, CDF
对标 Sionna RT 输出格式
"""
import json, math, os
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

C0 = 299792458.0
OUT = 'output/plots/channel'
os.makedirs(OUT, exist_ok=True)

def load_paths(fp):
    with open(fp,'r') as f: d=json.load(f)
    return d['paths'] if isinstance(d,dict) else d

def compute_aod_aoa(paths):
    """从几何节点提取 AoD (发射角) 和 AoA (到达角)"""
    aod_theta, aod_phi, aoa_theta, aoa_phi, powers = [], [], [], [], []
    for p in paths:
        if p['power_linear'] <= 1e-20: continue
        nodes = p.get('geometry_nodes',[])
        if len(nodes) < 2: continue
        # AoD: TX → 第一个交互点方向
        tx, n1 = nodes[0], nodes[1]
        dx,dy,dz = n1['x']-tx['x'], n1['y']-tx['y'], n1['z']-tx['z']
        r = math.sqrt(dx*dx+dy*dy+dz*dz)
        if r<1e-9: continue
        aod_theta.append(math.degrees(math.acos(dz/r)))
        aod_phi.append(math.degrees(math.atan2(dy, dx)))
        powers.append(p['power_linear'])
        # AoA: 最后交互点 → RX 方向
        rx, n2 = nodes[-1], nodes[-2]
        dx,dy,dz = rx['x']-n2['x'], rx['y']-n2['y'], rx['z']-n2['z']
        r = math.sqrt(dx*dx+dy*dy+dz*dz)
        if r<1e-9: continue
        aoa_theta.append(math.degrees(math.acos(dz/r)))
        aoa_phi.append(math.degrees(math.atan2(dy, dx)))
    return (np.array(aod_theta), np.array(aod_phi),
            np.array(aoa_theta), np.array(aoa_phi), np.array(powers))

def compute_cir(paths, bandwidth=1e6):
    """CIR: 复振幅 + 时延 → 离散抽头"""
    valid = [p for p in paths if p['power_linear'] > 1e-20]
    if not valid: return None, None, None
    taus = np.array([p['delay_s'] for p in valid])
    amps = np.array([complex(p.get('amplitude_real',math.sqrt(p['power_linear'])),
                             p.get('amplitude_imag',0)) for p in valid])
    order = np.argsort(taus)
    return taus[order], amps[order], np.array([p['power_linear'] for p in valid])[order]

def compute_pdp(paths):
    """PDP: 功率-时延分布"""
    valid = [p for p in paths if p['power_linear'] > 1e-20]
    if not valid: return None, None
    taus = np.array([p['delay_s']*1e9 for p in valid])
    pwrs = np.array([p['power_linear'] for p in valid])
    order = np.argsort(taus)
    return taus[order], pwrs[order]

def compute_cdf(values, label=""):
    """经验 CDF"""
    s = np.sort(values)
    cdf = np.arange(1,len(s)+1)/len(s)
    return s, cdf

def channel_params(paths):
    """信道参数 (对标 Sionna channel_params)"""
    valid = [p for p in paths if p['power_linear'] > 1e-20]
    if not valid: return {}
    pwrs = np.array([p['power_linear'] for p in valid])
    taus = np.array([p['delay_s'] for p in valid])
    total = np.sum(pwrs)
    mean_tau = np.sum(pwrs*taus)/total
    rms_ds = np.sqrt(np.sum(pwrs*(taus-mean_tau)**2)/total)
    los = [p for p in valid if p.get('is_los')]
    los_pwr = sum(p['power_linear'] for p in los) if los else 1e-30
    nlos_pwr = total - los_pwr
    k_db = 10*np.log10(max(los_pwr,1e-30)/max(nlos_pwr,1e-30)) if nlos_pwr>0 else 40
    path_loss = -10*np.log10(total)
    return {
        'num_paths': len(valid),
        'path_loss_dB': path_loss,
        'rms_ds_ns': rms_ds*1e9,
        'mean_delay_ns': mean_tau*1e9,
        'k_factor_dB': k_db,
        'total_power_dBm': 10*np.log10(total)+30,
    }

def plot_all(paths, title_prefix, tx_pos=None, rx_pos=None):
    """生成全套 Sionna 风格图表"""
    print(f"\n{'='*60}")
    print(f"  {title_prefix}: {len(paths)} paths")
    print(f"{'='*60}")

    # 信道参数
    cp = channel_params(paths)
    for k,v in cp.items():
        print(f"  {k}: {v:.2f}" if isinstance(v,float) else f"  {k}: {v}")

    # ── Fig1: CIR + PDP ──
    taus_cir, amps_cir, _ = compute_cir(paths)
    taus_pdp, pwrs_pdp = compute_pdp(paths)

    fig, axes = plt.subplots(1,2,figsize=(14,5))
    if taus_cir is not None:
        ax=axes[0]
        ax.stem(taus_cir*1e9, np.abs(amps_cir), linefmt='C0-', markerfmt='C0o', basefmt='k-')
        ax.set_xlabel('Delay (ns)'); ax.set_ylabel('|CIR|')
        ax.set_title(f'{title_prefix} — CIR'); ax.grid(True,alpha=0.3)

        ax=axes[1]
        pwr_dbm = 10*np.log10(np.maximum(pwrs_pdp,1e-30))+30
        ax.stem(taus_pdp, pwr_dbm, linefmt='C1-', markerfmt='C1o', basefmt='k-')
        ax.set_xlabel('Delay (ns)'); ax.set_ylabel('Power (dBm)')
        ax.set_title(f'{title_prefix} — PDP'); ax.grid(True,alpha=0.3)
    plt.tight_layout(); plt.savefig(f'{OUT}/{title_prefix.lower().replace(" ","_")}_cir_pdp.png',dpi=150); plt.close()

    # ── Fig2: AoA/AoD 极坐标 ──
    aod_t, aod_p, aoa_t, aoa_p, ang_pwr = compute_aod_aoa(paths)
    if len(aod_t) > 0:
        fig, axes = plt.subplots(1,2,figsize=(12,6),subplot_kw={'projection':'polar'})
        pwrn = ang_pwr/np.max(ang_pwr)
        ax=axes[0]; sc=ax.scatter(aod_p, 90-aod_t, c=pwrn, s=60, cmap='hot',alpha=0.8,edgecolors='k',linewidth=0.3)
        ax.set_title(f'{title_prefix} — AoD'); ax.set_theta_zero_location('N'); ax.set_theta_direction(-1)
        plt.colorbar(sc,ax=ax,label='Norm Power',shrink=0.7)
        ax=axes[1]; sc=ax.scatter(aoa_p, 90-aoa_t, c=pwrn, s=60, cmap='hot',alpha=0.8,edgecolors='k',linewidth=0.3)
        ax.set_title(f'{title_prefix} — AoA'); ax.set_theta_zero_location('N'); ax.set_theta_direction(-1)
        plt.colorbar(sc,ax=ax,label='Norm Power',shrink=0.7)
        plt.tight_layout(); plt.savefig(f'{OUT}/{title_prefix.lower().replace(" ","_")}_angles.png',dpi=150); plt.close()

    # ── Fig3: 功率 CDF + 功率-时延散点 ──
    fig, axes = plt.subplots(1,2,figsize=(14,5))
    if taus_pdp is not None:
        ax=axes[0]
        pwr_dbm = 10*np.log10(np.maximum(pwrs_pdp,1e-30))+30
        sizes = 20+180*pwrs_pdp/np.max(pwrs_pdp)
        sc=ax.scatter(taus_pdp, pwr_dbm, s=sizes, c=taus_pdp, cmap='plasma',alpha=0.8,edgecolors='k',linewidth=0.3)
        ax.set_xlabel('Delay (ns)'); ax.set_ylabel('Power (dBm)')
        ax.set_title(f'{title_prefix} — Power-Delay'); ax.grid(True,alpha=0.3)
        plt.colorbar(sc,ax=ax,label='Delay (ns)')

        ax=axes[1]
        s,cdf = compute_cdf(pwr_dbm)
        ax.plot(s, cdf, 'b-', linewidth=2); ax.fill_between(s,0,cdf,alpha=0.2)
        ax.set_xlabel('Power (dBm)'); ax.set_ylabel('CDF')
        ax.set_title(f'{title_prefix} — Power CDF'); ax.grid(True,alpha=0.3)
    plt.tight_layout(); plt.savefig(f'{OUT}/{title_prefix.lower().replace(" ","_")}_stats.png',dpi=150); plt.close()

    print(f"\n  Channel plots saved to {OUT}/")
    print(f"  CIR/PDP:  {title_prefix.lower().replace(' ','_')}_cir_pdp.png")
    print(f"  Angles:   {title_prefix.lower().replace(' ','_')}_angles.png")
    print(f"  Stats:    {title_prefix.lower().replace(' ','_')}_stats.png")

if __name__ == '__main__':
    # meeting 场景
    base = 'output/a1_real_chain/paths'
    if os.path.exists(f'{base}/precise_paths.json'):
        paths = load_paths(f'{base}/precise_paths.json')
        plot_all(paths, 'Meeting Room', tx_pos=(16,1.5,-12), rx_pos=(10,1.5,-10))
    print(f"\nDone → {OUT}/")
