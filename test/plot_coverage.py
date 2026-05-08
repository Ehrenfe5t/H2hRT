"""
功率覆盖热力图: 从sbr_coverage.json 3D数据中切Y平面绘制。
用法: python plot_coverage.py [--plane-y Y] [--plane-z Z]
"""
import json, math, sys, os
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np

BASE = 'F:/RT/RT/output/a1_real_chain/coverage'
OUT = 'F:/RT/RT/output/plots'

def load_json(p):
    with open(p, 'r', encoding='utf-8') as f:
        return json.load(f)

def plot_plane(sbr_path, plane_y=None, plane_z=None, tol=0.05):
    os.makedirs(OUT, exist_ok=True)
    data = load_json(sbr_path)
    records = data['records']

    if plane_y is not None:
        layer = [r for r in records if abs(r['y'] - plane_y) < tol]
        title = f'Power Coverage at Y={plane_y:.1f}m'
        fname = f'coverage_Y{plane_y:.1f}.png'
    elif plane_z is not None:
        layer = [r for r in records if abs(r['z'] - plane_z) < tol]
        title = f'Power Coverage at Z={plane_z:.1f}m'
        fname = f'coverage_Z{plane_z:.1f}.png'
    else:
        layer = records
        title = 'Power Coverage (all points)'
        fname = 'coverage_all.png'

    if not layer:
        print(f"No points found for the specified plane")
        return

    xs = sorted(set(r['x'] for r in layer))
    ys = sorted(set(r['z'] if plane_y is not None else r['y'] for r in layer))

    grid = np.zeros((len(ys), len(xs)))
    hits = np.zeros((len(ys), len(xs)), dtype=int)

    for r in layer:
        xi = xs.index(r['x'])
        yi = ys.index(r['z'] if plane_y is not None else r['y'])
        pwr = r['power_dBm'] if r['ray_hit_count'] > 0 else np.nan
        grid[yi, xi] = pwr
        hits[yi, xi] = r['ray_hit_count']

    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(16, 6))

    # Power heatmap
    masked = np.ma.masked_where(grid == 0, grid)
    im1 = ax1.pcolormesh(xs, ys, masked, cmap='jet', shading='auto', vmin=-60, vmax=-20)
    ax1.set_xlabel('X (m)')
    ax1.set_ylabel('Z (m)' if plane_y is not None else 'Y (m)')
    ax1.set_title(f'{title} - Power (dBm)')
    ax1.set_aspect('equal')
    plt.colorbar(im1, ax=ax1, label='dBm')

    # Hit count
    masked_hits = np.ma.masked_where(hits == 0, hits)
    im2 = ax2.pcolormesh(xs, ys, masked_hits, cmap='YlOrRd', shading='auto')
    ax2.set_xlabel('X (m)')
    ax2.set_ylabel('Z (m)' if plane_y is not None else 'Y (m)')
    ax2.set_title(f'{title} - Ray Hits')
    ax2.set_aspect('equal')
    plt.colorbar(im2, ax=ax2, label='hits')

    # Mark Tx position
    tx_x, tx_y, tx_z = 16.0, 1.5, -12.0
    if plane_y is not None:
        ax1.plot(tx_x, tx_z, 'w*', markersize=15, markeredgecolor='black')
        ax2.plot(tx_x, tx_z, 'w*', markersize=15, markeredgecolor='black')
    else:
        ax1.plot(tx_x, tx_y, 'w*', markersize=15, markeredgecolor='black')
        ax2.plot(tx_x, tx_y, 'w*', markersize=15, markeredgecolor='black')

    plt.tight_layout()
    outpath = os.path.join(OUT, fname)
    plt.savefig(outpath, dpi=150)
    plt.close()

    # Stats
    active = [r for r in layer if r['ray_hit_count'] > 0]
    pwr_vals = [r['power_dBm'] for r in active]
    print(f'{title}: {len(layer)} Rx, {len(active)} active '
          f'({100*len(active)/max(1,len(layer)):.1f}%)')
    if pwr_vals:
        print(f'  Power: {min(pwr_vals):.1f} ~ {max(pwr_vals):.1f} dBm '
              f'(median {np.median(pwr_vals):.1f})')
    print(f'  Saved: {outpath}')

if __name__ == '__main__':
    import argparse
    p = argparse.ArgumentParser()
    p.add_argument('--plane-y', type=float)
    p.add_argument('--plane-z', type=float)
    p.add_argument('--sbr', default=f'{BASE}/sbr_coverage.json')
    args = p.parse_args()

    plot_plane(args.sbr, plane_y=args.plane_y, plane_z=args.plane_z)
