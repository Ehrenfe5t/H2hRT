#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
RT 场景可视化工具 (matplotlib版, 无需PyQt)
用法: python test/rt_visual_mpl.py [obj_file] [paths_json]

默认:
  obj = demo/meeting.obj (或 demo/412/412-6k.obj)
  paths = output/412/rx1/paths/precise_paths.json
"""

import sys, json, os
from pathlib import Path
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from mpl_toolkits.mplot3d.art3d import Poly3DCollection

ROOT = Path(__file__).resolve().parents[1]

# ── 默认路径 ──
OBJ_FILE = ROOT / "demo" / "412" / "412-6k.obj"
PATHS_FILE = ROOT / "output" / "412" / "rx1" / "paths" / "precise_paths.json"

def load_obj(obj_path):
    """加载OBJ文件, 返回vertices和faces"""
    verts, faces = [], []
    with open(obj_path, 'r', encoding='utf-8', errors='replace') as f:
        for line in f:
            if line.startswith('v '):
                parts = line.split()
                verts.append([float(parts[1]), float(parts[2]), float(parts[3])])
            elif line.startswith('f '):
                parts = line.split()
                idxs = []
                for p in parts[1:]:
                    idxs.append(int(p.split('/')[0]) - 1)
                if len(idxs) == 3:
                    faces.append(idxs)
                elif len(idxs) == 4:
                    faces.append([idxs[0], idxs[1], idxs[2]])
                    faces.append([idxs[0], idxs[2], idxs[3]])
    return np.array(verts), np.array(faces)

def load_paths(paths_file):
    """加载路径JSON"""
    if not os.path.exists(paths_file):
        return []
    with open(paths_file, 'r', encoding='utf-8') as f:
        data = json.load(f)
    return data.get('paths', [])

def plot_scene(verts, faces, paths=None, max_paths=30):
    """绘制场景 + 路径"""
    fig = plt.figure(figsize=(16, 10))
    ax = fig.add_subplot(111, projection='3d')

    # 场景网格 (半透明)
    mesh = Poly3DCollection(verts[faces], alpha=0.15, facecolor='lightgray',
                            edgecolor='gray', linewidth=0.1)
    ax.add_collection3d(mesh)

    # 自动缩放
    if len(verts) > 0:
        xlim = (verts[:,0].min(), verts[:,0].max())
        ylim = (verts[:,1].min(), verts[:,1].max())
        zlim = (verts[:,2].min(), verts[:,2].max())
        ax.set_xlim(xlim); ax.set_ylim(ylim); ax.set_zlim(zlim)

    # 路径
    colors = plt.cm.turbo(np.linspace(0, 1, max(1, min(max_paths, len(paths or [])))))
    if paths:
        for i, p in enumerate(paths[:max_paths]):
            nodes = p.get('geometry_nodes', [])
            if len(nodes) < 2: continue
            pts = np.array([[n['x'], n['y'], n['z']] for n in nodes])
            ax.plot(pts[:,0], pts[:,1], pts[:,2], color=colors[i], linewidth=0.8, alpha=0.7)

            # 标记交互类型
            for n in nodes:
                it = n.get('interaction_type', 0)
                if it in (4, 5, 6):
                    marker = {4: 's', 5: '^', 6: 'D'}.get(it, 'o')
                    size = {4: 15, 5: 25, 6: 20}.get(it, 10)
                    clr = {4: 'orange', 5: 'green', 6: 'purple'}.get(it, 'red')
                    ax.scatter(n['x'], n['y'], n['z'], c=clr, marker=marker, s=size, alpha=0.9)

            # Tx/Rx 标记
            if nodes:
                ax.scatter(nodes[0]['x'], nodes[0]['y'], nodes[0]['z'], c='red', marker='*', s=200, label='Tx' if i==0 else '')
                ax.scatter(nodes[-1]['x'], nodes[-1]['y'], nodes[-1]['z'], c='blue', marker='*', s=200, label='Rx' if i==0 else '')

    ax.set_xlabel('X (m)'); ax.set_ylabel('Y (m)'); ax.set_zlabel('Z (m)')
    ax.set_title(f'RT Scene + {min(max_paths, len(paths or []))} paths')
    handles, labels = ax.get_legend_handles_labels()
    if handles:
        ax.legend(handles[:2], labels[:2], loc='upper right')
    plt.tight_layout()
    return fig, ax

def main():
    obj_path = Path(sys.argv[1]) if len(sys.argv) > 1 else OBJ_FILE
    paths_path = Path(sys.argv[2]) if len(sys.argv) > 2 else PATHS_FILE

    print(f"Loading OBJ: {obj_path}")
    if not obj_path.exists():
        print(f"ERROR: {obj_path} not found. Available OBJs:")
        for f in sorted((ROOT / "demo").rglob("*.obj")):
            print(f"  {f}")
        sys.exit(1)

    verts, faces = load_obj(obj_path)
    print(f"  Vertices: {len(verts)}, Faces: {len(faces)}")

    paths = load_paths(paths_path)
    print(f"  Paths: {len(paths)}")

    fig, ax = plot_scene(verts, faces, paths, max_paths=30)
    plt.show()

if __name__ == '__main__':
    main()
