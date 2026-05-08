"""
scene_translator: 将自实现RT的 OBJ+JSON 格式翻译为参考实现 (RT.XD.SBR.CGAL.25.05) 的 CSV+JSON 格式。
用法: python scene_translator.py --obj <file> --config <app_config.json> --out <output_dir>
"""
import argparse, json, os, sys, math
from pathlib import Path
from collections import defaultdict

# ── 材质类型编号映射 (ITU名称 → 参考实现TypeNumber) ──
# 参考实现 ScenarioMaterial.csv 中的编号体系
ITU_TO_REFTYPE = {
    "Air": -999, "Vacuum": -999,
    "Concrete": 3,
    "Brick": -1,       # 参考实现无Brick型, 用Concrete近似
    "Plasterboard": 1, # → Ceiling board (εr~1.48 vs 2.73, 近似)
    "Wood": 5,
    "Glass": -1,       # 参考实现无Glass型, 用Chipboard近似
    "Ceiling board": 1,
    "Chipboard": 2,
    "Plywood": 6,
    "Marble": 4,
    "Metal": 7,
    "Very dry ground": 0,
    "Medium dry ground": -1,
    "Wet ground": -1,
}

# 参考实现材质CSV模板 (基于其 ScenarioMaterial.csv)
REF_MATERIAL_TEMPLATE = [
    # id, name, TypeNumber, Freq(Hz), εr, σ, μr, σm, ColorIndex
    (0, "Vacuum (air)", -999, 3000000000, 1.0, 0.0, 1.0, 0.0, 9),
    (1, "Very dry ground", 0, 3000000000, 3.0, 0.00239, 1.0, 0.0, 23),
    (2, "Ceiling board", 1, 3000000000, 1.48, 0.00358, 1.0, 0.0, 16),
    (3, "Chipboard", 2, 3000000000, 2.58, 0.05112, 1.0, 0.0, 18),
    (4, "Concrete", 3, 3000000000, 5.24, 0.0795, 1.0, 0.0, 20),
    (5, "Marble", 4, 3000000000, 7.07, 0.0105, 1.0, 0.0, 22),
    (6, "Wood", 5, 3000000000, 1.99, 0.0099, 1.0, 0.0, 24),
    (7, "Plywood", 6, 3000000000, 2.71, 0.33, 1.0, 0.0, 25),
    (8, "Metal", 7, 3000000000, 1.0, 1e7, 1.0, 0.0, 26),
]

def parse_obj(filepath):
    """解析OBJ文件, 返回(vertices, faces). faces=(v1,v2,v3, object_name)."""
    vertices, faces = [], []
    current_obj = "unknown"
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        for line in f:
            line = line.strip()
            if not line: continue
            if line.startswith('o '):
                current_obj = line[2:].strip()
            elif line.startswith('v '):
                parts = line.split()
                vertices.append((float(parts[1]), float(parts[2]), float(parts[3])))
            elif line.startswith('f '):
                parts = line.split()[1:]
                # 支持 v, v//vn, v/vt/vn 格式
                idxs = []
                for p in parts:
                    idxs.append(int(p.split('/')[0]) - 1)  # OBJ 1-based → 0-based
                # 三角剖分 (fan triangulation for quads+)
                for i in range(1, len(idxs) - 1):
                    faces.append((idxs[0], idxs[i], idxs[i+1], current_obj))
    return vertices, faces

def load_app_config(config_path):
    with open(config_path, 'r', encoding='utf-8') as f:
        return json.load(f)

def load_material_rules(rules_path):
    with open(rules_path, 'r', encoding='utf-8') as f:
        return json.load(f)

def compute_normal(v0, v1, v2):
    """计算三角面的单位法向量."""
    ux, uy, uz = v1[0]-v0[0], v1[1]-v0[1], v1[2]-v0[2]
    vx, vy, vz = v2[0]-v0[0], v2[1]-v0[1], v2[2]-v0[2]
    nx = uy*vz - uz*vy
    ny = uz*vx - ux*vz
    nz = ux*vy - uy*vx
    length = math.sqrt(nx*nx + ny*ny + nz*nz)
    if length < 1e-12: return (0.0, 0.0, 1.0)
    return (nx/length, ny/length, nz/length)

def translate_scene(obj_path, app_config, material_rules, freq_hz, output_dir):
    """主翻译函数: OBJ+JSON → 参考实现CSV+JSON."""
    os.makedirs(output_dir, exist_ok=True)

    vertices, faces = parse_obj(obj_path)
    print(f"[翻译] OBJ: {len(vertices)} 顶点, {len(faces)} 三角面")

    # ── 构建 Object名 → 材质规则 映射 ──
    obj_rules = {}
    for rule in material_rules.get("objects", []):
        obj_rules[rule["object_name"]] = rule

    # ── 构建 Object名 → 材质名 → TypeNumber 映射 ──
    # 同时收集所有用到的材质以生成 ScenarioMaterial.csv
    used_materials = set()
    obj_material_type = {}  # obj_name → (upType, downType)
    for face in faces:
        obj_name = face[3]
        if obj_name not in obj_material_type:
            rule = obj_rules.get(obj_name)
            if rule:
                front_mat = rule.get("front_material_name", "Air")
                back_mat = rule.get("surface_material_name", "Concrete")
                # normal_rule=front_is_air: 法向指向空气侧
                # 参考实现: upObjectType = 正面介质, downObjectType = 背面介质
                up_type = ITU_TO_REFTYPE.get(front_mat, -999)
                down_type = ITU_TO_REFTYPE.get(back_mat, 3)
                obj_material_type[obj_name] = (up_type, down_type)
                used_materials.add(front_mat)
                used_materials.add(back_mat)
            else:
                obj_material_type[obj_name] = (-999, 3)  # Air/Concrete default
                used_materials.add("Air")
                used_materials.add("Concrete")

    # ── 1. 写顶点CSV: ScenarioAcceleratePoint3D.csv ──
    with open(os.path.join(output_dir, "ScenarioAcceleratePoint3D.csv"), 'w') as f:
        f.write("x,y,z\n")
        for v in vertices:
            f.write(f"{v[0]:.14g},{v[1]:.14g},{v[2]:.14g}\n")

    # ── 2. 写三角面CSV: ScenarioAccelerateTriangle3D.csv ──
    with open(os.path.join(output_dir, "ScenarioAccelerateTriangle3D.csv"), 'w') as f:
        f.write("triangleP1Index,triangleP2Index,triangleP3Index,upObjectType,downObjectType,roughness,nx,ny,nz\n")
        for fid, (v1, v2, v3, obj_name) in enumerate(faces):
            up_type, down_type = obj_material_type.get(obj_name, (-999, 3))
            nx, ny, nz = compute_normal(vertices[v1], vertices[v2], vertices[v3])
            # roughness = 0 (无漫散射)
            f.write(f"{v1},{v2},{v3},{up_type},{down_type},0,{nx:.14g},{ny:.14g},{nz:.14g}\n")
    print(f"[翻译] 三角面CSV: {len(faces)} 面")

    # ── 3. 写角落CSV: ScenarioAccelerateCorner3D.csv (构建楔边) ──
    # 从三角面邻接关系构建边: 被恰好2个面共享的边 = 可能产生绕射
    edge_faces = defaultdict(list)  # (min_v, max_v) → [face_idx]
    for fid, (v1, v2, v3, obj_name) in enumerate(faces):
        for a, b in [(v1,v2), (v2,v3), (v3,v1)]:
            key = (min(a,b), max(a,b))
            edge_faces[key].append(fid)

    corners = []
    for (ev1, ev2), face_list in edge_faces.items():
        if len(face_list) != 2:
            continue  # 非流形边或边界边
        f0, f1 = face_list[0], face_list[1]
        v1a, v2a, v3a, on0 = faces[f0]
        v1b, v2b, v3b, on1 = faces[f1]
        # 找面0中不在边上的第三个顶点
        third0 = v3a if (v1a in (ev1,ev2) and v2a in (ev1,ev2)) else \
                 (v2a if (v1a in (ev1,ev2) and v3a in (ev1,ev2)) else v1a)
        third1 = v3b if (v1b in (ev1,ev2) and v2b in (ev1,ev2)) else \
                 (v2b if (v1b in (ev1,ev2) and v3b in (ev1,ev2)) else v1b)
        _, face0_type = obj_material_type.get(on0, (-999, 3))
        _, face1_type = obj_material_type.get(on1, (-999, 3))
        corners.append((ev1, ev2, third0, third1, face0_type, face1_type))

    with open(os.path.join(output_dir, "ScenarioAccelerateCorner3D.csv"), 'w') as f:
        f.write("P1Index,P2Index,P3Face0Index,P3FaceNIndex,Face0Index,FaceNIndex\n")
        for c in corners:
            f.write(f"{c[0]},{c[1]},{c[2]},{c[3]},{c[4]},{c[5]}\n")
    print(f"[翻译] 角落CSV: {len(corners)} 楔边")

    # ── 4. 写材质CSV: ScenarioMaterial.csv ──
    with open(os.path.join(output_dir, "ScenarioMaterial.csv"), 'w') as f:
        f.write("id,name,TypeNumber,Frequency(Hz),relativePermittivity,conductivity,relativePermeability,magnetoconductivity,ColorIndex\n")
        for row in REF_MATERIAL_TEMPLATE:
            f.write(f"{row[0]},{row[1]},{row[2]},{row[3]},{row[4]},{row[5]},{row[6]},{row[7]},{row[8]}\n")

    # ── 5. 写Rx天线CSV ──
    rx_x = app_config.get("path_search", {}).get("debug_rx_x", 0)
    rx_y = app_config.get("path_search", {}).get("debug_rx_y", 0)
    rx_z = app_config.get("path_search", {}).get("debug_rx_z", 0)
    with open(os.path.join(output_dir, "ReceiverAntenna.csv"), 'w') as f:
        f.write("id,x(m),y(m),z(m),frequency(Hz),radiationPatternId,polarization3DModelId\n")
        f.write(f"1,{rx_x:.14g},{rx_y:.14g},{rx_z:.14g},{freq_hz:.0f},-1,-1\n")

    # ── 6. 写Tx天线数据库JSON ──
    tx_x = app_config.get("path_search", {}).get("debug_tx_x", 0)
    tx_y = app_config.get("path_search", {}).get("debug_tx_y", 0)
    tx_z = app_config.get("path_search", {}).get("debug_tx_z", 0)
    tx_db = {
        "inputAntennaPatternDatabaseJsonFileName": "AntennaPatternDatabaseZero.json",
        "inputPolarization3DDatabaseJsonFileName": "Polarization3DDatabase.json",
        "transmittingAntennaJsonFiles": [{
            "transmittingAntennaId": 0,
            "center_location_x": tx_x,
            "center_location_y": tx_y,
            "center_location_z": tx_z,
            "emissionPower": 1.0,
            "frequencys": [freq_hz],
            "inputReceivingAntennaCsvFileName": "ReceiverAntenna.csv",
            "materialTypeNumber": -999,
            "radiationPatternId": -1,
            "polarization3DModelId": 1
        }]
    }
    with open(os.path.join(output_dir, "TransmittingAntennaDatabase1.json"), 'w') as f:
        json.dump(tx_db, f, indent=2)

    # ── 7. 写极化数据库JSON (最小: 单线极化) ──
    pol_db = {
        "database": [{
            "multiLinearPolarization3D": [{
                "linearPolarization3DObject": {
                    "phi0": 0.0,
                    "vec": {"x": 0.0, "y": 0.0, "z": 1.0}
                },
                "weight": 1.0
            }],
            "polarization3DModelId": 1
        }]
    }
    with open(os.path.join(output_dir, "Polarization3DDatabase.json"), 'w') as f:
        json.dump(pol_db, f, indent=2)

    # ── 8. 写零增益天线方向图 ──
    pattern_db = {"database": [{"radiationPattern": [0.0]*360}]}
    with open(os.path.join(output_dir, "AntennaPatternDatabaseZero.json"), 'w') as f:
        json.dump(pattern_db, f)

    # ── 9. 写参考实现主配置JSON ──
    ref_config = {
        "commonParameterConfig": {
            "airSubstanceType": -999,
            "deduplicateRadius": 0.1,
            "electricFieldCalculationMode": 1,
            "energyOutputMode": 0,
            "powerThreshold": -327.0,
            "rebuildEdge": False
        },
        "dataInputCsvFileParameterConfig": {
            "inputMaterialTableCsvFileName": "ScenarioMaterial.csv",
            "inputScenarioCorner3DCsvFileName": "ScenarioAccelerateCorner3D.csv",
            "inputScenarioPoint3DCsvFileName": "ScenarioAcceleratePoint3D.csv",
            "inputScenarioTriangle3DCsvFileName": "ScenarioAccelerateTriangle3D.csv",
            "inputTransmittingAntennaDatabaseJsonFileName": "TransmittingAntennaDatabase1.json"
        },
        "dataOutputParameterConfig": {
            "outPutDirectoryPathName": "Output",
            "outPutLogTxtFileName": "Log.txt",
            "switchOfBigChannelParameterInfo": True,
            "switchOfMultipleSignalSourceSuperposition": False,
            "switchOfPathInfo": True,
            "switchOfSmallChannelParameterInfo": True,
            "switchOfStatisticChannelParameterInfo": False
        },
        "geometricSpaceAccelerateParameterConfig": {
            "geometricSpaceAccelerateType": 1,
            "lengthOfPixel": 1.97
        },
        "multithreadParameterConfig": {
            "multithreadConfigSwitchOfMultithread": True,
            "multithreadConfigThreadNum": 8,
            "multithreadConfigThreadOneCpuCalNum": 50
        },
        "numericalMethodParameterConfig": {
            "numericalMethodMaxLevelOfGuess": 6,
            "numericalMethodNumbersOfGuess": 3
        },
        "rayEjectionParameterConfig": {
            "ejectionsMaxTotalNumber": 7,
            "ejectionsOfDiffractionMaxNumber": 0,
            "ejectionsOfDiffuseScatteringMaxNumber": 0,
            "ejectionsOfReflectionMaxNumber": 4,
            "ejectionsOfTransmissionMaxNumber": 0,
            "switchOfLos": True
        },
        "rtSbr3DForRay3DPrivateParameterConfig": {
            "cylindricalTube": False,
            "diffuseScatteringParameter": {
                "diffuseScatteringAr": 2,
                "diffuseScatteringCoefficient": 0.5,
                "diffuseScatteringRayleighRange": 8.0
            },
            "gapDiffractionRad": 0.05,
            "gapDiffuseScatteringAzimuth": 0.05,
            "gapDiffuseScatteringPitchAngle": 0.05,
            "radiusCorner": 0.03,
            "radiusRx": 0.5,
            "rayNumber": 500000,
            "realWorldRefraction": True
        }
    }
    with open(os.path.join(output_dir, "RtSbr3DForRay3D.Config.json"), 'w') as f:
        json.dump(ref_config, f, indent=4)

    print(f"[翻译] 参考实现场景已生成: {output_dir}")
    return {
        "vertices": len(vertices),
        "faces": len(faces),
        "corners": len(corners),
        "tx": (tx_x, tx_y, tx_z),
        "rx": (rx_x, rx_y, rx_z),
        "freq_hz": freq_hz
    }

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="场景翻译器: OBJ+JSON → 参考实现CSV+JSON")
    parser.add_argument("--obj", required=True)
    parser.add_argument("--config", required=True, help="自实现AppConfig JSON")
    parser.add_argument("--out", required=True, help="输出目录")
    args = parser.parse_args()

    app_config = load_app_config(args.config)
    rules_file = app_config.get("scene_import", {}).get("scene_material_map_file",
                app_config.get("material", {}).get("material_mapping_file", ""))
    material_rules = load_material_rules(rules_file) if rules_file and os.path.exists(rules_file) else {"objects": []}
    freq_hz = app_config.get("em_solver", {}).get("frequency_hz", 3e9)

    info = translate_scene(args.obj, app_config, material_rules, freq_hz, args.out)
    print(f"\n翻译摘要: {info}")
