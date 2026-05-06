#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
RT 全功能可视化工具 — 场景 · Tx/Rx · 多径 · 材质 · 裁剪 · 回写配置
一键运行: python test/rt_visual.py
"""

import sys, json, os, warnings
from pathlib import Path
from typing import Dict, List, Optional, Tuple

import numpy as np
import trimesh
import pyvista as pv
from pyvistaqt import QtInteractor
from PyQt5.QtCore import Qt, QTimer
from PyQt5.QtWidgets import (
    QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout,
    QLabel, QComboBox, QCheckBox, QDoubleSpinBox, QPushButton,
    QGroupBox, QFormLayout, QPlainTextEdit, QFileDialog, QMessageBox,
    QTabWidget, QProgressBar, QSplitter,
)

warnings.filterwarnings("ignore")
pv.set_plot_theme("document")

# ─── 路径常量 ─────────────────────────────────────────
ROOT = Path(__file__).resolve().parents[1]
DEFAULT_OBJ = ROOT / "demo" / "meeting.obj"
DEFAULT_CFG = ROOT / "configs" / "app" / "meeting_v3.json"
DEFAULT_MAT = ROOT / "configs" / "scenes" / "scene_material_map.json"
DEFAULT_PTH = ROOT / "output" / "a1_real_chain" / "paths" / "precise_paths.json"

# ─── 样式常量 ─────────────────────────────────────────
TX_CLR, RX_CLR, PATH_CLR, MESH_CLR = "#ff5a5f", "#2d9cdb", "#ff9900", "#c7ccd6"
NODE_CLR = {4: "#ffcc33", 5: "#66cc66", 6: "#cc66ff"}  # R / T / D
SEG_CLR  = {1: TX_CLR, 2: RX_CLR, 3: PATH_CLR, 4: "#ffcc33", 5: "#66cc66", 6: "#cc66ff"}
MAT_CLR = {"Concrete": "#8c8c8c", "Glass": "#66c2ff", "Wood": "#c68642",
           "Brick": "#b55239", "Metal": "#555555", "Air": "#ddd"}
GROUP_CSS = """QGroupBox{font-weight:bold;border:1px solid #ccc;border-radius:4px;margin-top:8px;padding-top:12px}
QGroupBox::title{subcontrol-origin:margin;left:10px;padding:0 4px}"""

# ─── 工具函数 ────────────────────────────────────────
def _load_json(p): return json.loads(Path(p).read_text(encoding="utf-8")) if Path(p).exists() else {}
def _txrx(cfg): ps = cfg.get("path_search", {}); return ((ps.get("debug_tx_x", 1.), ps.get("debug_tx_y", 1.), ps.get("debug_tx_z", 1.)), (ps.get("debug_rx_x", 3.), ps.get("debug_rx_y", 1.), ps.get("debug_rx_z", 1.)))
def _bounds(v): return {"X": (float(v[:, 0].min()), float(v[:, 0].max())), "Y": (float(v[:, 1].min()), float(v[:, 1].max())), "Z": (float(v[:, 2].min()), float(v[:, 2].max()))}
def _radius(b): return max(b["X"][1]-b["X"][0], b["Y"][1]-b["Y"][0], b["Z"][1]-b["Z"][0]) * 0.018
def _clip_box(b, ax, v):
    x0, x1 = b["X"]; y0, y1 = b["Y"]; z0, z1 = b["Z"]
    if ax == "X": x1 = v
    elif ax == "Y": y1 = v
    elif ax == "Z": z1 = v
    return x0, x1, y0, y1, z0, z1


class RTVisualizer(QMainWindow):
    def __init__(self, obj: Path, cfg: Path, mat: Optional[Path], pth: Optional[Path]):
        super().__init__()
        self.obj_p, self.cfg_p, self.mat_p, self.pth_p = obj, cfg, mat, pth
        self.data_ok = False; self.verts = np.zeros((0, 3)); self.tris = np.zeros((0, 3), int)
        self.paths = []; self.cfg = {}; self.mat_rules = {}; self.face_mat = {}
        self.setWindowTitle(f"RT Visual — {obj.name}")
        self.resize(1800, 1050)
        self._ui()
        QTimer.singleShot(20, self._load)

    # ── UI ────────────────────────────────────────────
    def _ui(self):
        cw = QWidget(); self.setCentralWidget(cw)
        root = QHBoxLayout(cw); root.setContentsMargins(6, 6, 6, 6)
        # left panel
        left = QWidget(); left.setFixedWidth(400); ll = QVBoxLayout(left); ll.setSpacing(6)
        self.tabs = QTabWidget()
        self.tabs.addTab(self._tab_scene(), "场景")
        self.tabs.addTab(self._tab_paths(), "路径")
        self.tabs.addTab(self._tab_info(), "信息")
        ll.addWidget(self.tabs)
        self.progress = QProgressBar(); self.progress.setVisible(False); ll.addWidget(self.progress)
        ll.addStretch()
        root.addWidget(left)
        # plotter
        self.plt = QtInteractor(self); self.plt.set_background("white")
        root.addWidget(self.plt.interactor, 1)
        self.setStyleSheet(GROUP_CSS)

    def _tab_scene(self):
        w = QWidget(); l = QVBoxLayout(w)
        # file bar
        fb = QHBoxLayout()
        b1 = QPushButton("OBJ…"); b1.clicked.connect(lambda: self._pick("obj"))
        b2 = QPushButton("Config…"); b2.clicked.connect(lambda: self._pick("cfg"))
        fb.addWidget(b1); fb.addWidget(b2); fb.addStretch(); l.addLayout(fb)
        # Tx/Rx
        g = QGroupBox("Tx / Rx"); gl = QFormLayout(g)
        self.tx_s = [QDoubleSpinBox() for _ in range(3)]; self.rx_s = [QDoubleSpinBox() for _ in range(3)]
        for s, lb in zip(self.tx_s, "XYZ"): s.setRange(-1e4, 1e4); s.setDecimals(4); gl.addRow(f"Tx {lb}", s); s.valueChanged.connect(self._redraw)
        for s, lb in zip(self.rx_s, "XYZ"): s.setRange(-1e4, 1e4); s.setDecimals(4); gl.addRow(f"Rx {lb}", s); s.valueChanged.connect(self._redraw)
        bl = QHBoxLayout(); b3 = QPushButton("回写Config"); b3.clicked.connect(self._writeback); bl.addWidget(b3)
        l.addWidget(g); l.addLayout(bl)
        # clip
        g2 = QGroupBox("裁剪"); gl2 = QFormLayout(g2)
        self.clip_on = QCheckBox("启用"); self.clip_on.stateChanged.connect(self._redraw)
        self.clip_ax = QComboBox(); self.clip_ax.addItems(["X","Y","Z"]); self.clip_ax.currentTextChanged.connect(self._redraw)
        self.clip_v = QDoubleSpinBox(); self.clip_v.setRange(-1e5, 1e5); self.clip_v.setDecimals(2); self.clip_v.valueChanged.connect(self._redraw)
        gl2.addRow(self.clip_on); gl2.addRow("轴", self.clip_ax); gl2.addRow("阈值", self.clip_v)
        l.addWidget(g2); l.addStretch(); return w

    def _tab_paths(self):
        w = QWidget(); l = QVBoxLayout(w)
        self.path_on = QCheckBox("显示路径"); self.path_on.setChecked(True); self.path_on.stateChanged.connect(self._redraw)
        l.addWidget(self.path_on)
        self.path_info = QLabel(); self.path_info.setWordWrap(True); l.addWidget(self.path_info)
        l.addStretch(); return w

    def _tab_info(self):
        w = QWidget(); l = QVBoxLayout(w)
        self.info_txt = QPlainTextEdit(); self.info_txt.setReadOnly(True); l.addWidget(self.info_txt)
        return w

    # ── Data loading ──────────────────────────────────
    def _load(self):
        self.progress.setVisible(True); self.progress.setFormat("加载 OBJ…"); QApplication.processEvents()
        try:
            m = trimesh.load(str(self.obj_p), process=False, skip_materials=True)
            self.verts = m.vertices.astype(float); self.tris = m.faces.astype(int)
            self.progress.setFormat("加载配置…"); QApplication.processEvents()
            self.cfg = _load_json(self.cfg_p) if self.cfg_p.exists() else {}
            tx, rx = _txrx(self.cfg)
            for s, v in zip(self.tx_s, tx): s.setValue(v)
            for s, v in zip(self.rx_s, rx): s.setValue(v)
            if self.mat_p and self.mat_p.exists():
                self.progress.setFormat("加载材质…"); QApplication.processEvents()
                raw = _load_json(self.mat_p)
                self.mat_rules = {o["object_name"]: o for o in raw.get("objects", [])}
            if self.pth_p and self.pth_p.exists():
                self.progress.setFormat("加载路径…"); QApplication.processEvents()
                j = _load_json(self.pth_p)
                self.paths = j.get("paths", []) if isinstance(j, dict) else []
            self.data_ok = True
            self.path_info.setText(f"{len(self.paths)} 条路径")
            self._update_info()
            self._redraw()
        except Exception as e:
            QMessageBox.critical(self, "加载失败", str(e))
        self.progress.setVisible(False)

    # ── Redraw ────────────────────────────────────────
    def _redraw(self):
        if not self.data_ok: return
        self.plt.clear()
        b = _bounds(self.verts); r = _radius(b)
        # scene mesh
        if self.clip_on.isChecked():
            cb = _clip_box(b, self.clip_ax.currentText(), self.clip_v.value())
            pd = pv.PolyData.from_regular_faces(self.verts, self.tris).clip_box(cb, invert=False)
        else:
            pd = pv.PolyData.from_regular_faces(self.verts, self.tris)
        show_e = len(self.tris) <= 3000
        self.plt.add_mesh(pd, color=MESH_CLR, opacity=0.85, show_edges=show_e,
                          edge_color="#888888", smooth_shading=True, name="scene")
        # Tx/Rx
        tx = tuple(s.value() for s in self.tx_s); rx = tuple(s.value() for s in self.rx_s)
        self.plt.add_mesh(pv.Sphere(radius=r, center=tx), color=TX_CLR, name="tx")
        self.plt.add_mesh(pv.Sphere(radius=r, center=rx), color=RX_CLR, name="rx")
        self.plt.add_point_labels(np.array([tx, rx]), ["Tx", "Rx"], point_size=0, font_size=12, always_visible=True)
        # paths — batch render, color-coded by interaction type
        if self.path_on.isChecked() and self.paths:
            groups = {3: [], 4: [], 5: [], 6: []}  # LOS / R / T / D
            for p in self.paths[:200]:
                nds = p.get("geometry_nodes", [])
                if len(nds) < 2: continue
                for i in range(len(nds) - 1):
                    nxt = nds[i + 1]
                    seg_type = nxt.get("interaction_type", 3) if nxt.get("interaction_type") != 2 else 3
                    if seg_type not in groups: seg_type = 3
                    p0 = (nds[i]["x"], nds[i]["y"], nds[i]["z"])
                    p1 = (nxt["x"], nxt["y"], nxt["z"])
                    groups[seg_type].append((p0, p1))
            for seg_type, segs in groups.items():
                if not segs: continue
                pts = []; lns = []; k = 0
                for a, b in segs:
                    pts.extend([a, b]); lns.extend([2, k, k + 1]); k += 2
                poly = pv.PolyData(np.array(pts, float), lines=np.array(lns, int))
                clr = SEG_CLR.get(seg_type, PATH_CLR)
                self.plt.add_mesh(poly, color=clr, line_width=seg_type * 0.8, render_lines_as_tubes=False, name=f"seg_{seg_type}")
        self.plt.add_axes(); self.plt.reset_camera()
        try: self._update_info()
        except Exception: pass

    def _update_info(self):
        if not self.data_ok: return
        tx = tuple(s.value() for s in self.tx_s); rx = tuple(s.value() for s in self.rx_s)
        lines = ["图例:  Tx(红)  Rx(蓝)  LOS(橙)  R(黄)  T(绿)  D(紫)",
                 f"OBJ: {self.obj_p.name}  |  顶点 {len(self.verts)}  面 {len(self.tris)}",
                 f"Tx: ({tx[0]:.4f}, {tx[1]:.4f}, {tx[2]:.4f})   Rx: ({rx[0]:.4f}, {rx[1]:.4f}, {rx[2]:.4f})",
                 f"路径: {len(self.paths)} 条"]
        # classify
        from collections import Counter
        c = Counter(); c["LOS"] = 0
        for p in self.paths:
            ts = [n.get("interaction_type", 0) for n in p.get("geometry_nodes", []) if n.get("interaction_type") not in (1, 2)]
            if not ts: c["LOS"] += 1
            else:
                for t in ts: c[{4: "R", 5: "T", 6: "D"}.get(t, "?")] += 1
        lines.append("路径分布: " + ", ".join(f"{k}={v}" for k, v in c.most_common(12)))
        self.info_txt.setPlainText("\n".join(lines))

    # ── Actions ───────────────────────────────────────
    def _pick(self, kind):
        d, flt = str(ROOT / "demo"), "OBJ (*.obj)" if kind == "obj" else "JSON (*.json)"
        p, _ = QFileDialog.getOpenFileName(self, "选择", d, flt)
        if not p: return
        if kind == "obj": self.obj_p = Path(p)
        elif kind == "cfg": self.cfg_p = Path(p)
        self._load()

    def _writeback(self):
        tx = tuple(s.value() for s in self.tx_s); rx = tuple(s.value() for s in self.rx_s)
        cfg = _load_json(self.cfg_p)
        ps = cfg.setdefault("path_search", {})
        ps["debug_tx_x"], ps["debug_tx_y"], ps["debug_tx_z"] = tx
        ps["debug_rx_x"], ps["debug_rx_y"], ps["debug_rx_z"] = rx
        self.cfg_p.write_text(json.dumps(cfg, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
        QMessageBox.information(self, "OK", f"已写入 {self.cfg_p.name}")

    def closeEvent(self, ev):
        self.plt.close(); super().closeEvent(ev)


# ─── Entry ────────────────────────────────────────────
if __name__ == "__main__":
    import argparse
    ap = argparse.ArgumentParser(description="RT 全功能可视化")
    ap.add_argument("--obj", default=str(DEFAULT_OBJ)); ap.add_argument("--config", default=str(DEFAULT_CFG))
    ap.add_argument("--material-map", default=str(DEFAULT_MAT)); ap.add_argument("--paths", default=str(DEFAULT_PTH))
    args = ap.parse_args()
    app = QApplication(sys.argv)
    w = RTVisualizer(Path(args.obj), Path(args.config), Path(args.material_map) if args.material_map else None, Path(args.paths) if args.paths else None)
    w.show()
    sys.exit(app.exec_())
