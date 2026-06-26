#!/usr/bin/env python3
"""
H2hRT precise_paths.json visualizer.

This script intentionally keeps the standalone SBR visualize.py interaction
style and only changes the data adapter to read H2hRT output files such as:

  output/sbr_compare_412/rx01/paths/precise_paths.json

Usage:
  python tools/visualize_h2hrt_precise.py <precise_paths.json> --scene <scene.obj>

Dependencies:
  pip install pyvista pyvistaqt PyQt5 trimesh
"""

import argparse
import json
import os
import sys
from collections import defaultdict

import numpy as np


TYPE_NAME = {
    0: "None",
    1: "Tx",
    2: "Rx",
    3: "Los",
    4: "Reflection",
    5: "Transmission",
    6: "Diffraction",
    7: "Scattering",
}

SEQ_CHAR = {
    "Tx": "T",
    "Rx": "R",
    "Los": "l",
    "Reflection": "r",
    "Transmission": "t",
    "Diffraction": "d",
    "Scattering": "s",
}


def load_json(path):
    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)


def interaction_name(node):
    return TYPE_NAME.get(int(node.get("interaction_type", 0)), "None")


def node_xyz(node):
    return [float(node.get("x", 0.0)), float(node.get("y", 0.0)), float(node.get("z", 0.0))]


def path_sequence(nodes):
    return "".join(SEQ_CHAR.get(interaction_name(n), "?") for n in nodes)


def path_points(nodes):
    return [node_xyz(n) for n in nodes]


def infer_rx_label(path):
    parts = os.path.normpath(path).split(os.sep)
    for part in reversed(parts):
        if part.lower().startswith("rx"):
            return part
    return os.path.basename(path)


def adapt_h2hrt_precise(path, scene_file=""):
    raw = load_json(path)
    raw_paths = raw.get("paths", [])

    paths = []
    tx_pos = [0.0, 0.0, 0.0]
    rx_pos = [0.0, 0.0, 0.0]

    for i, p in enumerate(raw_paths):
        nodes = p.get("geometry_nodes", [])
        if not nodes:
            continue
        pts = path_points(nodes)
        seq = path_sequence(nodes)
        if i == 0:
            tx_pos = pts[0]
            rx_pos = pts[-1]
        paths.append({
            "source_index": i,
            "path_id": p.get("path_id", i),
            "signature": p.get("source_path_signature", ""),
            "len": float(p.get("delay_s", 0.0)) * 299792458.0,
            "nodes": len(nodes),
            "los": bool(p.get("is_los", False)),
            "contains_transmission": bool(p.get("contains_transmission", False)),
            "sequence": seq,
            "points": pts,
            "geometry_nodes": nodes,
            "power_linear": float(p.get("power_linear", 0.0)),
            "free_space_loss_db": float(p.get("free_space_loss_db", 0.0)),
            "delay_s": float(p.get("delay_s", 0.0)),
            "phase_rad": float(p.get("phase_rad", 0.0)),
            "geometry_residual": float(p.get("geometry_residual", 0.0)),
            "reflection_residual_m": float(p.get("reflection_residual_m", 0.0)),
            "max_snell_residual": float(p.get("max_snell_residual", 0.0)),
            "max_keller_residual": float(p.get("max_keller_residual", 0.0)),
        })

    rx_label = infer_rx_label(path)
    return {
        "scene_file": scene_file,
        "tx": {"x": tx_pos[0], "y": tx_pos[1], "z": tx_pos[2]},
        "stats": {
            "total_faces": 0,
            "total_wedges": 0,
            "total_rays": 0,
            "bvh_build_ms": 0,
            "sbr_trace_ms": 0,
        },
        "rx_records": [{
            "rx_index": 0,
            "rx_label": rx_label,
            "position": rx_pos,
            "total_power_dBm": 0.0,
            "hit_count": len(paths),
            "paths": paths,
        }],
        "h2hrt_source_file": path,
        "h2hrt_path_count": raw.get("path_count", len(paths)),
        "h2hrt_primary_input_source": raw.get("primary_input_source", ""),
    }


def load_obj_mesh(obj_path):
    if not obj_path:
        return None
    try:
        import trimesh
        import pyvista as pv
        mesh = trimesh.load(obj_path, process=False, skip_materials=True)
        return pv.wrap(mesh)
    except Exception:
        try:
            import pyvista as pv
            return pv.read(obj_path)
        except Exception:
            return None


try:
    from PyQt5.QtWidgets import (
        QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout,
        QLabel, QComboBox, QPushButton, QFileDialog, QSpinBox,
        QGroupBox, QSplitter, QTextEdit, QCheckBox
    )
    from PyQt5.QtCore import Qt
    import pyvista as pv
    from pyvistaqt import QtInteractor
    HAS_GUI = True
except ImportError:
    HAS_GUI = False


if HAS_GUI:
    class H2hRtPreciseVisualizer(QMainWindow):
        def __init__(self, result_path=None, scene_path=""):
            super().__init__()
            self.setWindowTitle("H2hRT precise_paths.json — 3D Visualizer")
            self.setGeometry(100, 50, 1800, 1100)

            self.result_data = None
            self.current_rx = 0
            self.current_path = 0
            self.show_scene = True
            self.show_all_rx = True
            self.show_all_paths = False
            self.max_paths_all = 200
            self.scene_mesh = None
            self.scene_file = scene_path
            self.result_file = ""
            self._camera_pos = None
            self._first_render = True
            self._seq_filter = ""
            self._seq_paths = []

            self.PATH_COLORS = [
                [0.10, 0.55, 0.95], [0.95, 0.35, 0.05], [0.05, 0.75, 0.35],
                [0.85, 0.15, 0.55], [0.55, 0.35, 0.05], [0.05, 0.55, 0.75],
                [0.75, 0.05, 0.15], [0.35, 0.35, 0.75],
            ]
            self.INTERACTION_COLORS = {'r': 'orange', 't': 'cyan', 'd': 'magenta', 's': 'yellow'}

            self._init_ui()
            if result_path:
                self._load_file(result_path, scene_path)

        def _init_ui(self):
            central = QWidget()
            self.setCentralWidget(central)
            main_layout = QVBoxLayout(central)
            main_layout.setContentsMargins(8, 8, 8, 8)

            ctrl = QWidget()
            ctrl_layout = QHBoxLayout(ctrl)
            ctrl_layout.setContentsMargins(0, 0, 0, 0)

            file_grp = QGroupBox("Result File")
            fl = QHBoxLayout(file_grp)
            self.file_label = QLabel("(none)")
            self.file_label.setMaximumWidth(320)
            fl.addWidget(self.file_label)
            btn_open = QPushButton("Open JSON...")
            btn_open.clicked.connect(self._on_open)
            fl.addWidget(btn_open)
            btn_scene = QPushButton("Open OBJ...")
            btn_scene.clicked.connect(self._on_scene)
            fl.addWidget(btn_scene)
            btn_refresh = QPushButton("Refresh")
            btn_refresh.clicked.connect(self._on_refresh)
            fl.addWidget(btn_refresh)
            ctrl_layout.addWidget(file_grp)

            rx_grp = QGroupBox("Rx Selection")
            rl = QHBoxLayout(rx_grp)
            rl.addWidget(QLabel("Rx:"))
            self.rx_combo = QComboBox()
            self.rx_combo.setMinimumWidth(250)
            self.rx_combo.currentIndexChanged.connect(self._on_rx_change)
            rl.addWidget(self.rx_combo)
            ctrl_layout.addWidget(rx_grp)

            path_grp = QGroupBox("Path Selection")
            pl = QHBoxLayout(path_grp)
            pl.addWidget(QLabel("Path:"))
            self.path_spin = QSpinBox()
            self.path_spin.setMinimum(0)
            self.path_spin.setMaximum(999999)
            self.path_spin.setValue(0)
            self.path_spin.valueChanged.connect(self._on_path_change)
            pl.addWidget(self.path_spin)
            pl.addWidget(QLabel("/"))
            self.path_total = QLabel("0")
            pl.addWidget(self.path_total)
            ctrl_layout.addWidget(path_grp)

            seq_grp = QGroupBox("Sequence Filter")
            sl = QHBoxLayout(seq_grp)
            sl.addWidget(QLabel("Type:"))
            self.seq_combo = QComboBox()
            self.seq_combo.setMinimumWidth(120)
            self.seq_combo.currentIndexChanged.connect(self._on_seq_change)
            sl.addWidget(self.seq_combo)
            ctrl_layout.addWidget(seq_grp)

            disp_grp = QGroupBox("Display")
            dl = QHBoxLayout(disp_grp)
            self.chk_scene = QCheckBox("Scene")
            self.chk_scene.setChecked(True)
            self.chk_scene.stateChanged.connect(self._update_render)
            dl.addWidget(self.chk_scene)
            self.chk_all_rx = QCheckBox("All Rx")
            self.chk_all_rx.setChecked(True)
            self.chk_all_rx.stateChanged.connect(self._update_render)
            dl.addWidget(self.chk_all_rx)
            self.chk_all_paths = QCheckBox("All Paths")
            self.chk_all_paths.setChecked(False)
            self.chk_all_paths.stateChanged.connect(self._update_render)
            dl.addWidget(self.chk_all_paths)
            ctrl_layout.addWidget(disp_grp)

            main_layout.addWidget(ctrl)

            splitter = QSplitter(Qt.Horizontal)
            self.plotter = QtInteractor(self)
            self.plotter.set_background("white")
            self.plotter.enable_anti_aliasing()
            splitter.addWidget(self.plotter)

            info_widget = QWidget()
            info_widget.setMaximumWidth(350)
            info_layout = QVBoxLayout(info_widget)
            info_layout.setContentsMargins(4, 4, 4, 4)

            info_layout.addWidget(QLabel("Path Info"))
            self.info_text = QTextEdit()
            self.info_text.setReadOnly(True)
            self.info_text.setMaximumHeight(240)
            info_layout.addWidget(self.info_text)

            info_layout.addWidget(QLabel("Summary"))
            self.summary_text = QTextEdit()
            self.summary_text.setReadOnly(True)
            info_layout.addWidget(self.summary_text)

            splitter.addWidget(info_widget)
            splitter.setSizes([1400, 350])
            main_layout.addWidget(splitter, stretch=1)

        def _load_file(self, path, scene_path=""):
            try:
                self.result_file = path
                if scene_path:
                    self.scene_file = scene_path
                self.result_data = adapt_h2hrt_precise(path, self.scene_file)
                self.file_label.setText(os.path.basename(path))

                self.scene_mesh = load_obj_mesh(self.scene_file)

                self.rx_combo.blockSignals(True)
                self.rx_combo.clear()
                records = self.result_data.get('rx_records', [])
                for rec in records:
                    p = rec.get('position', [0, 0, 0])
                    n = rec.get('hit_count', 0)
                    label = rec.get('rx_label', f"Rx[{rec['rx_index']}]")
                    self.rx_combo.addItem(f"{label} ({p[0]:.1f},{p[1]:.1f},{p[2]:.1f}) {n}paths")
                self.rx_combo.blockSignals(False)

                records = self.result_data.get('rx_records', [])
                total_p = sum(r.get('hit_count', 0) for r in records)
                rx_hit = sum(1 for r in records if r.get('hit_count', 0) > 0)
                counts = defaultdict(int)
                seq_counts = defaultdict(int)
                for r in records:
                    for p in r.get('paths', []):
                        seq_counts[p.get('sequence', '?')] += 1
                        for ch in p.get('sequence', ''):
                            if ch == 'r':
                                counts['R'] += 1
                            elif ch == 't':
                                counts['T'] += 1
                            elif ch == 'd':
                                counts['D'] += 1
                top_seq = "\n".join(
                    f"  {k}: {v}" for k, v in sorted(seq_counts.items(), key=lambda kv: -kv[1])[:12]
                )
                self.summary_text.setPlainText(
                    f"Source: {self.result_data.get('h2hrt_primary_input_source','')}\n"
                    f"File paths: {self.result_data.get('h2hrt_path_count', total_p)}\n"
                    f"Loaded paths: {total_p}\n"
                    f"Rx hit: {rx_hit}/{len(records)}\n"
                    f"R={counts['R']} T={counts['T']} D={counts['D']}\n\n"
                    f"Top sequences:\n{top_seq}"
                )

                self.current_rx = 0
                self.current_path = 0
                self._first_render = True
                self._camera_pos = None
                self._on_rx_change(0)
                self._update_render()

            except Exception as e:
                self.info_text.setPlainText(f"Error: {e}")

        def _on_open(self):
            path, _ = QFileDialog.getOpenFileName(self, "Open H2hRT precise_paths.json", "", "JSON Files (*.json);;All (*)")
            if path:
                self._load_file(path, self.scene_file)

        def _on_scene(self):
            path, _ = QFileDialog.getOpenFileName(self, "Open Scene OBJ", "", "OBJ Files (*.obj);;All (*)")
            if path:
                self.scene_file = path
                self.scene_mesh = load_obj_mesh(path)
                self._update_render()

        def _on_refresh(self):
            if self.result_file:
                self._load_file(self.result_file, self.scene_file)

        def _on_rx_change(self, idx):
            if idx < 0 or not self.result_data:
                return
            self.current_rx = idx
            records = self.result_data.get('rx_records', [])
            if idx < len(records):
                paths = records[idx].get('paths', [])
                seq_map = {}
                for pi, p in enumerate(paths):
                    sq = p.get('sequence', '?')
                    seq_map.setdefault(sq, []).append(pi)
                sorted_seqs = sorted(seq_map.keys(), key=lambda s: (-len(s), s))
                self.seq_combo.blockSignals(True)
                self.seq_combo.clear()
                self.seq_combo.addItem(f"(all) [{len(paths)}]", "")
                for sq in sorted_seqs:
                    self.seq_combo.addItem(f"{sq} [{len(seq_map[sq])}]", sq)
                self.seq_combo.blockSignals(False)
                self._apply_seq_filter(paths)

        def _on_seq_change(self, _idx):
            if not self.result_data:
                return
            records = self.result_data.get('rx_records', [])
            if self.current_rx < len(records):
                self._apply_seq_filter(records[self.current_rx].get('paths', []))
            self._update_render()

        def _apply_seq_filter(self, paths):
            sq = self.seq_combo.currentData()
            if sq:
                self._seq_filter = sq
                self._seq_paths = [pi for pi, p in enumerate(paths) if p.get('sequence', '') == sq]
            else:
                self._seq_filter = ""
                self._seq_paths = list(range(len(paths)))
            self.current_path = 0
            self.path_spin.blockSignals(True)
            self.path_spin.setMaximum(max(0, len(self._seq_paths) - 1))
            self.path_spin.setValue(0)
            self.path_total.setText(str(len(self._seq_paths)))
            self.path_spin.blockSignals(False)

        def _on_path_change(self, val):
            self.current_path = val
            self._update_render()

        def _update_render(self):
            if not self.result_data:
                return
            if not self._first_render and self.plotter.renderer.camera:
                self._camera_pos = self.plotter.camera_position

            self.plotter.clear()
            records = self.result_data.get('rx_records', [])

            if self.chk_scene.isChecked() and self.scene_mesh:
                self.plotter.add_mesh(
                    self.scene_mesh, color='lightgray', opacity=0.35,
                    style='surface', show_edges=True, edge_color='gray', line_width=0.5
                )

            tx = self.result_data.get('tx', {})
            tx_pt = np.array([[tx.get('x', 0), tx.get('y', 0), tx.get('z', 0)]], dtype=np.float32)
            self.plotter.add_points(tx_pt, color='red', point_size=35, render_points_as_spheres=True)

            if self.chk_all_rx.isChecked():
                rx_pts = [rec.get('position', [0, 0, 0]) for rec in records]
                if rx_pts:
                    self.plotter.add_points(np.array(rx_pts, dtype=np.float32),
                                            color='black', point_size=22,
                                            render_points_as_spheres=True)

            if self.current_rx < len(records):
                pos = records[self.current_rx].get('position', [0, 0, 0])
                self.plotter.add_points(np.array([pos], dtype=np.float32),
                                        color='lime', point_size=30,
                                        render_points_as_spheres=True)

            if self.current_rx < len(records):
                paths = records[self.current_rx].get('paths', [])
                if self.chk_all_paths.isChecked():
                    for i, pi in enumerate(self._seq_paths):
                        if i >= self.max_paths_all:
                            break
                        self._draw_path(paths[pi], pi, highlight=(i == self.current_path))
                else:
                    if self.current_path < len(self._seq_paths):
                        pi = self._seq_paths[self.current_path]
                        self._draw_path(paths[pi], pi, highlight=True)

            self.plotter.show_grid(color='gray', font_size=8)

            if self._camera_pos is not None:
                self.plotter.camera_position = self._camera_pos
            elif self._first_render:
                self.plotter.camera_position = 'iso'
            self._first_render = False

            if self.current_rx < len(records):
                paths = records[self.current_rx].get('paths', [])
                if self.current_path < len(self._seq_paths):
                    pi = self._seq_paths[self.current_path]
                    p = paths[pi]
                    self.info_text.setPlainText(
                        f"Rx[{self.current_rx}] Path #{pi} (filtered #{self.current_path})\n"
                        f"Path id: {p.get('path_id')}\n"
                        f"Signature: {p.get('signature')}\n"
                        f"Length: {p.get('len', 0):.6f}m\n"
                        f"Delay: {p.get('delay_s', 0):.6e}s\n"
                        f"Power: {p.get('power_linear', 0):.6e}\n"
                        f"FS loss: {p.get('free_space_loss_db', 0):.3f}dB\n"
                        f"Nodes: {p.get('nodes', 0)}\n"
                        f"LoS: {p.get('los', False)}\n"
                        f"Contains T: {p.get('contains_transmission', False)}\n"
                        f"Sequence: {p.get('sequence', '?')}\n"
                        f"Residual: {p.get('geometry_residual', 0):.6g}\n"
                        f"Points: {len(p.get('points', []))}"
                    )

        def _draw_path(self, p, pi, highlight=True):
            pts = p.get('points', [])
            if len(pts) < 2:
                return
            arr = np.array(pts, dtype=np.float32)
            seq = p.get('sequence', '')
            is_los = p.get('los', False)
            seq_colors = {
                'T': [1.0, 0.2, 0.2], 'R': [0.2, 0.6, 1.0],
                'r': [1.0, 0.5, 0.0], 't': [0.0, 0.7, 0.7],
                'd': [0.8, 0.0, 0.8], 's': [0.8, 0.8, 0.0],
            }

            if highlight:
                for si in range(len(pts) - 1):
                    ntype = seq[si + 1] if (si + 1) < len(seq) else 'R'
                    seg_color = seq_colors.get(ntype, [0.5, 0.5, 0.5])
                    seg = np.array([pts[si], pts[si + 1]], dtype=np.float32)
                    self.plotter.add_lines(seg, color=seg_color, width=4, connected=True)
                for ni in range(1, len(pts) - 1):
                    ntype = seq[ni] if ni < len(seq) else '?'
                    nc = self.INTERACTION_COLORS.get(ntype, 'gray')
                    self.plotter.add_points(arr[ni:ni+1], color=nc, point_size=14,
                                            render_points_as_spheres=True)
            else:
                width = 1.2 if is_los else 0.6
                color = self.PATH_COLORS[pi % len(self.PATH_COLORS)]
                alpha_color = [c * (0.6 if is_los else 0.15) for c in color]
                self.plotter.add_lines(arr, color=alpha_color, width=width, connected=True)
                for ni in range(1, len(pts) - 1):
                    ntype = seq[ni] if ni < len(seq) else '?'
                    nc = self.INTERACTION_COLORS.get(ntype, 'gray')
                    self.plotter.add_points(arr[ni:ni+1], color=nc, point_size=5,
                                            render_points_as_spheres=True)


def run_headless(result_path):
    r = adapt_h2hrt_precise(result_path)
    records = r.get('rx_records', [])
    total_p = sum(rec.get('hit_count', 0) for rec in records)
    counts = defaultdict(int)
    seq_counts = defaultdict(int)
    for rec in records:
        for p in rec.get('paths', []):
            seq_counts[p.get('sequence', '?')] += 1
            for ch in p.get('sequence', ''):
                if ch == 'r':
                    counts['R'] += 1
                elif ch == 't':
                    counts['T'] += 1
                elif ch == 'd':
                    counts['D'] += 1
    print("=" * 65)
    print("  H2hRT precise_paths.json")
    print(f"  File: {result_path}")
    print(f"  Paths: {total_p}  |  R={counts['R']} T={counts['T']} D={counts['D']}")
    print("=" * 65)
    for rec in records:
        paths = rec.get('paths', [])
        pos = rec.get('position', [0, 0, 0])
        print(f"\n  Rx[{rec['rx_index']}] ({pos[0]:.1f},{pos[1]:.1f},{pos[2]:.1f}) [{len(paths)} paths]")
        for seq, n in sorted(seq_counts.items(), key=lambda kv: -kv[1])[:12]:
            print(f"    {seq}: {n}")
        for i, p in enumerate(paths[:3]):
            print(f"    #{i}: {p['len']:.2f}m {p['nodes']}nodes {'LoS' if p.get('los') else 'NLoS'} {p.get('sequence','?')}")
    print("\n[NOTE] Install GUI deps: pip install PyQt5 pyvista pyvistaqt trimesh")


def parse_args(argv):
    parser = argparse.ArgumentParser()
    parser.add_argument("json", nargs="?", help="H2hRT paths/precise_paths.json")
    parser.add_argument("--scene", default="", help="Scene OBJ path")
    parser.add_argument("--headless", action="store_true", help="Print summary only")
    return parser.parse_args(argv)


if __name__ == '__main__':
    args = parse_args(sys.argv[1:])

    if not args.json:
        print("Usage: python visualize_h2hrt_precise.py <precise_paths.json> --scene <scene.obj>")
        sys.exit(0)

    if args.headless or not HAS_GUI:
        run_headless(args.json)
        if not HAS_GUI and not args.headless:
            print("Install GUI deps: pip install PyQt5 pyvista pyvistaqt trimesh")
        sys.exit(0)

    app = QApplication(sys.argv)
    window = H2hRtPreciseVisualizer(args.json, args.scene)
    window.show()
    sys.exit(app.exec_())
