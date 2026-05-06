# RT 射线追踪电磁仿真系统 — 项目文档

## 1. Blender 建模指南

### 1.1 大白话版

**你要在 Blender 里建一个"房间"：**

1. 房间的每一面墙、地板、天花板都是一个独立的 Object
2. 每个 Object 的**法向必须朝外**（Blender 里红色的面是背面，蓝色是正面）
3. 墙是**单面**建模的——用 Plane 而非 Cube。只有玻璃、隔断等需要穿透的才考虑双面
4. 每个 Object 在 `scene_material_map.json` 里有对应的一条规则，指定它是什么材质

**两个物体紧贴在一起（比如桌子贴着地板）：**
- 贴在一起的**面不会被射线看到**（被挡住了），所以它们的材质不重要
- **不需要在 Blender 里做布尔运算**——直接让它们重叠即可
- 但是贴在一起的**边**会产生楔边（用于绕射），这个由算法自动处理
- 如果有意让一面墙和地板之间有缝隙（形成绕射边），留 1cm 以上的间隔

### 1.2 正式版

#### 1.2.1 OBJ 导出规范
- 导出格式：Wavefront (.obj)，**三角化面**
- 坐标轴：Blender Y-up → 算法 Z-up（算法不做坐标变换）
- 单位：1 Blender unit = 1 meter
- 每个独立对象设置独立的 Object Name（算法以此匹配材质规则）

#### 1.2.2 面元法向规范
- **单面材质**（墙/地板/天花板）：法向指向**房间内部**（或"朝向空气"的一侧）
- **双面材质**（玻璃/隔断）：法向朝向物体外部（与 Blender 默认一致）
- 算法通过 `front_is_air` 规则判断入射侧：射线从法向指向的一侧入射 = front = "Air"

#### 1.2.3 材质绑定规则
- 材质规则文件：`configs/scenes/scene_material_map.json`
- 规则通过 Object Name 精确匹配（当前不支持通配符）
- 每条规则指定：
  - `object_name`：OBJ 中的 Object 名称
  - `object_type`：语义类型（floor/ceiling/wall/window/door/table/partition）
  - `surface_material_name`：表面材质
  - `front_material_name`：通常 "Air"（入射侧介质）
  - `back_material_name`：实体材质（如 "Concrete"）
  - `normal_rule`：`"front_is_air"` 表示法向指向空气侧
  - `reflection_enabled` / `transmission_enabled` / `diffraction_candidate_enabled`

#### 1.2.4 ITU 材质-频率-电参数表

| ID | 材质 | 1GHz ε_r/σ | 2GHz ε_r/σ | 4GHz ε_r/σ |
|----|------|-----------|-----------|-----------|
| 0 | Vacuum(≈Air) | 1.00/0.0000 | 1.00/0.0000 | 1.00/0.0000 |
| 1 | Concrete | 5.24/0.0462 | 5.24/0.0795 | 5.24/0.1366 |
| 2 | Brick | 3.91/0.0238 | 3.91/0.0266 | 3.91/0.0297 |
| 3 | Plasterboard | 2.73/0.0085 | 2.73/0.0163 | 2.73/0.0313 |
| 4 | Wood | 1.99/0.0047 | 1.99/0.0099 | 1.99/0.0208 |
| 5 | Glass | 6.31/0.0036 | 6.31/0.0091 | 6.31/0.0231 |
| 7 | Ceiling board | 1.48/0.0011 | 1.48/0.0023 | 1.48/0.0049 |
| 9 | Chipboard | 2.58/0.0217 | 2.58/0.0373 | 2.58/0.0640 |
| 10 | Plywood | 2.71/0.3300 | 2.71/0.3300 | 2.71/0.3300 |
| 11 | Marble | 7.07/0.0055 | 7.07/0.0105 | 7.07/0.0199 |
| 13 | Metal | 1.00/10⁷ | 1.00/10⁷ | 1.00/10⁷ |
| 14 | Very dry ground | 3.00/0.0002 | 3.00/0.0009 | 3.00/0.0049 |
| 15 | Medium dry ground | 15.0/0.0350 | 14.0/0.1083 | 13.1/0.3353 |
| 16 | Wet ground | 30.0/0.1500 | 22.7/0.3693 | 17.2/0.9094 |

数据来源：ITU-R P.2040-1 建议书。仿真频率在表内频点之间时，ε_r 和 σ 线性插值。

#### 1.2.5 紧贴物体的处理
- 物体 A 贴在地板 B 上：A 的底面会被 B 遮挡，算法在求交时会自然忽略
- 建模时**直接让 A 的顶点与 B 的顶点重合即可**（不需布尔运算）
- **注意**：A 的底面如果参与材质绑定，会被赋予材质但实际不会被射线命中（被 B 遮挡）
- 紧贴处的**边**会被识别为 boundary edge（不产生楔边），算法自动处理

#### 1.2.6 物体或面元需要封闭（水密）吗？

**不需要。** 算法对每个三角面独立做射线求交，不要求面元之间形成封闭体。

- 回字形墙壁没有上表面封口 → 天花板用单面平面封顶 → **完全可行**
- 墙壁侧面没有建模 → 射线从缝隙穿过 → 漏射（不想漏就补面）
- 天花板和墙壁之间有 1mm 缝 → 射线穿过 → 漏射
- **原则：凡是射线可能到达的面，都建模出来。不考虑"水密性"。**

### 1.3 `scene_material_map.json` 详解（大白话）

这个文件告诉算法："OBJ 里名叫 XXX 的那些面，是什么材质、能不能穿透、能不能绕射。"

#### 完整字段表

```json
{
  "default_medium": "Air",        // ① 默认介质
  "objects": [                    // ② 规则列表
    {
      "rule_name": "外墙规则",                    // ③ 规则名（给人看的）
      "object_name": "wall_outer",               // ④ OBJ对象名（精确匹配！）
      "object_type": "wall",                     // ⑤ 语义类型
      "surface_material_name": "Concrete",        // ⑥ 表面材质名
      "front_material_name": "Air",              // ⑦ 正面介质（法向指向侧）
      "back_material_name": "Concrete",           // ⑧ 背面介质（法向背对侧）
      "normal_rule": "front_is_air",             // ⑨ 法向规则
      "reflection_enabled": true,                // ⑩ 允许反射
      "transmission_enabled": false,             // ⑪ 允许透射（穿透）
      "diffraction_candidate_enabled": false      // ⑫ 此Object边缘可绕射
    }
  ]
}
```

#### 字段大白话解释

**① `default_medium`** — 场景默认介质。一般是 `"Air"`。如果某个面没匹配到规则，正面介质用这个。

**② `objects`** — 规则数组。一个 Object 匹配一条规则。

**③ `rule_name`** — 规则名称。给你自己看的备注，算法不用于匹配。

**④ `object_name`** — **这是匹配的关键！** 必须与 OBJ 文件中的 `o` 名称一模一样。当前不支持通配符。一个 Object 只能匹配一条规则。

**⑤ `object_type`** — 语义类型。可选值：`wall`(墙)、`floor`(地板)、`ceiling`(天花板)、`window`(窗户)、`door`(门)、`table`(桌子)、`partition`(隔断)。目前主要用于统计和可视化着色，不直接影响物理计算。

**⑥ `surface_material_name`** — 面的实体材质名。必须是 `ItuMaterial.csv` 中存在的名称（如 `Concrete`、`Glass`、`Wood`）。算法用它去查 ε_r 和 σ 用于 Fresnel/UTD 计算。

**⑦ `front_material_name`** — **法向指向的那一侧**是什么介质。对于外墙，法向指向室外空气 → `"Air"`。对于内墙，法向指向室内 → `"Air"`。

**⑧ `back_material_name`** — **法向背对的那一侧**是什么介质。对于实心混凝土墙 → `"Concrete"`。对于玻璃窗（两面都是空气）→ `"Air"`。对于地板（下面是地基）→ `"Concrete"`。

**⑨ `normal_rule`** — 固定写 `"front_is_air"`。意思是：法向指向的那一侧介质是 `front_material_name`。算法靠这个规则判断射线是从正面（front）入射还是背面（back）入射，从而决定透射时介质切换方向。

**⑩ `reflection_enabled`** — 射线打到这个面会不会反射。几乎所有建筑面都应该设为 `true`。

**⑪ `transmission_enabled`** — 射线能不能穿透这个面。`true` = 可以穿透（如玻璃、门、双面建模的墙体）。`false` = 只能反射（如单面建模的地板）。**如果你按 1.2.6 的方法做了墙体双面建模，外墙面应该开透射。**

**⑫ `diffraction_candidate_enabled`** — 这个 Object 的边缘能不能形成绕射楔边。`true` = 边缘可绕射（如窗框、门框、隔断顶边）。大面墙面通常设为 `false` 以减少无效楔边数量。

#### 典型配置速查

| 场景 | front | back | transmission | diffraction |
|------|-------|------|-------------|-------------|
| 外墙（双面建模，非穿透） | Air | Concrete | true | true |
| 内墙（双面建模） | Air | Concrete | true | true |
| 玻璃窗 | Air | Air | true | true |
| 木门 | Air | Wood | false | true |
| 地板/天花板（单面） | Air | Concrete | false | false |
| 金属柱子 | Air | Metal | false | true |
| 室内桌子 | Air | Wood | false | true |

#### 重要提醒

- **不同材质必须分不同 Object**（如 `wall_concrete` 和 `wall_glass`）。
- **Object 名称是精确匹配**，`wall` 不会匹配 `wall_outer`。
- **介质名称必须与 `ItuMaterial.csv` 一致**，否则 Fresnel 查询失败回退到 ε_r=1。
- **透射使能的面，算法会用 Fresnel 公式计算透射频。如果两面介质相同且不是 Air，透射后介质不变。**

---

## 2. 可视化能力

| 能力 | 状态 | 脚本 |
|------|------|------|
| 场景三角网格 + 线框 | ✅ | `test/visualize_scene_tx_rx.py` |
| Tx/Rx 天线位置标记 | ✅ | 同上 |
| 路径折线（按交互类型着色：R=黄/T=绿/D=紫） | ✅ | 同上 |
| 交互节点标记球（彩色） | ✅ | 同上 |
| 材质绑定着色（按背侧材质/语义/对象名称） | ✅ | 同上 |
| 对象筛选 + 材质摘要 | ✅ | 同上 |
| 去顶裁剪 | ✅ | 同上 |
| 显示坐标变换（SWAP_YZ等） | ✅ | 同上 |
| **功率覆盖热力图** | **❌ 预留接口** | B10 后待补 |
| PDP/APS 图形化 | ❌ | v4 |

使用方法：
```bash
# 可视化场景 + 路径（默认 meeting.obj）
python test/visualize_scene_tx_rx.py

# B1 混合路径场景
python test/visualize_scene_tx_rx.py --obj demo/b1_mixed_path_test.obj --config configs/app/b1_mixed_path_test.json --material-map configs/scenes/scene_material_map_b1_test.json
```

---

## 3. 文件夹结构

```
RT/                                  # 项目根目录
├── RT.sln                           # VS2022 解决方案
├── RT.vcxproj                       # VS2022 项目文件
├── RT.vcxproj.filters               # VS 文件筛选器
│
├── app/                             # 应用层（入口+流水线）
│   ├── main.cpp                     # 程序入口
│   ├── RtPipeline.cpp/.h            # 主流水线（编排 B0~B10）
│   ├── A1RealChainRunner.cpp/.h     # A1 精确模式主链（Search→EM→Export）
│   ├── Batch9ExportReporter.cpp/.h  # [过渡资产] 导出报告器
│   └── legacy/                      # [历史资产] 旧批次自检 Reporter
│       └── *Reporter.cpp/.h         # Batch2~8 自检代码（已不再调用）
│
├── core/                            # 核心库
│   ├── common/                      # 模块1：基础设施
│   │   ├── config/                  # 配置加载/校验/快照
│   │   ├── error/                   # RtError + ErrorCode
│   │   ├── log/                     # Logger
│   │   ├── math/                    # 统一数学库 (B0)
│   │   │   ├── Vec3.h              # 向量运算
│   │   │   ├── Complex.h           # 复数
│   │   │   ├── Matrix3x3.h         # 3x3矩阵
│   │   │   ├── MathConstants.h     # 物理常数
│   │   │   └── CoordinateFrame.h   # 坐标系变换
│   │   ├── material/               # 材质数据库 (B5)
│   │   │   └── MaterialDatabase.h  # CSV加载+频率插值
│   │   ├── numeric/                # 容差配置
│   │   └── version/                # 版本信息
│   ├── antenna/                     # 模块3：天线
│   │   ├── AntennaModel.cpp/.h     # 天线对象/响应
│   │   ├── AntennaFactory.cpp/.h   # 天线工厂
│   │   ├── AntennaResponse.cpp/.h  # 天线响应评估
│   │   └── AntennaPattern.h        # 方向图加载+插值 (B9)
│   ├── scene/                       # 模块2：场景数据结构
│   │   ├── Face.h                  # 三角面 + Vec3/AABB/LocalFrame
│   │   ├── Edge.h                  # 拓扑边
│   │   ├── Wedge.h                 # 绕射楔边
│   │   ├── Scene.h                 # 场景容器 + BVH/加速结构
│   │   ├── SceneMeta.h             # 场景元信息
│   │   └── SceneMaterialBinding.h  # 材质绑定
│   ├── path/                        # 模块4：路径数据结构
│   │   ├── PathNode.h              # 路径节点
│   │   ├── PathState.h             # 搜索状态
│   │   ├── GeometricPath.h         # 几何路径
│   │   ├── PathSearchContext.h     # 搜索上下文
│   │   └── InteractionType.h       # 交互类型枚举
│   ├── query/                       # 模块2：场景查询门面
│   │   └── SceneQuery.cpp/.h       # BVH遍历/可见性/楔边查询
│   ├── search/                      # 模块4：搜索引擎
│   │   ├── SearchEngine.cpp/.h     # Image Method + 优先级队列 (B2)
│   │   ├── SbrEngine.cpp/.h        # SBR Coverage 引擎 (B4)
│   │   ├── ReflectionExpander.cpp/.h # 反射扩展器 (B2-BVH)
│   │   ├── TransmissionExpander.cpp/.h # 透射扩展器 (B6-Snell)
│   │   ├── DiffractionExpander.cpp/.h # 绕射扩展器 (B3-UTD几何)
│   │   ├── GeometryValidity.cpp/.h  # 几何合法性检查 (B1-控制规则)
│   │   ├── ResolveMediumTransition.cpp/.h # 介质切换解析
│   │   ├── StateSignatureBuilder.cpp/.h # 状态签名uint64 (B2)
│   │   └── PathSignatureBuilder.cpp/.h  # 路径签名uint64 (B2)
│   ├── em/                          # 模块5：电磁计算
│   │   ├── FieldAccumulator.h      # 场累积器
│   │   ├── EMSolverInput.h         # EM求解输入
│   │   ├── EMProfile.h             # EM求解模式
│   │   ├── EMPathResult.h          # 路径EM结果
│   │   ├── InitializeTxField.cpp/.h # 发射场初始化 (B9-天线注入)
│   │   ├── ApplyFreeSpaceSegment.cpp/.h # 自由空间传播 (B8-FSPL修正)
│   │   ├── ApplyReflectionInteraction.cpp/.h # 反射Fresnel (B5)
│   │   ├── ApplyTransmissionInteraction.cpp/.h # 透射Fresnel+Snell (B6)
│   │   ├── ApplyDiffractionInteraction.cpp/.h # UTD绕射 (B7)
│   │   ├── FinalizeAtReceiver.cpp/.h # 接收端收敛 (B8-FSPL+B9-天线)
│   │   ├── PreparePathForEM.cpp/.h  # EM前准备
│   │   ├── PreciseEMProfile.cpp/.h  # 精确模式profile
│   │   ├── CoverageEMProfile.cpp/.h # 覆盖模式profile
│   │   ├── BuildCIR.cpp/.h         # CIR构建
│   │   ├── BuildPDP.cpp/.h         # PDP构建
│   │   ├── BuildAPS.cpp/.h         # APS构建
│   │   ├── BuildChannelStatistics.cpp/.h # 信道统计
│   │   ├── BuildCoverageResult.cpp/.h # 覆盖结果
│   │   └── BuildISACFeatureSet.cpp/.h # 通感特征
│   └── result/                      # 模块6：导出
│       ├── ExportBundle.cpp/.h      # 导出包
│       ├── ExportPaths.cpp/.h       # 路径导出(JSON/CSV)
│       ├── ExportChannel.cpp/.h     # 信道导出
│       ├── ExportCoverage.cpp/.h    # 覆盖导出
│       ├── ExportISAC.cpp/.h        # ISAC导出
│       ├── ExportVisualization.cpp/.h # 可视化导出
│       ├── ValidationReport.cpp/.h  # 验证报告
│       ├── ValidationReportWriter.cpp/.h # 验证报告写出
│       ├── RegressionReport.cpp/.h  # 回归报告
│       ├── RegressionReportWriter.cpp/.h # 回归报告写出
│       ├── ResultExportContext.h    # 导出上下文
│       └── ResultExportUtils.cpp/.h # 导出工具
│
├── preprocess/                      # 预处理管线
│   ├── import/
│   │   └── OBJImporter.cpp/.h      # OBJ导入
│   ├── binding/
│   │   ├── MaterialRuleLoader.cpp/.h # 材质规则加载
│   │   └── ResolveFaceDualSideMaterial.cpp/.h # 双侧材质解析
│   ├── build/
│   │   ├── SceneBatch2Builder.cpp/.h  # Batch2: 导入+材质
│   │   ├── SceneBatch3Builder.cpp/.h  # Batch3: 拓扑+加速
│   │   ├── SceneBatch4Builder.cpp/.h  # Batch4: 查询+缓存
│   │   ├── EdgeBuilder.cpp/.h       # 边构建
│   │   ├── WedgeBuilder.cpp/.h      # 楔边构建
│   │   ├── SceneDiagnostics.cpp/.h  # 场景诊断
│   │   └── SceneMaterialBinding.cpp/.h # 材质绑定过程
│   ├── accel/
│   │   ├── FaceBVHBuilder.cpp/.h    # 面元BVH构建
│   │   ├── WedgeAccelerationBuilder.cpp/.h # 楔边加速
│   │   └── SceneAcceleration.cpp/.h # 统一加速入口
│   └── cache/
│       ├── SceneCache.cpp/.h        # 场景缓存
│       ├── SceneCacheContent.h      # 缓存内容
│       └── SceneCacheMeta.h         # 缓存元信息
│
├── configs/                         # 运行配置
│   ├── app/                         # 应用配置JSON
│   │   ├── minimal.json             # [过渡] 最小配置
│   │   ├── a3_transmission_minimal.json # 透射测试配置
│   │   ├── b1_mixed_path_test.json  # B1混合路径测试配置
│   │   ├── b4_sbr_test.json         # B4 SBR测试配置
│   │   ├── invalid_*.json           # 模块1自检阴性用例
│   │   └── README.md
│   └── scenes/                      # 材质映射配置
│       ├── scene_material_map.json  # meeting.obj 材质映射
│       ├── scene_material_map_a3_transmission_minimal.json
│       └── scene_material_map_b1_test.json
│
├── demo/                            # 演示场景文件
│   ├── meeting.obj                  # [过渡] 会议室场景
│   ├── b1_mixed_path_test.obj       # B1~B10 混合路径测试盒
│   ├── a3_transmission_minimal.obj  # 单面透射测试
│   ├── ItuMaterial.csv              # ITU材质电参数表
│   └── dipole_pattern.csv           # 半波偶极子方向图
│
├── document/                        # 文档
│   ├── README.md                    # 本文档（建模指南+项目总览）
│   ├── RT算法优化开发文档 v3.md     # v3 开发主文档
│   ├── RT算法优化开发文档 v2.md     # v2 开发文档
│   ├── RT算法正式开发文档 v1.md     # v1 开发文档
│   ├── RT算法优化设计草案 v2.md     # v2 设计字典
│   ├── 新RT算法总体设计方案与开发手册草案.md
│   ├── RT算法代码全量分析报告.md
│   ├── RT算法成熟源码详细分析文档.md
│   ├── RT算法_开题需求对照与不足全量报告.md
│   ├── A8_*.md                      # A8 交接文档
│   └── 西安电子科技大学硕士学位论文开题报告表v1.23.doc
│
├── test/                            # 测试与可视化工具
│   ├── rt_utils.py                  # 共享工具库 (OBJ/Config/Path/Material)
│   ├── visualize_scene_tx_rx.py     # 统一可视化工具
│   ├── rt_grid_search.py            # 参数化网格搜索工具
│   ├── check_batch2_binding.py      # → 重定向到 visualize
│   ├── *_legacy.py                  # [历史] 旧搜索脚本
│   ├── README*.md                   # 各批次说明
│   └── output/                      # 测试输出
│
├── output/                          # 运行时输出（自动生成）
│   ├── a1_real_chain/               # A1 精确模式输出
│   │   ├── paths/precise_paths.json # 路径结果JSON
│   │   ├── channel/
│   │   ├── coverage/
│   │   ├── isac/
│   │   ├── reports/                 # validation_report + regression_report
│   │   └── visualization/
│   ├── config_snapshots/            # 配置快照
│   ├── cache/                       # 场景缓存（可选）
│   └── logs/                        # 运行日志
│
├── reference/                       # 参考实现
│   └── bvh_reference/               # [参考] 独立BVH参考实现
│
├── 算法/                            # 成熟参考实现
│   └── RT.XD.SBR.CGAL.25.05/        # ~640K行SBR完整实现
│
├── x64/Debug/                       # 编译输出
│   ├── RT.exe
│   └── RT.pdb
│
└── .git/ .gitignore .vs/
```

---

## 4. 配置文件参数说明

### 4.1 应用配置文件 (`configs/app/*.json`)

#### `app_runtime` — 运行时
| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| mode | string | "debug" | 运行模式 debug/production |
| log_level | string | "INFO" | 日志级别 TRACE/DEBUG/INFO/WARN/ERROR/FATAL |
| enable_console_logging | bool | true | 控制台日志 |
| enable_file_logging | bool | true | 文件日志 |
| log_file_path | string | "output/logs/rt.log" | 日志文件路径 |
| run_id | string | "local-run" | 本次运行标识（用于输出子目录） |
| worker_threads | int | 1 | 工作线程数 |

#### `scene_import` — 场景导入
| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| source_file | string | — | OBJ 场景文件路径 |
| source_format | string | "obj" | 格式（当前仅支持 obj） |
| scene_material_map_file | string | — | 材质映射JSON路径 |
| normalize_object_names | bool | true | 规范化对象名 |
| allow_name_auto_cleanup | bool | true | 自动清理对象名 |

#### `scene_preprocess` — 场景预处理
| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| enable_wedge_build | bool | true | 构建绕射楔边 |
| enable_scene_cache | bool | false | 启用场景缓存 |
| wedge_min_angle_deg | double | 1.0 | 楔边最小角度（度） |
| wedge_max_angle_deg | double | 179.0 | 楔边最大角度（度） |
| bvh_leaf_size | int | 8 | BVH叶节点最大面数 |
| filter_non_manifold_wedge_sources | bool | true | 过滤非流形边 |
| skip_coplanar_edges_for_wedge | bool | true | 跳过共面边 |

#### `material` — 材质
| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| material_database_file | string | "demo/ItuMaterial.csv" | ITU 材质电参数 CSV |
| material_mapping_file | string | — | 场景材质映射 JSON |
| frequency_query_mode | string | "exact" | 频率查询模式 |

#### `antenna` — 天线
| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| source_type | string | "Ideal" | 天线类型（Ideal / 文件导入） |
| pattern_file | string | "" | 方向图 CSV 路径（空=全向） |
| polarization_file | string | "" | 极化文件路径（预留） |

#### `path_search` — 几何寻径
| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| max_path_depth | int | 2 | 最大总交互次数 |
| max_reflection_count | int | 2 | 最大反射次数 |
| max_transmission_count | int | 0 | 最大透射次数 |
| max_diffraction_count | int | 0 | 最大绕射次数 |
| max_consecutive_same_interaction | int | 5 | 最大连续同类型交互（B1） |
| enable_los | bool | true | 启用LOS路径 |
| enable_reflection | bool | true | 启用反射 |
| enable_transmission | bool | false | 启用透射 |
| enable_diffraction | bool | false | 启用绕射 |
| max_candidate_face_hits | int | 64 | 面元候选上限 |
| max_candidate_wedges | int | 64 | 楔边候选上限 |
| debug_tx_x/y/z | double | — | Tx 坐标 |
| debug_rx_x/y/z | double | — | Rx 坐标 |

#### `sbr` — SBR 覆盖仿真 (B4)
| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| enabled | bool | false | 启用 SBR 模式 |
| ray_count | int | 10000 | 发射射线数量 |
| max_ray_depth | int | 6 | 射线最大反射次数 |
| ray_power_threshold_linear | double | 1e-6 | 射线功率阈值 |
| rx_sphere_radius_factor | double | 1.0 | 接收球半径系数 |
| rx_grid_min_x/max_x | double | ±5 | Rx网格 X范围 |
| rx_grid_min_y/max_y | double | ±5 | Rx网格 Y范围 |
| rx_grid_z | double | 1.5 | Rx网格 Z坐标 |
| rx_grid_step_x/step_y | double | 1.0 | Rx网格间距 |

#### `em_solver` — EM求解
| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| frequency_hz | double | 2.4e9 | 仿真频率 (Hz) |
| solver_mode | string | "Precise" | 求解模式 Precise/Coverage |
| enable_polarization | bool | true | 启用极化计算 |

#### `output` — 输出控制
| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| output_directory | string | "output" | 输出根目录 |
| export_paths | bool | true | 导出路径结果 |
| export_cir | bool | false | 导出CIR |
| export_pdp | bool | false | 导出PDP |
| export_aps | bool | false | 导出APS |
| export_config_snapshot | bool | true | 导出配置快照 |

#### `numeric_tolerance` — 数值容差
| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| eps_length | double | 1e-6 | 长度容差 |
| eps_intersection | double | 1e-7 | 交点容差 |
| eps_deduplicate | double | 1e-5 | 去重容差 |
| self_hit_ignore_distance | double | 1e-5 | 自交忽略距离 |
| visibility_origin_offset | double | 1e-5 | 可见性起点偏移 |
| visibility_target_shrink | double | 1e-5 | 可见性终点收缩 |

---

## 5. 快速开始

```bash
# 编译
# VS2022 打开 RT.sln → Build (Debug x64)

# 运行精确路径仿真（混合路径）
x64/Debug/RT.exe configs/app/b1_mixed_path_test.json

# 运行透射仿真
x64/Debug/RT.exe configs/app/a3_transmission_minimal.json

# 运行 SBR 覆盖仿真
x64/Debug/RT.exe configs/app/b4_sbr_test.json

# 可视化结果
python test/visualize_scene_tx_rx.py --obj demo/b1_mixed_path_test.obj --config configs/app/b1_mixed_path_test.json
```
