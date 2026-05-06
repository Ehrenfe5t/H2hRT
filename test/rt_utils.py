#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
RT 测试共享工具库
===============
为所有 test/ 脚本提供统一的 OBJ 解析、配置加载、路径读取、
坐标变换、可视化默认参数等基础能力。

使用方式：
    from rt_utils import *
    或
    from rt_utils import parse_obj, load_app_config, ...

扩展接口：
    新增工具函数直接在此文件中添加。
    每个函数应独立、无副作用、可单独测试。
"""

from __future__ import annotations

import json
import math
import subprocess
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Dict, Iterable, List, Optional, Sequence, Tuple

import numpy as np

# =============================================================================
# 仓库根路径
# =============================================================================

def resolve_repo_root() -> Path:
    return Path(__file__).resolve().parents[1]

REPO_ROOT = resolve_repo_root()

# =============================================================================
# 默认路径（可被调用方覆盖）
# =============================================================================

DEFAULT_OBJ_PATH = REPO_ROOT / "demo" / "meeting.obj"
DEFAULT_CONFIG_PATH = REPO_ROOT / "configs" / "app" / "meeting_v3.json"
DEFAULT_MATERIAL_MAP_PATH = REPO_ROOT / "configs" / "scenes" / "scene_material_map.json"
DEFAULT_PATH_JSON = REPO_ROOT / "output" / "a1_real_chain" / "paths" / "precise_paths.json"
DEFAULT_EXE_PATH = REPO_ROOT / "x64" / "Debug" / "RT.exe"

# =============================================================================
# 可视化默认参数
# =============================================================================

@dataclass(frozen=True)
class VisDefaults:
    """可视化样式默认值。"""
    window_title: str = "RT 场景可视化工具"
    window_width: int = 1800
    window_height: int = 1100
    mesh_color: str = "#c7ccd6"
    mesh_opacity: float = 0.92
    edge_color: str = "#4f5b66"
    tx_color: str = "#ff5a5f"
    rx_color: str = "#2d9cdb"
    path_color: str = "#ff9900"
    reflection_color: str = "#ffcc33"
    transmission_color: str = "#66cc66"
    diffraction_color: str = "#cc66ff"
    los_color: str = "#33cc33"
    marker_radius_ratio: float = 0.018
    bg_color: str = "white"

# 材质着色映射
COLOR_BY_BACK_MATERIAL: Dict[str, str] = {
    "Concrete": "#8c8c8c",
    "Glass": "#66c2ff",
    "Wood": "#c68642",
    "Brick": "#b55239",
    "Air": "#dddddd",
    "Metal": "#555555",
    "": "#ff66aa",
}

COLOR_BY_OBJECT_TYPE: Dict[str, str] = {
    "floor": "#999999",
    "ceiling": "#cfcfcf",
    "wall": "#8b8b8b",
    "window": "#66c2ff",
    "door": "#c68642",
    "table": "#d2a679",
    "partition": "#cc7a5c",
    "": "#ff66aa",
}

# 交互类型编码 (与 InteractionType 枚举一致)
INTERACTION_TYPE_MAP: Dict[int, str] = {
    1: "Tx",
    2: "Rx",
    3: "LOS",
    4: "Reflection",
    5: "Transmission",
    6: "Diffraction",
    7: "Scattering",
}

# =============================================================================
# OBJ 解析
# =============================================================================

def parse_face_vertex_index(token: str) -> int:
    """解析 OBJ face token 中的 vertex index（1-based→0-based）。"""
    return int(token.split("/")[0]) - 1


def triangulate_polygon(indices: List[int]) -> List[Tuple[int, int, int]]:
    """扇形三角化多边形。"""
    if len(indices) < 3:
        return []
    if len(indices) == 3:
        return [(indices[0], indices[1], indices[2])]
    result = []
    for i in range(1, len(indices) - 1):
        result.append((indices[0], indices[i], indices[i + 1]))
    return result


def parse_obj(obj_path: Path) -> Tuple[np.ndarray, np.ndarray, np.ndarray, List[str], List[Dict]]:
    """解析 OBJ 文件。
    返回: (vertices[Nx3], normals[Mx3], triangles[Kx3], object_names, face_records)

    face_record: {face_id, object_id, object_name, vertex_indices, normal_index}
    """
    vertices: List[Tuple[float, float, float]] = []
    normals: List[Tuple[float, float, float]] = []
    objects: List[Dict] = []
    faces: List[Dict] = []
    current_object_name: Optional[str] = None
    current_object_id = -1

    with obj_path.open("r", encoding="utf-8", errors="ignore") as f:
        for raw in f:
            line = raw.strip()
            if not line or line.startswith("#"):
                continue
            parts = line.split()
            tag = parts[0]
            if tag == "o":
                current_object_name = parts[1] if len(parts) > 1 else f"obj_{len(objects)}"
                current_object_id = len(objects)
                objects.append({"object_id": current_object_id, "object_name": current_object_name, "face_ids": []})
            elif tag == "v" and len(parts) >= 4:
                vertices.append((float(parts[1]), float(parts[2]), float(parts[3])))
            elif tag == "vn" and len(parts) >= 4:
                normals.append((float(parts[1]), float(parts[2]), float(parts[3])))
            elif tag == "f" and len(parts) >= 4:
                poly = [parse_face_vertex_index(t) for t in parts[1:]]
                for tri in triangulate_polygon(poly):
                    face_id = len(faces)
                    normal_idx = -1
                    if "//" in parts[1]:
                        normal_idx = int(parts[1].split("//")[1]) - 1
                    faces.append({
                        "face_id": face_id, "object_id": current_object_id,
                        "object_name": current_object_name or "",
                        "vertex_indices": list(tri), "normal_index": normal_idx,
                    })
                    if current_object_id >= 0:
                        objects[current_object_id]["face_ids"].append(face_id)

    if not vertices or not faces:
        raise ValueError("OBJ 中无有效顶点或三角面。")
    return (
        np.array(vertices, dtype=float),
        np.array(normals, dtype=float) if normals else np.zeros((1, 3)),
        np.array([f["vertex_indices"] for f in faces], dtype=int),
        [o["object_name"] for o in objects],
        faces,
    )


def parse_obj_geometry(obj_path: Path) -> Tuple[np.ndarray, np.ndarray, List[str]]:
    """简化版 OBJ 解析：返回顶点、三角形索引和对象名。"""
    v, _, tri, names, _ = parse_obj(obj_path)
    return v, tri, names


# =============================================================================
# 配置加载
# =============================================================================

def load_app_config(config_path: Path) -> Dict[str, Any]:
    """加载 RT 应用 JSON 配置。"""
    with config_path.open("r", encoding="utf-8") as f:
        return json.load(f)


def save_app_config(config: Dict[str, Any], config_path: Path) -> None:
    """保存 RT 应用 JSON 配置。"""
    config_path.write_text(json.dumps(config, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")


def load_tx_rx_from_config(config_path: Path) -> Tuple[Tuple[float, float, float], Tuple[float, float, float]]:
    """从配置读取 Tx/Rx。"""
    try:
        cfg = load_app_config(config_path)
        ps = cfg.get("path_search", {})
        tx = (float(ps.get("debug_tx_x", 1.0)), float(ps.get("debug_tx_y", 1.0)), float(ps.get("debug_tx_z", 1.0)))
        rx = (float(ps.get("debug_rx_x", 3.0)), float(ps.get("debug_rx_y", 1.0)), float(ps.get("debug_rx_z", 1.0)))
        return tx, rx
    except Exception:
        return (1.0, 1.0, 1.0), (3.0, 1.0, 1.0)


def write_tx_rx_to_config(
    config_path: Path, tx: Tuple[float, float, float], rx: Tuple[float, float, float]
) -> None:
    """将 Tx/Rx 回写到配置文件。"""
    cfg = load_app_config(config_path)
    ps = cfg.setdefault("path_search", {})
    ps["debug_tx_x"], ps["debug_tx_y"], ps["debug_tx_z"] = tx
    ps["debug_rx_x"], ps["debug_rx_y"], ps["debug_rx_z"] = rx
    save_app_config(cfg, config_path)


def load_material_rules(rule_path: Path) -> Tuple[str, Dict[str, Dict[str, Any]]]:
    """加载材质规则文件。返回 (default_medium, {object_name: rule})。"""
    with rule_path.open("r", encoding="utf-8") as f:
        data = json.load(f)
    default_medium = data.get("default_medium", "Air")
    rules = {}
    for obj_rule in data.get("objects", []):
        name = obj_rule.get("object_name")
        if name:
            rules[name] = obj_rule
    return default_medium, rules


# =============================================================================
# 路径数据加载
# =============================================================================

def load_exported_paths(path_json: Path) -> List[Dict[str, Any]]:
    """读取 A1/A2 导出的路径 JSON。"""
    if not path_json.exists():
        return []
    try:
        data = json.loads(path_json.read_text(encoding="utf-8"))
    except Exception:
        return []
    return data.get("paths", []) if isinstance(data, dict) else []


def get_path_interaction_types(path: Dict) -> List[int]:
    """提取路径中的交互类型序列（排除 Tx=1, Rx=2）。"""
    types = path.get("interaction_types", [])
    return [t for t in types if t not in (1, 2)]


def classify_paths(paths: List[Dict]) -> Dict[str, int]:
    """对路径按交互类型组合分类统计。"""
    from collections import Counter
    stats = Counter()
    for p in paths:
        types = tuple(get_path_interaction_types(p))
        label = "-".join(str(t) for t in types)
        stats[label] += 1
    return dict(stats)


# =============================================================================
# 坐标变换
# =============================================================================

def transform_point_display(
    point: Sequence[float], mode: str = "RAW"
) -> Tuple[float, float, float]:
    """显示坐标变换。mode: RAW / SWAP_YZ / SWAP_YZ_FLIP_Z"""
    x, y, z = float(point[0]), float(point[1]), float(point[2])
    if mode == "RAW":
        return x, y, z
    if mode == "SWAP_YZ":
        return x, z, y
    if mode == "SWAP_YZ_FLIP_Z":
        return x, z, -y
    return x, y, z


def transform_points_display(points: np.ndarray, mode: str = "RAW") -> np.ndarray:
    """批量显示坐标变换。"""
    if points.size == 0:
        return points.copy()
    return np.array([transform_point_display(p, mode) for p in points], dtype=float)


# =============================================================================
# 几何工具
# =============================================================================

def compute_bounds(pts: np.ndarray) -> Dict[str, Tuple[float, float]]:
    """计算点云包围盒。"""
    return {
        "X": (float(np.min(pts[:, 0])), float(np.max(pts[:, 0]))),
        "Y": (float(np.min(pts[:, 1])), float(np.max(pts[:, 1]))),
        "Z": (float(np.min(pts[:, 2])), float(np.max(pts[:, 2]))),
    }


def build_polydata(vertices: np.ndarray, triangles: np.ndarray):
    """构建 PyVista PolyData。"""
    import pyvista as pv
    faces_arr = np.hstack([
        np.full((triangles.shape[0], 1), 3, dtype=np.int64),
        triangles.astype(np.int64),
    ]).ravel()
    return pv.PolyData(vertices, faces_arr)


def build_clip_box(bounds: Dict[str, Tuple[float, float]], axis: str, threshold: float):
    """构造保留下半部分的 clip box。"""
    x0, x1 = bounds["X"]
    y0, y1 = bounds["Y"]
    z0, z1 = bounds["Z"]
    if axis == "X":   x1 = threshold
    elif axis == "Y": y1 = threshold
    elif axis == "Z": z1 = threshold
    return x0, x1, y0, y1, z0, z1


def compute_sphere_radius(bounds: Dict[str, Tuple[float, float]], ratio: float = 0.018) -> float:
    """根据场景尺寸计算标记球半径。"""
    span = max(bounds["X"][1] - bounds["X"][0],
               bounds["Y"][1] - bounds["Y"][0],
               bounds["Z"][1] - bounds["Z"][0])
    return max(span * ratio, 0.03)


# =============================================================================
# RT.exe 运行器
# =============================================================================

def run_rt_pipeline(
    config_path: str = "configs/app/minimal.json",
    exe_path: Optional[Path] = None,
    timeout: int = 60,
) -> Dict[str, Any]:
    """运行 RT.exe 并解析输出。
    返回: {returncode, stdout, stderr, a1_ok, paths, mixed, accepted, states}
    """
    exe = exe_path or DEFAULT_EXE_PATH
    proc = subprocess.run(
        [str(exe), config_path],
        cwd=str(REPO_ROOT),
        capture_output=True, text=True, encoding="utf-8", errors="ignore",
        timeout=timeout,
    )
    import re
    output = (proc.stdout or "") + "\n" + (proc.stderr or "")
    result = {"returncode": proc.returncode, "stdout": proc.stdout, "stderr": proc.stderr, "output": output}
    result["a1_ok"] = "A1 real production chain closed loop completed." in output
    for key, pat in [("paths", r"search_paths=(\d+)"), ("em_results", r"em_results=(\d+)"),
                     ("mixed", r"mixed_path_generated=(\d+)"), ("states", r"generated_state_count=(\d+)")]:
        m = re.search(pat, output)
        result[key] = int(m.group(1)) if m else 0
    return result


# =============================================================================
# 材质绑定摘要构建
# =============================================================================

def build_material_binding_summary(
    objects: List[Dict], faces: List[Dict], rules: Dict[str, Dict], default_medium: str
) -> Tuple[List[Dict], Dict[int, Dict]]:
    """根据规则构造对象级和面元级材质绑定摘要。
    返回: (object_summaries, face_binding_map)
    """
    face_binding_map: Dict[int, Dict] = {}

    for obj in objects:
        obj_name = obj["object_name"]
        rule = rules.get(obj_name)
        face_ids = obj["face_ids"]
        if rule is None:
            for fid in face_ids:
                face_binding_map[fid] = {"object_name": obj_name, "resolved": False, "back_material": "", "object_type": ""}
            continue
        front = rule.get("front_material_name") or default_medium
        back = rule.get("back_material_name") or rule.get("surface_material_name", "")
        for fid in face_ids:
            face_binding_map[fid] = {
                "object_name": obj_name,
                "object_type": rule.get("object_type", ""),
                "surface_material": rule.get("surface_material_name", ""),
                "front_material": front,
                "back_material": back,
                "resolved": True,
                "reflection_enabled": bool(rule.get("reflection_enabled", True)),
                "transmission_enabled": bool(rule.get("transmission_enabled", False)),
                "diffraction_candidate_enabled": bool(rule.get("diffraction_candidate_enabled", False)),
            }

    for f in faces:
        if f["face_id"] not in face_binding_map:
            face_binding_map[f["face_id"]] = {"object_name": f.get("object_name", ""), "resolved": False, "back_material": "", "object_type": ""}

    obj_summaries = []
    for obj in objects:
        obj_name = obj["object_name"]
        rule = rules.get(obj_name)
        fid_list = obj["face_ids"]
        resolved = rule is not None
        obj_summaries.append({
            "object_name": obj_name,
            "object_type": rule.get("object_type", "") if rule else "",
            "front_material": (rule.get("front_material_name") or default_medium) if rule else "",
            "back_material": (rule.get("back_material_name") or rule.get("surface_material_name", "")) if rule else "",
            "resolved": resolved,
            "total_faces": len(fid_list),
            "reflection_enabled": bool(rule.get("reflection_enabled", True)) if rule else False,
            "transmission_enabled": bool(rule.get("transmission_enabled", False)) if rule else False,
            "diffraction_candidate_enabled": bool(rule.get("diffraction_candidate_enabled", False)) if rule else False,
        })
    return obj_summaries, face_binding_map


# =============================================================================
# 材质数据库加载（B5+ 使用）
# =============================================================================

def load_material_database(csv_path: Path) -> Dict[int, Dict[float, Dict[str, float]]]:
    """加载 ItuMaterial.csv。
    返回: {material_id: {frequency_hz: {epsilon_r, sigma, mu_r, name}}}
    """
    db: Dict[int, Dict[float, Dict[str, float]]] = {}
    with csv_path.open("r", encoding="utf-8") as f:
        header = f.readline().strip().split(",")
        for line in f:
            line = line.strip()
            if not line:
                continue
            parts = line.split(",")
            mid = int(parts[0])
            name = parts[1]
            freq = float(parts[3])
            eps_r = float(parts[4])
            sigma = float(parts[5])
            mu_r = float(parts[6])
            if mid not in db:
                db[mid] = {}
            db[mid][freq] = {"epsilon_r": eps_r, "sigma": sigma, "mu_r": mu_r, "name": name}
    return db


def query_material_at_freq(db: Dict, material_id: int, freq_hz: float) -> Dict[str, float]:
    """查询指定 material_id 在 freq_hz 处的材质参数（线性插值）。"""
    if material_id not in db:
        return {"epsilon_r": 1.0, "sigma": 0.0, "mu_r": 1.0}
    freqs = sorted(db[material_id].keys())
    if freq_hz <= freqs[0]:
        return db[material_id][freqs[0]]
    if freq_hz >= freqs[-1]:
        return db[material_id][freqs[-1]]
    for i in range(len(freqs) - 1):
        if freqs[i] <= freq_hz <= freqs[i + 1]:
            f0, f1 = freqs[i], freqs[i + 1]
            t = (freq_hz - f0) / (f1 - f0) if f1 != f0 else 0.0
            p0, p1 = db[material_id][f0], db[material_id][f1]
            return {
                "epsilon_r": p0["epsilon_r"] + t * (p1["epsilon_r"] - p0["epsilon_r"]),
                "sigma": p0["sigma"] + t * (p1["sigma"] - p0["sigma"]),
                "mu_r": p0["mu_r"] + t * (p1["mu_r"] - p0["mu_r"]),
            }
    return db[material_id][freqs[-1]]


# =============================================================================
# 便捷：获取当前运行输出目录
# =============================================================================

def get_latest_output_dir(run_id: Optional[str] = None) -> Path:
    """获取指定 run_id 的输出目录。默认返回 a1_real_chain。"""
    if run_id:
        return REPO_ROOT / "output" / run_id
    return REPO_ROOT / "output" / "a1_real_chain"


def get_path_output_files(run_id: Optional[str] = None) -> Tuple[Path, Path, Path]:
    """返回 (precise_paths.json, precise_paths.csv, path_manifest.json) 路径。"""
    d = get_latest_output_dir(run_id) / "paths"
    return d / "precise_paths.json", d / "precise_paths.csv", d / "path_manifest.json"

# =============================================================================
# 主入口：启动可视化工具
# =============================================================================
if __name__ == "__main__":
    import subprocess, sys
    script = Path(__file__).resolve().parent / "rt_visual.py"
    sys.exit(subprocess.run([sys.executable, str(script)] + sys.argv[1:]).returncode)
