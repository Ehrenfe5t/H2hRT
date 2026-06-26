#!/usr/bin/env python3
"""
H2hRT SBR geometric path visualizer.

This follows the rendering style of the standalone SBR visualizer, but reads
H2hRT geometry output:

  output/<run>/<rx_id>/coverage/paths/rx_0_sbr_paths.json

Usage:
  python tools/visualize_h2hrt_sbr_paths.py <json-or-output-dir> --scene <scene.obj>

Examples:
  python tools/visualize_h2hrt_sbr_paths.py output/sbr_compare_412 --scene G:/RT/sbr/sbr/demo/Scene/412-6k.obj
  python tools/visualize_h2hrt_sbr_paths.py output/sbr_compare_412/rx01/coverage/paths/rx_0_sbr_paths.json --scene G:/RT/sbr/sbr/demo/Scene/412-6k.obj

Dependencies:
  pip install PyQt5 pyvista pyvistaqt trimesh
"""

import argparse
import glob
import json
import os
import sys
from collections import Counter, defaultdict

import numpy as np


TYPE_BY_ID = {
    0: "None",
    1: "Tx",
    2: "Rx",
    3: "Los",
    4: "Reflection",
    5: "Transmission",
    6: "Diffraction",
    7: "Scattering",
}

SEQ_BY_NAME = {
    "Tx": "T",
    "Rx": "R",
    "Los": "l",
    "Reflection": "r",
    "Transmission": "t",
    "Diffraction": "d",
    "Scattering": "s",
}


def interaction_name(node):
    name = node.get("interaction_name")
    if name:
        return name
    return TYPE_BY_ID.get(int(node.get("interaction_type", 0)), "None")


def node_point(node):
    return [
        float(node.get("x", 0.0)),
        float(node.get("y", 0.0)),
        float(node.get("z", 0.0)),
    ]


def normalize_path(path):
    nodes = path.get("nodes")
    if nodes is None:
        nodes = path.get("geometry_nodes", [])

    points = [node_point(n) for n in nodes]
    names = [interaction_name(n) for n in nodes]
    seq = "".join(SEQ_BY_NAME.get(name, "?") for name in names)

    return {
        "path_id": int(path.get("path_id", -1)),
        "signature": str(path.get("path_signature", path.get("source_path_signature", ""))),
        "valid": bool(path.get("valid", True)),
        "los": bool(path.get("is_los", False)),
        "contains_transmission": bool(path.get("contains_transmission", False)),
        "len": float(path.get("total_length_m", path.get("total_length", 0.0))),
        "sequence": seq,
        "points": points,
        "nodes": nodes,
        "geometry_residual": float(path.get("geometry_residual", 0.0)),
        "reflection_residual_m": float(path.get("reflection_residual_m", 0.0)),
        "max_snell_residual": float(path.get("max_snell_residual", 0.0)),
        "max_keller_residual": float(path.get("max_keller_residual", 0.0)),
    }


def read_json(path):
    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)


def read_h2hrt_geometry_file(path, max_paths=0):
    if max_paths and max_paths > 0:
        try:
            import ijson

            meta = {
                "trace_profile": "",
                "rx_index": 0,
                "tx_position": None,
                "rx_position": [0.0, 0.0, 0.0],
                "path_count": 0,
            }
            paths = []
            with open(path, "rb") as f:
                for prefix, event, value in ijson.parse(f):
                    if prefix == "trace_profile" and event == "string":
                        meta["trace_profile"] = value
                    elif prefix == "rx_index" and event in ("number", "integer"):
                        meta["rx_index"] = int(value)
                    elif prefix == "path_count" and event in ("number", "integer"):
                        meta["path_count"] = int(value)
                    elif prefix == "tx_position.item" and event in ("number", "integer", "double"):
                        if meta["tx_position"] is None:
                            meta["tx_position"] = []
                        meta["tx_position"].append(float(value))
                    elif prefix == "rx_position.item" and event in ("number", "integer", "double"):
                        if len(meta["rx_position"]) == 3 and meta["rx_position"] == [0.0, 0.0, 0.0]:
                            meta["rx_position"] = []
                        meta["rx_position"].append(float(value))
                    elif prefix == "paths.item" and event == "start_map":
                        break

            with open(path, "rb") as f:
                for item in ijson.items(f, "paths.item"):
                    paths.append(item)
                    if len(paths) >= max_paths:
                        break

            if meta["tx_position"] is None:
                meta["tx_position"] = [0.0, 0.0, 0.0]
            meta["paths"] = paths
            return meta
        except Exception:
            pass

    return read_json(path)


def collect_input_files(path):
    path = os.path.abspath(path)
    if os.path.isfile(path):
        return [path]
    if not os.path.isdir(path):
        raise FileNotFoundError(path)
    files = glob.glob(os.path.join(path, "**", "rx_*_sbr_paths.json"), recursive=True)
    return sorted(files)


def infer_rx_label(path):
    parts = os.path.normpath(path).split(os.sep)
    for part in reversed(parts):
        if part.lower().startswith("rx"):
            return part
    return os.path.basename(path)


def load_h2hrt_paths(input_path, max_paths_per_rx=0):
    files = collect_input_files(input_path)
    if not files:
        raise RuntimeError("No rx_*_sbr_paths.json files found")

    records = []
    tx_position = None
    trace_profile = ""
    total_declared = 0

    for file_path in files:
        data = read_h2hrt_geometry_file(file_path, max_paths_per_rx)
        trace_profile = trace_profile or data.get("trace_profile", "")
        tx_position = tx_position or data.get("tx_position")
        paths_raw = data.get("paths", [])
        total_declared += int(data.get("path_count", len(paths_raw)))

        if max_paths_per_rx > 0 and len(paths_raw) > max_paths_per_rx:
            paths_raw = paths_raw[:max_paths_per_rx]

        paths = [normalize_path(p) for p in paths_raw]
        rec = {
            "rx_index": int(data.get("rx_index", len(records))),
            "rx_label": infer_rx_label(file_path),
            "position": data.get("rx_position", [0.0, 0.0, 0.0]),
            "path_count_declared": int(data.get("path_count", len(paths_raw))),
            "paths": paths,
            "file": file_path,
        }
        records.append(rec)

    return {
        "source": "h2hrt_sbr_geometry_paths",
        "trace_profile": trace_profile,
        "input_path": os.path.abspath(input_path),
        "tx": tx_position or [0.0, 0.0, 0.0],
        "records": records,
        "path_count_declared": total_declared,
    }


def load_obj_mesh(obj_path):
    if not obj_path:
        return None
    try:
        import pyvista as pv
        import trimesh

        mesh = trimesh.load(obj_path, process=False, skip_materials=True)
        return pv.wrap(mesh)
    except Exception:
        try:
            import pyvista as pv

            return pv.read(obj_path)
        except Exception:
            return None


try:
    from PyQt5.QtCore import Qt
    from PyQt5.QtWidgets import (
        QApplication,
        QCheckBox,
        QComboBox,
        QFileDialog,
        QGroupBox,
        QHBoxLayout,
        QLabel,
        QMainWindow,
        QPushButton,
        QSpinBox,
        QSplitter,
        QTextEdit,
        QVBoxLayout,
        QWidget,
    )
    import pyvista as pv
    from pyvistaqt import QtInteractor

    HAS_GUI = True
except Exception:
    HAS_GUI = False


if HAS_GUI:

    class H2hRtSbrVisualizer(QMainWindow):
        def __init__(self, input_path=None, scene_path=None, max_paths_per_rx=0):
            super().__init__()
            self.setWindowTitle("H2hRT SBR Geometry Paths")
            self.setGeometry(80, 40, 1800, 1100)

            self.data = None
            self.input_path = input_path
            self.scene_path = scene_path
            self.scene_mesh = None
            self.max_paths_per_rx = max_paths_per_rx
            self.current_rx = 0
            self.current_path = 0
            self.seq_filter = ""
            self.seq_paths = []
            self.max_paths_all = 300
            self.camera_pos = None
            self.first_render = True

            self.path_colors = [
                [0.10, 0.55, 0.95],
                [0.95, 0.35, 0.05],
                [0.05, 0.75, 0.35],
                [0.85, 0.15, 0.55],
                [0.55, 0.35, 0.05],
                [0.05, 0.55, 0.75],
                [0.75, 0.05, 0.15],
                [0.35, 0.35, 0.75],
            ]
            self.node_colors = {
                "r": "orange",
                "t": "cyan",
                "d": "magenta",
                "s": "yellow",
            }
            self.segment_colors = {
                "T": [1.0, 0.1, 0.1],
                "R": [0.1, 0.35, 1.0],
                "r": [1.0, 0.5, 0.0],
                "t": [0.0, 0.75, 0.8],
                "d": [0.85, 0.0, 0.85],
                "s": [0.8, 0.8, 0.0],
            }

            self._init_ui()
            if input_path:
                self._load(input_path, scene_path)

        def _init_ui(self):
            central = QWidget()
            self.setCentralWidget(central)
            main_layout = QVBoxLayout(central)
            main_layout.setContentsMargins(8, 8, 8, 8)

            ctrl = QWidget()
            ctrl_layout = QHBoxLayout(ctrl)
            ctrl_layout.setContentsMargins(0, 0, 0, 0)

            file_grp = QGroupBox("Input")
            fl = QHBoxLayout(file_grp)
            self.file_label = QLabel("(none)")
            self.file_label.setMinimumWidth(260)
            fl.addWidget(self.file_label)
            btn_open = QPushButton("Open JSON/Dir")
            btn_open.clicked.connect(self._on_open)
            fl.addWidget(btn_open)
            btn_scene = QPushButton("Scene OBJ")
            btn_scene.clicked.connect(self._on_scene)
            fl.addWidget(btn_scene)
            btn_reload = QPushButton("Reload")
            btn_reload.clicked.connect(self._on_reload)
            fl.addWidget(btn_reload)
            ctrl_layout.addWidget(file_grp)

            rx_grp = QGroupBox("Rx")
            rl = QHBoxLayout(rx_grp)
            self.rx_combo = QComboBox()
            self.rx_combo.setMinimumWidth(320)
            self.rx_combo.currentIndexChanged.connect(self._on_rx_change)
            rl.addWidget(self.rx_combo)
            ctrl_layout.addWidget(rx_grp)

            path_grp = QGroupBox("Path")
            pl = QHBoxLayout(path_grp)
            self.path_spin = QSpinBox()
            self.path_spin.setMinimum(0)
            self.path_spin.setMaximum(0)
            self.path_spin.valueChanged.connect(self._on_path_change)
            pl.addWidget(self.path_spin)
            pl.addWidget(QLabel("/"))
            self.path_total = QLabel("0")
            pl.addWidget(self.path_total)
            ctrl_layout.addWidget(path_grp)

            seq_grp = QGroupBox("Sequence")
            sl = QHBoxLayout(seq_grp)
            self.seq_combo = QComboBox()
            self.seq_combo.setMinimumWidth(150)
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

            limit_grp = QGroupBox("All Limit")
            ll = QHBoxLayout(limit_grp)
            self.limit_spin = QSpinBox()
            self.limit_spin.setMinimum(1)
            self.limit_spin.setMaximum(10000)
            self.limit_spin.setValue(self.max_paths_all)
            self.limit_spin.valueChanged.connect(self._on_limit_change)
            ll.addWidget(self.limit_spin)
            ctrl_layout.addWidget(limit_grp)

            main_layout.addWidget(ctrl)

            splitter = QSplitter(Qt.Horizontal)
            self.plotter = QtInteractor(self)
            self.plotter.set_background("white")
            self.plotter.enable_anti_aliasing()
            splitter.addWidget(self.plotter)

            info_widget = QWidget()
            info_widget.setMaximumWidth(390)
            info_layout = QVBoxLayout(info_widget)
            info_layout.addWidget(QLabel("Path Info"))
            self.info_text = QTextEdit()
            self.info_text.setReadOnly(True)
            self.info_text.setMaximumHeight(260)
            info_layout.addWidget(self.info_text)
            info_layout.addWidget(QLabel("Summary"))
            self.summary_text = QTextEdit()
            self.summary_text.setReadOnly(True)
            info_layout.addWidget(self.summary_text)
            splitter.addWidget(info_widget)
            splitter.setSizes([1380, 380])
            main_layout.addWidget(splitter, stretch=1)

        def _load(self, input_path, scene_path=None):
            self.input_path = input_path
            if scene_path:
                self.scene_path = scene_path
            self.data = load_h2hrt_paths(input_path, self.max_paths_per_rx)
            self.scene_mesh = load_obj_mesh(self.scene_path)
            self.file_label.setText(os.path.basename(os.path.abspath(input_path)))
            self.first_render = True
            self.camera_pos = None

            self.rx_combo.blockSignals(True)
            self.rx_combo.clear()
            for rec in self.data["records"]:
                pos = rec["position"]
                loaded = len(rec["paths"])
                declared = rec["path_count_declared"]
                self.rx_combo.addItem(
                    f"{rec['rx_label']} | Rx[{rec['rx_index']}] "
                    f"({pos[0]:.2f},{pos[1]:.2f},{pos[2]:.2f}) "
                    f"{loaded}/{declared} paths"
                )
            self.rx_combo.blockSignals(False)
            self.current_rx = 0
            self._update_summary()
            self._on_rx_change(0)
            self._update_render()

        def _on_open(self):
            path, _ = QFileDialog.getOpenFileName(
                self, "Open H2hRT SBR JSON", "", "JSON Files (*.json);;All (*)"
            )
            if not path:
                path = QFileDialog.getExistingDirectory(self, "Open H2hRT output directory")
            if path:
                self._load(path, self.scene_path)

        def _on_scene(self):
            path, _ = QFileDialog.getOpenFileName(
                self, "Open scene OBJ", "", "OBJ Files (*.obj);;All (*)"
            )
            if path:
                self.scene_path = path
                self.scene_mesh = load_obj_mesh(path)
                self._update_render()

        def _on_reload(self):
            if self.input_path:
                self._load(self.input_path, self.scene_path)

        def _on_limit_change(self, value):
            self.max_paths_all = int(value)
            self._update_render()

        def _on_rx_change(self, idx):
            if not self.data or idx < 0:
                return
            self.current_rx = idx
            rec = self.data["records"][idx]
            paths = rec["paths"]
            seq_map = defaultdict(list)
            for pi, path in enumerate(paths):
                seq_map[path["sequence"]].append(pi)

            self.seq_combo.blockSignals(True)
            self.seq_combo.clear()
            self.seq_combo.addItem(f"(all) [{len(paths)}]", "")
            for seq in sorted(seq_map.keys(), key=lambda s: (-len(s), s)):
                self.seq_combo.addItem(f"{seq} [{len(seq_map[seq])}]", seq)
            self.seq_combo.blockSignals(False)
            self._apply_seq_filter()

        def _on_seq_change(self, _idx):
            self._apply_seq_filter()
            self._update_render()

        def _apply_seq_filter(self):
            if not self.data:
                return
            paths = self.data["records"][self.current_rx]["paths"]
            self.seq_filter = self.seq_combo.currentData() or ""
            if self.seq_filter:
                self.seq_paths = [
                    pi for pi, path in enumerate(paths) if path["sequence"] == self.seq_filter
                ]
            else:
                self.seq_paths = list(range(len(paths)))
            self.current_path = 0
            self.path_spin.blockSignals(True)
            self.path_spin.setMaximum(max(0, len(self.seq_paths) - 1))
            self.path_spin.setValue(0)
            self.path_total.setText(str(len(self.seq_paths)))
            self.path_spin.blockSignals(False)

        def _on_path_change(self, value):
            self.current_path = int(value)
            self._update_render()

        def _update_summary(self):
            if not self.data:
                return
            seq_counts = Counter()
            interaction_counts = Counter()
            loaded = 0
            declared = 0
            for rec in self.data["records"]:
                loaded += len(rec["paths"])
                declared += rec["path_count_declared"]
                for path in rec["paths"]:
                    seq_counts[path["sequence"]] += 1
                    for ch in path["sequence"]:
                        interaction_counts[ch] += 1

            lines = [
                f"Input: {self.data['input_path']}",
                f"Trace profile: {self.data.get('trace_profile', '')}",
                f"Rx records: {len(self.data['records'])}",
                f"Paths loaded/declared: {loaded:,}/{declared:,}",
                f"Tx: {self.data['tx']}",
                "",
                "Interactions:",
                f"  Rfl(r): {interaction_counts['r']:,}",
                f"  Trn(t): {interaction_counts['t']:,}",
                f"  Dif(d): {interaction_counts['d']:,}",
                "",
                "Top sequences:",
            ]
            for seq, count in seq_counts.most_common(12):
                lines.append(f"  {seq}: {count:,}")
            self.summary_text.setPlainText("\n".join(lines))

        def _update_render(self):
            if not self.data:
                return
            if not self.first_render and self.plotter.renderer.camera:
                self.camera_pos = self.plotter.camera_position

            self.plotter.clear()

            if self.chk_scene.isChecked() and self.scene_mesh is not None:
                self.plotter.add_mesh(
                    self.scene_mesh,
                    color="lightgray",
                    opacity=0.35,
                    style="surface",
                    show_edges=True,
                    edge_color="gray",
                    line_width=0.5,
                )

            tx = np.array([self.data["tx"]], dtype=np.float32)
            self.plotter.add_points(tx, color="red", point_size=35, render_points_as_spheres=True)

            records = self.data["records"]
            if self.chk_all_rx.isChecked():
                rx_pts = [rec["position"] for rec in records]
                if rx_pts:
                    self.plotter.add_points(
                        np.array(rx_pts, dtype=np.float32),
                        color="black",
                        point_size=16,
                        render_points_as_spheres=True,
                    )

            if self.current_rx < len(records):
                rec = records[self.current_rx]
                self.plotter.add_points(
                    np.array([rec["position"]], dtype=np.float32),
                    color="lime",
                    point_size=32,
                    render_points_as_spheres=True,
                )
                paths = rec["paths"]
                if self.chk_all_paths.isChecked():
                    for shown_i, pi in enumerate(self.seq_paths[: self.max_paths_all]):
                        self._draw_path(paths[pi], pi, highlight=(shown_i == self.current_path))
                elif self.current_path < len(self.seq_paths):
                    pi = self.seq_paths[self.current_path]
                    self._draw_path(paths[pi], pi, highlight=True)
                    self._update_path_info(paths[pi], pi, rec)

            self.plotter.show_grid(color="gray", font_size=8)
            if self.camera_pos is not None:
                self.plotter.camera_position = self.camera_pos
            elif self.first_render:
                self.plotter.camera_position = "iso"
            self.first_render = False

        def _draw_path(self, path, path_index, highlight=True):
            pts = path["points"]
            if len(pts) < 2:
                return
            arr = np.array(pts, dtype=np.float32)
            seq = path["sequence"]
            if highlight:
                for si in range(len(pts) - 1):
                    end_type = seq[si + 1] if si + 1 < len(seq) else "R"
                    color = self.segment_colors.get(end_type, [0.4, 0.4, 0.4])
                    seg = np.array([pts[si], pts[si + 1]], dtype=np.float32)
                    self.plotter.add_lines(seg, color=color, width=4, connected=True)
                for ni in range(1, len(pts) - 1):
                    ntype = seq[ni] if ni < len(seq) else "?"
                    color = self.node_colors.get(ntype, "gray")
                    self.plotter.add_points(
                        arr[ni : ni + 1],
                        color=color,
                        point_size=15,
                        render_points_as_spheres=True,
                    )
            else:
                color = self.path_colors[path_index % len(self.path_colors)]
                width = 1.4 if path["los"] else 0.7
                self.plotter.add_lines(arr, color=color, width=width, connected=True)

        def _update_path_info(self, path, path_index, rec):
            lines = [
                f"Rx: {rec['rx_label']} index={rec['rx_index']}",
                f"Path index: {path_index}",
                f"Path id: {path['path_id']}",
                f"Signature: {path['signature']}",
                f"Length: {path['len']:.6f} m",
                f"Sequence: {path['sequence']}",
                f"LoS: {path['los']}",
                f"Contains T: {path['contains_transmission']}",
                f"Geometry residual: {path['geometry_residual']:.6g}",
                f"Reflection residual: {path['reflection_residual_m']:.6g}",
                f"Max Snell residual: {path['max_snell_residual']:.6g}",
                f"Max Keller residual: {path['max_keller_residual']:.6g}",
                "",
                "Nodes:",
            ]
            for i, node in enumerate(path["nodes"]):
                name = interaction_name(node)
                lines.append(
                    f"  {i}: {name} "
                    f"face={node.get('face_id', -1)} "
                    f"wedge={node.get('wedge_id', -1)} "
                    f"obj={node.get('object_id', -1)} "
                    f"p=({node.get('x',0):.4f},{node.get('y',0):.4f},{node.get('z',0):.4f})"
                )
            self.info_text.setPlainText("\n".join(lines))


def run_headless(input_path, max_paths_per_rx=0):
    data = load_h2hrt_paths(input_path, max_paths_per_rx=max_paths_per_rx)
    print("=" * 72)
    print("H2hRT SBR geometry paths")
    print(f"Input: {data['input_path']}")
    print(f"Trace profile: {data.get('trace_profile', '')}")
    print(f"Rx records: {len(data['records'])}")
    print(f"Declared paths: {data['path_count_declared']:,}")
    print("=" * 72)
    total_loaded = 0
    total_declared = 0
    for rec in data["records"]:
        paths = rec["paths"]
        total_loaded += len(paths)
        total_declared += rec["path_count_declared"]
        seq_counts = Counter(p["sequence"] for p in paths)
        pos = rec["position"]
        print(
            f"{rec['rx_label']} Rx[{rec['rx_index']}] "
            f"({pos[0]:.3f},{pos[1]:.3f},{pos[2]:.3f}) "
            f"loaded/declared={len(paths):,}/{rec['path_count_declared']:,}"
        )
        for seq, count in seq_counts.most_common(8):
            print(f"  {seq}: {count:,}")
    print("-" * 72)
    print(f"Total loaded/declared: {total_loaded:,}/{total_declared:,}")


def parse_args(argv):
    parser = argparse.ArgumentParser(description="Visualize H2hRT SBR geometry paths")
    parser.add_argument("input", help="rx_0_sbr_paths.json or an output directory")
    parser.add_argument("--scene", default="", help="optional scene OBJ path")
    parser.add_argument(
        "--max-load-per-rx",
        type=int,
        default=0,
        help="truncate loaded paths per Rx after JSON parsing; 0 loads all",
    )
    parser.add_argument(
        "--headless",
        action="store_true",
        help="print summary only, do not open GUI",
    )
    return parser.parse_args(argv)


def main(argv):
    args = parse_args(argv)
    if args.headless or not HAS_GUI:
        run_headless(args.input, args.max_load_per_rx)
        if not HAS_GUI and not args.headless:
            print("\nInstall GUI deps: pip install PyQt5 pyvista pyvistaqt trimesh")
        return 0

    app = QApplication(sys.argv)
    window = H2hRtSbrVisualizer(args.input, args.scene, args.max_load_per_rx)
    window.show()
    return app.exec_()


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
