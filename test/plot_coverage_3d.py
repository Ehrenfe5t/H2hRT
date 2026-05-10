#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
RT 功率覆盖 3D 交互可视化
=========================
基于 coverage.py 的 UI/功能布局, 引入 scene_positions.py 渲染风格。

功能:
  - 3D 场景建筑面元渲染 (smooth_shading + 3光源系统)
  - 天花板裁剪 (去顶): X/Y/Z 轴可调, 立刻预览
  - 功率覆盖切片: 选择 X/Y/Z 平面 + 拖动滑块位置, 该平面功率热力图
  - 无平滑: 每个 Rx 原始功率值直接 StructuredGrid 显示
  - 全空间: 无功率 Rx 按 colorbar 最低值绘制, 不空白
  - Tx/Rx 天线球标记

用法:
  python test/plot_coverage_3d.py [--obj PATH] [--sbr PATH]
"""
import json, os, sys, warnings
from pathlib import Path

import numpy as np

warnings.filterwarnings("ignore", category=DeprecationWarning)

from PyQt5.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout,
    QHBoxLayout, QLabel, QCheckBox, QGroupBox, QComboBox, QDoubleSpinBox)
import pyvista as pv
from pyvistaqt import QtInteractor
import trimesh

GROUP_BOX_STYLE = """
    QGroupBox { border: 1px solid #cccccc; border-radius: 6px;
        padding-top: 10px; margin-top: 8px; font-size: 12px; }
    QGroupBox::title { subcontrol-origin: margin; left: 15px;
        padding: 0 8px 0 8px; color: #555555; }
"""

MODULE_DIR = Path(__file__).resolve().parent
REPO_ROOT = MODULE_DIR.parent
DEFAULT_OBJ = str(REPO_ROOT / "demo" / "meeting.obj")
DEFAULT_SBR = str(REPO_ROOT / "output" / "a1_real_chain" / "coverage" / "sbr_coverage.json")


def make_clip_box(bounds_dict, axis, threshold):
    x0, x1 = bounds_dict["X"]; y0, y1 = bounds_dict["Y"]; z0, z1 = bounds_dict["Z"]
    if axis == "X": x1 = threshold
    elif axis == "Y": y1 = threshold
    else: z1 = threshold
    return [x0, x1, y0, y1, z0, z1]


class CoverageWindow(QMainWindow):
    """3D 场景 + 功率覆盖切片 交互窗口。"""

    def __init__(self, obj_path, sbr_path):
        super().__init__()
        self.obj_path = obj_path
        self.sbr_path = sbr_path
        self.setWindowTitle("RT Power Coverage")
        self.setGeometry(100, 50, 1800, 1000)

        # 场景
        self.verts = None; self.tris = None; self.bounds = {}
        self.clip_on = True; self.clip_axis = "Y"; self.clip_val = 100.0
        self.surf_color = "lightgray"; self.surf_opacity = 0.92
        self.wire_color = "gray"; self.wire_width = 3.0

        # 功率覆盖
        self.rx_x = None; self.rx_y = None; self.rx_z = None
        self.rx_pwr = None; self.rx_hit = None
        self.slice_axis = "Y"; self.slice_val = 1.5
        self.show_power = True; self.power_opacity = 0.55

        self._load_obj()
        self._load_sbr()
        self._init_ui()
        self._render()

    # ── 数据 ──

    def _load_obj(self):
        mesh = trimesh.load(self.obj_path, process=False,
                           skip_materials=True, skip_textures=True)
        self.verts = mesh.vertices.astype(np.float32)
        self.tris = mesh.faces.astype(np.int32)
        v = self.verts
        self.bounds = {"X": (float(v[:, 0].min()), float(v[:, 0].max())),
                       "Y": (float(v[:, 1].min()), float(v[:, 1].max())),
                       "Z": (float(v[:, 2].min()), float(v[:, 2].max()))}
        self.clip_val = self.bounds["Y"][1] - 0.05

    def _load_sbr(self):
        with open(self.sbr_path, 'r') as f:
            sbr = json.load(f)
        records = sbr.get('records', [])
        n = len(records)
        self.rx_x = np.zeros(n); self.rx_y = np.zeros(n); self.rx_z = np.zeros(n)
        self.rx_pwr = np.zeros(n); self.rx_hit = np.zeros(n, dtype=bool)
        for i, r in enumerate(records):
            self.rx_x[i] = float(r.get('x', 0))
            self.rx_y[i] = float(r.get('y', 0))
            self.rx_z[i] = float(r.get('z', 0))
            self.rx_pwr[i] = float(r.get('power_dBm', -200))
            self.rx_hit[i] = int(r.get('ray_hit_count', 0)) > 0
        # Tx/Rx 位置: JSON 自动检测, 回退默认
        if 'tx_position' in sbr:
            tp = sbr['tx_position']
            self.tx = (float(tp[0]), float(tp[1]), float(tp[2]))
        else:
            self.tx = (16.0, 1.5, -12.0)
        if 'rx_positions' in sbr and len(sbr['rx_positions']) > 0:
            rp = sbr['rx_positions'][0]
            self.rx = (float(rp[0]), float(rp[1]), float(rp[2]))
        else:
            self.rx = (10.0, 1.5, -10.0)
        self.slice_val = round((self.bounds["Y"][0] + self.bounds["Y"][1]) / 2, 1)

    # ── UI (匹配 coverage.py 布局) ──

    def _init_ui(self):
        main_widget = QWidget()
        self.setCentralWidget(main_widget)
        layout = QVBoxLayout(main_widget)
        layout.setContentsMargins(0, 0, 0, 0); layout.setSpacing(0)

        ctrl = QWidget(); ctrl.setMaximumHeight(130)
        ctrl_outer = QVBoxLayout(ctrl)
        ctrl_outer.setContentsMargins(15, 6, 15, 6); ctrl_outer.setSpacing(6)

        # Row1: 裁剪 + 切片
        row1 = QHBoxLayout()

        clip_group = QGroupBox("天花板裁剪")
        clip_group.setStyleSheet(GROUP_BOX_STYLE)
        cl = QHBoxLayout(clip_group); cl.setContentsMargins(15, 5, 15, 5)
        self.clip_check = QCheckBox("启用"); self.clip_check.setChecked(self.clip_on)
        self.clip_check.stateChanged.connect(self._on_update)
        cl.addWidget(self.clip_check)
        cl.addWidget(QLabel("轴:"))
        self.clip_axis_combo = QComboBox(); self.clip_axis_combo.addItems(["Y", "Z", "X"])
        self.clip_axis_combo.currentTextChanged.connect(self._on_clip_axis)
        cl.addWidget(self.clip_axis_combo)
        cl.addWidget(QLabel("阈值:"))
        self.clip_val_spin = QDoubleSpinBox()
        self.clip_val_spin.setRange(-1e5, 1e5); self.clip_val_spin.setDecimals(2)
        self.clip_val_spin.setValue(self.clip_val)
        self.clip_val_spin.valueChanged.connect(self._on_update)
        cl.addWidget(self.clip_val_spin)
        row1.addWidget(clip_group)

        slice_group = QGroupBox("功率切片")
        slice_group.setStyleSheet(GROUP_BOX_STYLE)
        sl = QHBoxLayout(slice_group); sl.setContentsMargins(15, 5, 15, 5)
        self.pwr_check = QCheckBox("显示功率"); self.pwr_check.setChecked(self.show_power)
        self.pwr_check.stateChanged.connect(self._on_update)
        sl.addWidget(self.pwr_check)
        sl.addWidget(QLabel("轴:"))
        self.slice_axis_combo = QComboBox(); self.slice_axis_combo.addItems(["Y", "Z", "X"])
        self.slice_axis_combo.currentTextChanged.connect(self._on_slice_axis)
        sl.addWidget(self.slice_axis_combo)
        sl.addWidget(QLabel("位置:"))
        self.slice_val_spin = QDoubleSpinBox()
        self.slice_val_spin.setRange(-1e5, 1e5); self.slice_val_spin.setDecimals(2)
        self.slice_val_spin.setValue(self.slice_val)
        self.slice_val_spin.valueChanged.connect(self._on_update)
        sl.addWidget(self.slice_val_spin)
        sl.addWidget(QLabel("透明度:"))
        self.pwr_opacity_spin = QDoubleSpinBox()
        self.pwr_opacity_spin.setRange(0.1, 1.0); self.pwr_opacity_spin.setSingleStep(0.05)
        self.pwr_opacity_spin.setValue(self.power_opacity)
        self.pwr_opacity_spin.valueChanged.connect(self._on_update)
        sl.addWidget(self.pwr_opacity_spin)
        row1.addWidget(slice_group)
        row1.addStretch()
        ctrl_outer.addLayout(row1)

        # Row2: 样式
        row2 = QHBoxLayout()
        style_group = QGroupBox("面元/边线样式")
        style_group.setStyleSheet(GROUP_BOX_STYLE)
        st = QHBoxLayout(style_group); st.setContentsMargins(15, 5, 15, 5)
        st.addWidget(QLabel("面色:"))
        self.surf_color_combo = QComboBox()
        self.surf_color_combo.addItems(["lightgray", "#c7ccd6", "white", "#e8e8e8", "#d0d0d0"])
        self.surf_color_combo.setCurrentText(self.surf_color)
        self.surf_color_combo.currentTextChanged.connect(self._on_style)
        st.addWidget(self.surf_color_combo)
        st.addWidget(QLabel("不透明:"))
        self.surf_opacity_spin = QDoubleSpinBox()
        self.surf_opacity_spin.setRange(0.1, 1.0); self.surf_opacity_spin.setSingleStep(0.05)
        self.surf_opacity_spin.setValue(self.surf_opacity)
        self.surf_opacity_spin.valueChanged.connect(self._on_style)
        st.addWidget(self.surf_opacity_spin)
        st.addWidget(QLabel("边线色:"))
        self.wire_color_combo = QComboBox()
        self.wire_color_combo.addItems(["gray", "#9a9fa8", "#666666", "black", "white", "none"])
        self.wire_color_combo.setCurrentText(self.wire_color)
        self.wire_color_combo.currentTextChanged.connect(self._on_style)
        st.addWidget(self.wire_color_combo)
        st.addWidget(QLabel("边线宽:"))
        self.wire_width_spin = QDoubleSpinBox()
        self.wire_width_spin.setRange(0.0, 5.0); self.wire_width_spin.setSingleStep(0.1)
        self.wire_width_spin.setValue(self.wire_width)
        self.wire_width_spin.valueChanged.connect(self._on_style)
        st.addWidget(self.wire_width_spin)
        row2.addWidget(style_group)
        row2.addStretch()
        ctrl_outer.addLayout(row2)

        layout.addWidget(ctrl)
        self.plotter = QtInteractor(main_widget)
        layout.addWidget(self.plotter, stretch=1)

    # ── 渲染器 (scene_positions.py 风格) ──

    def _init_renderer(self):
        p = self.plotter
        p.enable_anti_aliasing()
        p.background_color = "#f5f5f5"
        p.renderer.lights.clear()
        light_kw = dict(light_type="camera light", intensity=0.6)
        p.renderer.add_light(pv.Light(position=(0, 1, 1), **light_kw))
        p.renderer.add_light(pv.Light(position=(1, 0.5, 0.5), **light_kw))
        p.renderer.add_light(pv.Light(position=(-0.5, 0.3, -0.5), **light_kw))

    # ── 渲染 ──

    def _render(self):
        self.plotter.clear()
        self._init_renderer()

        # 1. 场景几何
        pd = pv.PolyData.from_regular_faces(self.verts, self.tris)
        if self.clip_on:
            cb = make_clip_box(self.bounds, self.clip_axis, self.clip_val)
            pd = pd.clip_box(cb, invert=False)
        if pd.n_points > 0:
            pd = pd.extract_surface()
            pd = pd.compute_normals(auto_orient_normals=True, consistent_normals=True,
                                    split_vertices=True)
            self.plotter.add_mesh(pd, color=self.surf_color, opacity=self.surf_opacity,
                                  style="surface", smooth_shading=True,
                                  ambient=0.15, diffuse=0.75, specular=0.3,
                                  specular_power=15.0, reset_camera=True)
            if self.wire_color != "none" and self.wire_width > 0:
                edges = pd.extract_all_edges()
                self.plotter.add_mesh(edges, color=self.wire_color,
                                      line_width=self.wire_width, style="wireframe",
                                      ambient=0.1, reset_camera=False)

        # 2. 功率覆盖层 (StructuredGrid, 无平滑, 全空间填充)
        if self.show_power and self.rx_x is not None:
            axis_idx = {"X": 0, "Y": 1, "Z": 2}[self.slice_axis]
            free = [i for i in range(3) if i != axis_idx]
            ax_data = {0: self.rx_x, 1: self.rx_y, 2: self.rx_z}[axis_idx]

            mask = np.abs(ax_data - self.slice_val) < 0.08
            n_plane = int(np.sum(mask))
            if n_plane >= 4:
                c0 = ({0: self.rx_x, 1: self.rx_y, 2: self.rx_z}[free[0]])[mask]
                c1 = ({0: self.rx_x, 1: self.rx_y, 2: self.rx_z}[free[1]])[mask]
                pp = self.rx_pwr[mask]
                ph = self.rx_hit[mask]

                u0 = np.sort(np.unique(np.round(c0, 3)))
                u1 = np.sort(np.unique(np.round(c1, 3)))
                n0, n1 = len(u0), len(u1)

                gp = np.full((n1, n0), np.nan)
                gh = np.full((n1, n0), False)
                for i in range(len(pp)):
                    i0 = np.searchsorted(u0, round(c0[i], 3))
                    i1 = np.searchsorted(u1, round(c1[i], 3))
                    if 0 <= i0 < n0 and 0 <= i1 < n1:
                        gp[i1, i0] = pp[i]
                        gh[i1, i0] = ph[i]

                # 功率范围: 仅统计有命中的 Rx
                hit_pwr = gp[gh]
                pwr_min = float(np.min(hit_pwr)) if len(hit_pwr) > 0 else -200.0
                pwr_max = float(np.max(hit_pwr)) if len(hit_pwr) > 0 else -50.0
                if pwr_max <= pwr_min:
                    pwr_max = pwr_min + 10.0
                # 全空间覆盖: NaN 或 无命中 → clim 最低值
                gp_fill = np.where(np.isnan(gp) | ~gh, pwr_min - 3.0, gp)

                pts = np.zeros((n0 * n1, 3))
                for j in range(n1):
                    for i in range(n0):
                        idx = j * n0 + i
                        pts[idx, free[0]] = u0[i]
                        pts[idx, free[1]] = u1[j]
                        pts[idx, axis_idx] = self.slice_val

                sg = pv.StructuredGrid()
                sg.dimensions = (n0, n1, 1)
                sg.points = pts
                sg.point_data["power"] = gp_fill.ravel(order='C')

                self.plotter.add_mesh(sg, scalars="power", cmap="jet",
                    clim=[pwr_min, pwr_max], opacity=self.power_opacity,
                    style="surface", ambient=0.3,
                    reset_camera=False, show_scalar_bar=True,
                    scalar_bar_args={"title": "Power (dBm)", "font_family": "times",
                                     "height": 0.5})

        # 3. Tx / Rx 标记
        self.plotter.add_mesh(pv.Sphere(center=self.tx, radius=0.30),
            color="white", style="surface", ambient=0.5, reset_camera=False)
        self.plotter.add_mesh(pv.Sphere(center=self.rx, radius=0.25),
            color="#00ff88", style="surface", ambient=0.5, reset_camera=False)

        self.plotter.add_axes()
        self.plotter.reset_camera()
        self.plotter.update()

    # ── 回调 ──

    def _on_update(self, *a):
        self.clip_on = self.clip_check.isChecked()
        self.clip_val = self.clip_val_spin.value()
        self.show_power = self.pwr_check.isChecked()
        self.slice_val = self.slice_val_spin.value()
        self.power_opacity = self.pwr_opacity_spin.value()
        self._render()

    def _on_style(self, *a):
        self.surf_color = self.surf_color_combo.currentText()
        self.surf_opacity = self.surf_opacity_spin.value()
        self.wire_color = self.wire_color_combo.currentText()
        self.wire_width = self.wire_width_spin.value()
        self._render()

    def _on_clip_axis(self, ax):
        self.clip_axis = ax
        self.clip_val_spin.setValue(self.bounds[ax][1] - 0.05)
        self._render()

    def _on_slice_axis(self, ax):
        self.slice_axis = ax
        mid = round((self.bounds[ax][0] + self.bounds[ax][1]) / 2, 1)
        self.slice_val_spin.setValue(mid)
        self._render()


if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(description="3D Coverage Visualization")
    parser.add_argument("--obj", type=str, default=None)
    parser.add_argument("--sbr", type=str, default=None)
    args = parser.parse_args()

    OBJ = args.obj or DEFAULT_OBJ
    SBR = args.sbr or DEFAULT_SBR
    # 回退搜索
    if not os.path.exists(OBJ):
        alt = os.path.join(os.path.dirname(__file__), "..", "demo/meeting.obj")
        if os.path.exists(alt): OBJ = alt
    if not os.path.exists(SBR):
        alt = os.path.join(os.path.dirname(__file__), "..",
                          "output/meeting-cov-hires/coverage/sbr_coverage.json")
        if os.path.exists(alt): SBR = alt
    for p in [OBJ, SBR]:
        if not os.path.exists(p):
            print(f"File not found: {p}"); sys.exit(1)

    app = QApplication(sys.argv)
    win = CoverageWindow(os.path.abspath(OBJ), os.path.abspath(SBR))
    win.show()
    sys.exit(app.exec_())
