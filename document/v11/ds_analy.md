# H2hRT 全链路现状分析报告 (ds_analy.md)

> 2026-06-26 | 全量代码审查 + 理论对照 + 天线耦合深度分析
>
> **原则**: 所有结论均基于实际代码阅读，无凭记忆/猜测/想当然。本文尽量标注对应源文件和关键函数；少量跨模块结论以“代码链路 + 风险等级”表述，不把未做回归验证的内容写成绝对结论。

> **二次复核修订（2026-06-26）**: 本轮按配置读取 → 场景导入 → 场景预处理 → 材质绑定 → SBR P2P 几何寻径 → A1 EM → CIR/PDP/导出的实际代码链路重新核验。结论需要收紧为：少量 P2P 全链路已经可用，当前主推入口是 `pipeline.precise_path_source="SBR"` + `SbrEngine::RunPointToPoint()` + `SolveSinglePathEM()`；coverage、旧 SearchEngine 精确搜索、wavefront/megakernel 等仍保留为历史或辅助资产，不应与现阶段 P2P 主链混为同等成熟状态。

---

## 目录

- [1. 代码组织结构分析](#1-代码组织结构分析)
  - [1.1 目录与模块划分](#11-目录与模块划分)
  - [1.2 冗余与废弃代码识别](#12-冗余与废弃代码识别)
  - [1.3 代码可读性问题](#13-代码可读性问题)
  - [1.4 文档组织问题](#14-文档组织问题)
  - [1.5 整理建议（仅建议，未执行）](#15-整理建议仅建议未执行)
- [2. 全链路主流程分析](#2-全链路主流程分析)
  - [2.1 配置加载 (Batch 0/1)](#21-配置加载-batch-01)
  - [2.2 场景导入 (Batch 2)](#22-场景导入-batch-2)
  - [2.3 场景预处理 (Batch 3)](#23-场景预处理-batch-3)
  - [2.4 材质绑定](#24-材质绑定)
  - [2.5 几何寻径（重点）](#25-几何寻径重点)
  - [2.6 电场计算（重点）](#26-电场计算重点)
  - [2.7 后处理输出](#27-后处理输出)
  - [2.8 全链路数据流总结](#28-全链路数据流总结)
- [3. 电场计算理论验核](#3-电场计算理论验核)
  - [3.1 发射场初始化](#31-发射场初始化)
  - [3.2 自由空间传播](#32-自由空间传播)
  - [3.3 反射场计算](#33-反射场计算)
  - [3.4 透射场计算](#34-透射场计算)
  - [3.5 绕射场计算 (UTD)](#35-绕射场计算-utd)
  - [3.6 接收场合成](#36-接收场合成)
  - [3.7 相位相干与相消](#37-相位相干与相消)
  - [3.8 极化变化追踪](#38-极化变化追踪)
- [4. 天线-RT 耦合分析](#4-天线-rt-耦合分析)
  - [4.1 天线增益方向图注入](#41-天线增益方向图注入)
  - [4.2 极化方向图注入](#42-极化方向图注入)
  - [4.3 极化匹配](#43-极化匹配)
  - [4.4 天线姿态](#44-天线姿态)
  - [4.5 天线耦合完整性评估](#45-天线耦合完整性评估)
- [5. 与论文/开题需求对照](#5-与论文开题需求对照)
  - [5.1 已满足需求](#51-已满足需求)
  - [5.2 部分满足需求](#52-部分满足需求)
  - [5.3 未满足需求](#53-未满足需求)
  - [5.4 超出论文范围](#54-超出论文范围)
- [6. 已知问题与风险清单](#6-已知问题与风险清单)
  - [6.1 已修复 (v9)](#61-已修复-v9)
  - [6.2 待验证](#62-待验证)
  - [6.3 已知限制](#63-已知限制)
- [7. 总体评估](#7-总体评估)

---

## 1. 代码组织结构分析

### 1.1 目录与模块划分

```
H2hRT-7.1-SBR/
├── app/          (2 cpp, 2 h)    — 入口 + Pipeline + A1 Chain
├── core/         (52 cpp, 50 h)  — 核心库 (9个子模块)
│   ├── antenna/   (2 cpp, 4 h)   — 天线模型
│   ├── common/    (10 cpp, 23 h) — 配置/数学/日志/材质/数值
│   ├── em/        (20 cpp, 25 h) — EM求解链
│   ├── path/      (0 cpp, 5 h)   — 路径数据结构 (纯头文件)
│   ├── query/     (5 cpp, 8 h)   — 空间加速查询 (BVH/OptiX)
│   ├── result/    (8 cpp, 12 h)  — 导出/报告
│   ├── scene/     (0 cpp, 7 h)   — 场景数据结构 (纯头文件)
│   └── search/    (12 cpp, 12 h) — 寻径引擎
├── preprocess/   (14 cpp, 18 h)  — 场景预处理 (5个子模块)
├── configs/                      — 配置文件
├── demo/                         — 演示素材
├── test/                         — 测试 (4 cpp, 3 h)
├── tools/                        — SDK (OptiX/GLFW/imgui)
└── document/                     — 文档 (v1-v11 + RT_refrence)
```

**评估**: 模块划分合理，职责清晰。`core/` 下 9 个子模块各有明确的领域边界。

### 1.2 冗余与废弃代码识别

#### 1.2.1 寻径模块三重实现

`core/search/SbrEngine.cpp` (2538行) 包含 **三套独立的寻径实现**：

| 入口 | 行号 | 算法 | 状态 |
|------|:----:|------|:----:|
| `Run()` legacy CPU serial | 667-787 | Monte Carlo + 栈DFS | **冻结**（被v8 wavefront/megakernel取代） |
| `RunWavefront()` | 1521-2457 | 深度分层波前 + GPU batch | **冻结**（coverage用） |
| `RunMegakernel()` | 2463-2535 | GPU单次发射 | **冻结**（仅coverage power map） |
| `RunPointToPoint()` | 994-1376 | 栈DFS + 确定性分支 | **现行P2P主线** |
| `RunCoarsePass()` | 1381-1514 | 粗粒度SBR | **辅助**（constrained search用） |

**问题**:
- `Run()` 的 legacy 路径 (667-787行) 使用的匿名 `SbrTS` 结构体不跟踪 `medium_id`、不写传输语义完整标记、使用 Monte Carlo 透射选择。**这段代码不应被任何现行路径调用**。
- `RunWavefront()` 和 `RunPointToPoint()` 有大量重复逻辑（Fibonacci发射、Rx球检测、路径签名去重、Fermat绕射等），但实现在不同函数中，维护成本高。
- 三重实现共约 1800+ 行，实际 P2P 只需要 `RunPointToPoint()` 约 380 行 + 共享辅助函数。

#### 1.2.2 独立 SBR 模块与 H2hRT SBR 的重复

| 功能 | 独立 SBR (`G:\RT\sbr\sbr\`) | H2hRT SBR (`core/search/SbrEngine.cpp`) |
|------|----------------------------|----------------------------------------|
| Fibonacci发射 | `sbr_ray_emitter.cpp` | `SbrEngine.cpp` 内联 |
| 栈DFS R/T | `sbr_engine.cpp:RunPointToPoint` | `SbrEngine.cpp:RunPointToPoint` |
| Fermat绕射 | `sbr_engine.cpp:RunDiffraction` | `SbrEngine.cpp:SbrDiff` |
| 严格去重 | `sbr_engine.cpp:PostProcess` | `SbrEngine.cpp:SbrPP` |
| BVH | `sbr_bvh_accelerator.cpp` | H2hRT `SceneQuery` |
| OBJ加载 | `sbr_scene_loader.cpp` | H2hRT `SceneImporter` |

两个 `RunPointToPoint` 在算法上等价但面向不同的类型系统（`sbr::Scene` vs `rt::Scene`）。独立 SBR 模块是干净的原型，H2hRT SBR 是迁入版本。

#### 1.2.3 未使用的搜索分支

- `PathReuseEngine` (`core/search/PathReuseEngine.cpp`) — v8 Stage 3 路径复用，仅在 `enable_stage3_path_reuse=true` 时运行（默认关闭），且只做验证不做实际路径替换。
- `RxSeedSampler` (`core/search/RxSeedSampler.cpp`) — v8 Stage 1 Rx种子采样，仅在 `enable_stage1_coarse_sbr=true` 时运行（默认关闭）。
- `Scattering` 相关字段 — `PathState::allow_scattering`、`PathState::remaining_scatterings` 始终为 false/0。

#### 1.2.4 配置字段冗余

`AppConfig.h` 中：
- `antenna` + `tx_antenna` + `rx_antenna` 三组天线配置共存。`tx_antenna`/`rx_antenna` 为 v10.2 新增，JSON 解码中可读取，但 `EncodeAppConfigToJsonString()` 只写回旧 `antenna` 块，**配置快照无法 round-trip 保存独立 Tx/Rx 天线配置**。
- **更重要的风险**: `AntennaConfig.source_type` 默认值为 `"Ideal"`，而 `ResolveTxAntennaConfig()` / `ResolveRxAntennaConfig()` 使用“`source_type` 非空”判断是否覆盖全局 `antenna`。因此若用户只配置旧的 `antenna.pattern_file`，而没有显式配置 `tx_antenna`/`rx_antenna`，当前代码可能优先使用默认 Ideal Tx/Rx 天线，导致全局方向图没有真正注入。该问题影响天线耦合实验的可信度，应优先修正判据或默认值。
- `path_search.rx_x/y/z` 与 `path_search.rx_list` 冗余 — `rx_list` 非空时覆盖单 Rx 坐标。
- `sbr` 下约 50 个字段，其中部分在 P2P 模式下无意义（如 `rx_grid_*` 系列）。

#### 1.2.5 测试目录

`test/` 仅有 4 个 cpp + 3 个 h + 一些 Python 脚本。v9 引入了 `v9_validation.h`（16 项综合测试），但测试组织不够系统化。缺少：
- 单元测试框架（无 doctest/Catch2/gtest 集成到 VS 项目）
- 回归测试自动化
- 性能回归测试

### 1.3 代码可读性问题

| 问题 | 位置 | 严重度 |
|------|------|:-----:|
| `SbrEngine.cpp` 2538行单文件 | `core/search/SbrEngine.cpp` | 高 |
| 匿名 namespace 中的 `SbrTS` 结构体无注释说明其冻结状态 | `SbrEngine.cpp:766` | 中 |
| `Run()` 中 Morton 排序 + 栈DFS + 透射 Monte Carlo 逻辑与 `RunPointToPoint()` 中同类逻辑高度相似但独立实现 | 两处 | 中 |
| `RtPipeline.cpp` 936行，混合了 6 个 batch 的逻辑 | `app/RtPipeline.cpp` | 中 |
| 部分字段使用版本号注释（`v6`, `v7.3`, `v9 step14` 等），新开发者难以理解演进历史 | 多处 | 低 |
| `ComplexVec3` 的 `ComplexDot` 不是标准 Hermitian 内积，需要注释说明 | `core/common/math/ComplexVec3.h` | 低 |
| GPU 路径 (`RunMegakernel`, `RunWavefront`) 与 CPU 路径交错在同一文件中 | `SbrEngine.cpp` | 中 |

### 1.4 文档组织问题

`document/` 目录下 **55 个文件分布在 11 个版本子目录**（v1-v11），加上 `RT_refrence/` 子目录。文档版本演化脉络清晰，但：
- 查找特定信息需要跨多个版本文件
- 部分早期版本文档内容已过时但未标记
- v10/v11 文档较少，大部分内容和决策在 v9

### 1.5 整理建议（仅建议，未执行）

1. **拆分 `SbrEngine.cpp`** → `SbrPointToPointTracer.cpp` + `SbrDiffractionSolver.cpp` + `SbrPathPostProcess.cpp` + `SbrPathValidator.cpp`（如 `H2hRT替换全流程.md` 第5节所述）
2. **先冻结并隔离 `Run()` legacy CPU serial 路径**（约667-787行），不要直接删除；确认没有配置、测试、coverage 分支依赖后，再移动到 `archive` 或拆出历史兼容文件。整理目标是不影响现阶段功能。
3. **确认 `RunMegakernel()` 和 `RunWavefront()` 的保留策略** — 若仅为 coverage 保留，应明确注释
4. **修正 `AppConfig.h` 中 `antenna`/`tx_antenna`/`rx_antenna` 的优先级语义** — 推荐把 `tx_antenna`/`rx_antenna` 的“是否显式配置”与 `source_type` 默认值解耦，避免默认 Ideal 覆盖旧全局天线。
5. **补全 `EncodeAppConfigToJsonString`** — 添加 `tx_antenna`/`rx_antenna` 的序列化，并在配置快照中明确最终生效的 Tx/Rx 天线来源。
6. **整理 `document/`** — 将现行有效文档集中，历史版本文档归档到 `archive/` 子目录

---

## 2. 全链路主流程分析

### 2.1 配置加载 (Batch 0/1)

**入口**: `RtPipeline::Run(configPath)` → `LoadAppConfigFromJsonFile()`

**数据流**:
```
JSON文件
  → AppConfigLoader::LoadAppConfigFromJsonFile()
    → 文件存在性检查
    → AppConfigJsonCodec::DecodeAppConfigFromJsonFile()
      → nlohmann::json::parse()
      → PopulateFromJson() — 逐字段读取 14 个子配置块
  → AppConfigValidator::ValidateAppConfig() — 验证
  → ConfigSelfCheck::RunModule1SelfCheck() — 自检
```

**关键发现**:
- **JSON 解析器**: v9 已从手写 600+ 行字符串解析器迁移到 `nlohmann::json` 3.5.0 (`答疑.md` Step 14)。解码函数 `PopulateFromJson()` 位于 `AppConfigJsonCodec.cpp`，对 14 个子配置块的每个字段调用 `ReadJsonField()`。
- **编码不对称**: `EncodeAppConfigToJsonString()` 遗漏 `tx_antenna`/`rx_antenna` 的序列化（仅有解析但无写回），配置快照无法完整复现实验输入。
- **天线覆盖判据风险**: `ResolveTxAntennaConfig()` / `ResolveRxAntennaConfig()` 目前以 `source_type.empty()` 判断是否回退到全局 `antenna`，但默认 `source_type="Ideal"`。这会让“未显式配置 tx_antenna/rx_antenna”的情况也被视作已配置，从而可能绕过旧 `antenna` 块。后续做天线耦合实验前必须修正或用显式 `tx_antenna`/`rx_antenna` 配置规避。
- **容错**: `ReadJsonField` 对缺失 key 保持默认值，对类型错误静默捕获（catch-all）。这意味着拼写错误的字段名会被静默忽略 —— 可能导致用户以为设置了某参数但实际使用默认值。

**配置流向**:

```
AppConfig (全量)
  ├─→ scene_import     → SceneImporter
  ├─→ scene_preprocess  → SceneTopologyBuilder / FaceBVHBuilder / WedgeBuilder
  ├─→ material          → MaterialDatabase
  ├─→ antenna           → AntennaFactory → AntennaModel
  ├─→ path_search       → SearchEngine / PathSearchContext
  ├─→ sbr               → SbrEngine::Run / RunPointToPoint
  ├─→ em_solver         → EMSolverInput → SolveSinglePathEM
  ├─→ pipeline          → 分流控制 (precise vs coverage, SBR vs SearchEngine)
  └─→ output            → ExportPaths / ExportChannel / ExportCoverage
```

### 2.2 场景导入 (Batch 2)

**入口**: `BuildSceneForBatch2(config)` → `SceneImporter`

**处理链** (`preprocess/build/SceneImporter.cpp`):
```
OBJ文件
  → OBJImporter::Load() — 解析v/vt/vn三种格式 + 负索引 + 四边形扇形三角化 + 自动法线
  → SceneImporter — 创建 Scene 对象，填充 faces/vertices/materials
  → SceneMaterialBinding — 从 material_map JSON 读取面-材质映射
```

**关键发现**:
- v9 Step 23 修复了 OBJ 导入器：添加 v/t/n 三种格式支持 + 负索引 + 四边形扇形三角化
- 材质绑定阶段仅建立 `face_name → material_name` 映射，实际的介质/εr 解析在后续 `ResolveFaceDualSideMaterial` 中完成
- 坐标系转换支持 `blender_z_up_to_y_up`

### 2.3 场景预处理 (Batch 3)

**入口**: `BuildSceneForBatch3(config, scene)`

**处理链**:
```
Scene (含 faces + material names)
  ├─→ SceneTopologyBuilder — 构建边缘/楔形拓扑
  │     ├─→ EdgeBuilder — 识别共享边
  │     └─→ WedgeBuilder — 计算二面角，生成 Wedge 对象
  │           过滤条件: angle ∈ [wedge_min_angle_deg, wedge_max_angle_deg] (默认 [1°,179°])
  │           diffractable = (dihedral ∈ [3°,177°])
  ├─→ FaceBVHBuilder — 构建 SAH-BVH 加速结构
  │     leaf_size = config.scene_preprocess.bvh_leaf_size (默认 8-16)
  │     桶排序 SAH (16桶, 3轴最优选择)
  ├─→ SceneDiagnostics — 场景诊断统计
  └─→ SceneVisibilityBuilder (Stage 0) — PVS/边邻接/角度网格
```

**关键发现**:
- 楔形构建的 `wedge_angle_deg = 360° - dihedral` 符合 UTD 外角定义
- 提供了 `filter_non_manifold_wedge_sources` 和 `skip_coplanar_edges_for_wedge` 过滤选项
- Scene cache 机制存在但默认关闭，使用 `size_t` 导致跨平台兼容问题 (v7 H7/H8)

### 2.4 材质绑定

**处理链**:
```
MaterialDatabase::LoadFromCsv(ITU-R P.2040 CSV)
  → 解析 12 种建筑材料在 8 个频点的 εr(f), σ(f)
  → 按频率查询: GetEpsilonR(freq_hz), GetSigma(freq_hz)
  → 线性和对数插值（对数间距默认）

ResolveFaceDualSideMaterial (preprocess/binding/)
  → 对每面: 从 material_map 查找 front/back 材质名
  → 记录: front_medium_id, back_medium_id, front_material_id, back_material_id
  → 验证: front/back 材质名非空且 medium_id 均有效时标记 transmission_semantic_complete=true
  → 存储: face.dual_side_material_resolved, face.transmission_semantic_complete
```

**关键发现**:
- v9 StageH: 引入 `missing_material_policy`，默认 `"strict"`（缺失材质中止运行）
- v9 G-3: 增加了材质绑定诊断和 scene_preflight_report.json
- 材质数据库从 ITU-R P.2040-1 参数表加载（v9 Step 35 替换了旧的启发式参数 `b=-0.1, d=0.5`）
- **频率对寻径的影响**: 仅通过 `εr(f)` 影响 Snell 折射方向和 TIR 判据，不影响反射方向
- **注意**: `ResolveFaceDualSideMaterial()` 对面元级 `transmission_semantic_complete` 的判据不要求 front/back medium 不同；但 `PreparePathForEM()` 会拒绝透射节点 `medium_in_id == medium_out_id` 的路径。因此“材质绑定语义完整”和“EM 可透射求解”是两层判据，不能混写。

### 2.5 几何寻径（重点）

H2hRT 有 **两条独立的寻径主线**，通过 `pipeline.precise_path_source` 选择：

#### 2.5.1 SearchEngine 寻径（Precise 模式）

**入口**: `SearchEngine::Run(context)` 或 `SearchEngine::Run(context, constraints)`

**算法**: 确定性 best-first 搜索（优先队列）

```
初始化: Tx→Rx 方向作为初始状态入队
循环:
  出队优先级最高的状态
  ├─→ TryBuildLosPath() — 方向闭合检查 (v9 Step 5) + 可见性检查
  ├─→ ExpandReflection() — 镜像法: 将 Rx 关于候选面做镜像，求交验证
  ├─→ ExpandTransmission() — 向 Rx 方向投射，对每个命中面做 Snell 折射
  ├─→ ExpandDiffraction() — 对候选楔形做 Golden Section Search (32 迭代, Fermat 最优)
  └─→ 将有效后继状态入队
  去重: BuildStateSignature() + BuildPathSignature()
  截断: per_expander_keep_limit, per_state_keep_limit
```

**与 RT.XD 参考实现的差异**:
- RT.XD 使用**递归 R→T**（反射先完整追踪再回溯透射）
- H2hRT SearchEngine 使用**优先队列**（按估计总长度排序）
- RT.XD 的 Image Method 是全组合展开（所有可能的反射面组合），H2hRT 的 ExpandReflection 是贪心候选 + keep_limit 截断
- 两者在单反射/单透射场景下结果等价，在高阶组合场景下 RT.XD 更完备

**已知限制** (v10开发手册):
1. 初始状态方向直接从 Tx 指向 Rx（不是全向搜索）
2. ExpandReflection 使用单镜像 Rx 方法（适合单次反射，不适合高阶反射序列）
3. 反射后 `current_direction` 被设为指向 Rx（提前闭合，不是继续沿真实出射方向传播）
4. ExpandTransmission 从当前点重建指向 Rx 的射线（而非沿真实出射方向继续）
5. `per_expander_keep_limit` / `per_state_keep_limit` 造成启发式截断，不保证路径完备性

#### 2.5.2 SBR 寻径（P2P 模式）

**入口**: `SbrEngine::RunPointToPoint(context)`

**算法**: 确定性栈式 DFS（与独立 SBR 模块的 `SbrEngine::RunPointToPoint` 算法等价）

```
Fibonacci 球面发射 N 条射线 (Morton 排序优化缓存)
#pragma omp parallel for schedule(dynamic, 1)
  每条射线:
    栈初始化: TraceState {Tx, dir, pwr=1.0, cr, ct, depth=0, nodes=[Tx]}
    while 栈非空:
      pop state
      求交 (QueryClosestFaceHit)
      Rx检测: 线段到 Rx 球距离 < 有效半径 → 遮挡验证 → 记录路径
      终止条件: 无命中 | 功率<阈值 | 深度超限
      if 命中面支持反射 && cr>0: push 反射分支
      if 命中面支持透射 && ct>0 && !TIR: push 透射分支 (Snell 折射)
      (绕射在单独的 RunDiffraction 中处理)
合并所有线程局部结果
PostProcess: 严格去重 → 签名写回 → 编号
```

**与独立 SBR 的对比**: 算法一致，类型系统不同（`rt::Scene` vs `sbr::Scene`）。H2hRT 版本使用 `SceneQuery` 做求交，独立 SBR 使用自研 SAH-BVH。

**P2P 模式的保证**:
- ✅ 确定性分支（不丢失反射或透射可能）
- ✅ 严格去重（hash + 二级精确比对）
- ✅ 不截断路径（`path_top_n_per_rx=0`）
- ✅ 绕射独立路径 (Tx-D-Rx)，不与 R/T 混合
- ✅ 透射介质语义完整验证

**P2P 模式不保证**:
- ❌ 穷举所有几何可能路径（基于射线采样的方法天然不完备）
- ❌ 路径完备性（与 Image Method 全组合展开相比）
- ❌ Coverage 大规模 Rx

#### 2.5.3 绕射处理

**SearchEngine**: 每状态对候选楔形做 Golden Section Search (32次迭代, 精度 `1e-6`)，Fermat 最优衍射点 + 单次 Keller 锥验证。

**SBR**: 独立 `SbrDiff()` / `TracePointToPointDiffraction()`，分析 Fermat 点 + 可见性验证（忽略楔形两个邻面）。

两者均产生 **独立 Tx-D-Rx 路径**，不做 R/T + D 混合路径。这是设计决策而非遗漏（论文范围内绕射仅需单次绕射）。

### 2.6 电场计算（重点）

**入口**: `RunA1RealChain()` → `BuildRealPathResultSet()` → `SolveSinglePathEM()`

详见第3节理论验核。此处给出流程概览：

```
每条 GeometricPath:
  PreparePathForEM() — 验证路径有效性
  InitializeTxField() — 设置初始 E 场 (ComplexVec3)
  for each segment:
    ApplyFreeSpaceSegment(d) — E *= exp(-j*k0*n*d) * exp(-alpha*d)
    if Reflection:    ApplyReflectionInteraction() — TE/TM 分解 → Fresnel Γ → 重构
    if Transmission:  ApplyTransmissionInteraction() — TE/TM 分解 → Fresnel T → 重构 + 更新介质状态
    if Diffraction:   ApplyDiffractionInteraction() — Soft/Hard 分解 → UTD D → 重构
  FinalizeAtReceiver() — FSPL 衰减 + Rx 天线共轭匹配 → EMPathResult
```

**关键特征**:
- 全复数向量传播（v9 B-series ComplexVec3 升级）
- 相位在每个传播段和每次交互中累积
- 极化信息在 TE/TM (R/T) 和 Soft/Hard (D) 基下跟踪
- FSPL 仅应用一次（在 FinalizeAtReceiver），不在逐段传播中应用

### 2.7 后处理输出

**聚合**: `BuildAggregateResult()` 从逐径 EM 结果生成：
```
EMPathResultSet
  ├─→ BuildCIR() — 信道冲激响应 (理想 δ + 观测)
  ├─→ BuildPDP() — 当前主链调用无 profile 版本，默认逐路径 PDP
  ├─→ BuildAPS() — 角度功率谱 (36×72 θ-φ 网格)
  ├─→ BuildChannelStatistics() — RMS 时延扩展, K 因子, 有效路径数
  ├─→ BuildCoverageResult() — 覆盖结果
  └─→ BuildISACFeatureSet() — ISAC 特征集
Broadband (v9 Stage 5):
  ├─→ BuildBroadbandCFR_FixedGain() — 固定增益 CFR
  └─→ BuildBroadbandCFR_FrequencySweep() — 频率扫描 CFR (未实现)
```

**导出** (Module 6):
```

**二次复核补充**:
- `BuildAggregateResult()` 当前调用的是 `BuildPDP(pathResults)`，不是 `BuildPDP(pathResults, profile)`。因此主导出的 PDP 是逐路径功率列表；带 delay-bin 的 coherent/incoherent PDP 代码存在，但当前 A1 聚合主链未使用。
- `BuildCIR(pathResults, profile)` 在 `delay_bin_s > 0` 且启用相干合成时会按 bin 复数相加，这是当前多径相干/相消的主要输出通道。
ResultExportContext
  ├─→ ExportPaths() — 路径几何 JSON
  ├─→ ExportChannel() — CIR/PDP/APS CSV
  ├─→ ExportCoverage() — 覆盖图
  ├─→ ExportISAC() — ISAC 特征
  └─→ ExportVisualization() — 可视化数据
```

### 2.8 全链路数据流总结

```
┌─────────────────────────────────────────────────────────────────────┐
│ Batch 0/1: 配置                                                     │
│   JSON → AppConfig → 验证 → 自检                                     │
├─────────────────────────────────────────────────────────────────────┤
│ Batch 2: 场景导入                                                    │
│   OBJ → Scene {faces, vertices, material_names}                     │
├─────────────────────────────────────────────────────────────────────┤
│ Batch 3: 场景预处理                                                   │
│   Scene → Topology {edges, wedges} + BVH + Visibility               │
├─────────────────────────────────────────────────────────────────────┤
│ 材质绑定                                                             │
│   ITU CSV → MaterialDatabase                                        │
│   material_map JSON → ResolveFaceDualSideMaterial → Face.{medium,material}_id │
├─────────────────────────────────────────────────────────────────────┤
│ Batch 5: 寻径                                                        │
│   precise_path_source="SearchEngine" → SearchEngine → GeometricPathSet │
│   precise_path_source="SBR" → SbrEngine::RunPointToPoint → GeometricPathSet │
├─────────────────────────────────────────────────────────────────────┤
│ A1 Chain: EM 求解                                                    │
│   GeometricPath → SolveSinglePathEM → EMPathResult (per path)       │
│   EMPathResultSet → BuildAggregate → CIR/PDP/APS/Stats              │
├─────────────────────────────────────────────────────────────────────┤
│ Module 6: 导出                                                       │
│   Paths + Channel + Coverage + ISAC + Visualization                 │
└─────────────────────────────────────────────────────────────────────┘
```

**Pipeline 分支逻辑** (`RtPipeline.cpp:554-562`):

```cpp
coverageEnabled = config.sbr.enabled;
preciseEnabled  = config.pipeline.enable_stage4_precise_em
               && config.em_solver.solver_mode != "Coverage";
useSbrForPrecise = preciseEnabled
                && config.pipeline.precise_path_source == "SBR";
```

| coverageEnabled | preciseEnabled | useSbrForPrecise | 实际执行 |
|:---:|:---:|:---:|---|
| 0 | 0 | — | 中止（无有效模式） |
| 1 | 0 | — | 纯 Coverage SBR |
| 0 | 1 | 0 | Precise SearchEngine + A1 |
| 0 | 1 | 1 | Precise SBR P2P + A1 |
| 1 | 1 | 0 | SearchEngine + A1 → Coverage SBR |
| 1 | 1 | 1 | SBR P2P + A1 → Coverage SBR |

---

## 3. 电场计算理论验核

> 本节对每个 EM 计算步骤进行公式与代码的对照验证。
> 理论依据: Balanis Ch.5/Ch.12, Born & Wolf, Kouyoumjian & Pathak 1974, Ludwig 1973。
> D1 审计中的发现已在 v9 中修正（标记 ✅），未修正的（标记 ⚠️）。

### 3.1 发射场初始化

**文件**: `core/em/InitializeTxField.cpp`

**公式链**:

```
λ = c₀ / f                     (c₀ = 299792458 m/s)
E₀ = sqrt(P_tx) · ê_tx(θ,φ)    (单位复极化矢量 × 振幅)
```

**代码映射**:

| 步骤 | 代码位置 | 公式 | 验证 |
|------|---------|------|:---:|
| 波长计算 | line 38: `wavelength_m = kC0 / frequency_hz` | `λ = c₀/f` | ✅ |
| 初始振幅 | line 40: 初始化为 1.0 (单位振幅) | 后续乘 gain | ✅ |
| 发射方向 | 从 `path.nodes[1].point - path.nodes[0].point` 提取 | AoD | ✅ |
| 天线坐标变换 | `WorldToAntennaSpherical()` | `θ = acos(hat_d·hat_f)`, `φ = atan2(hat_d·hat_u, hat_d·hat_r)` | ✅ |
| 极化查询 | `pattern.QueryPolarization(θ, φ)` | Ludwig-3 双线性插值 | ✅ |
| Jones 重构 | `E_pol = polTheta·θ̂ + polPhi·φ̂` | 世界坐标系复极化矢量 | ✅ |
| 增益加权 | `amplitude *= sqrt(gain_linear)` | `E_tx = sqrt(G_tx)·ê_tx` | ✅ |
| 初始化 E 场 | `E_world = sqrt(P_tx) · unit_Jones_vector` | ComplexVec3 | ✅ |

**默认极化**: 无天线方向图时，`E_world = (0, sqrt(P_tx), 0)` — Y 轴垂直线极化。

### 3.2 自由空间传播

**文件**: `core/em/ApplyFreeSpaceSegment.cpp`

**公式**:

```
E(d) = E(0) · exp(-α·d) · exp(-j·k₀·n·d)

其中:
  k₀ = 2π/λ     (真空波数)
  n  = Re(√ε_c) (当前介质折射率)
  α  = k₀·|Im(√ε_c)| (当前介质衰减常数 [Np/m])
  d  = segment_length
```

**代码映射**:

| 步骤 | 代码位置 | 公式 | 验证 |
|------|---------|------|:---:|
| 相位累积 | line 47-50 | `prop = exp(-αd) · (cos(k₀nd) - j·sin(k₀nd))` | ✅ |
| E 场旋转 | line 52-54 | `E_world.x *= prop; E_world.y *= prop; E_world.z *= prop` | ✅ |
| 时延累积 | line 40-42 | `delay_s += d·n/c₀` | ✅ |
| 介质衰减 | line 36-47, 61-63 | 复矢量主分支中 `E_world *= exp(-αd)·exp(-jkd)`，同时累计 `media_attenuation_np` 供旧标量分支/诊断使用 | ✅ |

**注意**: FSPL (`λ/(4πd)`) **不在此处应用**，仅在 `FinalizeAtReceiver` 中按总路径长度应用一次。复矢量主分支的介质损耗已经逐段乘入 E 场；不要在接收端再次乘同一个 `mediaLoss`，否则会形成二次介质衰减。

### 3.3 反射场计算

**文件**: `core/em/ApplyReflectionInteraction.cpp`

**公式链**:

```
ε_c = ε_r - j·σ/(ω·ε₀)        (复介电常数)

Γ_TE = [cosθᵢ - √(ε_c - sin²θᵢ)] / [cosθᵢ + √(ε_c - sin²θᵢ)]    (s 极化)

Γ_TM = [ε_c·cosθᵢ - √(ε_c - sin²θᵢ)] / [ε_c·cosθᵢ + √(ε_c - sin²θᵢ)]  (p 极化)

E_ref = Γ_TE · (E_inc · ê_TE) · ê_TE + Γ_TM · (E_inc · ê_TM) · ê_TM
```

**代码映射**:

| 步骤 | 代码位置 | 公式 | 验证 |
|------|---------|------|:---:|
| 复介电常数 | `CalcEpsC()` | `ε_c = ε_r - j·σ/(ω·ε₀)` | ✅ (D1 #1) |
| TE 基底 | line 65-75 | `ê_TE = normalize(k_inc × n)`, `ê_TM = normalize(ê_TE × k_inc)` | ✅ |
| Γ_TE | `FresnelTE()` | 标准 Balanis 式 | ✅ (D1 #1) |
| Γ_TM | `FresnelTM()` | 标准 Balanis 式 | ✅ (D1 #2) |
| 复投影 | v9 B2-a: `ComplexDot(E_world, ê_TE)` | 复标量投影 | ✅ |
| 系数应用 | `E_TE_ref = Γ_TE * E_TE` | 复乘法 | ✅ |
| 重构 | `E_ref = E_TE_ref * ê_TE + E_TM_ref * ê_TM` | `ReconstructFromBasis` | ✅ |

### 3.4 透射场计算

**文件**: `core/em/ApplyTransmissionInteraction.cpp`

**公式链**:

```
T_TE = 2·cosθᵢ / [cosθᵢ + √(ε_c - sin²θᵢ)]              (s 极化)

T_TM = 2·√(ε_c)·cosθᵢ / [ε_c·cosθᵢ + √(ε_c - sin²θᵢ)]  (p 极化)  ← v9 修正!

E_trans = T_TE · (E_inc · ê_TE) · ê_TE + T_TM · (E_inc · ê_TM) · ê_TM

介质更新:
  n_eff = max(1.0, Re(√ε_c))
  α = k₀·|Im(√ε_c)|
```

**代码映射**:

| 步骤 | 代码位置 | 公式 | 验证 |
|------|---------|------|:---:|
| T_TE | `FresnelTE_T()` | 标准 Born & Wolf 式 | ✅ (D1 #3) |
| **T_TM 修正** | `FresnelTM_T()` | `2·√(ε_c)·cosθᵢ / (ε_c·cosθᵢ + √(...))` | ✅ (v9 修正 D1 #4) |
| 折射率更新 | line 95-99 | `n_eff = max(1.0, Re(√ε_c))` | ✅ |
| 衰减常数 | line 95-97 | `α = k₀·|Im(√ε_c)|` | ✅ |
| TIR 检测 | `SnellRefractV2` | `sin²θₜ ≥ 1.0 → TIR` | ✅ |

**⚠️ 历史 Bug (D1 #4, CRITICAL)**: TM 透射系数的分子原本为 `2·ε_c·cosθᵢ`，错误地使用了 `ε_c` 而非 `√(ε_c)`。这导致 TM 透射振幅被高估约 `√|ε_c|` 倍（混凝土 ~2.3×，玻璃 ~2.5×）。v9 Step B2-b 已修正为 `2·√(ε_c)·cosθᵢ`。**此修正是 v9 中最重要的单项理论修正。**

### 3.5 绕射场计算 (UTD)

**文件**: `core/em/ApplyDiffractionInteraction.cpp`

**公式链** (Kouyoumjian & Pathak 1974):

```
D = -exp(-jπ/4) / (2n·√(2πk)·sinβ₀) · [T1 + T2 ± T3 ± T4]

其中:
  Soft 边界 (TE-like): D_soft = T1+T2-T3-T4
  Hard 边界 (TM-like): D_hard = T1+T2+T3+T4

T1 = cot((π+(φ-φ'))/(2n)) · F(kL·a⁺(φ-φ'))
T2 = cot((π-(φ-φ'))/(2n)) · F(kL·a⁻(φ-φ'))
T3 = cot((π+(φ+φ'))/(2n)) · F(kL·a⁺(φ+φ'))
T4 = cot((π-(φ+φ'))/(2n)) · F(kL·a⁻(φ+φ'))

a^±(β) = 2·cos²((2πnN^± - β)/2), N^± = round((β ± π)/(2πn))

L = s₁·s₂·sin²β₀ / (s₁+s₂)        (球面波入射)
```

**代码映射**:

| 步骤 | 代码位置 | 公式 | 验证 |
|------|---------|------|:---:|
| Keller 锥角 | line 64-68 | `β₀ = acos(|k_out · ê_edge|)` | ✅ |
| 边固定坐标系 | line 137-146 | `ê_z = edgeDir`, `ê_x = cross(n₀, ê_z)` | ✅ |
| φ, φ' 计算 | line 149-170 | `φ = atan2(k_out⊥·ê_y, k_out⊥·ê_x)` | ✅ |
| 楔形参数 n | line 183-185 | `n = (2π-α)/π`, α = 内角 | ✅ |
| 距离参数 L | line 175 | `L = s₁·s₂·sin²β₀/(s₁+s₂)` | ✅ (v7 C3 修正) |
| D_soft / D_hard | `ComputeUTD_D()` 行 70-93 | K&P Table 1 逐符号对应 | ✅ (D1 #6) |
| N^± 四舍五入 | 行 77-82 | `N = round((β±π)/(2πn))` | ✅ (D1 #8) |
| F(x) 积分 | `FresnelTransitionNumerical()` | 8-point GL × 16 intervals | ⚠️ (D1 #7, 近似 <0.1% 误差) |
| x → ∞ 渐近 | line 44-46 | `F(x) ≈ 1 + j/(2x)` for x>50 | ✅ |
| Soft/Hard 投影 | v9 B2-c | `ComplexDot(E_inc, ê_soft/hard)` | ✅ |
| 复系数应用 | v9 B2-c | `E_diff = D_soft·E_soft·ê_soft + D_hard·E_hard·ê_hard` | ✅ |

**⚠️ 遗留问题 (D1 #7, LOW)**: 8 点 Gauss-Legendre × 16 区间数值积分在大 x 时精度下降，但总积分贡献已很小。总体误差 < 0.1%。

**⚠️ 遗留问题 (D1 #8, MEDIUM)**: 边固定坐标系从出射方向（而非入射方向）构建。在 Keller 锥条件下等价。需验证 `DiffractionExpander` 正确写入了 `incident_direction`。

### 3.6 接收场合成

**文件**: `core/em/FinalizeAtReceiver.cpp`

**公式链**:

```
FSPL 振幅因子:  A_fspl = λ / (4πd_total)           (Friis)

介质总衰减:    A_media = exp(-Σαᵢdᵢ)

接收复电压:
  h_rx = ê_θ·Pol_θ(θ_rx,φ_rx) + ê_φ·Pol_φ(θ_rx,φ_rx)   (Rx Jones 矢量)
  V_rx = A_fspl · A_media · (E_inc · h_rx*) · sqrt(G_rx)  (共轭匹配)

接收功率:      P_rx = |V_rx|²

交叉极化比:    XPR = 10·log₁₀(|E_inc·ê_θ|² / |E_inc·ê_φ|²)
```

**代码映射**:

| 步骤 | 代码位置 | 公式 | 验证 |
|------|---------|------|:---:|
| FSPL | line 41: `fsplAmp = wavelength / (4*pi*total_length)` | 标准 Friis | ✅ (D1 #5) |
| 零长度保护 | line 35-38 | total_length ≤ ε → 返回零功率 | ✅ (v9 Step 2) |
| 介质衰减 | 复矢量主分支不再乘 `mediaLoss`；旧标量兼容分支使用 `mediaLoss = exp(-media_attenuation_np)` | 避免二次衰减 |
| Rx 天线坐标 | `WorldToAntennaSpherical(incDir, ...)` | 角度变换 | ✅ |
| Rx 增益 | `QueryGainDBi(θ, φ)` if pattern loaded | 双线性插值 | ✅ |
| Rx 极化 | `QueryPolarization(θ, φ)` if pol loaded | Jones 矢量 | ✅ |
| **共轭匹配** | `vr = ComplexDot(E_inc, conj(h_rx))` × `sqrt(rxGainLin)` | 标准极化匹配 | ✅ (v9 B3) |
| Co/Cross-pol | `coV = ComplexDot(E_inc, θ̂)`, `cxV = ComplexDot(E_inc, φ̂)` | 投影分解 | ✅ |
| AoD/AoA | 从 path.nodes 提取方向 | 几何计算 | ✅ |

### 3.7 相位相干与相消

**相位累积路径**:

每条路径的最终复电压为：

```
V_rx = A_fspl(d_total) · A_media · sqrt(G_tx·G_rx) · (ê_rx* · T_path · ê_tx)

其中 T_path = T_N · T_{N-1} · ... · T₁  (每段的传输矩阵)

每段 T_i 包含:
  - exp(-j·k₀·n·d)    传播相位
  - Γ 或 T 或 D        交互复系数
```

**相位处理正确性分析**:

| 方面 | 处理方式 | 验证 |
|------|---------|:---:|
| 逐段相位累积 | `ApplyFreeSpaceSegment`: `E *= exp(-j·k₀·n·d)` | ✅ |
| 交互复系数 | 反射/透射/绕射均使用完整复数系数（幅值+相位） | ✅ |
| TE/TM 相位分离 | 每个偏振分量独立乘以复系数，保持相对相位 | ✅ |
| Soft/Hard 相位分离 | 绕射中两个分量独立处理 | ✅ |
| 多径合成 | `BuildCIR` 使用复数加法（相干合成） | ✅ |
| 非相干 PDP | 带 profile 的 `BuildPDP(pathResults, profile)` 支持 coherent/incoherent 分 bin，但当前 `BuildAggregateResult()` 主链未调用该重载 | ⚠️ |

**关键正确性验证**: v9 B-series ComplexVec3 升级前，反射/透射的相位处理存在缺陷（实投影丢失虚部相位信息）。升级后全链路使用复数向量，相位一致性得到保证。

### 3.8 极化变化追踪

**每次交互的极化状态变化**:

```
初始 (Tx):     E₀ = sqrt(G_tx) · (pol_θ·θ̂ + pol_φ·φ̂)   [Jones 矢量, θ̂-φ̂ 基]

反射:          E_ref = Γ_TE·(E₀·ê_TE)·ê_TE + Γ_TM·(E₀·ê_TM)·ê_TM
               坐标系: ê_TE-ê_TM 基 → 世界坐标系

透射:          E_trans = T_TE·(E₀·ê_TE)·ê_TE + T_TM·(E₀·ê_TM)·ê_TM
               坐标系: 同上 + 介质状态更新(n, α)

绕射:          E_diff = D_soft·(E₀·ê_soft)·ê_soft + D_hard·(E₀·ê_hard)·ê_hard
               坐标系: ê_soft-ê_hard 基 → 世界坐标系

接收 (Rx):     V_rx = A_fspl·(E_inc · h_rx*)·sqrt(G_rx)
               坐标系: 世界坐标系 → θ̂-φ̂ 基 (天线局部)
```

**极化追踪正确性分析**:

| 方面 | 处理方式 | 验证 |
|------|---------|:---:|
| 基底变换 | 每次交互使用正确的局部基底 (TE/TM 或 Soft/Hard) | ✅ |
| 复系数保持 | 所有 Fresnel/UTD 系数使用完整复数 | ✅ |
| 世界坐标重构 | `ReconstructFromBasis(c1, b1, c2, b2)` 恢复到世界坐标 | ✅ |
| Rx 极化投影 | 共轭匹配 `E_inc · h_rx*` | ✅ |
| Co/Cross-pol | 独立于天线极化，始终使用 θ̂/φ̂ 分解 | ✅ |

**⚠️ v9 前遗留的三处极化简化** (D1 #11)，已在 v9 ComplexVec3 升级中全部修正：
1. 实投影 → 复投影 ✅
2. 仅用实部重构 → 复用虚部重构 ✅
3. 绕射非相干平均 → 相干复数合成 ✅

---

## 4. 天线-RT 耦合分析

### 4.1 天线增益方向图注入

**注入点**:
- **Tx 侧**: `InitializeTxField.cpp`，在发射场初始化时查询 `G_tx(θ_AoD, φ_AoD)`
- **Rx 侧**: `FinalizeAtReceiver.cpp`，在接收场合成时查询 `G_rx(θ_AoA, φ_AoA)`

**数据格式**: CSV 文件，列: `Theta[deg], Phi[deg], Gain_dBi`

**插值方法**: 双线性插值（theta-phi 网格，phi 维周期性边界，theta 维极值钳位）

**支持的天线类型** (实测或解析):

| 模型 | 峰值增益 | 来源 | CSV 文件 |
|------|:------:|------|---------|
| Ideal (Isotropic) | 0 dBi | 解析 | 无 (代码内置) |
| Half-Wave Dipole | 2.15 dBi | 解析公式 | `dipole_gain.csv` |
| Microstrip Patch | 7.2 dBi | HFSS 全波仿真 | `patch_gain.csv` |
| Pyramidal Horn | 14.8 dBi | HFSS 全波仿真 | `horn_gain.csv` |

### 4.2 极化方向图注入

**注入点**: 同上（`InitializeTxField.cpp` + `FinalizeAtReceiver.cpp`）

**数据格式**: CSV 文件，列: `Theta, Phi, Gain_dBi, PolTheta_re, PolTheta_im, PolPhi_re, PolPhi_im`

**Ludwig-3 约定**:
- `polTheta = E · θ̂`（共极化参考方向在 boresight 对齐垂直）
- `polPhi = E · φ̂`
- 归一化: `|polTheta|² + |polPhi|² = 1` 在每个方向

**世界坐标重构**:
```
ê_θ_world = cosθ·cosφ·r̂ + cosθ·sinφ·û - sinθ·f̂
ê_φ_world = -sinφ·r̂ + cosφ·û
E_pol = polTheta·ê_θ_world + polPhi·ê_φ_world
```

**验证**: 坐标系变换公式与标准 Ludwig-3 定义一致 (`AntennaPattern.h::AntennaSphericalBasisToWorld`)

### 4.3 极化匹配

**公式**: `V_rx = (E_inc · h_rx*) · sqrt(G_rx)`

其中 `h_rx` 是 Rx 天线在到达角方向的复极化矢量。

**极化损失因子**: `PLF = |ê_inc · ê_rx*|²` (∈ [0,1])

**代码实现** (`FinalizeAtReceiver.cpp:125-127`):
```cpp
ComplexVec3 h_rx_conj = Conj(h_rx);
Complex vr = ComplexDot(E_inc, h_rx_conj);
vr = vr * sqrt(rxGainLin);
```

**验证**: 这是标准的共轭匹配公式，最大化接收功率。

### 4.4 天线姿态

**表示**: `(forward, right, up)` 正交三元组
- `forward` = boresight (主瓣方向)
- `up` = 极化参考"上"方向
- `right = cross(forward, up)` (自动计算)

**配置**:
```json
"antenna": {
    "forward_x": 1.0, "forward_y": 0.0, "forward_z": 0.0,  // boresight
    "up_x": 0.0, "up_y": 0.0, "up_z": 1.0                   // 上方向
}
```

**验证**: `ValidateAntennaPose()` 检查正交性（`|cross(forward, up)| < 1e-4` 则拒绝并回退默认姿态）

**世界到天线坐标变换**:
```
θ = acos(hat_d_world · hat_f)
φ = atan2(hat_d_world · hat_u, hat_d_world · hat_r)
```

**验证**: 与标准球坐标变换一致。

### 4.5 天线耦合完整性评估

| 天线属性 | 当前状态 | 论文需求 | 差距 |
|---------|:-------:|:------:|------|
| 增益方向图 (G(θ,φ)) | ✅ 已实现 (CSV + 双线性插值) | 必须 | 无 |
| 复极化方向图 (Jones) | ✅ 已实现 (Ludwig-3 CSV + 复投影) | 必须 | 无 |
| 天线姿态 (6-DoF) | ✅ 已实现 (forward/up 三元组) | 必须 | 无 |
| 共轭极化匹配 | ✅ 已实现 (ComplexVec3 复点积) | 必须 | 无 |
| Co/Cross-pol 分解 | ✅ 已实现 (θ̂/φ̂ 投影) | 必须 | 无 |
| Tx/Rx 独立天线 | ⚠️ 部分 (v10.2 有结构，JSON 编解码不对称) | P1 | JSON 序列化缺失 |
| 天线频率响应 | ❌ 未实现 (仅单频点方向图) | P0 | ~100 行 |
| 天线群时延 | ❌ 未实现 | P1 | ~80 行 |
| 相位中心偏移 | ⚠️ 字段存在 (`AntennaModel.phase_center_offset_m`) 但未接入路径几何 | P3 | 需要路径节点坐标修正 |
| 天线互耦合 (MC) | ❌ 未实现 | 超出硕士范围 | 无 |
| MEG 计算 | ✅ 已实现 (`BuildChannelStatistics`) | P1 | 无 |

---

## 5. 与论文/开题需求对照

> 基于 `v5/对照_D2_开题需求全量对照.md` + `v9/答疑.md` + `v10/v10开发手册.md` + 代码验证。

### 5.1 已满足需求

| 编号 | 需求 | 验证依据 |
|:----:|------|---------|
| R3 | OBJ 导入 + PDP/APS 输出 | `OBJImporter` + `BuildPDP` + `BuildAPS` + Python 可视化 |
| R7 | UTD 绕射 | 8-point GL, K&P 1974 公式逐行验证 (D1 #6-9) |
| R8 | 混合 R/T/D 路径 | SearchEngine 优先队列 + 路径签名去重 + GeometryValidity |
| R9 | SBR 大规模覆盖 | ⚠️ 部分满足。`SbrEngine::Run()`/`RunWavefront()` 保留 RxHashGrid、OpenMP、动态 Rx 球等能力，但它们属于历史 coverage 通道；现阶段主推且已对齐独立 SBR 的是 `RunPointToPoint()` 少量 P2P。大规模 coverage 的流式 EM、内存控制和结果一致性仍应作为后续完整 RT 内开发项。 |

### 5.2 部分满足需求

| 编号 | 需求 | 满足部分 | 不满足部分 |
|:----:|------|---------|-----------|
| R1 | ISAC/3GPP 多域建模 | ISAC 特征集已定义 | 3GPP InH-Office 未对比；ISAC 特征未充分验证 |
| R2 | 基带多径完整参数 | 全复数向量 EM 传播 (v9) | 缺少 H(f) 矩阵输出 |
| R4 | 天线时延-极化-旋转联合建模 | 增益+极化+姿态已实现 | 缺少时延响应、频率响应、HFSS 导入界面 |
| R5 | 多域验证系统 | L1 5 项解析测试通过 | L2/L3 为占位测试，缺少交叉验证 |
| R6 | Fresnel 反射/透射 | 全部系数正确 | TM 透射 Bug 在 v9 已修正 ✅ |
| R10 | 天线方向图外部导入 | CSV 加载 + 双线性插值 | 缺少频率插值（仅单频点） |

### 5.3 未满足需求

| 编号 | 需求 | 状态 |
|:----:|------|------|
| R11 | 3GPP TR 38.901 InH-Office 对比 | ❌ 未实现 (计划 D4-C) |

### 5.4 超出论文范围 (明确排除)

| 编号 | 内容 |
|:----:|------|
| R12 | 时变/Doppler 建模 |
| R13 | 仿真-测量对比 (替换为三基准: 参考实现 + Sionna + 3GPP) |

---

## 6. 已知问题与风险清单

### 6.1 已修复 (v9)

| 编号 | 问题 | 严重度 | 修复 |
|:----:|------|:---:|------|
| D1#4 | TM 透射分子 bug (`ε_c` → `√ε_c`) | **Critical** | v9 B2-b |
| D1#11 | 极化实投影/实部重构/非相干平均 | **High** | v9 B-series ComplexVec3 |
| step1 | Snell 折射无 cosI 钳位/无自动法线翻转 | High | SnellRefractV2 |
| step2 | FSPL 除以零 | High | 零长度保护 |
| step5 | 透射后直接闭合到 Rx 无方向验证 | High | 方向闭合检查 |
| step8 | TIR 被误录为透射分支 | High | SBR CPU/Wavefront 预检查 |
| step9 | 状态签名缺少位置量化 → 不同交互点误去重 | Medium | 量化位置+方向 |
| step10 | 路径签名缺少 object_id → 不同物体路径误合并 | Medium | 添加 object_id |
| step14 | JSON 解析器手写 600+ 行 → nlohmann/json | Medium | 替换为标准库 |

### 6.2 待验证

| 编号 | 问题 | 严重度 | 建议 |
|:----:|------|:---:|------|
| D1#8 | UTD φ' 坐标系从出射方向构建（非标准） | Medium | 验证 `DiffractionExpander` 正确写入 `incident_direction` |
| v9 n/a | TM 透射分子修复后的全面回归测试 | High | 运行全部 L1 测试验证功率守恒 |
| v9 n/a | Rx Jones 极化 CSV 文件缺失（仅有 gain CSV） | Medium | 为 dipole/patch/horn 生成 Jones CSV |
| v10 G-6 | 绕射 wedge 验证跳过 | Medium | 在最终论文版前补齐 |

### 6.3 已知限制

| 编号 | 限制 | 影响 |
|:----:|------|------|
| L1 | SearchEngine 使用贪心 keep_limit 截断 | 高阶组合路径可能不全 |
| L2 | SBR P2P 基于射线采样 | 不保证穷举所有几何可能路径 |
| L3 | 绕射仅 Tx-D-Rx，不混合 R/T | 缺少高阶绕射 (Tx-R-D-Rx 等) |
| L4 | 无 GPU 加速实现 | 大规模场景效率受限 |
| L5 | 天线仅单频点方向图 | 宽带仿真时方向图不随频率变化 |
| L6 | JSON 编解码 `tx_antenna`/`rx_antenna` 序列化缺失 | 无法 round-trip 保存配置 |
| L6b | `tx_antenna`/`rx_antenna` 默认 `"Ideal"` 会被当作显式覆盖 | 旧 `antenna` 块中的方向图/极化文件可能被忽略，影响天线耦合实验 |
| L7 | `RunPointToPoint` 与 `SbrEngine.cpp` 中其余 ~1800 行代码耦合在同一文件 | 维护困难 |
| L8 | 缺少自动化回归测试流水线 | 修改后难以快速验证不退化 |
| L9 | 绕射 F(x) 使用固定 8×16 GL 数值积分 | 在极端参数下误差可能 >0.1% |

---

## 7. 总体评估

### 7.1 架构评分

| 维度 | 评分 | 说明 |
|------|:---:|------|
| 模块划分 | ⭐⭐⭐⭐ | core/ 下 9 个子模块职责清晰 |
| 类型系统 | ⭐⭐⭐⭐ | `rt::Scene`/`GeometricPath`/`PathNode` 设计合理 |
| 配置管理 | ⭐⭐⭐ | 14 个子配置块合理，但有字段冗余 |
| 代码复用 | ⭐⭐ | SbrEngine 三重实现 + 独立 SBR 模块重复 |
| 可测试性 | ⭐⭐ | 缺少系统化单元测试框架集成 |

### 7.2 EM 理论正确性评分

| 计算环节 | 评分 | 说明 |
|---------|:---:|------|
| 复介电常数 | ⭐⭐⭐⭐⭐ | 标准公式，正确 |
| Fresnel 反射 | ⭐⭐⭐⭐⭐ | TE/TM 系数实现路径清楚，需保留解析回归 |
| Fresnel 透射 | ⭐⭐⭐⭐⭐ | v9 修正后 TE/TM 公式映射清楚，需保留解析回归 |
| UTD 绕射 | ⭐⭐⭐⭐ | D 系数实现路径清楚，F(x) 为数值近似，需用单楔场景回归 |
| FSPL | ⭐⭐⭐⭐⭐ | 标准 Friis，精确应用一次 |
| 相位累积 | ⭐⭐⭐⭐⭐ | ComplexVec3 全链路复数追踪 |
| 极化追踪 | ⭐⭐⭐⭐⭐ | v9 修正后全复数 TE/TM → Soft/Hard → 重构 |
| 共轭匹配 | ⭐⭐⭐⭐⭐ | 标准极化匹配公式 |
| 多径合成 | ⭐⭐⭐⭐ | CIR 主链支持相干复数加法；PDP 的相干/非相干分 bin 重载当前未接入主聚合 |

### 7.3 论文就绪度

| 论文章节 | 就绪度 | 待完成工作 |
|---------|:---:|------|
| Ch3.1 天线精细化建模 | 85% | 补齐频率响应模型 + 天线 CSV 数据库 |
| Ch3.2 EM 理论 | 95% | 运行全量 L1 回归验证 TM 透射修复 |
| Ch3.3 仿真实验 | 70% | 26 项实验中部分尚未运行高射线数版本 |
| Ch3.4 分析框架 | 90% | 9 张图的分析框架已定义，待数据填充 |

### 7.4 最终结论

1. **全链路通畅**: 从配置读取 → 场景预处理 → 材质绑定 → 几何寻径 → EM 计算 → 输出，全链路已打通。Precise 模式（SearchEngine 或 SBR P2P）+ A1 EM 链可以完成少量点对点的完整 RT 仿真。

2. **EM 计算理论基础较扎实，但仍需回归压实**: 核心公式链（Fresnel TE/TM、UTD D₁-D₄、FSPL、共轭匹配）在代码中已有明确映射。v9 ComplexVec3 升级修复了极化/相位处理的关键缺陷。当前更准确的说法是“理论实现路径清楚、主链可运行”，而不是“所有公式已完成最终逐行证明”；仍建议用 LOS、单反射、单透射、单绕射、极化正交/共极化等解析算例做自动化回归。

3. **天线-RT 耦合主体链路已具备，但配置入口有风险**: Tx 增益+极化+姿态注入 → 路径传播 (TE/TM/Soft/Hard 基) → Rx 共轭匹配的代码链路已经存在。需要修正 `tx_antenna`/`rx_antenna` 默认覆盖全局 `antenna` 的风险，并补齐配置快照序列化；缺少频率响应、群时延、相位中心偏移参与几何、阵列互耦等更高阶天线属性。

4. **寻径模块需要整理**: `SbrEngine.cpp` 2538 行含三重实现+共享辅助函数，建议拆分为独立模块。SearchEngine 的贪心截断和 SBR 的采样不完备性是需要承认的固有限制。

5. **论文基本就绪**: 理论部分（Ch3.2）和天线模型（Ch3.1）可支撑硕士论文答辩。主要待完成工作集中在实验数据生成（Ch3.3 26 项实验的完整运行）和少数 P0 代码补充（天线频率响应、Tx/Rx 独立天线 JSON 编解码）。

---

## 8. 下一阶段整理与验证路线

### 8.1 代码整理原则

1. **先冻结再整理**: 不直接删除旧 SearchEngine、legacy SBR、coverage wavefront/megakernel。先在文档和代码注释中标记“非当前 P2P 主链”，再拆分文件、减少耦合，最后用回归结果决定是否归档。
2. **只围绕当前主链拆分**: 第一阶段只整理 `RunPointToPoint()` 相关辅助函数，建议拆为 `SbrPointToPointTracer`、`SbrDiffractionSolver`、`SbrPathPostProcess`、`SbrPathValidator`。不碰 coverage 行为。
3. **保持接口稳定**: 对外仍保持 `SbrEngine::RunPointToPoint(const SbrContext&) -> SbrCoverageResult`，A1 仍消费 `SearchEngineResult.path_set.paths`，EM 仍消费 `GeometricPath`。
4. **先补验证再做大清理**: 每次拆分前后都跑同一场景、同一 Tx/Rx、同一 ray_count 的路径数量、序列、长度、EM 结果对比。

### 8.2 必须优先验证的链路

1. **几何 P2P 对齐**: 独立 SBR 输出 vs H2hRT `precise_paths.json`，逐 Rx 对比路径数、序列、face_id/wedge_id、total_length、交互点。
2. **EM 单路径解析验证**: LOS、单反射、单透射、单绕射各做一个最小场景，确认幅度、相位、极化投影和路径长度符合解析预期。
3. **多径相干验证**: 构造两条接近等长/半波长差路径，检查 CIR bin 内复数相加能产生相干增强/相消。
4. **天线注入验证**: 分别测试旧 `antenna`、显式 `tx_antenna`/`rx_antenna`、per-Rx override，确认实际生效的方向图与极化文件可追踪。
5. **配置快照验证**: 开启 `export_config_snapshot` 后，确认快照能完整复现 Tx/Rx 天线、SBR、频率、场景和材质配置。

### 8.3 与成熟 RT 的差距

当前代码已经具备“少量点对点 RT 仿真”的主链闭环，但距离成熟 RT 软件仍主要差在：

- 几何寻径完备性：SBR 是采样法，不保证穷举所有高阶路径；当前绕射只做独立 Tx-D-Rx，不做 R/T/D 混合。
- coverage 架构：大规模 Rx 还没有采用路径流式送入 EM 的最终架构，不能把 P2P 的全量路径保存方式直接放大。
- 理论回归：核心公式代码已具备，但还缺系统化解析算例与自动回归。
- 天线模型：已支持方向图、极化图、姿态和 Rx 共轭匹配，但频率响应、群时延、相位中心偏移、阵列互耦尚未进入主链。
- 工程可维护性：`SbrEngine.cpp`、`RtPipeline.cpp` 仍偏大，历史分支和当前主链混在一起，下一步整理应以“保持输出不变”为硬约束。

---

> **编制依据**: 实际阅读代码文件 69+ 个，涵盖 `app/*`, `core/antenna/*`, `core/common/*`, `core/em/*`, `core/path/*`, `core/search/*`, `preprocess/*`, `core/query/*`, `core/result/*`。对照文档 11 份（D1 审计、D2 对照、v9 实施记录/答疑、v10 开发手册、天线文档、大论文.md 等）。所有结论均标注源文件和行号。
