#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Interactive 3D viewer for RT coverage and geometric paths.

This keeps the original coverage viewer's scene rendering style:
OBJ mesh + clipping + smooth shading + optional coverage slice.
The V10 addition is an optional path overlay loaded from
output/<run_id>/<rx_id>/paths/precise_paths.json.

Examples
--------
python test/plot_coverage_3d.py

python test/plot_coverage_3d.py \
  --obj demo/412/412-6k.obj \
  --sbr output/v10_finechannel_pointcheck_tmp/coverage/sbr_coverage.json \
  --paths output/v10_finechannel_pointcheck_tmp/rx1/paths/precise_paths.json

python test/plot_coverage_3d.py \
  --run-dir output/v10_finechannel_pointcheck_tmp --rx rx1 --path-limit 300
"""

from __future__ import annotations

import argparse
import json
import os
import sys
import warnings
from collections import Counter
from pathlib import Path
from typing import Dict, Iterable, List, Optional, Sequence, Tuple

import numpy as np

warnings.filterwarnings("ignore", category=DeprecationWarning)

from qtpy.QtWidgets import (
    QApplication,
    QCheckBox,
    QComboBox,
    QDoubleSpinBox,
    QGroupBox,
    QHBoxLayout,
    QLabel,
    QMainWindow,
    QSpinBox,
    QVBoxLayout,
    QWidget,
)
import pyvista as pv
from pyvistaqt import QtInteractor
import trimesh


MODULE_DIR = Path(__file__).resolve().parent
REPO_ROOT = MODULE_DIR.parent
DEFAULT_OBJ = REPO_ROOT / "demo" / "412" / "412-6k.obj"
DEFAULT_SBR = REPO_ROOT / "output" / "v10_finechannel_template" / "coverage" / "sbr_coverage.json"
DEFAULT_PATHS = REPO_ROOT / "output" / "v10_sbr_ptp_template" / "rx1" / "paths" / "precise_paths.json"

GROUP_BOX_STYLE = """
    QGroupBox {
        border: 1px solid #cccccc;
        border-radius: 6px;
        padding-top: 10px;
        margin-top: 8px;
        font-size: 12px;
    }
    QGroupBox::title {
        subcontrol-origin: margin;
        left: 15px;
        padding: 0 8px 0 8px;
        color: #555555;
    }
"""

PATH_COLORS = {
    3: "#ff9900",  # LOS / final free-space segment
    4: "#ffcc33",  # Reflection
    5: "#45c66f",  # Transmission
    6: "#b56cff",  # Diffraction
}

NODE_COLORS = {
    4: "#ffcc33",
    5: "#45c66f",
    6: "#b56cff",
}


def load_json(path: Path) -> Dict:
    with path.open("r", encoding="utf-8") as f:
        return json.load(f)


def make_clip_box(bounds_dict: Dict[str, Tuple[float, float]], axis: str, threshold: float) -> List[float]:
    x0, x1 = bounds_dict["X"]
    y0, y1 = bounds_dict["Y"]
    z0, z1 = bounds_dict["Z"]
    if axis == "X":
        x1 = threshold
    elif axis == "Y":
        y1 = threshold
    else:
        z1 = threshold
    return [x0, x1, y0, y1, z0, z1]


def point_from_node(node: Dict) -> Tuple[float, float, float]:
    return (float(node.get("x", 0.0)), float(node.get("y", 0.0)), float(node.get("z", 0.0)))


def path_power_dbm(path: Dict) -> float:
    if "power_dBm" in path:
        return float(path.get("power_dBm", -300.0))
    p = float(path.get("power_linear", 0.0))
    return 10.0 * np.log10(max(p, 1.0e-30) * 1000.0)


def path_sort_power(path: Dict) -> float:
    return float(path.get("power_linear", 0.0))


def interaction_label(interactions: Sequence[int]) -> str:
    mids = [t for t in interactions if t not in (1, 2)]
    if not mids:
        return "LOS"
    return "".join({4: "R", 5: "T", 6: "D"}.get(t, "?") for t in mids)


def resolve_paths_path(paths: Optional[str], run_dir: Optional[str], rx_id: Optional[str]) -> Optional[Path]:
    if paths:
        return Path(paths)
    if run_dir:
        root = Path(run_dir)
        if rx_id:
            return root / rx_id / "paths" / "precise_paths.json"
        rx_dirs = sorted([p for p in root.iterdir() if p.is_dir() and p.name.lower().startswith("rx")])
        if rx_dirs:
            return rx_dirs[0] / "paths" / "precise_paths.json"
    if DEFAULT_PATHS.exists():
        return DEFAULT_PATHS
    return None


def resolve_sbr_path(sbr: Optional[str], run_dir: Optional[str]) -> Optional[Path]:
    if sbr:
        return Path(sbr)
    if run_dir:
        candidate = Path(run_dir) / "coverage" / "sbr_coverage.json"
        if candidate.exists():
            return candidate
    if DEFAULT_SBR.exists():
        return DEFAULT_SBR
    return None


class CoverageWindow(QMainWindow):
    """3D scene + coverage slice + geometric path overlay."""

    def __init__(
        self,
        obj_path: Path,
        sbr_path: Optional[Path],
        paths_path: Optional[Path],
        path_limit: int,
        min_path_power_dbm: float,
    ) -> None:
        super().__init__()
        self.obj_path = obj_path
        self.sbr_path = sbr_path
        self.paths_path = paths_path
        self.setWindowTitle("RT Coverage + Path Viewer")
        self.setGeometry(100, 50, 1850, 1050)

        self.verts: Optional[np.ndarray] = None
        self.tris: Optional[np.ndarray] = None
        self.bounds: Dict[str, Tuple[float, float]] = {}
        self.clip_on = True
        self.clip_axis = "Y"
        self.clip_val = 100.0
        self.surf_color = "lightgray"
        self.surf_opacity = 0.92
        self.wire_color = "gray"
        self.wire_width = 3.0

        self.rx_x: Optional[np.ndarray] = None
        self.rx_y: Optional[np.ndarray] = None
        self.rx_z: Optional[np.ndarray] = None
        self.rx_pwr: Optional[np.ndarray] = None
        self.rx_hit: Optional[np.ndarray] = None
        self.slice_axis = "Y"
        self.slice_val = 1.5
        self.show_power = bool(sbr_path and sbr_path.exists())
        self.power_opacity = 0.55

        self.paths: List[Dict] = []
        self.show_paths = bool(paths_path and paths_path.exists())
        self.show_nodes = True
        self.path_limit = path_limit
        self.min_path_power_dbm = min_path_power_dbm
        self.path_line_width = 3.0

        self.tx = (16.0, 1.5, -12.0)
        self.rx = (10.0, 1.5, -10.0)

        self._load_obj()
        self._load_sbr()
        self._load_paths()
        self._init_ui()
        self._render()

    def _load_obj(self) -> None:
        mesh = trimesh.load(str(self.obj_path), process=False, skip_materials=True, skip_textures=True)
        self.verts = mesh.vertices.astype(np.float32)
        self.tris = mesh.faces.astype(np.int32)
        v = self.verts
        self.bounds = {
            "X": (float(v[:, 0].min()), float(v[:, 0].max())),
            "Y": (float(v[:, 1].min()), float(v[:, 1].max())),
            "Z": (float(v[:, 2].min()), float(v[:, 2].max())),
        }
        self.clip_val = self.bounds["Y"][1] - 0.05

    def _load_sbr(self) -> None:
        if not self.sbr_path or not self.sbr_path.exists():
            return
        sbr = load_json(self.sbr_path)
        records = sbr.get("records", [])
        n = len(records)
        self.rx_x = np.zeros(n)
        self.rx_y = np.zeros(n)
        self.rx_z = np.zeros(n)
        self.rx_pwr = np.zeros(n)
        self.rx_hit = np.zeros(n, dtype=bool)
        for i, r in enumerate(records):
            self.rx_x[i] = float(r.get("x", 0.0))
            self.rx_y[i] = float(r.get("y", 0.0))
            self.rx_z[i] = float(r.get("z", 0.0))
            self.rx_pwr[i] = float(r.get("power_dBm", -200.0))
            self.rx_hit[i] = int(r.get("ray_hit_count", 0)) > 0
        if "tx_position" in sbr:
            tp = sbr["tx_position"]
            self.tx = (float(tp[0]), float(tp[1]), float(tp[2]))
        if "rx_positions" in sbr and sbr["rx_positions"]:
            rp = sbr["rx_positions"][0]
            self.rx = (float(rp[0]), float(rp[1]), float(rp[2]))
        self.slice_val = round((self.bounds["Y"][0] + self.bounds["Y"][1]) / 2.0, 1)

    def _load_paths(self) -> None:
        if not self.paths_path or not self.paths_path.exists():
            return
        data = load_json(self.paths_path)
        self.paths = data.get("paths", []) if isinstance(data, dict) else []
        if not self.paths:
            return
        first_nodes = self.paths[0].get("geometry_nodes", [])
        tx_node = next((n for n in first_nodes if int(n.get("interaction_type", 0)) == 1), None)
        rx_node = next((n for n in reversed(first_nodes) if int(n.get("interaction_type", 0)) == 2), None)
        if tx_node:
            self.tx = point_from_node(tx_node)
        if rx_node:
            self.rx = point_from_node(rx_node)

    def _init_ui(self) -> None:
        main_widget = QWidget()
        self.setCentralWidget(main_widget)
        layout = QVBoxLayout(main_widget)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(0)

        ctrl = QWidget()
        ctrl.setMaximumHeight(190)
        ctrl_outer = QVBoxLayout(ctrl)
        ctrl_outer.setContentsMargins(15, 6, 15, 6)
        ctrl_outer.setSpacing(6)

        row1 = QHBoxLayout()

        clip_group = QGroupBox("Scene Clip")
        clip_group.setStyleSheet(GROUP_BOX_STYLE)
        cl = QHBoxLayout(clip_group)
        cl.setContentsMargins(15, 5, 15, 5)
        self.clip_check = QCheckBox("Enable")
        self.clip_check.setChecked(self.clip_on)
        self.clip_check.stateChanged.connect(self._on_update)
        cl.addWidget(self.clip_check)
        cl.addWidget(QLabel("Axis"))
        self.clip_axis_combo = QComboBox()
        self.clip_axis_combo.addItems(["Y", "Z", "X"])
        self.clip_axis_combo.currentTextChanged.connect(self._on_clip_axis)
        cl.addWidget(self.clip_axis_combo)
        cl.addWidget(QLabel("Max"))
        self.clip_val_spin = QDoubleSpinBox()
        self.clip_val_spin.setRange(-1e5, 1e5)
        self.clip_val_spin.setDecimals(2)
        self.clip_val_spin.setValue(self.clip_val)
        self.clip_val_spin.valueChanged.connect(self._on_update)
        cl.addWidget(self.clip_val_spin)
        row1.addWidget(clip_group)

        slice_group = QGroupBox("Coverage Slice")
        slice_group.setStyleSheet(GROUP_BOX_STYLE)
        sl = QHBoxLayout(slice_group)
        sl.setContentsMargins(15, 5, 15, 5)
        self.pwr_check = QCheckBox("Show")
        self.pwr_check.setChecked(self.show_power)
        self.pwr_check.setEnabled(self.rx_x is not None)
        self.pwr_check.stateChanged.connect(self._on_update)
        sl.addWidget(self.pwr_check)
        sl.addWidget(QLabel("Axis"))
        self.slice_axis_combo = QComboBox()
        self.slice_axis_combo.addItems(["Y", "Z", "X"])
        self.slice_axis_combo.currentTextChanged.connect(self._on_slice_axis)
        sl.addWidget(self.slice_axis_combo)
        sl.addWidget(QLabel("Position"))
        self.slice_val_spin = QDoubleSpinBox()
        self.slice_val_spin.setRange(-1e5, 1e5)
        self.slice_val_spin.setDecimals(2)
        self.slice_val_spin.setValue(self.slice_val)
        self.slice_val_spin.valueChanged.connect(self._on_update)
        sl.addWidget(self.slice_val_spin)
        sl.addWidget(QLabel("Opacity"))
        self.pwr_opacity_spin = QDoubleSpinBox()
        self.pwr_opacity_spin.setRange(0.1, 1.0)
        self.pwr_opacity_spin.setSingleStep(0.05)
        self.pwr_opacity_spin.setValue(self.power_opacity)
        self.pwr_opacity_spin.valueChanged.connect(self._on_update)
        sl.addWidget(self.pwr_opacity_spin)
        row1.addWidget(slice_group)
        row1.addStretch()
        ctrl_outer.addLayout(row1)

        row2 = QHBoxLayout()
        style_group = QGroupBox("Scene Style")
        style_group.setStyleSheet(GROUP_BOX_STYLE)
        st = QHBoxLayout(style_group)
        st.setContentsMargins(15, 5, 15, 5)
        st.addWidget(QLabel("Surface"))
        self.surf_color_combo = QComboBox()
        self.surf_color_combo.addItems(["lightgray", "#c7ccd6", "white", "#e8e8e8", "#d0d0d0"])
        self.surf_color_combo.setCurrentText(self.surf_color)
        self.surf_color_combo.currentTextChanged.connect(self._on_style)
        st.addWidget(self.surf_color_combo)
        st.addWidget(QLabel("Opacity"))
        self.surf_opacity_spin = QDoubleSpinBox()
        self.surf_opacity_spin.setRange(0.1, 1.0)
        self.surf_opacity_spin.setSingleStep(0.05)
        self.surf_opacity_spin.setValue(self.surf_opacity)
        self.surf_opacity_spin.valueChanged.connect(self._on_style)
        st.addWidget(self.surf_opacity_spin)
        st.addWidget(QLabel("Edges"))
        self.wire_color_combo = QComboBox()
        self.wire_color_combo.addItems(["gray", "#9a9fa8", "#666666", "black", "white", "none"])
        self.wire_color_combo.setCurrentText(self.wire_color)
        self.wire_color_combo.currentTextChanged.connect(self._on_style)
        st.addWidget(self.wire_color_combo)
        st.addWidget(QLabel("Width"))
        self.wire_width_spin = QDoubleSpinBox()
        self.wire_width_spin.setRange(0.0, 5.0)
        self.wire_width_spin.setSingleStep(0.1)
        self.wire_width_spin.setValue(self.wire_width)
        self.wire_width_spin.valueChanged.connect(self._on_style)
        st.addWidget(self.wire_width_spin)
        row2.addWidget(style_group)

        path_group = QGroupBox("Path Overlay")
        path_group.setStyleSheet(GROUP_BOX_STYLE)
        pl = QHBoxLayout(path_group)
        pl.setContentsMargins(15, 5, 15, 5)
        self.path_check = QCheckBox("Show")
        self.path_check.setChecked(self.show_paths)
        self.path_check.setEnabled(bool(self.paths))
        self.path_check.stateChanged.connect(self._on_update)
        pl.addWidget(self.path_check)
        self.node_check = QCheckBox("Nodes")
        self.node_check.setChecked(self.show_nodes)
        self.node_check.setEnabled(bool(self.paths))
        self.node_check.stateChanged.connect(self._on_update)
        pl.addWidget(self.node_check)
        pl.addWidget(QLabel("Top N"))
        self.path_limit_spin = QSpinBox()
        self.path_limit_spin.setRange(1, 10000)
        self.path_limit_spin.setValue(max(1, self.path_limit))
        self.path_limit_spin.valueChanged.connect(self._on_update)
        pl.addWidget(self.path_limit_spin)
        pl.addWidget(QLabel("Min dBm"))
        self.min_power_spin = QDoubleSpinBox()
        self.min_power_spin.setRange(-300.0, 300.0)
        self.min_power_spin.setDecimals(1)
        self.min_power_spin.setValue(self.min_path_power_dbm)
        self.min_power_spin.valueChanged.connect(self._on_update)
        pl.addWidget(self.min_power_spin)
        pl.addWidget(QLabel("Line"))
        self.path_width_spin = QDoubleSpinBox()
        self.path_width_spin.setRange(0.5, 12.0)
        self.path_width_spin.setSingleStep(0.5)
        self.path_width_spin.setValue(self.path_line_width)
        self.path_width_spin.valueChanged.connect(self._on_update)
        pl.addWidget(self.path_width_spin)
        row2.addWidget(path_group)
        row2.addStretch()
        ctrl_outer.addLayout(row2)

        self.summary_label = QLabel("")
        self.summary_label.setStyleSheet("color: #333333; padding-left: 4px;")
        ctrl_outer.addWidget(self.summary_label)

        layout.addWidget(ctrl)
        self.plotter = QtInteractor(main_widget)
        layout.addWidget(self.plotter, stretch=1)

    def _init_renderer(self) -> None:
        p = self.plotter
        p.enable_anti_aliasing()
        p.background_color = "#f5f5f5"
        p.renderer.lights.clear()
        light_kw = dict(light_type="camera light", intensity=0.6)
        p.renderer.add_light(pv.Light(position=(0, 1, 1), **light_kw))
        p.renderer.add_light(pv.Light(position=(1, 0.5, 0.5), **light_kw))
        p.renderer.add_light(pv.Light(position=(-0.5, 0.3, -0.5), **light_kw))

    def _filtered_paths(self) -> List[Dict]:
        paths = [p for p in self.paths if path_power_dbm(p) >= self.min_path_power_dbm]
        paths.sort(key=path_sort_power, reverse=True)
        return paths[: self.path_limit]

    def _render_paths(self, selected_paths: List[Dict]) -> None:
        segment_groups: Dict[int, List[Tuple[Tuple[float, float, float], Tuple[float, float, float]]]] = {
            3: [],
            4: [],
            5: [],
            6: [],
        }
        node_groups: Dict[int, List[Tuple[float, float, float]]] = {4: [], 5: [], 6: []}

        for path in selected_paths:
            nodes = path.get("geometry_nodes", [])
            if len(nodes) < 2:
                continue
            for node in nodes:
                t = int(node.get("interaction_type", 0))
                if t in node_groups:
                    node_groups[t].append(point_from_node(node))
            for i in range(len(nodes) - 1):
                cur = nodes[i]
                nxt = nodes[i + 1]
                cur_t = int(cur.get("interaction_type", 0))
                nxt_t = int(nxt.get("interaction_type", 0))
                if nxt_t == 2:
                    seg_t = cur_t if cur_t in (4, 5, 6) else 3
                else:
                    seg_t = nxt_t if nxt_t in (4, 5, 6) else 3
                segment_groups.setdefault(seg_t, []).append((point_from_node(cur), point_from_node(nxt)))

        for seg_t, segments in segment_groups.items():
            if not segments:
                continue
            points: List[Tuple[float, float, float]] = []
            lines: List[int] = []
            idx = 0
            for a, b in segments:
                points.extend([a, b])
                lines.extend([2, idx, idx + 1])
                idx += 2
            poly = pv.PolyData(np.array(points, dtype=float))
            poly.lines = np.array(lines, dtype=np.int64)
            self.plotter.add_mesh(
                poly,
                color=PATH_COLORS.get(seg_t, "#ff9900"),
                line_width=self.path_line_width,
                render_lines_as_tubes=False,
                reset_camera=False,
            )

        if self.show_nodes:
            radius = self._marker_radius() * 0.55
            for node_t, points in node_groups.items():
                if not points:
                    continue
                cloud = pv.PolyData(np.array(points, dtype=float))
                glyph = cloud.glyph(scale=False, geom=pv.Sphere(radius=radius, theta_resolution=12, phi_resolution=8))
                self.plotter.add_mesh(glyph, color=NODE_COLORS[node_t], ambient=0.35, reset_camera=False)

    def _marker_radius(self) -> float:
        return max(
            self.bounds["X"][1] - self.bounds["X"][0],
            self.bounds["Y"][1] - self.bounds["Y"][0],
            self.bounds["Z"][1] - self.bounds["Z"][0],
        ) * 0.018

    def _render(self) -> None:
        self.plotter.clear()
        self._init_renderer()

        assert self.verts is not None and self.tris is not None
        pd = pv.PolyData.from_regular_faces(self.verts, self.tris)
        if self.clip_on:
            cb = make_clip_box(self.bounds, self.clip_axis, self.clip_val)
            pd = pd.clip_box(cb, invert=False)
        if pd.n_points > 0:
            pd = pd.extract_surface()
            pd = pd.compute_normals(auto_orient_normals=True, consistent_normals=True, split_vertices=True)
            self.plotter.add_mesh(
                pd,
                color=self.surf_color,
                opacity=self.surf_opacity,
                style="surface",
                smooth_shading=True,
                ambient=0.15,
                diffuse=0.75,
                specular=0.3,
                specular_power=15.0,
                reset_camera=True,
            )
            if self.wire_color != "none" and self.wire_width > 0:
                edges = pd.extract_all_edges()
                self.plotter.add_mesh(
                    edges,
                    color=self.wire_color,
                    line_width=self.wire_width,
                    style="wireframe",
                    ambient=0.1,
                    reset_camera=False,
                )

        if self.show_power and self.rx_x is not None:
            axis_idx = {"X": 0, "Y": 1, "Z": 2}[self.slice_axis]
            free = [i for i in range(3) if i != axis_idx]
            ax_data = {0: self.rx_x, 1: self.rx_y, 2: self.rx_z}[axis_idx]
            mask = np.abs(ax_data - self.slice_val) < 0.08
            if int(np.sum(mask)) >= 4:
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
                hit_pwr = gp[gh]
                pwr_min = float(np.min(hit_pwr)) if len(hit_pwr) > 0 else -200.0
                pwr_max = float(np.max(hit_pwr)) if len(hit_pwr) > 0 else -50.0
                if pwr_max <= pwr_min:
                    pwr_max = pwr_min + 10.0
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
                sg.point_data["power"] = gp_fill.ravel(order="C")
                self.plotter.add_mesh(
                    sg,
                    scalars="power",
                    cmap="jet",
                    clim=[pwr_min, pwr_max],
                    opacity=self.power_opacity,
                    style="surface",
                    ambient=0.3,
                    reset_camera=False,
                    show_scalar_bar=True,
                    scalar_bar_args={"title": "Power (dBm)", "font_family": "times", "height": 0.5},
                )

        selected_paths = self._filtered_paths() if self.show_paths else []
        if selected_paths:
            self._render_paths(selected_paths)

        r = self._marker_radius()
        self.plotter.add_mesh(pv.Sphere(center=self.tx, radius=r), color="#ff4f5f", ambient=0.5, reset_camera=False)
        self.plotter.add_mesh(pv.Sphere(center=self.rx, radius=r * 0.82), color="#00cc88", ambient=0.5, reset_camera=False)
        self.plotter.add_point_labels(
            np.array([self.tx, self.rx]),
            ["Tx", "Rx"],
            point_size=0,
            font_size=12,
            always_visible=True,
            reset_camera=False,
        )

        self.plotter.add_axes()
        self.plotter.reset_camera()
        self._update_summary(len(selected_paths))
        self.plotter.update()

    def _update_summary(self, shown_count: int) -> None:
        labels = Counter()
        for path in self._filtered_paths():
            labels[interaction_label(path.get("interaction_types", []))] += 1
        top = ", ".join(f"{k}:{v}" for k, v in labels.most_common(8)) if labels else "none"
        sbr_name = str(self.sbr_path) if self.sbr_path and self.sbr_path.exists() else "none"
        path_name = str(self.paths_path) if self.paths_path and self.paths_path.exists() else "none"
        self.summary_label.setText(
            f"OBJ: {self.obj_path} | SBR: {sbr_name} | Paths: {path_name} | "
            f"loaded={len(self.paths)} shown={shown_count} | types={top}"
        )

    def _on_update(self, *args) -> None:
        self.clip_on = self.clip_check.isChecked()
        self.clip_val = self.clip_val_spin.value()
        self.show_power = self.pwr_check.isChecked()
        self.slice_val = self.slice_val_spin.value()
        self.power_opacity = self.pwr_opacity_spin.value()
        self.show_paths = self.path_check.isChecked()
        self.show_nodes = self.node_check.isChecked()
        self.path_limit = self.path_limit_spin.value()
        self.min_path_power_dbm = self.min_power_spin.value()
        self.path_line_width = self.path_width_spin.value()
        self._render()

    def _on_style(self, *args) -> None:
        self.surf_color = self.surf_color_combo.currentText()
        self.surf_opacity = self.surf_opacity_spin.value()
        self.wire_color = self.wire_color_combo.currentText()
        self.wire_width = self.wire_width_spin.value()
        self._render()

    def _on_clip_axis(self, axis: str) -> None:
        self.clip_axis = axis
        self.clip_val_spin.setValue(self.bounds[axis][1] - 0.05)
        self._render()

    def _on_slice_axis(self, axis: str) -> None:
        self.slice_axis = axis
        mid = round((self.bounds[axis][0] + self.bounds[axis][1]) / 2.0, 1)
        self.slice_val_spin.setValue(mid)
        self._render()


def main(argv: Optional[Sequence[str]] = None) -> int:
    parser = argparse.ArgumentParser(description="3D RT coverage and geometric-path visualization")
    parser.add_argument("--obj", type=str, default=str(DEFAULT_OBJ), help="OBJ scene file")
    parser.add_argument("--sbr", type=str, default=None, help="sbr_coverage.json; optional")
    parser.add_argument("--paths", type=str, default=None, help="precise_paths.json; optional")
    parser.add_argument("--run-dir", type=str, default=None, help="output/<run_id> directory for auto-locating coverage and rx paths")
    parser.add_argument("--rx", type=str, default=None, help="Rx directory name under --run-dir, for example rx1")
    parser.add_argument("--path-limit", type=int, default=300, help="maximum strongest paths to render")
    parser.add_argument("--min-power-dbm", type=float, default=-300.0, help="minimum path power for rendering")
    args = parser.parse_args(argv)

    obj_path = Path(args.obj)
    sbr_path = resolve_sbr_path(args.sbr, args.run_dir)
    paths_path = resolve_paths_path(args.paths, args.run_dir, args.rx)

    if not obj_path.exists():
        print(f"OBJ not found: {obj_path}")
        return 1
    if sbr_path and not sbr_path.exists():
        print(f"Coverage file not found, continuing without coverage: {sbr_path}")
        sbr_path = None
    if paths_path and not paths_path.exists():
        print(f"Path file not found, continuing without path overlay: {paths_path}")
        paths_path = None

    app = QApplication(sys.argv)
    win = CoverageWindow(
        obj_path=obj_path.resolve(),
        sbr_path=sbr_path.resolve() if sbr_path else None,
        paths_path=paths_path.resolve() if paths_path else None,
        path_limit=args.path_limit,
        min_path_power_dbm=args.min_power_dbm,
    )
    win.show()
    return app.exec_()


if __name__ == "__main__":
    raise SystemExit(main())
