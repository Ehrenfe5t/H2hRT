"""
功率覆盖最终版: 平滑插值 + 三角面轮廓叠加 + 任意平面切片
"""
import json, math, os
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from scipy.interpolate import griddata

BASE = 'output/a1_real_chain/coverage'
OBJ_PATH = 'demo/meeting.obj'
OUT = 'output/plots'
os.makedirs(OUT, exist_ok=True)

def load_obj(filepath):
    vertices, faces = [], []
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        for line in f:
            line = line.strip()
            if line.startswith('v '):
                p = line.split()
                vertices.append((float(p[1]), float(p[2]), float(p[3])))
            elif line.startswith('f '):
                parts = line.split()[1:]
                idxs = [int(p.split('/')[0])-1 for p in parts]
                for i in range(1, len(idxs)-1):
                    faces.append((idxs[0], idxs[i], idxs[i+1]))
    return vertices, faces

def extract_slice_edges(vertices, faces, plane_axis, plane_val, tol=0.05):
    """提取三角面与指定平面的交线."""
    ax = {'x':0,'y':1,'z':2}[plane_axis]
    oa = [i for i in range(3) if i != ax]
    segments = []
    for f in faces:
        v = [vertices[f[i]] for i in range(3)]
        vals = [v[i][ax] for i in range(3)]
        crossings = []
        for i in range(3):
            j = (i+1)%3
            if (vals[i]-plane_val)*(vals[j]-plane_val) <= 0 and abs(vals[i]-vals[j])>1e-9:
                t = (plane_val - vals[i])/(vals[j]-vals[i])
                if 0 <= t <= 1:
                    cx = v[i][oa[0]] + t*(v[j][oa[0]]-v[i][oa[0]])
                    cy = v[i][oa[1]] + t*(v[j][oa[1]]-v[i][oa[1]])
                    crossings.append((cx, cy))
            elif abs(vals[i]-plane_val) < tol:
                crossings.append((v[i][oa[0]], v[i][oa[1]]))
        # 去重并形成线段
        if len(crossings) >= 2:
            # 取前两个唯一点
            unique = []
            for c in crossings:
                if not any(abs(c[0]-u[0])<tol and abs(c[1]-u[1])<tol for u in unique):
                    unique.append(c)
            for i in range(0, len(unique)-1, 2):
                if i+1 < len(unique):
                    segments.append((unique[i], unique[i+1]))
    return segments

def plot(plane_y=None, plane_z=None, interp=2):
    with open(f'{BASE}/sbr_coverage.json') as f:
        sbr = json.load(f)
    verts, faces = load_obj(OBJ_PATH)
    records = sbr['records']

    if plane_y is not None:
        ys = sorted(set(r['y'] for r in records))
        target = min(ys, key=lambda y: abs(y-plane_y))
        layer = [r for r in records if abs(r['y']-target)<0.06]
        label = f'Y = {target:.2f} m'
        edge_ax, edge_val = 'y', target
        xl, yl = 'X (m)', 'Z (m)'
    else:
        zs = sorted(set(r['z'] for r in records))
        target = min(zs, key=lambda z: abs(z-plane_z))
        layer = [r for r in records if abs(r['z']-target)<0.06]
        label = f'Z = {target:.2f} m'
        edge_ax, edge_val = 'z', target
        xl, yl = 'X (m)', 'Y (m)'

    active = [r for r in layer if r['ray_hit_count']>0]
    if not active:
        print(f'No active Rx at {label}'); return

    # 插值网格 (更高密度)
    pts = np.array([[r['x'], r['z'] if plane_y is not None else r['y']] for r in active])
    pwr = np.array([r['power_dBm'] for r in active])
    nx = int((pts[:,0].max()-pts[:,0].min())/0.02)*interp
    ny = int((pts[:,1].max()-pts[:,1].min())/0.02)*interp
    xi = np.linspace(pts[:,0].min(), pts[:,0].max(), max(nx, 100))
    yi = np.linspace(pts[:,1].min(), pts[:,1].max(), max(ny, 50))
    XI, YI = np.meshgrid(xi, yi)
    ZI = griddata(pts, pwr, (XI, YI), method='cubic', fill_value=np.nan)

    # 建筑物轮廓
    segs = extract_slice_edges(verts, faces, edge_ax, edge_val)

    fig, ax = plt.subplots(figsize=(18, 8))
    cmap = plt.cm.jet.copy()
    cmap.set_bad((1,1,1,0))
    im = ax.pcolormesh(XI, YI, ZI, cmap=cmap, shading='auto', vmin=-70, vmax=25)

    # 建筑轮廓 (加粗深色线)
    for (x1,y1),(x2,y2) in segs:
        ax.plot([x1,x2], [y1,y2], 'k-', linewidth=1.5, alpha=0.7, solid_capstyle='round')

    # Tx/Rx
    tx = (16.0, 1.5, -12.0)
    rx = (10.0, 1.5, -10.0)
    txy = tx[2] if plane_y is not None else tx[1]
    rxy = rx[2] if plane_y is not None else rx[1]
    ax.plot(tx[0], txy, '*', color='white', markersize=16, markeredgecolor='black', markeredgewidth=1.5, zorder=10)
    ax.plot(rx[0], rxy, 'D', color='lime', markersize=8, markeredgecolor='black', markeredgewidth=1, zorder=10)

    cbar = plt.colorbar(im, ax=ax, label='Received Power (dBm)', shrink=0.8)
    ax.set_xlabel(xl, fontsize=13); ax.set_ylabel(yl, fontsize=13)
    ax.set_title(f'SBR Power Coverage — {label}     Tx=★  Rx=◆', fontsize=14, fontweight='bold')
    ax.set_aspect('equal')
    ax.grid(True, alpha=0.12, linestyle=':')

    # 统计
    median_pwr = np.median(pwr)
    ax.text(0.015, 0.985,
        f'Active Rx: {len(active)}/{len(layer)}  |  '
        f'Median: {median_pwr:.1f} dBm  |  '
        f'Range: [{pwr.min():.0f}, {pwr.max():.0f}] dBm',
        transform=ax.transAxes, fontsize=9, va='top',
        bbox=dict(boxstyle='round', facecolor='white', alpha=0.85))

    fn = f'coverage_final_{"Y" if plane_y else "Z"}{target:.1f}.png'
    fp = os.path.join(OUT, fn)
    plt.tight_layout()
    plt.savefig(fp, dpi=200, bbox_inches='tight')
    plt.close()
    print(f'{label}: {len(active)}/{len(layer)} active, median={median_pwr:.1f}dBm → {fp}')

if __name__ == '__main__':
    for y in [0.8, 1.2, 1.5, 1.8, 2.2]:
        plot(plane_y=y)
    for z in [-11.0, -10.0, -9.0, -8.0]:
        plot(plane_z=z)
    print('\nDone. All plots in', OUT)
