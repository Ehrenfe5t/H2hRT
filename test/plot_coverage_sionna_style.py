"""
Sionna RT 风格功率覆盖图: 建筑轮廓填充 + 全覆盖插值 + 专业配色
关键改进:
  1. 所有网格点都有值(NaN→最低功率填充)
  2. 建筑墙壁以填充多边形显示
  3. 仅开放空间显示功率热力
  4. 类似 Sionna scene_pathgain_topdown 风格
"""
import json, math, os
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.patches import Polygon
from scipy.interpolate import griddata

BASE = 'output/a1_real_chain/coverage'
OBJ_PATH = 'demo/meeting.obj'
OUT = 'output/plots'
os.makedirs(OUT, exist_ok=True)

def load_obj(filepath):
    verts, faces, objects = [], [], []
    cur_obj = "unknown"
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        for line in f:
            line = line.strip()
            if line.startswith('o '): cur_obj = line[2:].strip()
            elif line.startswith('v '):
                p = line.split(); verts.append((float(p[1]),float(p[2]),float(p[3])))
            elif line.startswith('f '):
                parts = line.split()[1:]
                idxs = [int(p.split('/')[0])-1 for p in parts]
                for i in range(1, len(idxs)-1):
                    faces.append((idxs[0], idxs[i], idxs[i+1], cur_obj))
    return verts, faces

def extract_wall_polygons(verts, faces, plane_axis, plane_val, tol=0.08):
    """提取指定平面处墙壁占用的多边形区域(用于填充)."""
    ax = {'x':0,'y':1,'z':2}[plane_axis]
    oa = [i for i in range(3) if i != ax]
    polygons = []
    for f in faces:
        v = [verts[f[i]] for i in range(3)]
        vals = [v[i][ax] for i in range(3)]
        # 三角面与平面相交会产生线段
        crossings = []
        for i in range(3):
            j = (i+1)%3
            if (vals[i]-plane_val)*(vals[j]-plane_val) < 0 and abs(vals[i]-vals[j])>1e-9:
                t = (plane_val-vals[i])/(vals[j]-vals[i])
                if 0<=t<=1:
                    cx = v[i][oa[0]]+t*(v[j][oa[0]]-v[i][oa[0]])
                    cy = v[i][oa[1]]+t*(v[j][oa[1]]-v[i][oa[1]])
                    crossings.append((cx,cy))
            elif abs(vals[i]-plane_val)<tol:
                crossings.append((v[i][oa[0]], v[i][oa[1]]))
        # 如果有>=3个交点, 形成多边形
        if len(crossings)>=3:
            unique=[]
            for c in crossings:
                if not any(abs(c[0]-u[0])<tol and abs(c[1]-u[1])<tol for u in unique):
                    unique.append(c)
            if len(unique)>=3:
                cx = sum(u[0] for u in unique)/len(unique)
                cy = sum(u[1] for u in unique)/len(unique)
                unique.sort(key=lambda u: math.atan2(u[1]-cy, u[0]-cx))
                polygons.append(unique)
    return polygons

def plot_sionna_style(plane_y=None, plane_z=None, interp=4):
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
        tx_xy, rx_xy = (16.0,-12.0), (10.0,-10.0)
    else:
        zs = sorted(set(r['z'] for r in records))
        target = min(zs, key=lambda z: abs(z-plane_z))
        layer = [r for r in records if abs(r['z']-target)<0.06]
        label = f'Z = {target:.2f} m'
        edge_ax, edge_val = 'z', target
        xl, yl = 'X (m)', 'Y (m)'
        tx_xy, rx_xy = (16.0,1.5), (10.0,1.5)

    # 全量Rx (含非激活): 激活→功率值, 非激活→floor_pwr
    all_pts = [(r['x'], r['z'] if plane_y is not None else r['y'],
                r['power_dBm'] if r['ray_hit_count']>0 else np.nan)
               for r in layer]
    if not all_pts: print(f'No Rx at {label}'); return

    pts_arr = np.array([(p[0],p[1]) for p in all_pts])
    pwr_arr = np.array([p[2] for p in all_pts])
    active_pwr = pwr_arr[~np.isnan(pwr_arr)]

    if len(active_pwr) == 0:
        print(f'No active Rx at {label}'); return

    floor_pwr = float(np.min(active_pwr)) - 3.0

    # 网格从全量Rx构建 (覆盖整个空间)
    xs = sorted(set(pts_arr[:,0]))
    ys_ = sorted(set(pts_arr[:,1]))
    xi = np.linspace(min(xs), max(xs), max(len(xs)*interp, 200))
    yi = np.linspace(min(ys_), max(ys_), max(len(ys_)*interp, 100))
    XI, YI = np.meshgrid(xi, yi)

    # 仅用激活点插值, NaN区域→floor_pwr
    active_mask = ~np.isnan(pwr_arr)
    pts_act = pts_arr[active_mask]; pwr_act = pwr_arr[active_mask]
    Z_cubic = griddata(pts_act, pwr_act, (XI, YI), method='cubic')
    Z_near = griddata(pts_act, pwr_act, (XI, YI), method='nearest')
    ZI = np.where(np.isnan(Z_cubic), Z_near, Z_cubic)
    ZI = np.where(np.isnan(ZI), floor_pwr, ZI)

    # 建筑物轮廓多边形 (用于灰色填充)
    wall_polys = extract_wall_polygons(verts, faces, edge_ax, edge_val)

    fig, ax = plt.subplots(figsize=(18, 8))

    # 1. 建筑墙壁灰色填充 (用多边形)
    for poly in wall_polys:
        if len(poly) >= 3:
            patch = Polygon(poly, facecolor='#3a3a3a', edgecolor='#555555',
                          linewidth=1.2, alpha=0.85, zorder=2)
            ax.add_patch(patch)

    # 2. 功率热力 (在墙壁之上, 开放空间可见)
    im = ax.pcolormesh(XI, YI, ZI, cmap='jet', shading='auto',
                       vmin=floor_pwr, vmax=max(np.max(active_pwr), floor_pwr+20),
                       alpha=0.92, zorder=1)

    # 3. 墙壁轮廓线 (加强)
    for poly in wall_polys:
        if len(poly) >= 3:
            poly_closed = list(poly) + [poly[0]]
            xs_p = [p[0] for p in poly_closed]
            ys_p = [p[1] for p in poly_closed]
            ax.plot(xs_p, ys_p, 'k-', linewidth=1.8, alpha=0.9, zorder=3)

    # 4. Tx/Rx 标记
    ax.plot(tx_xy[0], tx_xy[1], '*', color='white', markersize=18,
            markeredgecolor='black', markeredgewidth=2, zorder=10)
    ax.plot(rx_xy[0], rx_xy[1], 'D', color='#00ff88', markersize=10,
            markeredgecolor='black', markeredgewidth=1.5, zorder=10)

    # Colorbar
    cbar = plt.colorbar(im, ax=ax, label='Received Power (dBm)', shrink=0.8)
    cbar.ax.tick_params(labelsize=10)

    ax.set_xlabel(xl, fontsize=13)
    ax.set_ylabel(yl, fontsize=13)
    ax.set_title(f'SBR Power Coverage — {label}     ★ Tx   ◆ Rx', fontsize=14, fontweight='bold')
    ax.set_aspect('equal')
    ax.grid(False)

    # 图例
    active_n = int(np.sum(active_mask))
    total_n = len(all_pts)
    ax.text(0.015, 0.985,
        f'Active: {active_n}/{total_n} Rx ({100*active_n//total_n}%)  |  '
        f'Median: {np.median(active_pwr):.1f} dBm  |  '
        f'Floor: {floor_pwr:.0f} dBm  |  '
        f'Tx=0dBm, 2.4GHz, 200K rays',
        transform=ax.transAxes, fontsize=9, va='top',
        bbox=dict(boxstyle='round', facecolor='white', alpha=0.85))

    fn = f'coverage_sionna_{"Y" if plane_y else "Z"}{target:.1f}.png'
    fp = os.path.join(OUT, fn)
    plt.tight_layout()
    plt.savefig(fp, dpi=200, bbox_inches='tight')
    plt.close()
    print(f'{label}: {active_n}/{total_n} → {fp}')

if __name__ == '__main__':
    for y in [0.8, 1.2, 1.5, 1.8, 2.2]:
        plot_sionna_style(plane_y=y)
    for z in [-12.0, -11.0, -10.0, -9.0, -8.0]:
        plot_sionna_style(plane_z=z)
    print(f'\nDone → {OUT}/')
