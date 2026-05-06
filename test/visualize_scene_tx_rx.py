#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
文件目标：
- 提供一个面向 A1/A2 及后续批次的 OBJ 场景与 Tx/Rx 位置可视化工具。

主要功能：
- 读取 OBJ 场景文件并显示三角网格；
- 在界面中输入算法使用的 Tx / Rx 坐标，并实时显示收发天线位置；
- 明确区分“算法原始坐标”和“仅用于显示的可选坐标变换”；
- 确保在默认模式下，可视化坐标与当前 RT 算法导入 OBJ 时使用的坐标完全一致；
- 为后续路径、命中点、覆盖结果等可视化功能继续扩展保留统一入口。

使用方式：
- 面向 Anaconda + PyCharm 使用；
- 不依赖命令行参数；
- 需要调整的路径和默认值，请修改脚本顶部“可修改配置区”；
- 在 PyCharm 中直接运行本脚本即可。
"""

from __future__ import annotations

import json
import math
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, Iterable, List, Sequence, Tuple

import numpy as np
import pyvista as pv
from pyvistaqt import QtInteractor
from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import (
    QApplication,
    QCheckBox,
    QComboBox,
    QDoubleSpinBox,
    QFormLayout,
    QGroupBox,
    QHBoxLayout,
    QLabel,
    QMainWindow,
    QMessageBox,
    QPushButton,
    QPlainTextEdit,
    QVBoxLayout,
    QWidget,
)


def resolve_repo_root() -> Path:
    """解析仓库根目录。"""
    return Path(__file__).resolve().parents[1]


REPO_ROOT = resolve_repo_root()


# =============================================================================
# 可修改配置区
# =============================================================================

OBJ_FILE_PATH = REPO_ROOT / "demo" / "meeting.obj"
APP_CONFIG_PATH = REPO_ROOT / "configs" / "app" / "minimal.json"

WINDOW_TITLE = "RT场景与Tx/Rx坐标可视化工具"
WINDOW_WIDTH = 1800
WINDOW_HEIGHT = 1100

# 说明：
# 1. 当前 RT 算法的 OBJImporter 没有做坐标变换，而是直接读取 OBJ 原始坐标；
# 2. 因此要保证“算法坐标 == 可视化坐标”，默认必须使用 RAW_IMPORTER_COORDS；
# 3. 若仅为了观察 Blender 坐标关系，可切换 display transform；
# 4. 切换后场景与 Tx/Rx 会一起做“显示变换”，保证视觉上仍对应同一算法坐标。
DEFAULT_DISPLAY_TRANSFORM = "RAW_IMPORTER_COORDS"

DEFAULT_TX = (1.0, 1.0, 1.0)
DEFAULT_RX = (3.0, 1.0, 1.0)
DEFAULT_PATH_JSON_PATH = REPO_ROOT / "output" / "a1_real_chain" / "paths" / "precise_paths.json"

MESH_COLOR = "#c7ccd6"
MESH_OPACITY = 0.92
EDGE_COLOR = "#4f5b66"
TX_COLOR = "#ff5a5f"
RX_COLOR = "#2d9cdb"
PATH_COLOR = "#ff9900"
REFLECTION_NODE_COLOR = "#ffcc33"
TRANSMISSION_NODE_COLOR = "#66cc66"
DIFFRACTION_NODE_COLOR = "#cc66ff"
RX_PATH_NODE_COLOR = "#2d9cdb"
TXRX_MARKER_RADIUS_RATIO = 0.018


# =============================================================================


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
    left: 12px;
    padding: 0 6px 0 6px;
    color: #555555;
}
QPushButton {
    background-color: #f0f0f0;
    border: 1px solid #cccccc;
    border-radius: 4px;
    padding: 5px 12px;
    font-size: 12px;
}
QPushButton:hover {
    background-color: #e5e5e5;
}
"""


@dataclass(frozen=True)
class DisplayTransformSpec:
    key: str
    title: str
    description: str


DISPLAY_TRANSFORMS: List[DisplayTransformSpec] = [
    DisplayTransformSpec(
        key="RAW_IMPORTER_COORDS",
        title="算法一致坐标（默认）",
        description="与当前 RT 算法 OBJImporter 完全一致：不做任何坐标轴变换。",
    ),
    DisplayTransformSpec(
        key="SWAP_YZ",
        title="实验：交换 Y/Z 显示",
        description="仅用于观察 Blender 与算法坐标差异；场景和 Tx/Rx 会一起变换，视觉对应保持一致。",
    ),
    DisplayTransformSpec(
        key="SWAP_YZ_FLIP_Z",
        title="实验：交换 Y/Z 并翻转新 Z",
        description="用于进一步试探 Blender 坐标系差异；不改变算法实际输入，只改变显示。",
    ),
]


def get_transform_spec(key: str) -> DisplayTransformSpec:
    for spec in DISPLAY_TRANSFORMS:
        if spec.key == key:
            return spec
    raise ValueError(f"未知显示变换模式: {key}")


def load_default_tx_rx_from_config(config_path: Path) -> Tuple[Tuple[float, float, float], Tuple[float, float, float]]:
    """从当前算法配置中读取默认 Tx/Rx；读取失败则回退到脚本默认值。"""
    try:
        with config_path.open("r", encoding="utf-8") as handle:
            data = json.load(handle)
        path_search = data.get("path_search", {})
        tx = (
            float(path_search.get("debug_tx_x", DEFAULT_TX[0])),
            float(path_search.get("debug_tx_y", DEFAULT_TX[1])),
            float(path_search.get("debug_tx_z", DEFAULT_TX[2])),
        )
        rx = (
            float(path_search.get("debug_rx_x", DEFAULT_RX[0])),
            float(path_search.get("debug_rx_y", DEFAULT_RX[1])),
            float(path_search.get("debug_rx_z", DEFAULT_RX[2])),
        )
        return tx, rx
    except Exception:
        return DEFAULT_TX, DEFAULT_RX


def transform_point_for_display(point: Sequence[float], transform_key: str) -> Tuple[float, float, float]:
    """将算法坐标变换为显示坐标。"""
    x, y, z = float(point[0]), float(point[1]), float(point[2])
    if transform_key == "RAW_IMPORTER_COORDS":
        return x, y, z
    if transform_key == "SWAP_YZ":
        return x, z, y
    if transform_key == "SWAP_YZ_FLIP_Z":
        return x, z, -y
    raise ValueError(f"不支持的显示变换: {transform_key}")


def transform_points_for_display(points: np.ndarray, transform_key: str) -> np.ndarray:
    """批量执行显示坐标变换。"""
    if points.size == 0:
        return points.copy()
    transformed = np.array([transform_point_for_display(point, transform_key) for point in points], dtype=float)
    return transformed


def parse_face_vertex_index(token: str) -> int:
    """解析 OBJ face token 中的 vertex index。"""
    if "/" in token:
        raw_index = token.split("/")[0]
    else:
        raw_index = token
    return int(raw_index) - 1


def triangulate_polygon(indices: List[int]) -> Iterable[Tuple[int, int, int]]:
    """将多边形按扇形方式三角化。"""
    if len(indices) < 3:
        return []
    if len(indices) == 3:
        return [(indices[0], indices[1], indices[2])]
    triangles = []
    for i in range(1, len(indices) - 1):
        triangles.append((indices[0], indices[i], indices[i + 1]))
    return triangles


def parse_obj_geometry(obj_path: Path) -> Tuple[np.ndarray, np.ndarray, List[str]]:
    """解析 OBJ，返回顶点、三角面索引和对象名列表。"""
    vertices: List[Tuple[float, float, float]] = []
    triangles: List[Tuple[int, int, int]] = []
    object_names: List[str] = []

    with obj_path.open("r", encoding="utf-8", errors="ignore") as handle:
        for raw_line in handle:
            line = raw_line.strip()
            if not line or line.startswith("#"):
                continue

            parts = line.split()
            tag = parts[0]
            if tag == "o":
                object_names.append(parts[1] if len(parts) > 1 else f"object_{len(object_names)}")
            elif tag == "v" and len(parts) >= 4:
                vertices.append((float(parts[1]), float(parts[2]), float(parts[3])))
            elif tag == "f" and len(parts) >= 4:
                polygon = [parse_face_vertex_index(token) for token in parts[1:]]
                triangles.extend(triangulate_polygon(polygon))

    if not vertices or not triangles:
        raise ValueError("OBJ 中未解析到有效顶点或三角面。")

    return np.array(vertices, dtype=float), np.array(triangles, dtype=int), object_names


def load_exported_paths(path_json: Path) -> List[dict]:
    """读取导出的路径 JSON；若不存在则返回空列表。"""
    if not path_json.exists():
        return []
    try:
        data = json.loads(path_json.read_text(encoding="utf-8"))
    except Exception:
        return []
    return data.get("paths", []) if isinstance(data, dict) else []


def build_polydata(vertices: np.ndarray, triangles: np.ndarray) -> pv.PolyData:
    """根据顶点与三角面构建 PyVista PolyData。"""
    faces = np.hstack([np.full((triangles.shape[0], 1), 3, dtype=np.int64), triangles.astype(np.int64)]).ravel()
    return pv.PolyData(vertices, faces)


def compute_bounds(points: np.ndarray) -> Dict[str, Tuple[float, float]]:
    """计算点云包围盒。"""
    return {
        "X": (float(np.min(points[:, 0])), float(np.max(points[:, 0]))),
        "Y": (float(np.min(points[:, 1])), float(np.max(points[:, 1]))),
        "Z": (float(np.min(points[:, 2])), float(np.max(points[:, 2]))),
    }


def build_clip_box_bounds(
    bounds: Dict[str, Tuple[float, float]],
    clip_axis: str,
    clip_threshold: float,
) -> Tuple[float, float, float, float, float, float]:
    """按“保留下半部分”的规则构造 clip box 边界。"""
    x_min, x_max = bounds["X"]
    y_min, y_max = bounds["Y"]
    z_min, z_max = bounds["Z"]

    if clip_axis == "X":
        x_max = clip_threshold
    elif clip_axis == "Y":
        y_max = clip_threshold
    elif clip_axis == "Z":
        z_max = clip_threshold
    else:
        raise ValueError(f"未知裁剪轴: {clip_axis}")

    return x_min, x_max, y_min, y_max, z_min, z_max


def format_bounds(bounds: Dict[str, Tuple[float, float]]) -> str:
    """格式化包围盒字符串。"""
    return (
        f"X:[{bounds['X'][0]:.3f}, {bounds['X'][1]:.3f}]  "
        f"Y:[{bounds['Y'][0]:.3f}, {bounds['Y'][1]:.3f}]  "
        f"Z:[{bounds['Z'][0]:.3f}, {bounds['Z'][1]:.3f}]"
    )


class SceneTxRxVisualizer(QMainWindow):
    """OBJ 场景与 Tx/Rx 可视化主窗口。"""

    def __init__(self, obj_path: Path, config_path: Path):
        super().__init__()
        self.obj_path = obj_path
        self.config_path = config_path

        self.raw_vertices, self.triangles, self.object_names = parse_obj_geometry(self.obj_path)
        self.raw_bounds = compute_bounds(self.raw_vertices)
        self.default_tx, self.default_rx = load_default_tx_rx_from_config(self.config_path)
        self.path_json_path = DEFAULT_PATH_JSON_PATH

        self.mesh_actor = None
        self.tx_actor = None
        self.rx_actor = None
        self.tx_label_actor = None
        self.rx_label_actor = None

        self.setWindowTitle(WINDOW_TITLE)
        self.resize(WINDOW_WIDTH, WINDOW_HEIGHT)
        self._build_ui()
        self._apply_default_values()
        self.refresh_scene()

    def _build_ui(self) -> None:
        central_widget = QWidget()
        self.setCentralWidget(central_widget)

        root_layout = QHBoxLayout(central_widget)
        root_layout.setContentsMargins(8, 8, 8, 8)
        root_layout.setSpacing(8)

        left_panel = QWidget()
        left_panel.setFixedWidth(460)
        left_layout = QVBoxLayout(left_panel)
        left_layout.setSpacing(8)

        self.plotter = QtInteractor(self)
        self.plotter.set_background("white")

        left_layout.addWidget(self._build_file_info_group())
        left_layout.addWidget(self._build_transform_group())
        left_layout.addWidget(self._build_clip_group())
        left_layout.addWidget(self._build_path_group())
        left_layout.addWidget(self._build_txrx_group())
        left_layout.addWidget(self._build_status_group())
        left_layout.addStretch(1)

        root_layout.addWidget(left_panel)
        root_layout.addWidget(self.plotter.interactor, 1)

        self.setStyleSheet(GROUP_BOX_STYLE)

    def _build_file_info_group(self) -> QGroupBox:
        group = QGroupBox("输入文件与坐标约定")
        layout = QVBoxLayout(group)

        self.obj_path_label = QLabel(f"OBJ: {self.obj_path}")
        self.obj_path_label.setWordWrap(True)
        self.config_path_label = QLabel(f"Config: {self.config_path}")
        self.config_path_label.setWordWrap(True)

        self.coord_rule_label = QLabel(
            "当前算法 OBJImporter 不做坐标轴变换，默认显示模式与算法坐标完全一致。"
        )
        self.coord_rule_label.setWordWrap(True)
        self.coord_rule_label.setStyleSheet("color: #1f5f99;")

        self.raw_bounds_label = QLabel()
        self.raw_bounds_label.setWordWrap(True)

        layout.addWidget(self.obj_path_label)
        layout.addWidget(self.config_path_label)
        layout.addWidget(self.coord_rule_label)
        layout.addWidget(self.raw_bounds_label)
        return group

    def _build_transform_group(self) -> QGroupBox:
        group = QGroupBox("显示坐标变换（只影响显示，不改算法输入）")
        layout = QVBoxLayout(group)

        self.transform_combo = QComboBox()
        for spec in DISPLAY_TRANSFORMS:
            self.transform_combo.addItem(spec.title, spec.key)
        self.transform_combo.currentIndexChanged.connect(self.refresh_scene)

        self.transform_desc_label = QLabel()
        self.transform_desc_label.setWordWrap(True)

        layout.addWidget(QLabel("显示模式："))
        layout.addWidget(self.transform_combo)
        layout.addWidget(self.transform_desc_label)
        return group

    def _build_txrx_group(self) -> QGroupBox:
        group = QGroupBox("Tx / Rx 算法坐标输入")
        layout = QVBoxLayout(group)

        tx_form = QFormLayout()
        self.tx_x_spin = self._create_coord_spin()
        self.tx_y_spin = self._create_coord_spin()
        self.tx_z_spin = self._create_coord_spin()
        tx_form.addRow("Tx X", self.tx_x_spin)
        tx_form.addRow("Tx Y", self.tx_y_spin)
        tx_form.addRow("Tx Z", self.tx_z_spin)

        rx_form = QFormLayout()
        self.rx_x_spin = self._create_coord_spin()
        self.rx_y_spin = self._create_coord_spin()
        self.rx_z_spin = self._create_coord_spin()
        rx_form.addRow("Rx X", self.rx_x_spin)
        rx_form.addRow("Rx Y", self.rx_y_spin)
        rx_form.addRow("Rx Z", self.rx_z_spin)

        for spin in [self.tx_x_spin, self.tx_y_spin, self.tx_z_spin, self.rx_x_spin, self.rx_y_spin, self.rx_z_spin]:
            spin.valueChanged.connect(self.refresh_scene)

        button_layout = QHBoxLayout()
        reset_button = QPushButton("恢复配置默认值")
        reset_button.clicked.connect(self.reset_tx_rx_to_config)
        save_button = QPushButton("回写当前Tx/Rx到配置文件")
        save_button.clicked.connect(self.write_current_tx_rx_to_config)
        refresh_button = QPushButton("刷新显示")
        refresh_button.clicked.connect(self.refresh_scene)
        button_layout.addWidget(reset_button)
        button_layout.addWidget(save_button)
        button_layout.addWidget(refresh_button)

        layout.addLayout(tx_form)
        layout.addSpacing(6)
        layout.addLayout(rx_form)
        layout.addSpacing(6)
        layout.addLayout(button_layout)
        return group

    def _build_path_group(self) -> QGroupBox:
        group = QGroupBox("A2 路径可视化")
        layout = QVBoxLayout(group)

        self.show_paths_checkbox = QCheckBox("显示导出的搜索路径")
        self.show_paths_checkbox.setChecked(True)
        self.show_paths_checkbox.stateChanged.connect(self.refresh_scene)

        self.path_json_label = QLabel(f"路径文件: {self.path_json_path}")
        self.path_json_label.setWordWrap(True)

        refresh_path_button = QPushButton("刷新路径显示")
        refresh_path_button.clicked.connect(self.refresh_scene)

        layout.addWidget(self.show_paths_checkbox)
        layout.addWidget(self.path_json_label)
        layout.addWidget(refresh_path_button)
        return group

    def _build_clip_group(self) -> QGroupBox:
        group = QGroupBox("按轴去顶裁剪（几何裁剪，不是整面隐藏）")
        layout = QVBoxLayout(group)

        self.enable_clip_checkbox = QCheckBox("启用去顶裁剪")
        self.enable_clip_checkbox.stateChanged.connect(self.refresh_scene)

        self.clip_axis_combo = QComboBox()
        self.clip_axis_combo.addItems(["X", "Y", "Z"])
        self.clip_axis_combo.currentIndexChanged.connect(self._on_clip_axis_changed)

        self.clip_threshold_spin = self._create_coord_spin()
        self.clip_threshold_spin.setSingleStep(0.1)
        self.clip_threshold_spin.valueChanged.connect(self.refresh_scene)

        self.clip_range_label = QLabel()
        self.clip_range_label.setWordWrap(True)

        form = QFormLayout()
        form.addRow("裁剪轴", self.clip_axis_combo)
        form.addRow("保留到该轴阈值", self.clip_threshold_spin)

        button_layout = QHBoxLayout()
        reset_clip_button = QPushButton("裁剪阈值设为当前轴最大值")
        reset_clip_button.clicked.connect(self.reset_clip_threshold_to_axis_max)
        button_layout.addWidget(reset_clip_button)

        layout.addWidget(self.enable_clip_checkbox)
        layout.addLayout(form)
        layout.addWidget(self.clip_range_label)
        layout.addLayout(button_layout)
        return group

    def _build_status_group(self) -> QGroupBox:
        group = QGroupBox("当前状态与人工核查提示")
        layout = QVBoxLayout(group)

        self.status_text = QPlainTextEdit()
        self.status_text.setReadOnly(True)
        self.status_text.setMinimumHeight(260)
        layout.addWidget(self.status_text)
        return group

    def _create_coord_spin(self) -> QDoubleSpinBox:
        spin = QDoubleSpinBox()
        spin.setDecimals(6)
        spin.setRange(-1000000.0, 1000000.0)
        spin.setSingleStep(0.1)
        return spin

    def _apply_default_values(self) -> None:
        default_index = next(
            (index for index, spec in enumerate(DISPLAY_TRANSFORMS) if spec.key == DEFAULT_DISPLAY_TRANSFORM),
            0,
        )
        self.transform_combo.setCurrentIndex(default_index)
        self.enable_clip_checkbox.setChecked(False)
        self.clip_axis_combo.setCurrentText("Z")
        self.reset_tx_rx_to_config()
        self._update_clip_controls_from_current_transform()

    def reset_tx_rx_to_config(self) -> None:
        tx, rx = load_default_tx_rx_from_config(self.config_path)
        self.tx_x_spin.setValue(tx[0])
        self.tx_y_spin.setValue(tx[1])
        self.tx_z_spin.setValue(tx[2])
        self.rx_x_spin.setValue(rx[0])
        self.rx_y_spin.setValue(rx[1])
        self.rx_z_spin.setValue(rx[2])

    def current_transform_key(self) -> str:
        return str(self.transform_combo.currentData())

    def current_tx_raw(self) -> Tuple[float, float, float]:
        return (self.tx_x_spin.value(), self.tx_y_spin.value(), self.tx_z_spin.value())

    def current_rx_raw(self) -> Tuple[float, float, float]:
        return (self.rx_x_spin.value(), self.rx_y_spin.value(), self.rx_z_spin.value())

    def current_clip_axis(self) -> str:
        return self.clip_axis_combo.currentText()

    def current_clip_threshold(self) -> float:
        return self.clip_threshold_spin.value()

    def _on_clip_axis_changed(self) -> None:
        self._update_clip_controls_from_current_transform()
        self.refresh_scene()

    def _update_clip_controls_from_current_transform(self) -> None:
        transform_key = self.current_transform_key()
        display_vertices = transform_points_for_display(self.raw_vertices, transform_key)
        display_bounds = compute_bounds(display_vertices)
        axis = self.current_clip_axis()
        axis_min, axis_max = display_bounds[axis]

        self.clip_threshold_spin.blockSignals(True)
        old_value = self.clip_threshold_spin.value()
        self.clip_threshold_spin.setRange(axis_min, axis_max)
        clamped_value = min(max(old_value, axis_min), axis_max)
        if math.isclose(old_value, 0.0, abs_tol=1e-12) and not self.enable_clip_checkbox.isChecked():
            clamped_value = axis_max
        self.clip_threshold_spin.setValue(clamped_value)
        self.clip_threshold_spin.blockSignals(False)

        self.clip_range_label.setText(
            f"当前显示坐标下 {axis} 轴范围：[{axis_min:.3f}, {axis_max:.3f}]；"
            f"启用裁剪后，保留 {axis} <= 阈值 的部分。"
        )

    def reset_clip_threshold_to_axis_max(self) -> None:
        transform_key = self.current_transform_key()
        display_vertices = transform_points_for_display(self.raw_vertices, transform_key)
        display_bounds = compute_bounds(display_vertices)
        _, axis_max = display_bounds[self.current_clip_axis()]
        self.clip_threshold_spin.setValue(axis_max)

    def write_current_tx_rx_to_config(self) -> None:
        tx_raw = self.current_tx_raw()
        rx_raw = self.current_rx_raw()
        with self.config_path.open("r", encoding="utf-8") as handle:
            config_data = json.load(handle)

        if "path_search" not in config_data or not isinstance(config_data["path_search"], dict):
            raise ValueError("配置文件中缺少 path_search 对象，无法回写 Tx/Rx。")

        config_data["path_search"]["debug_tx_x"] = tx_raw[0]
        config_data["path_search"]["debug_tx_y"] = tx_raw[1]
        config_data["path_search"]["debug_tx_z"] = tx_raw[2]
        config_data["path_search"]["debug_rx_x"] = rx_raw[0]
        config_data["path_search"]["debug_rx_y"] = rx_raw[1]
        config_data["path_search"]["debug_rx_z"] = rx_raw[2]

        with self.config_path.open("w", encoding="utf-8") as handle:
            json.dump(config_data, handle, ensure_ascii=False, indent=2)
            handle.write("\n")

        QMessageBox.information(self, "回写成功", f"已将当前 Tx/Rx 回写到配置文件:\n{self.config_path}")

    def refresh_scene(self) -> None:
        try:
            transform_key = self.current_transform_key()
            spec = get_transform_spec(transform_key)
            display_vertices = transform_points_for_display(self.raw_vertices, transform_key)
            display_bounds = compute_bounds(display_vertices)
            self._update_clip_controls_from_current_transform()

            self.raw_bounds_label.setText(
                "算法原始坐标包围盒：" + format_bounds(self.raw_bounds)
            )
            self.transform_desc_label.setText(spec.description)

            polydata = build_polydata(display_vertices, self.triangles)
            clip_enabled = self.enable_clip_checkbox.isChecked()
            clip_axis = self.current_clip_axis()
            clip_threshold = self.current_clip_threshold()
            if clip_enabled:
                clip_bounds = build_clip_box_bounds(display_bounds, clip_axis, clip_threshold)
                polydata = polydata.clip_box(clip_bounds, invert=False)

            self.plotter.clear()
            self.plotter.add_axes()
            self.mesh_actor = self.plotter.add_mesh(
                polydata,
                color=MESH_COLOR,
                opacity=MESH_OPACITY,
                show_edges=True,
                edge_color=EDGE_COLOR,
                line_width=1.0,
                name="scene_mesh",
            )

            tx_raw = self.current_tx_raw()
            rx_raw = self.current_rx_raw()
            tx_display = transform_point_for_display(tx_raw, transform_key)
            rx_display = transform_point_for_display(rx_raw, transform_key)

            scene_span = max(
                display_bounds["X"][1] - display_bounds["X"][0],
                display_bounds["Y"][1] - display_bounds["Y"][0],
                display_bounds["Z"][1] - display_bounds["Z"][0],
            )
            marker_radius = max(scene_span * TXRX_MARKER_RADIUS_RATIO, 0.03)

            tx_sphere = pv.Sphere(radius=marker_radius, center=tx_display)
            rx_sphere = pv.Sphere(radius=marker_radius, center=rx_display)
            self.tx_actor = self.plotter.add_mesh(tx_sphere, color=TX_COLOR, name="tx_marker")
            self.rx_actor = self.plotter.add_mesh(rx_sphere, color=RX_COLOR, name="rx_marker")

            self.plotter.add_point_labels(
                np.array([tx_display, rx_display], dtype=float),
                ["Tx", "Rx"],
                point_size=0,
                font_size=14,
                shape=None,
                always_visible=True,
                name="txrx_labels",
            )

            exported_paths = load_exported_paths(self.path_json_path) if self.show_paths_checkbox.isChecked() else []
            self._draw_exported_paths(exported_paths, transform_key, marker_radius)

            self.plotter.reset_camera()
            self._update_status_text(
                spec,
                display_bounds,
                tx_raw,
                rx_raw,
                tx_display,
                rx_display,
                clip_enabled,
                clip_axis,
                clip_threshold,
                exported_paths,
            )
        except Exception as ex:
            QMessageBox.critical(self, "刷新显示失败", str(ex))

    def _draw_exported_paths(self, exported_paths: List[dict], transform_key: str, marker_radius: float) -> None:
        if not exported_paths:
            return

        for path_index, path_item in enumerate(exported_paths):
            geometry_nodes = path_item.get("geometry_nodes", [])
            if len(geometry_nodes) < 2:
                continue

            points = []
            for node in geometry_nodes:
                display_point = transform_point_for_display((node["x"], node["y"], node["z"]), transform_key)
                points.append(display_point)

            points_np = np.array(points, dtype=float)
            polyline = pv.lines_from_points(points_np, close=False)
            self.plotter.add_mesh(polyline, color=PATH_COLOR, line_width=4.0, name=f"path_line_{path_index}")

            for node_index, node in enumerate(geometry_nodes):
                interaction_type = int(node.get("interaction_type", -1))
                if interaction_type == 4:
                    node_color = REFLECTION_NODE_COLOR
                elif interaction_type == 5:
                    node_color = TRANSMISSION_NODE_COLOR
                elif interaction_type == 6:
                    node_color = DIFFRACTION_NODE_COLOR
                elif interaction_type == 2:
                    node_color = RX_PATH_NODE_COLOR
                else:
                    node_color = PATH_COLOR

                sphere = pv.Sphere(radius=max(marker_radius * 0.45, 0.015), center=points[node_index])
                self.plotter.add_mesh(sphere, color=node_color, name=f"path_node_{path_index}_{node_index}")

    def _update_status_text(
        self,
        spec: DisplayTransformSpec,
        display_bounds: Dict[str, Tuple[float, float]],
        tx_raw: Tuple[float, float, float],
        rx_raw: Tuple[float, float, float],
        tx_display: Tuple[float, float, float],
        rx_display: Tuple[float, float, float],
        clip_enabled: bool,
        clip_axis: str,
        clip_threshold: float,
        exported_paths: List[dict],
    ) -> None:
        lines: List[str] = []
        lines.append("当前可视化状态")
        lines.append("=" * 72)
        lines.append(f"OBJ文件: {self.obj_path}")
        lines.append(f"配置文件: {self.config_path}")
        lines.append(f"对象块数量: {len(self.object_names)}")
        lines.append(f"顶点数: {len(self.raw_vertices)}")
        lines.append(f"三角面数: {len(self.triangles)}")
        lines.append("")
        lines.append("坐标规则")
        lines.append("- 当前算法 OBJImporter 直接读取 OBJ 原始坐标，不做轴变换。")
        lines.append("- 因此在“算法一致坐标（默认）”模式下，你在界面输入的 Tx/Rx 就是算法里该填的值。")
        lines.append("- 若切到实验性显示模式，脚本只会同步改变“显示坐标”，不会改变算法输入本身。")
        lines.append("")
        lines.append(f"当前显示模式: {spec.title}")
        lines.append(f"模式说明: {spec.description}")
        lines.append("算法原始包围盒: " + format_bounds(self.raw_bounds))
        lines.append("显示包围盒: " + format_bounds(display_bounds))
        lines.append("")
        lines.append("去顶裁剪状态")
        if clip_enabled:
            lines.append(f"- 已启用：保留 {clip_axis} <= {clip_threshold:.6f} 的部分")
            lines.append("- 当前采用的是几何裁剪：三角面跨越阈值时，只裁掉超出阈值的那一部分，不会整面直接隐藏。")
        else:
            lines.append("- 未启用")
        lines.append("")
        lines.append("算法输入坐标（直接可用于 RT 配置）")
        lines.append(f"- Tx raw = ({tx_raw[0]:.6f}, {tx_raw[1]:.6f}, {tx_raw[2]:.6f})")
        lines.append(f"- Rx raw = ({rx_raw[0]:.6f}, {rx_raw[1]:.6f}, {rx_raw[2]:.6f})")
        lines.append("")
        lines.append("当前显示坐标")
        lines.append(f"- Tx display = ({tx_display[0]:.6f}, {tx_display[1]:.6f}, {tx_display[2]:.6f})")
        lines.append(f"- Rx display = ({rx_display[0]:.6f}, {rx_display[1]:.6f}, {rx_display[2]:.6f})")
        lines.append("")
        lines.append("A2 路径可视化状态")
        lines.append(f"- 路径文件: {self.path_json_path}")
        lines.append(f"- 当前读到的路径条数: {len(exported_paths)}")
        lines.append("- 若路径条数为 0，请先运行 RT 主程序生成最新导出。")
        lines.append("")
        lines.append("人工核查建议")
        lines.append("1. 默认先保持“算法一致坐标（默认）”，这样看到的位置就是算法位置。")
        lines.append("2. 修改 Tx/Rx 输入框后，看红色 Tx、蓝色 Rx 标记是否落在你期望的室内位置。")
        lines.append("3. 若启用去顶裁剪，当前显示的是几何裁掉后的上半部分，不是简单按面元整块隐藏。")
        lines.append("4. 若启用路径显示，请检查折线是否从 Tx 合理经过反射/透射/绕射节点到达 Rx，而不是穿墙乱飞。")
        lines.append("5. 若你怀疑 Blender 坐标系和算法坐标不一致，可切换实验模式观察，但不要直接把实验显示坐标当算法输入。")
        lines.append("6. 当你确认某组 raw 坐标位置合适后，可点“回写当前Tx/Rx到配置文件”，脚本会直接修改当前读取的配置文件中的 debug_tx_* / debug_rx_* 字段。")
        self.status_text.setPlainText("\n".join(lines))


def main() -> int:
    if not OBJ_FILE_PATH.exists():
        print(f"OBJ文件不存在: {OBJ_FILE_PATH}")
        return 1

    app = QApplication(sys.argv)
    pv.global_theme.allow_empty_mesh = True

    try:
        window = SceneTxRxVisualizer(OBJ_FILE_PATH, APP_CONFIG_PATH)
    except Exception as ex:
        QMessageBox.critical(None, "初始化失败", str(ex))
        return 1

    window.show()
    return app.exec_()


if __name__ == "__main__":
    sys.exit(main())
