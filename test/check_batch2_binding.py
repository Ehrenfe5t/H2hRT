#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
文件目标：
- 提供一个面向批次2人工核查的交互式可视化工具。

主要功能：
- 解析 OBJ 文件中的对象、顶点、法向和三角面；
- 解析 scene_material_map.json 中的对象级规则；
- 以 PyQt + PyVista 的方式显示场景并支持交互筛选；
- 支持按对象语义或背侧材质着色；
- 支持按坐标轴对上半部分做去顶裁剪；
- 支持在界面中查看对象级绑定摘要与当前过滤结果。

使用方式：
- 面向 Anaconda + PyCharm 使用；
- 不依赖命令行参数；
- 需要调整的路径和默认行为，请修改下方“可修改配置区”；
- 在 PyCharm 中直接运行本脚本即可。
"""

from __future__ import annotations

import json
import sys
from collections import defaultdict
from pathlib import Path
from typing import Any, Dict, List, Tuple

import numpy as np
import pyvista as pv
from pyvistaqt import QtInteractor
from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import (
    QApplication,
    QCheckBox,
    QComboBox,
    QDoubleSpinBox,
    QGroupBox,
    QHBoxLayout,
    QLabel,
    QMainWindow,
    QMessageBox,
    QPushButton,
    QPlainTextEdit,
    QScrollArea,
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

# 输入文件路径
OBJ_FILE_PATH = REPO_ROOT / "demo" / "meeting.obj"
RULE_FILE_PATH = REPO_ROOT / "configs" / "scenes" / "scene_material_map.json"

# 可视化默认行为
WINDOW_TITLE = "批次2对象语义与双侧材质检查工具"
WINDOW_WIDTH = 1800
WINDOW_HEIGHT = 1100
DEFAULT_COLOR_MODE = "按背侧材质"
DEFAULT_SHOW_EDGE = True
DEFAULT_SHOW_LEGEND = True
DEFAULT_SHOW_LABELS = False
DEFAULT_SUMMARY_FACE_COUNT = 20

# 裁剪默认行为
DEFAULT_ENABLE_CLIP = False
DEFAULT_CLIP_AXIS = "Z"
DEFAULT_CLIP_KEEP_LOWER_RATIO = 0.50

# 显示参数
DEFAULT_FACE_OPACITY = 0.90
DEFAULT_EDGE_WIDTH = 1.0
SWAP_YZ_FOR_DISPLAY = False

# 导出检查结果
EXPORT_JSON_RESULT = True
EXPORT_JSON_PATH = REPO_ROOT / "test" / "output" / "batch2_gui_check.json"

# 调色板
COLOR_BY_BACK_MATERIAL = {
    "Concrete": "#8c8c8c",
    "Glass": "#66c2ff",
    "Wood": "#c68642",
    "Brick": "#b55239",
    "Air": "#dddddd",
    "": "#ff66aa",
}

COLOR_BY_OBJECT_TYPE = {
    "floor": "#999999",
    "ceiling": "#cfcfcf",
    "wall": "#8b8b8b",
    "window": "#66c2ff",
    "door": "#c68642",
    "table": "#d2a679",
    "partition": "#cc7a5c",
    "": "#ff66aa",
}

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
    left: 15px;
    padding: 0 8px 0 8px;
    color: #555555;
}
QPushButton {
    background-color: #f0f0f0;
    border: 1px solid #cccccc;
    border-radius: 4px;
    padding: 4px 12px;
    font-size: 12px;
}
QPushButton:hover {
    background-color: #e0e0e0;
}
"""


def validate_config() -> None:
    """校验脚本顶部配置是否合法。"""
    if DEFAULT_COLOR_MODE not in {"按背侧材质", "按对象语义", "按对象名称"}:
        raise ValueError("DEFAULT_COLOR_MODE 仅允许为：按背侧材质 / 按对象语义 / 按对象名称")
    if DEFAULT_CLIP_AXIS not in {"X", "Y", "Z"}:
        raise ValueError("DEFAULT_CLIP_AXIS 仅允许为 X / Y / Z")
    if not (0.05 <= DEFAULT_CLIP_KEEP_LOWER_RATIO <= 0.95):
        raise ValueError("DEFAULT_CLIP_KEEP_LOWER_RATIO 建议位于 [0.05, 0.95] 之间")


def convert_display_coordinate(vertex: Tuple[float, float, float]) -> Tuple[float, float, float]:
    """按显示配置决定是否交换 Y/Z 坐标。"""
    if not SWAP_YZ_FOR_DISPLAY:
        return vertex
    x, y, z = vertex
    return x, z, y


def parse_obj(obj_path: Path) -> Tuple[np.ndarray, np.ndarray, List[Dict[str, Any]], List[Dict[str, Any]]]:
    """解析 OBJ 文本，返回顶点数组、法向数组、对象记录和面元记录。"""
    vertices: List[Tuple[float, float, float]] = []
    normals: List[Tuple[float, float, float]] = []
    objects: List[Dict[str, Any]] = []
    faces: List[Dict[str, Any]] = []

    current_object_name = None
    current_object_id = -1

    with obj_path.open("r", encoding="utf-8", errors="ignore") as handle:
        for raw_line in handle:
            line = raw_line.strip()
            if not line or line.startswith("#"):
                continue

            parts = line.split()
            tag = parts[0]

            if tag == "o":
                current_object_name = parts[1]
                current_object_id = len(objects)
                objects.append(
                    {
                        "object_id": current_object_id,
                        "object_name": current_object_name,
                        "face_ids": [],
                    }
                )
            elif tag == "v":
                vertices.append(convert_display_coordinate(tuple(map(float, parts[1:4]))))
            elif tag == "vn":
                normals.append(convert_display_coordinate(tuple(map(float, parts[1:4]))))
            elif tag == "f":
                vertex_indices: List[int] = []
                normal_index = -1
                for token in parts[1:4]:
                    v_str, n_str = token.split("//")
                    vertex_indices.append(int(v_str) - 1)
                    normal_index = int(n_str) - 1

                face_id = len(faces)
                faces.append(
                    {
                        "face_id": face_id,
                        "object_id": current_object_id,
                        "object_name": current_object_name,
                        "vertex_indices": vertex_indices,
                        "normal_index": normal_index,
                    }
                )
                if current_object_id >= 0:
                    objects[current_object_id]["face_ids"].append(face_id)

    return np.array(vertices, dtype=float), np.array(normals, dtype=float), objects, faces


def load_rule_file(rule_path: Path) -> Tuple[str, Dict[str, Dict[str, Any]]]:
    """读取规则文件，返回默认介质和对象名到规则的映射。"""
    with rule_path.open("r", encoding="utf-8") as handle:
        data = json.load(handle)

    default_medium = data.get("default_medium", "Air")
    rules: Dict[str, Dict[str, Any]] = {}
    for obj_rule in data.get("objects", []):
        object_name = obj_rule.get("object_name")
        if object_name:
            rules[object_name] = obj_rule
    return default_medium, rules


def build_binding_summary(
    objects: List[Dict[str, Any]],
    faces: List[Dict[str, Any]],
    rules: Dict[str, Dict[str, Any]],
    default_medium: str,
) -> Tuple[List[Dict[str, Any]], Dict[int, Dict[str, Any]]]:
    """根据规则构造对象级和面元级绑定摘要。"""
    object_summaries: List[Dict[str, Any]] = []
    face_binding_map: Dict[int, Dict[str, Any]] = {}

    for obj in objects:
        object_name = obj["object_name"]
        rule = rules.get(object_name)
        face_ids = obj["face_ids"]

        if rule is None:
            summary = {
                "object_name": object_name,
                "object_type": "",
                "surface_material": "",
                "front_material": "",
                "back_material": "",
                "resolved": False,
                "resolved_faces": 0,
                "total_faces": len(face_ids),
                "normal_rule": "",
                "reflection_enabled": False,
                "transmission_enabled": False,
                "diffraction_candidate_enabled": False,
            }
            object_summaries.append(summary)
            continue

        front_material = rule.get("front_material_name") or default_medium
        back_material = rule.get("back_material_name") or rule.get("surface_material_name", "")

        summary = {
            "object_name": object_name,
            "object_type": rule.get("object_type", ""),
            "surface_material": rule.get("surface_material_name", ""),
            "front_material": front_material,
            "back_material": back_material,
            "resolved": True,
            "resolved_faces": len(face_ids),
            "total_faces": len(face_ids),
            "normal_rule": rule.get("normal_rule", ""),
            "reflection_enabled": bool(rule.get("reflection_enabled", True)),
            "transmission_enabled": bool(rule.get("transmission_enabled", False)),
            "diffraction_candidate_enabled": bool(rule.get("diffraction_candidate_enabled", False)),
        }
        object_summaries.append(summary)

        for face_id in face_ids:
            face_binding_map[face_id] = {
                "object_name": object_name,
                "object_type": summary["object_type"],
                "front_material": front_material,
                "back_material": back_material,
                "normal_rule": summary["normal_rule"],
                "reflection_enabled": summary["reflection_enabled"],
                "transmission_enabled": summary["transmission_enabled"],
                "diffraction_candidate_enabled": summary["diffraction_candidate_enabled"],
                "resolved": True,
            }

    for face in faces:
        face_id = face["face_id"]
        if face_id not in face_binding_map:
            face_binding_map[face_id] = {
                "object_name": face["object_name"],
                "object_type": "",
                "front_material": "",
                "back_material": "",
                "normal_rule": "",
                "reflection_enabled": False,
                "transmission_enabled": False,
                "diffraction_candidate_enabled": False,
                "resolved": False,
            }

    return object_summaries, face_binding_map


class Batch2BindingDataModel:
    """批次2检查工具数据层。"""

    def __init__(self, obj_path: Path, rule_path: Path):
        self.obj_path = obj_path
        self.rule_path = rule_path

        self.vertices, self.normals, self.objects, self.faces = parse_obj(obj_path)
        self.default_medium, self.rules = load_rule_file(rule_path)
        self.object_summaries, self.face_binding_map = build_binding_summary(
            self.objects,
            self.faces,
            self.rules,
            self.default_medium,
        )

        self.object_summary_map = {
            item["object_name"]: item for item in self.object_summaries
        }

        self.object_types = sorted({item["object_type"] for item in self.object_summaries if item["object_type"]})
        self.back_materials = sorted({item["back_material"] for item in self.object_summaries if item["back_material"]})
        self.object_names = [item["object_name"] for item in self.object_summaries]

        self.scene_bounds = self._calculate_bounds()
        self.object_centers = self._calculate_object_centers()

    def _calculate_bounds(self) -> Dict[str, Tuple[float, float]]:
        """计算场景包围范围。"""
        xs = self.vertices[:, 0]
        ys = self.vertices[:, 1]
        zs = self.vertices[:, 2]
        return {
            "X": (float(np.min(xs)), float(np.max(xs))),
            "Y": (float(np.min(ys)), float(np.max(ys))),
            "Z": (float(np.min(zs)), float(np.max(zs))),
        }

    def _calculate_object_centers(self) -> Dict[str, Tuple[float, float, float]]:
        """计算每个对象的几何中心，用于标签显示。"""
        centers: Dict[str, Tuple[float, float, float]] = {}
        for obj in self.objects:
            vertex_indices = []
            for face_id in obj["face_ids"]:
                vertex_indices.extend(self.faces[face_id]["vertex_indices"])
            if not vertex_indices:
                continue
            unique_indices = sorted(set(vertex_indices))
            coords = self.vertices[unique_indices]
            center = np.mean(coords, axis=0)
            centers[obj["object_name"]] = (float(center[0]), float(center[1]), float(center[2]))
        return centers

    def get_filtered_faces(
        self,
        object_name_filter: str,
        object_type_filter: str,
        back_material_filter: str,
        unresolved_only: bool,
        clip_enabled: bool,
        clip_axis: str,
        clip_keep_lower_ratio: float,
    ) -> List[Dict[str, Any]]:
        """根据当前过滤条件返回保留的面元列表。"""
        filtered_faces: List[Dict[str, Any]] = []

        axis_index = {"X": 0, "Y": 1, "Z": 2}[clip_axis]
        axis_min, axis_max = self.scene_bounds[clip_axis]
        clip_threshold = axis_min + (axis_max - axis_min) * clip_keep_lower_ratio

        for face in self.faces:
            bind = self.face_binding_map[face["face_id"]]

            if object_name_filter != "全部对象" and face["object_name"] != object_name_filter:
                continue
            if object_type_filter != "全部语义" and bind["object_type"] != object_type_filter:
                continue
            if back_material_filter != "全部背侧材质" and bind["back_material"] != back_material_filter:
                continue
            if unresolved_only and bind["resolved"]:
                continue

            if clip_enabled:
                vertex_coords = self.vertices[face["vertex_indices"]]
                centroid = np.mean(vertex_coords, axis=0)
                if centroid[axis_index] > clip_threshold:
                    continue

            filtered_faces.append(face)

        return filtered_faces

    def build_summary_text(self, filtered_faces: List[Dict[str, Any]]) -> str:
        """构造摘要文本。"""
        lines: List[str] = []
        lines.append("批次2检查摘要")
        lines.append("=" * 72)
        lines.append(f"OBJ文件: {self.obj_path}")
        lines.append(f"规则文件: {self.rule_path}")
        lines.append(f"对象数: {len(self.objects)}")
        lines.append(f"顶点数: {len(self.vertices)}")
        lines.append(f"法向数: {len(self.normals)}")
        lines.append(f"总面元数: {len(self.faces)}")
        lines.append(f"当前过滤后面元数: {len(filtered_faces)}")
        lines.append("")
        lines.append("对象级绑定摘要")
        lines.append("-" * 72)

        for item in self.object_summaries:
            lines.append(
                f"object={item['object_name']:>4} | "
                f"type={item['object_type']:<10} | "
                f"surface={item['surface_material']:<10} | "
                f"front={item['front_material']:<10} | "
                f"back={item['back_material']:<10} | "
                f"faces={item['resolved_faces']}/{item['total_faces']} | "
                f"T={item['transmission_enabled']} | "
                f"D={item['diffraction_candidate_enabled']}"
            )

        lines.append("")
        lines.append(f"面元级抽样摘要（前 {DEFAULT_SUMMARY_FACE_COUNT} 条）")
        lines.append("-" * 72)

        for face in filtered_faces[:DEFAULT_SUMMARY_FACE_COUNT]:
            bind = self.face_binding_map[face["face_id"]]
            lines.append(
                f"face={face['face_id']:>4} | object={face['object_name']:>4} | "
                f"type={bind['object_type']:<10} | front={bind['front_material']:<10} | "
                f"back={bind['back_material']:<10} | resolved={bind['resolved']}"
            )

        return "\n".join(lines)

    def export_json(self, output_path: Path, filtered_faces: List[Dict[str, Any]]) -> None:
        """导出当前检查结果到 JSON。"""
        output_path.parent.mkdir(parents=True, exist_ok=True)
        payload = {
            "obj_path": str(self.obj_path),
            "rule_path": str(self.rule_path),
            "object_count": len(self.objects),
            "vertex_count": len(self.vertices),
            "normal_count": len(self.normals),
            "face_count": len(self.faces),
            "filtered_face_count": len(filtered_faces),
            "object_summaries": self.object_summaries,
            "face_reports": [
                {
                    "face_id": face["face_id"],
                    "object_name": face["object_name"],
                    "object_type": self.face_binding_map[face["face_id"]]["object_type"],
                    "front_material": self.face_binding_map[face["face_id"]]["front_material"],
                    "back_material": self.face_binding_map[face["face_id"]]["back_material"],
                    "resolved": self.face_binding_map[face["face_id"]]["resolved"],
                }
                for face in filtered_faces[:DEFAULT_SUMMARY_FACE_COUNT]
            ],
        }
        with output_path.open("w", encoding="utf-8") as handle:
            json.dump(payload, handle, ensure_ascii=False, indent=2)


class Batch2BindingMainWindow(QMainWindow):
    """批次2可视化检查主窗口。"""

    def __init__(self, data_model: Batch2BindingDataModel):
        super().__init__()
        self.data_model = data_model

        self.current_color_mode = DEFAULT_COLOR_MODE
        self.current_face_opacity = DEFAULT_FACE_OPACITY
        self.current_edge_width = DEFAULT_EDGE_WIDTH

        self.setWindowTitle(WINDOW_TITLE)
        self.setGeometry(220, 60, WINDOW_WIDTH, WINDOW_HEIGHT)

        self._init_ui()
        self._update_summary_and_render()

    def _init_ui(self) -> None:
        """初始化界面布局。"""
        main_widget = QWidget()
        self.setCentralWidget(main_widget)
        root_layout = QHBoxLayout(main_widget)
        root_layout.setContentsMargins(0, 0, 0, 0)
        root_layout.setSpacing(0)

        left_panel = QWidget()
        left_panel.setMinimumWidth(430)
        left_panel.setMaximumWidth(520)
        left_layout = QVBoxLayout(left_panel)
        left_layout.setContentsMargins(12, 12, 12, 12)
        left_layout.setSpacing(10)

        scroll = QScrollArea()
        scroll.setWidgetResizable(True)
        scroll_content = QWidget()
        scroll_layout = QVBoxLayout(scroll_content)
        scroll_layout.setContentsMargins(8, 8, 8, 8)
        scroll_layout.setSpacing(10)

        scroll_layout.addWidget(self._build_core_group())
        scroll_layout.addWidget(self._build_filter_group())
        scroll_layout.addWidget(self._build_style_group())
        scroll_layout.addWidget(self._build_clip_group())
        scroll_layout.addWidget(self._build_action_group())

        self.summary_text = QPlainTextEdit()
        self.summary_text.setReadOnly(True)
        self.summary_text.setMinimumHeight(280)
        self.summary_text.setStyleSheet("font-family: Consolas, Microsoft YaHei; font-size: 12px;")
        scroll_layout.addWidget(self.summary_text)
        scroll_layout.addStretch()

        scroll.setWidget(scroll_content)
        left_layout.addWidget(scroll)

        self.plotter = QtInteractor(main_widget)
        self.plotter.background_color = "white"
        self.plotter.enable_anti_aliasing()

        root_layout.addWidget(left_panel)
        root_layout.addWidget(self.plotter, stretch=1)

    def _build_core_group(self) -> QGroupBox:
        """构建核心信息与显示控制分组。"""
        group = QGroupBox("核心控制")
        group.setStyleSheet(GROUP_BOX_STYLE)
        layout = QVBoxLayout(group)

        self.info_label = QLabel(
            f"对象数: {len(self.data_model.objects)} | 顶点数: {len(self.data_model.vertices)} | 面元数: {len(self.data_model.faces)}"
        )
        layout.addWidget(self.info_label)

        color_layout = QHBoxLayout()
        color_layout.addWidget(QLabel("着色模式："))
        self.color_mode_combo = QComboBox()
        self.color_mode_combo.addItems(["按背侧材质", "按对象语义", "按对象名称"])
        self.color_mode_combo.setCurrentText(DEFAULT_COLOR_MODE)
        self.color_mode_combo.currentTextChanged.connect(self._update_summary_and_render)
        color_layout.addWidget(self.color_mode_combo)
        layout.addLayout(color_layout)

        check_layout = QHBoxLayout()
        self.show_edge_check = QCheckBox("显示线框")
        self.show_edge_check.setChecked(DEFAULT_SHOW_EDGE)
        self.show_edge_check.stateChanged.connect(self._update_summary_and_render)
        check_layout.addWidget(self.show_edge_check)

        self.show_legend_check = QCheckBox("显示图例")
        self.show_legend_check.setChecked(DEFAULT_SHOW_LEGEND)
        self.show_legend_check.stateChanged.connect(self._update_summary_and_render)
        check_layout.addWidget(self.show_legend_check)

        self.show_label_check = QCheckBox("显示对象标签")
        self.show_label_check.setChecked(DEFAULT_SHOW_LABELS)
        self.show_label_check.stateChanged.connect(self._update_summary_and_render)
        check_layout.addWidget(self.show_label_check)
        layout.addLayout(check_layout)

        return group

    def _build_filter_group(self) -> QGroupBox:
        """构建筛选分组。"""
        group = QGroupBox("筛选条件")
        group.setStyleSheet(GROUP_BOX_STYLE)
        layout = QVBoxLayout(group)

        object_layout = QHBoxLayout()
        object_layout.addWidget(QLabel("对象："))
        self.object_combo = QComboBox()
        self.object_combo.addItem("全部对象")
        self.object_combo.addItems(self.data_model.object_names)
        self.object_combo.currentTextChanged.connect(self._update_summary_and_render)
        object_layout.addWidget(self.object_combo)
        layout.addLayout(object_layout)

        type_layout = QHBoxLayout()
        type_layout.addWidget(QLabel("对象语义："))
        self.object_type_combo = QComboBox()
        self.object_type_combo.addItem("全部语义")
        self.object_type_combo.addItems(self.data_model.object_types)
        self.object_type_combo.currentTextChanged.connect(self._update_summary_and_render)
        type_layout.addWidget(self.object_type_combo)
        layout.addLayout(type_layout)

        material_layout = QHBoxLayout()
        material_layout.addWidget(QLabel("背侧材质："))
        self.back_material_combo = QComboBox()
        self.back_material_combo.addItem("全部背侧材质")
        self.back_material_combo.addItems(self.data_model.back_materials)
        self.back_material_combo.currentTextChanged.connect(self._update_summary_and_render)
        material_layout.addWidget(self.back_material_combo)
        layout.addLayout(material_layout)

        self.unresolved_only_check = QCheckBox("仅显示未解析面元")
        self.unresolved_only_check.setChecked(False)
        self.unresolved_only_check.stateChanged.connect(self._update_summary_and_render)
        layout.addWidget(self.unresolved_only_check)

        return group

    def _build_style_group(self) -> QGroupBox:
        """构建显示样式分组。"""
        group = QGroupBox("显示样式")
        group.setStyleSheet(GROUP_BOX_STYLE)
        layout = QVBoxLayout(group)

        opacity_layout = QHBoxLayout()
        opacity_layout.addWidget(QLabel("面元透明度："))
        self.opacity_spin = QDoubleSpinBox()
        self.opacity_spin.setRange(0.05, 1.0)
        self.opacity_spin.setSingleStep(0.05)
        self.opacity_spin.setValue(DEFAULT_FACE_OPACITY)
        self.opacity_spin.valueChanged.connect(self._update_summary_and_render)
        opacity_layout.addWidget(self.opacity_spin)
        layout.addLayout(opacity_layout)

        edge_layout = QHBoxLayout()
        edge_layout.addWidget(QLabel("线框宽度："))
        self.edge_width_spin = QDoubleSpinBox()
        self.edge_width_spin.setRange(0.1, 5.0)
        self.edge_width_spin.setSingleStep(0.1)
        self.edge_width_spin.setValue(DEFAULT_EDGE_WIDTH)
        self.edge_width_spin.valueChanged.connect(self._update_summary_and_render)
        edge_layout.addWidget(self.edge_width_spin)
        layout.addLayout(edge_layout)

        return group

    def _build_clip_group(self) -> QGroupBox:
        """构建裁剪控制分组。"""
        group = QGroupBox("去顶裁剪")
        group.setStyleSheet(GROUP_BOX_STYLE)
        layout = QVBoxLayout(group)

        self.enable_clip_check = QCheckBox("启用上半部分去顶裁剪")
        self.enable_clip_check.setChecked(DEFAULT_ENABLE_CLIP)
        self.enable_clip_check.stateChanged.connect(self._update_summary_and_render)
        layout.addWidget(self.enable_clip_check)

        axis_layout = QHBoxLayout()
        axis_layout.addWidget(QLabel("裁剪轴："))
        self.clip_axis_combo = QComboBox()
        self.clip_axis_combo.addItems(["X", "Y", "Z"])
        self.clip_axis_combo.setCurrentText(DEFAULT_CLIP_AXIS)
        self.clip_axis_combo.currentTextChanged.connect(self._update_summary_and_render)
        axis_layout.addWidget(self.clip_axis_combo)
        layout.addLayout(axis_layout)

        ratio_layout = QHBoxLayout()
        ratio_layout.addWidget(QLabel("保留下半比例："))
        self.clip_ratio_spin = QDoubleSpinBox()
        self.clip_ratio_spin.setRange(0.05, 0.95)
        self.clip_ratio_spin.setSingleStep(0.05)
        self.clip_ratio_spin.setValue(DEFAULT_CLIP_KEEP_LOWER_RATIO)
        self.clip_ratio_spin.valueChanged.connect(self._update_summary_and_render)
        ratio_layout.addWidget(self.clip_ratio_spin)
        layout.addLayout(ratio_layout)

        tip_label = QLabel("说明：启用后，将删除所选坐标轴上方超出阈值的上半部分面元。")
        tip_label.setWordWrap(True)
        layout.addWidget(tip_label)
        return group

    def _build_action_group(self) -> QGroupBox:
        """构建动作按钮分组。"""
        group = QGroupBox("操作")
        group.setStyleSheet(GROUP_BOX_STYLE)
        layout = QHBoxLayout(group)

        reset_filter_button = QPushButton("重置筛选")
        reset_filter_button.clicked.connect(self._reset_filters)
        layout.addWidget(reset_filter_button)

        reset_view_button = QPushButton("重置视角")
        reset_view_button.clicked.connect(self._reset_camera)
        layout.addWidget(reset_view_button)

        export_button = QPushButton("导出当前检查结果")
        export_button.clicked.connect(self._export_current_result)
        layout.addWidget(export_button)

        return group

    def _reset_filters(self) -> None:
        """重置筛选与显示配置。"""
        self.object_combo.setCurrentText("全部对象")
        self.object_type_combo.setCurrentText("全部语义")
        self.back_material_combo.setCurrentText("全部背侧材质")
        self.unresolved_only_check.setChecked(False)
        self.color_mode_combo.setCurrentText(DEFAULT_COLOR_MODE)
        self.enable_clip_check.setChecked(DEFAULT_ENABLE_CLIP)
        self.clip_axis_combo.setCurrentText(DEFAULT_CLIP_AXIS)
        self.clip_ratio_spin.setValue(DEFAULT_CLIP_KEEP_LOWER_RATIO)
        self.show_edge_check.setChecked(DEFAULT_SHOW_EDGE)
        self.show_legend_check.setChecked(DEFAULT_SHOW_LEGEND)
        self.show_label_check.setChecked(DEFAULT_SHOW_LABELS)
        self.opacity_spin.setValue(DEFAULT_FACE_OPACITY)
        self.edge_width_spin.setValue(DEFAULT_EDGE_WIDTH)

    def _reset_camera(self) -> None:
        """重置三维视角。"""
        self.plotter.reset_camera()

    def _export_current_result(self) -> None:
        """导出当前过滤状态下的检查结果。"""
        filtered_faces = self._collect_filtered_faces()
        self.data_model.export_json(EXPORT_JSON_PATH, filtered_faces)
        QMessageBox.information(self, "导出完成", f"当前检查结果已导出到：\n{EXPORT_JSON_PATH}")

    def _collect_filtered_faces(self) -> List[Dict[str, Any]]:
        """收集当前筛选状态下的面元列表。"""
        return self.data_model.get_filtered_faces(
            object_name_filter=self.object_combo.currentText(),
            object_type_filter=self.object_type_combo.currentText(),
            back_material_filter=self.back_material_combo.currentText(),
            unresolved_only=self.unresolved_only_check.isChecked(),
            clip_enabled=self.enable_clip_check.isChecked(),
            clip_axis=self.clip_axis_combo.currentText(),
            clip_keep_lower_ratio=self.clip_ratio_spin.value(),
        )

    def _build_mesh_groups(self, filtered_faces: List[Dict[str, Any]]) -> Dict[str, List[List[int]]]:
        """按当前着色模式对面元进行分组。"""
        groups: Dict[str, List[List[int]]] = defaultdict(list)
        for face in filtered_faces:
            bind = self.data_model.face_binding_map[face["face_id"]]
            if self.color_mode_combo.currentText() == "按背侧材质":
                group_key = bind["back_material"]
            elif self.color_mode_combo.currentText() == "按对象语义":
                group_key = bind["object_type"]
            else:
                group_key = face["object_name"]
            groups[group_key].append(face["vertex_indices"])
        return groups

    def _resolve_group_color(self, group_key: str) -> str:
        """根据当前着色模式决定分组颜色。"""
        if self.color_mode_combo.currentText() == "按背侧材质":
            return COLOR_BY_BACK_MATERIAL.get(group_key, "#ff66aa")
        if self.color_mode_combo.currentText() == "按对象语义":
            return COLOR_BY_OBJECT_TYPE.get(group_key, "#ff66aa")

        # 按对象名称着色时采用简单稳定的伪调色规则。
        palette = [
            "#4e79a7", "#f28e2b", "#e15759", "#76b7b2",
            "#59a14f", "#edc948", "#b07aa1", "#ff9da7",
            "#9c755f", "#bab0ab",
        ]
        index = abs(hash(group_key)) % len(palette)
        return palette[index]

    def _render_scene(self, filtered_faces: List[Dict[str, Any]]) -> None:
        """执行三维场景渲染。"""
        self.plotter.clear()

        mesh_groups = self._build_mesh_groups(filtered_faces)
        legend_entries = []

        for group_key, triangles in mesh_groups.items():
            faces_array = np.array(triangles, dtype=int)
            if faces_array.size == 0:
                continue

            mesh = pv.PolyData.from_regular_faces(self.data_model.vertices, faces_array)
            color = self._resolve_group_color(group_key)
            legend_entries.append([group_key if group_key else "UNRESOLVED", color])

            self.plotter.add_mesh(
                mesh,
                color=color,
                opacity=self.opacity_spin.value(),
                style="surface",
                reset_camera=False,
            )

            if self.show_edge_check.isChecked():
                self.plotter.add_mesh(
                    mesh,
                    color="black",
                    style="wireframe",
                    line_width=self.edge_width_spin.value(),
                    reset_camera=False,
                )

        if self.show_legend_check.isChecked() and legend_entries:
            unique_legend_entries = []
            seen = set()
            for item in legend_entries:
                key = tuple(item)
                if key not in seen:
                    unique_legend_entries.append(item)
                    seen.add(key)
            self.plotter.add_legend(unique_legend_entries, bcolor="white")

        if self.show_label_check.isChecked():
            label_points = []
            label_texts = []
            object_name_filter = self.object_combo.currentText()
            for object_name, center in self.data_model.object_centers.items():
                if object_name_filter != "全部对象" and object_name != object_name_filter:
                    continue
                label_points.append(center)
                summary = self.data_model.object_summary_map.get(object_name, {})
                object_type = summary.get("object_type", "")
                label_texts.append(f"{object_name}:{object_type}")

            if label_points:
                self.plotter.add_point_labels(
                    np.array(label_points),
                    label_texts,
                    point_size=8,
                    font_size=10,
                    shape_opacity=0.5,
                    show_points=True,
                    reset_camera=False,
                )

        self.plotter.show_axes()
        self.plotter.reset_camera()

    def _update_summary_and_render(self) -> None:
        """更新文本摘要和三维渲染。"""
        filtered_faces = self._collect_filtered_faces()
        summary_text = self.data_model.build_summary_text(filtered_faces)
        self.summary_text.setPlainText(summary_text)
        self._render_scene(filtered_faces)


def main() -> None:
    """程序主入口。"""
    validate_config()

    if not OBJ_FILE_PATH.exists():
        raise FileNotFoundError(f"OBJ 文件不存在: {OBJ_FILE_PATH}")
    if not RULE_FILE_PATH.exists():
        raise FileNotFoundError(f"规则文件不存在: {RULE_FILE_PATH}")

    app = QApplication(sys.argv)
    data_model = Batch2BindingDataModel(OBJ_FILE_PATH, RULE_FILE_PATH)
    window = Batch2BindingMainWindow(data_model)

    if EXPORT_JSON_RESULT:
        filtered_faces = data_model.get_filtered_faces(
            object_name_filter="全部对象",
            object_type_filter="全部语义",
            back_material_filter="全部背侧材质",
            unresolved_only=False,
            clip_enabled=DEFAULT_ENABLE_CLIP,
            clip_axis=DEFAULT_CLIP_AXIS,
            clip_keep_lower_ratio=DEFAULT_CLIP_KEEP_LOWER_RATIO,
        )
        data_model.export_json(EXPORT_JSON_PATH, filtered_faces)

    window.show()
    sys.exit(app.exec_())


if __name__ == "__main__":
    main()
