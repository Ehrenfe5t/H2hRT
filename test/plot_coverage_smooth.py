"""
功率覆盖可视化: 平滑插值 + 建筑物轮廓 + 任意平面切片
用法: python plot_coverage_smooth.py [--plane-y Y] [--plane-z Z] [--interp-factor N]
"""
import json, math, os, sys
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.colors import LinearSegmentedColormap
from scipy.interpolate import griddata

BASE = 'output/a1_real_chain/coverage'
OBJ_PATH = 'demo/meeting.obj'
OUT = 'output/plots'

def parse_obj_edges(filepath, plane_axis, plane_val, tol=0.1):
    """从OBJ提取指定平面附近的建筑轮廓线."""
    vertices = []
    faces = []
    current_obj = "unknown"
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        for line in f:
            line = line.strip()
            if line.startswith('o '): current_obj = line[2:].strip()
            elif line.startswith('v '):
                p = line.split()
                vertices.append((float(p[1]), float(p[2]), float(p[3])))
            elif line.startswith('f '):
                parts = line.split()[1:]
                idxs = [int(p.split('/')[0])-1 for p in parts]
                for i in range(1, len(idxs)-1):
                    faces.append((idxs[0], idxs[i], idxs[i+1]))

    # 找与指定平面相交的边
    axis_map = {'x': 0, 'y': 1, 'z': 2}
    ax = axis_map[plane_axis]
    other_axes = [i for i in range(3) if i != ax]
    edges = []
    for f in faces:
        v0, v1, v2 = vertices[f[0]], vertices[f[1]], vertices[f[2]]
        vals = [v0[ax], v1[ax], v2[ax]]
        crosses = []
        for i in range(3):
            j = (i+1) % 3
            if (vals[i] - plane_val) * (vals[j] - plane_val) < 0:
                t = (plane_val - vals[i]) / (vals[j] - vals[i])
                pt = vertices[f[i] if i<3 else f[i%3]]
                ptn = vertices[f[j] if j<3 else f[j%3]]
                cx = pt[other_axes[0]] + t*(ptn[other_axes[0]]-pt[other_axes[0]])
                cy = pt[other_axes[1]] + t*(ptn[other_axes[1]]-pt[other_axes[1]])
                edges.append((cx, cy))
        # Also check if an edge lies exactly on the plane
        for i in range(3):
            j = (i+1)%3
            if abs(vals[i]-plane_val)<tol and abs(vals[j]-plane_val)<tol:
                pt = vertices[f[i]]
                ptn = vertices[f[j]]
                edges.append((pt[other_axes[0]], pt[other_axes[1]]))
                edges.append((ptn[other_axes[0]], ptn[other_axes[1]]))
    return edges

def plot_coverage(sbr_path, obj_path, plane_y=None, plane_z=None,
                  interp_factor=3, tol=0.1, tx_pos=None, rx_pos=None):
    os.makedirs(OUT, exist_ok=True)
    with open(sbr_path, 'r') as f:
        sbr = json.load(f)
    records = sbr['records']

    # 确定切片平面
    if plane_y is not None:
        ys = sorted(set(r['y'] for r in records))
        target = min(ys, key=lambda y: abs(y-plane_y))
        layer = [r for r in records if abs(r['y']-target) < tol]
        plane_label = f'Y={target:.2f}m'
        x_vals = [r['x'] for r in layer]
        y_vals = [r['z'] for r in layer]
        edge_axis, edge_val = 'y', target
        xlabel, ylabel = 'X (m)', 'Z (m)'
    elif plane_z is not None:
        zs = sorted(set(r['z'] for r in records))
        target = min(zs, key=lambda z: abs(z-plane_z))
        layer = [r for r in records if abs(r['z']-target) < tol]
        plane_label = f'Z={target:.2f}m'
        x_vals = [r['x'] for r in layer]
        y_vals = [r['y'] for r in layer]
        edge_axis, edge_val = 'z', target
        xlabel, ylabel = 'X (m)', 'Y (m)'
    else:
        layer = records
        x_vals = [r['x'] for r in layer]
        y_vals = [r['y'] for r in layer]
        plane_label = 'All'
        edge_axis, edge_val = 'y', 1.5
        xlabel, ylabel = 'X (m)', 'Y (m)'

    if not layer:
        print(f"No points at {plane_label}")
        return

    # 准备数据
    active = [r for r in layer if r['ray_hit_count'] > 0]
    if not active:
        print(f"No active Rx at {plane_label}")
        return

    pts = np.array([[r['x'], r['z'] if plane_y is not None else r['y']] for r in active])
    pwr = np.array([r['power_dBm'] for r in active])

    # 平滑插值
    xi = np.linspace(pts[:,0].min(), pts[:,0].max(), int((pts[:,0].max()-pts[:,0].min())/0.02)*interp_factor)
    yi = np.linspace(pts[:,1].min(), pts[:,1].max(), int((pts[:,1].max()-pts[:,1].min())/0.02)*interp_factor)
    XI, YI = np.meshgrid(xi, yi)
    ZI = griddata(pts, pwr, (XI, YI), method='cubic', fill_value=np.nan)

    # 建筑物轮廓
    edges = parse_obj_edges(obj_path, edge_axis, edge_val)

    # 绘图
    fig, ax = plt.subplots(figsize=(16, 8))

    # 自定义 colormap: 深蓝→青→绿→黄→红 (类似 Sionna jet 风格)
    cmap = plt.cm.jet.copy()
    cmap.set_bad('white', 0)

    im = ax.pcolormesh(XI, YI, ZI, cmap=cmap, shading='auto',
                       vmin=-70, vmax=20)

    # 建筑物轮廓
    for (x1, y1), (x2, y2) in zip(edges[::2], edges[1::2]):
        ax.plot([x1, x2], [y1, y2], 'k-', linewidth=1.2, alpha=0.6)

    # 单独画散点填充覆盖不到的边缘区域 (如果插值有空洞)
    if np.sum(np.isnan(ZI)) > 0:
        ax.scatter(pts[:,0], pts[:,1], c=pwr, cmap=cmap, s=1, alpha=0.5,
                  vmin=-70, vmax=20)

    # Tx/Rx 标记
    if tx_pos: ax.plot(tx_pos[0], tx_pos[2] if plane_y is not None else tx_pos[1],
                       '*', color='white', markersize=14, markeredgecolor='black', markeredgewidth=1.5, zorder=5)
    if rx_pos: ax.plot(rx_pos[0], rx_pos[2] if plane_y is not None else rx_pos[1],
                       '^', color='cyan', markersize=10, markeredgecolor='black', markeredgewidth=1, zorder=5)

    cbar = plt.colorbar(im, ax=ax, label='Power (dBm)', shrink=0.85)
    cbar.ax.tick_params(labelsize=10)

    ax.set_xlabel(xlabel, fontsize=12)
    ax.set_ylabel(ylabel, fontsize=12)
    ax.set_title(f'SBR Coverage — {plane_label} (Tx=★ Rx=▲)', fontsize=14)
    ax.set_aspect('equal')
    ax.grid(True, alpha=0.15, linestyle='--')

    # 统计信息
    active_count = len(active)
    total_count = len(layer)
    median_pwr = np.median(pwr)
    ax.text(0.02, 0.98,
            f'{active_count}/{total_count} Rx active\nMedian: {median_pwr:.1f} dBm\nRange: {pwr.min():.0f}~{pwr.max():.0f} dBm',
            transform=ax.transAxes, fontsize=9, verticalalignment='top',
            bbox=dict(boxstyle='round', facecolor='white', alpha=0.85))

    fn = f'coverage_smooth_{"Y" if plane_y is not None else "Z"}{target:.1f}.png'
    fp = os.path.join(OUT, fn)
    plt.tight_layout()
    plt.savefig(fp, dpi=200, bbox_inches='tight')
    plt.close()

    print(f'{plane_label}: {total_count} Rx, {active_count} active → {fp}')
    print(f'  Power: {pwr.min():.0f}~{pwr.max():.0f} dBm, median {median_pwr:.1f}')

if __name__ == '__main__':
    import argparse
    p = argparse.ArgumentParser()
    p.add_argument('--plane-y', type=float)
    p.add_argument('--plane-z', type=float)
    p.add_argument('--interp', type=int, default=3)
    p.add_argument('--sbr', default=f'{BASE}/sbr_coverage.json')
    p.add_argument('--obj', default=OBJ_PATH)
    args = p.parse_args()

    tx = (16.0, 1.5, -12.0)
    rx = (10.0, 1.5, -10.0)

    plot_coverage(args.sbr, args.obj,
                  plane_y=args.plane_y, plane_z=args.plane_z,
                  interp_factor=args.interp, tx_pos=tx, rx_pos=rx)
