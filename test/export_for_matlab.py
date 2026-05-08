"""导出SBR覆盖数据为MATLAB .mat格式 + 建筑物轮廓"""
import json, math, os, sys
import numpy as np
from scipy.io import savemat

BASE = 'output/a1_real_chain/coverage'
OBJ_PATH = 'demo/meeting.obj'
OUT = 'output/matlab'
os.makedirs(OUT, exist_ok=True)

# 1. 加载SBR数据
with open(f'{BASE}/sbr_coverage.json') as f:
    sbr = json.load(f)
records = sbr['records']

# 提取XYZ和功率
n = len(records)
xs = np.zeros(n); ys = np.zeros(n); zs = np.zeros(n)
powers = np.zeros(n); hits = np.zeros(n, dtype=int)
for i, r in enumerate(records):
    xs[i] = r['x']; ys[i] = r['y']; zs[i] = r['z']
    powers[i] = r['power_dBm']; hits[i] = r['ray_hit_count']

# 2. 加载OBJ, 提取三角面
verts = []; faces = []
with open(OBJ_PATH, 'r', encoding='utf-8', errors='ignore') as f:
    for line in f:
        line = line.strip()
        if line.startswith('v '):
            p = line.split()
            verts.append([float(p[1]), float(p[2]), float(p[3])])
        elif line.startswith('f '):
            parts = line.split()[1:]
            idxs = [int(p.split('/')[0])-1 for p in parts]
            for i in range(1, len(idxs)-1):
                faces.append([idxs[0], idxs[i], idxs[i+1]])

verts = np.array(verts)
faces = np.array(faces, dtype=np.int32)

# 3. 场景边界
scene_x = [xs.min(), xs.max()]
scene_y = [ys.min(), ys.max()]
scene_z = [zs.min(), zs.max()]

savemat(os.path.join(OUT, 'sbr_coverage.mat'), {
    'rx_x': xs, 'rx_y': ys, 'rx_z': zs,
    'power_dBm': powers, 'ray_hits': hits,
    'n_total': n, 'n_active': int(np.sum(hits > 0)),
    'obj_vertices': verts, 'obj_faces': faces,
    'scene_xlim': scene_x, 'scene_ylim': scene_y, 'scene_zlim': scene_z,
    'tx_pos': np.array([16.0, 1.5, -12.0]),
    'rx_pos': np.array([10.0, 1.5, -10.0]),
    'freq_ghz': 2.4,
})
print(f'Exported to {OUT}/sbr_coverage.mat')
print(f'  Rx: {n} total, {int(np.sum(hits>0))} active')
print(f'  Vertices: {len(verts)}, Faces: {len(faces)}')
