# RT算法正式开发文档 v1

## 0. 文档定位与使用方式

本文档是当前 RT 新系统开发的**正式开发文档 v1**。其目的不是再做开放式讨论，而是作为：

1. 后续任意新对话中 AI 编程工具理解本项目的统一入口；
2. 人工开发与 AI 协同开发的冻结依据；
3. 开发阶段拆分、测试闭环、调试方式、输出验证方式的统一标准；
4. 项目结构、模块职责、输入输出、性能路线、精度路线的可执行方案。

本文档应被视为当前阶段的：

> **单一事实来源（Single Source of Truth）**

后续若继续开发、修订、优化，应优先基于本文档进行，不应再从零散对话中重新提炼结构。

### 0.1 本文档与《新RT算法总体设计方案与开发手册草案》的关系

当前项目采用：

> **主开发文档 + 详细设计草案字典**

的双文档体系。

具体关系如下：

#### A. 《RT算法正式开发文档 v1》

定位为：

1. 正式开发主文档；
2. 模块冻结边界说明；
3. 开发批次与闭环验证标准；
4. AI 编程工具在新对话中的主入口。

#### B. 《新RT算法总体设计方案与开发手册草案》

定位为：

1. 详细设计字典；
2. 字段级解释库；
3. 讨论沉淀与理论背景库；
4. 当正式文档需要进一步下钻细节时的权威参考。

### 0.2 文档使用优先级（正式冻结）

后续开发时，必须遵循以下优先级：

1. **先读《RT算法正式开发文档 v1》**
2. 若需要具体字段级、流程级、公式级细节，再查《新RT算法总体设计方案与开发手册草案》
3. 若两份文档表述看似不同：
   - 以《RT算法正式开发文档 v1》的模块边界、开发顺序、冻结项为准；
   - 以草案中的详细字段解释、推导过程、设计背景作为补充说明；
4. 不允许 AI 编程工具仅凭记忆或单条对话片段自行发明平行结构。

### 0.3 AI 编程工具使用规范（新对话适用）

在任意新对话中，如果要继续本项目开发，建议 AI 严格按以下步骤工作：

#### Step 1：先读取主文档

必须先读：

- `document/RT算法正式开发文档 v1.md`

理解：

1. 项目目标
2. 模块边界
3. 当前开发状态
4. 开发批次
5. 测试闭环要求

#### Step 2：定位当前开发模块

判断当前任务属于：

- 模块1
- 模块2
- 模块3
- 模块4
- 模块5
- 模块6

并只在当前模块边界内展开，不允许跨模块乱改结构。

#### Step 3：查草案对应章节

若需要详细字段、流程、理论细节，则继续读：

- `document/新RT算法总体设计方案与开发手册草案.md`

对应模块章节。

#### Step 4：按批次开发，不跳步

先确认当前属于哪个批次，再按该批次：

1. 开发文件范围
2. 闭环目标
3. 检测方式
4. 通过标准

执行。

#### Step 5：开发完成后必须输出闭环说明

每批次开发完成后，应明确说明：

1. 已修改/新增文件
2. 当前完成到哪一批的哪一点
3. 如何验证
4. 尚未完成什么
5. 是否影响后续模块

这一步是后续交接给新的 AI 会话时最关键的信息来源之一。

---

---

## 1. 项目总目标

本项目目标不是简单复刻已有成熟 RT 实现，也不是只做一个可运行 demo，而是：

> **构建一套面向开题目标、支持高保真室内/复杂场景多机制传播建模的新 RT 算法系统。**

必须满足以下总体目标：

1. 支持 **LOS / Reflection / Transmission / Diffraction** 的统一几何寻径；
2. 支持模块化、可扩展、可调试、可验证的工程架构；
3. 支持路径级严格电磁求解，而非仅给出粗略经验损耗；
4. 支持两类典型任务：
   - `Precise`：点对点/少量 Tx-Rx 高保真路径分析
   - `Coverage`：大规模覆盖仿真/大量虚拟 Rx
5. 输出满足报告需求的结果：
   - 路径级几何结果
   - 复电场/功率/相位/极化
   - CIR / PDP / APS
   - 大小尺度统计
   - Coverage 结果
   - ISAC 基础特征输出
6. 能够在后续任何时间点，被新的 AI 编程会话直接接续开发。

---

## 2. 当前开发阶段结论（必须优先阅读）

### 2.1 当前已冻结模块

以下模块已达到“**核心结构可冻结**”状态：

- 模块1：基础设施与配置
- 模块2：场景导入、预处理、加速结构、查询门面
- 模块3：天线接口结构（第一版可简化实现，但接口冻结）
- 模块4：几何寻径核心架构、状态机、扩展器主线、去重与模式化路线
- 模块5：路径级电磁求解主线、PreciseEM / CoverageEM、输出层对象
- 模块6：结果输出、可视化、验证与回归闭环

### 2.2 当前未完全收敛的部分

虽然主结构已冻结，但以下内容仍可在实现阶段继续细化，不构成结构性返工：

1. 具体公式实现细节；
2. 参数默认值与 profile 数值；
3. JSON/CSV schema 的字段顺序与命名微调；
4. 可视化脚本格式细节；
5. 第二阶段性能增强（GPU、复杂加速结构等）。

### 2.3 当前推荐开发路线

正式推荐开发顺序：

1. 模块1
2. 模块2
3. 模块4
4. 模块5
5. 模块6
6. 模块3补强/完善

这样安排的原因是：

- 模块1/2 是底座；
- 模块4/5 是核心；
- 模块6 是闭环；
- 模块3 第一版接口已冻结，可先简化，不应阻塞主链。

---

## 3. 开发总注意事项、铁律与协同开发规范

本节是整个项目的**开发铁律**。任何后续 AI 开发都应遵守。

### 3.1 架构铁律

1. **禁止跨模块职责污染**
   - 模块2不负责路径成立性判断
   - 模块4不负责场景导入与 BVH 构建
   - 模块5不负责几何求交与路径重建
   - 模块6不负责重新计算物理量

2. **禁止模块5重建几何语义**
   - 模块4给模块5的 `GeometricPath` 必须已经是几何真实、顺序明确、介质侧明确的路径对象

3. **禁止模块4依赖底层 BVH 实现细节**
   - 模块4只能通过 `SceneQuery` 查询，不直接操作 BVH 节点

4. **禁止模块1和模块2的大结构回退**
   - 允许微调字段名、实现细节
   - 不允许改变其职责边界与主架构

5. **禁止把 coverage 模式的工程近似污染 precise 模式主链**
   - `Precise` 与 `Coverage` 共享架构，但参数与求值模式不同

### 3.2 正确性铁律

1. **保留下来的路径必须几何真实**
2. **双侧材质与 transmission 介质链必须显式传递**
3. **候选发现不等于路径成立**
   - 尤其是 diffraction，`WedgeCandidate` 只是候选，不是已成立路径
4. **先几何正确，再物理正确，再做性能优化**
5. **任何优化不得破坏已冻结的结果契约**

### 3.3 工程铁律

1. **所有配置统一进入 `AppConfig`**
2. **所有日志统一经 `Logger` 输出**
3. **所有错误统一使用 `RtError / ErrorCode`**
4. **所有数值容差统一使用 `NumericToleranceConfig`**
5. **任何场景预处理结果必须可缓存、可回放、可追溯**
6. **任何模块都必须可单独测试，不依赖整系统才可运行**

### 3.4 AI 协同开发建议（开发习惯 / skill 约束）

建议后续 AI 编程工具在任意新对话中，遵循以下工程性“skill”风格：

#### Skill A：Small validated increments

每次开发只推进一个明确闭环，不做跨多个模块的大爆改。

#### Skill B：Single source of truth

优先阅读并遵守本文档，不自行发明平行结构。

#### Skill C：Read contract before code

先阅读：
- 当前模块职责
- 输入输出结构
- 上下游契约
- 测试与验证要求

再写代码。

#### Skill D：No silent fallback

对 transmission、材质映射、路径成立等关键链路，禁止静默回退默认值而不告警。

#### Skill E：Prefer explicit state

避免隐式状态和全局副作用，优先显式状态对象和结构化返回。

#### Skill F：Debuggability first

每个核心函数都应有足够明确的失败原因或中间输出，不要为了“简洁”牺牲可调试性。

#### Skill G：Comment contract first

后续所有正式开发代码应遵守以下注释规范：

1. **每个源码/头文件开头必须用注释明确本文件目标与主要功能**；
2. **关键实现必须补充足够详细的解释性注释**，说明其存在原因、边界和当前批次限制；
3. **所有核心函数必须使用标准 `/// <summary>` 风格注释**，明确：
   - 功能；
   - 入参；
   - 返回值；
   - 必要时补充失败语义与使用约束；
4. 若文件格式不适合逐行注释（如 JSON、CSV、部分二进制/导出 schema 文件），则必须在**同级目录**创建说明文档，解释：
   - 文件用途；
   - 关键字段；
   - 使用方式；
   - 当前批次限制。

#### Skill I：Test scripts under `test/`

后续所有面向人工核查、快速验证、可视化检查的 Python 脚本，应统一放入项目根目录下的 `test/` 目录，并遵守：

1. `test/` 目录用于存放：
   - 人工核查脚本；
   - 批次级快速验证脚本；
   - GUI/可视化检查工具；
   - 对应环境说明与依赖文件；
2. 若脚本主要面向 **Anaconda + PyCharm** 使用，则应优先采用：
   - 脚本内集中配置参数；
   - 尽量少依赖命令行参数；
   - 提供同级 `README.md` 或环境说明文件；
3. 若存在 GUI/三维交互检查工具，应明确其：
   - 用途；
   - 适用批次；
   - 是否属于主链闭环阻塞项；
4. 检测脚本属于**辅助验证工具**，不应反向污染主链模块职责边界。

#### Skill H：Batch closure before integration push

后续协同开发时，代码进入远端集成前必须满足：

1. **至少完成一个开发文档中定义的完整批次，或该批次形成明确闭环**；
2. **已完成相应编译/测试/运行验证**；
3. **已向用户汇报：修改文件、验证方式、遗留问题与下一步建议**；
4. 只有在**用户明确要求执行 git 提交/推送**时，才进行实际远端集成操作；
5. 未完成批次闭环、未通过验证或仍有关键失败项时，**禁止作为“完成开发结果”推送远端**。

---

## 4. 项目正式目录与文件结构建议

建议在新实现阶段采用如下项目结构：

```text
RTNew/
├── app/
│   ├── main.cpp
│   ├── AppBootstrap.cpp
│   └── RtPipeline.cpp
├── core/
│   ├── common/
│   │   ├── config/
│   │   ├── error/
│   │   ├── log/
│   │   ├── numeric/
│   │   ├── runtime/
│   │   └── version/
│   ├── scene/
│   ├── geometry/
│   ├── material/
│   ├── antenna/
│   ├── query/
│   ├── path/
│   ├── search/
│   ├── em/
│   ├── result/
│   └── validation/
├── preprocess/
│   ├── import/
│   ├── build/
│   ├── accel/
│   ├── binding/
│   ├── diagnostics/
│   └── cache/
├── io/
│   ├── json/
│   ├── csv/
│   ├── binary/
│   └── export/
├── tests/
│   ├── unit/
│   ├── integration/
│   ├── regression/
│   └── scenes/
├── configs/
│   ├── app/
│   ├── profiles/
│   └── scenes/
└── tools/
    ├── visualization/
    ├── regression/
    └── analysis/
```

### 4.1 说明

- `app/`：程序启动与主流水线组织
- `core/common/`：模块1
- `preprocess/`：模块2的场景预处理主实现
- `core/path/` + `core/search/`：模块4
- `core/em/`：模块5
- `core/result/` + `io/export/`：模块6
- `tests/`：分层测试
- `tools/`：辅助脚本与可视化工具

### 4.2 关键文档文件的正式角色

项目当前关键文档建议固定如下：

```text
document/
├── RT算法正式开发文档 v1.md
├── 新RT算法总体设计方案与开发手册草案.md
├── 批注.txt
├── RT算法成熟源码详细分析文档.md
├── RT算法代码全量分析报告.md
├── RT算法_开题需求对照与不足全量报告.md
└── BVH/
```

#### 角色说明

- `RT算法正式开发文档 v1.md`：开发主文档
- `新RT算法总体设计方案与开发手册草案.md`：详细设计字典
- `批注.txt`：阶段性人工修订意见来源
- `RT算法成熟源码详细分析文档.md`：成熟实现拆解说明
- `RT算法代码全量分析报告.md`：成熟代码全景分析
- `RT算法_开题需求对照与不足全量报告.md`：需求差距分析
- `BVH/`：加速结构参考实现分析材料

---

## 5. 模块冻结摘要（面向新会话快速理解）

### 5.1 模块1：基础设施与配置（已冻结）

#### 作用

- 统一配置、日志、错误码、容差、版本管理

#### 核心对象

- `AppConfig`
- `ConfigValidationResult`
- `RtError`
- `Logger`
- `NumericToleranceConfig`
- `VersionInfo`

#### 正式输入输出

- 输入：JSON 配置文件
- 输出：已校验配置对象与基础设施服务对象

#### 重点铁律

- 所有配置必须进入 `AppConfig`
- 所有输出必须通过 `Logger`

### 5.2 模块2：场景导入、预处理、查询与缓存（已冻结）

#### 作用

- Blender/OBJ/STL 场景导入
- 对象语义恢复
- 双侧材质解析
- Face / Edge / Wedge 构建
- Face BVH / WedgeAcceleration
- SceneQuery
- SceneCache

#### 核心对象

- `Scene`
- `Face`
- `Edge`
- `Wedge`
- `SceneMaterialBinding`
- `SceneDiagnostics`
- `SceneAcceleration`
- `FaceQueryRecord`
- `WedgeQueryRecord`
- `SceneQuery`
- `SceneCacheMeta/Content`

#### 正式主线

- Face BVH 归模块2构建
- 不采用单棵混合 BVH 作为第一版主线
- 双侧材质通过规则文件 + 自动解析生成

### 5.3 模块3：天线建模接口（接口冻结，实现可简化）

#### 作用

- 发射端/接收端天线模型接口
- 方向图、极化、姿态、外部天线文件预留

#### 当前策略

- 第一版实现可简化
- 接口必须完整

### 5.4 模块4：几何寻径核心（核心结构冻结）

#### 作用

- `SBR 主干 + 统一状态机`
- LOS / Reflection / Transmission / Diffraction
- 剪枝、去重、mixed path 控制

#### 核心对象

- `InteractionType`
- `PathNode`
- `GeometricPath`
- `PathState`
- `PathSearchContext`
- `SearchEngine`
- `StateSignature / PathSignature`

#### 关键原则

- 以射线发射为主干
- 以状态机组织路径扩展
- `FaceHit` 与 `WedgeCandidate` 语义分离
- 支持 `Precise / Coverage`

### 5.5 模块5：路径级电磁求解（核心结构冻结）

#### 作用

- 基于 `GeometricPath` 做路径级电磁求值
- 输出 `EMPathResult`
- 汇总 CIR/PDP/APS/统计量/Coverage/ISAC 基础特征

#### 核心对象

- `EMSolverInput`
- `FieldAccumulator`
- `EMPathResult`
- `EMSolverOutput`
- `CIRResult`
- `PDPResult`
- `APSResult`
- `ChannelStatistics`
- `CoverageResult`
- `ISACFeatureSet`

#### 关键原则

- `PreciseEM / CoverageEM` 双模式
- GO + Fresnel + UTD + 路径级极化演化主线

### 5.6 模块6：结果输出与验证闭环（结构冻结）

#### 作用

- 文件导出
- 可视化辅助输出
- Validation / Regression 报告
- 结果目录结构组织

#### 核心对象

- `ExportBundle`
- `ValidationReport`
- `RegressionReport`

#### 关键原则

- 模块6只负责“结果表达”，不重新算几何或电磁

### 5.7 模块3~6冻结状态总表（正式版）

| 模块 | 当前状态 | 已冻结内容 | 仍可细化内容 | 是否可进入开发 |
|---|---|---|---|---|
| 模块3 | 接口冻结，实现可简化 | 天线模型接口、默认值策略、姿态/方向图/极化/外部文件预留 | 外部仿真文件解析、阵列级复杂行为 | 是 |
| 模块4 | 核心结构冻结 | SBR 主干 + 统一状态机、PathState、PathNode、GeometricPath、SearchEngine、四类交互主线、Precise/Coverage、去重体系 | 局部几何构造细节、profile 数值、coverage 高级策略 | 是 |
| 模块5 | 主链冻结 | GO + Fresnel + UTD + 极化演化、FieldAccumulator、EMPathResult、PreciseEM/CoverageEM、输出层结构 | 具体公式实现、极化更新细节、统计派生细节 | 是 |
| 模块6 | 结构冻结 | ExportBundle、Validation/Regression、目录结构、文件格式分工、可视化辅助输出层 | schema 细节、模板样式、可视化格式细节 | 是 |

该表的意义是：

> 让任意新会话中的 AI 直接判断：当前哪些模块可以进入代码实现，哪些只需继续补细节而不应回退结构。

---

## 5A. 草案索引映射（面向详细设计查询）

当主文档中的某模块需要进一步细化时，建议到草案中查阅对应主题。以下映射是为了避免 AI 工具在 300KB 草案中盲找。

### 5A.1 模块1 细节索引

到草案中优先查：

- 模块1详细开发方案
- 配置系统设计
- 错误码体系
- 日志系统
- 数值容差与公共常量设计
- 版本号与缓存版本管理

### 5A.2 模块2 细节索引

到草案中优先查：

- 建模规范
- 双侧材质解析
- Scene / Face / Edge / Wedge 字段级设计
- SceneAcceleration / SceneQuery
- WedgeQueryConfig
- SceneCache

### 5A.3 模块3 细节索引

到草案中优先查：

- 天线接口结构
- 默认值策略
- 外部仿真接口预留

### 5A.4 模块4 细节索引

到草案中优先查：

- 模块4方法路线
- SearchEngine
- 四类交互合法性检查
- BuildReflection / BuildTransmission / BuildDiffractionState
- Precise / Coverage profile
- StateSignature / PathSignature

### 5A.5 模块5 细节索引

到草案中优先查：

- 模块5五层结构
- FieldAccumulator
- 三类交互函数设计
- PreciseEM / CoverageEM
- 输出层结构

### 5A.6 模块6 细节索引

到草案中优先查：

- ExportBundle
- ValidationReport / RegressionReport
- 目录结构
- 可视化辅助输出

### 5A.7 使用方式说明

AI 在新对话中应先看主文档的冻结表和批次计划，再去草案对应模块章节查：

1. 字段定义
2. 失败原因
3. 公式与背景
4. 讨论中的细节边界

这意味着：

> 主文档负责“告诉 AI 做什么、按什么顺序做”；草案负责“告诉 AI 细节为什么这么做、字段是什么意思、哪些地方容易错”。

---

## 6. 分批开发总流程（正式执行方案）

以下流程按“每批次形成明确闭环”的原则组织。每一批都必须有：

1. 实现文件范围
2. 闭环目标
3. 检测方式
4. 通过标准

---

## 7. 批次0：项目骨架与公共规则落地

### 7.1 开发文件

建议先创建：

- `app/main.cpp`
- `app/RtPipeline.cpp`
- `core/common/config/AppConfig.h`
- `core/common/error/RtError.h`
- `core/common/log/Logger.h`
- `core/common/numeric/NumericToleranceConfig.h`
- `core/common/version/VersionInfo.h`

### 7.2 闭环目标

1. 新工程可独立编译启动
2. 可加载空/最小 JSON 配置
3. 可输出统一日志
4. 可输出版本信息与配置快照

### 7.3 检测方式

- 启动程序
- 读取测试 JSON
- 输出日志到控制台和文件
- 观察配置校验输出

### 7.4 通过标准

- 程序启动成功
- 配置可正确加载和校验
- 日志格式符合统一要求

---

## 8. 批次1：模块1正式落地

### 8.1 开发文件

- `AppConfigLoader.cpp`
- `AppConfigValidator.cpp`
- `Logger.cpp`
- `RtError.cpp`
- `NumericToleranceConfig.cpp`
- `VersionInfo.cpp`

### 8.2 闭环目标

1. 完整 `AppConfig` 可加载
2. 分层配置校验可运行
3. 错误码与日志系统稳定
4. 版本号与容差配置可输出

### 8.3 检测方式

- 构造正常配置和错误配置
- 检查 `ConfigValidationResult`
- 检查 `RtError` 输出
- 检查 `Logger` 文件与控制台输出

### 8.4 通过标准

- 模块1可单独测试通过
- 错误输入能被前置拦截

---

## 9. 批次2：模块2场景导入与语义恢复主线

### 9.1 开发文件

- `SceneMeta.h`
- `Face.h`
- `Edge.h`
- `Wedge.h`
- `Scene.h`
- `OBJImporter.cpp`
- `MaterialRuleLoader.cpp`
- `ResolveFaceDualSideMaterial.cpp`
- `SceneMaterialBinding.cpp`

### 9.2 闭环目标

1. 成功读取 `demo/meeting.txt`（OBJ 内容）
2. 恢复对象命名与语义
3. 根据 `scene_material_map.json` 生成双侧材质
4. 形成完整 `Scene` 基础语义层

### 9.3 检测方式

- 输出 `SceneMeta`
- 输出对象级绑定记录
- 输出面元 front/back 材质摘要
- 对部分对象人工核查

### 9.4 通过标准

- `Scene` 可成功构建
- transmission 相关面均已得到可信双侧材质结果

### 9.5 当前实际完成情况（本轮代码开发同步）

截至当前阶段，批次2已完成第一版正式闭环，具体包括：

#### 已落地对象与过程

1. `SceneMeta`
   - 已输出对象数、顶点数、法向数、面元数等基础统计；

2. `Face / Scene / SceneMaterialBinding`
   - 已形成模块2批次2所需的 Scene 基础语义层；
   - `Edge / Wedge` 当前仅作为批次3稳定占位结构保留；

3. `OBJImporter`
   - 已可读取 `demo/meeting.txt` 中的 OBJ 文本；
   - 已恢复对象块、顶点、法向和三角面；

4. `MaterialRuleLoader`
   - 已可读取 `configs/scenes/scene_material_map.json`；
   - 已恢复默认介质与对象级材质规则；

5. `ResolveFaceDualSideMaterial`
   - 已根据对象名、对象类型和规则文件生成面元级 `front/back` 材质结果；
   - 已显式写入 transmission / diffraction 候选相关语义开关；

6. `SceneBatch2Builder`
   - 已串联：OBJ 导入 → 规则加载 → 对象级绑定 → Scene 基础语义层输出；

7. `SceneBatch2Reporter`
   - 已输出 `SceneMeta`、对象级绑定摘要和面元级 `front/back` 材质摘要。

#### 当前实际验证结果

已完成并通过：

1. VS2022 / MSBuild 编译通过；
2. 正常配置运行通过；
3. `demo/meeting.txt` 成功解析；
4. `scene_material_map.json` 成功加载；
5. 对象级语义绑定成功；
6. 面元双侧材质恢复成功；
7. 日志中已出现：
   - `Batch2 scene import and semantic recovery closed loop completed.`

#### 当前辅助检测工具状态

为便于人工核查，本轮已在项目根目录新增：

- `test/`

当前其中已包含：

1. `test/check_batch2_binding.py`
   - 面向批次2的 GUI / 可视化人工核查工具；
   - 面向 **Anaconda + PyCharm** 使用；
   - 采用脚本内集中配置参数方式；
2. `test/README.md`
3. `test/environment.yml`
4. `test/setup_conda_env.ps1`

注意：

> 该检测脚本属于**辅助验证工具**，当前即使 GUI 环境仍在调试中，也不影响批次2主链闭环已完成这一结论。

#### 当前结论

一句话结论：

> **批次2已完成“OBJ 导入 + 对象语义恢复 + 双侧材质解析 + Scene 基础语义层”的正式闭环。**

### 9.6 当前仍可继续优化但不阻塞批次3的内容

1. 规则文件当前以 `object_name` 精确匹配为主，后续可增强为更通用的 `name_pattern` 匹配；
2. GUI 检测工具当前可作为辅助核查入口，后续仍可继续增强图层显示、对象点击高亮、CSV 导出等能力；
3. 源码文件编码统一为 UTF-8 with BOM 后，可进一步减少 `C4819` 注释编码警告；
4. `SceneDiagnostics` / `Edge` / `Wedge` / `BVH` / `SceneQuery` / `SceneCache` 仍留待批次3/4继续完成，不属于批次2未完成项。

---

## 10. 批次3：模块2拓扑、诊断与加速结构

### 10.1 开发文件

- `EdgeBuilder.cpp`
- `WedgeBuilder.cpp`
- `SceneDiagnostics.cpp`
- `FaceBVHBuilder.cpp`
- `WedgeAccelerationBuilder.cpp`
- `SceneAcceleration.cpp`

### 10.2 闭环目标

1. 完成 Face / Edge / Wedge 构建
2. 生成 `SceneDiagnostics`
3. 建立 Face BVH
4. 建立第一版轻量 `WedgeAcceleration`

### 10.3 检测方式

- 输出：
  - 面元数
  - 棱边数
  - 楔边数
  - 非流形边数量
  - 双侧材质未解析面数量
- 输出 BVH 节点统计
- 抽样 brute-force vs BVH 一致性检查

### 10.4 通过标准

- SceneDiagnostics 合理
- Face BVH 查询与 brute-force 抽样一致
- Wedge 候选集可用

---

## 11. 批次4：模块2查询门面与缓存闭环

### 11.1 开发文件

- `SceneQuery.h/.cpp`
- `FaceQueryContext` / `VisibilityQueryContext` / `WedgeQueryContext`
- `SceneCacheMeta.h/.cpp`
- `SceneCacheContent.h/.cpp`
- `SceneCache.cpp`

### 11.2 闭环目标

1. `QueryClosestFaceHit / QueryAllFaceHits / IsVisible / QueryCandidateWedges` 可用
2. `SceneCache` 可生成并命中
3. debug / production preprocess mode 可区分

### 11.3 检测方式

- 小场景查询单元测试
- SceneCache 首次构建与二次加载对比
- 输出 cache 命中/失效日志
- 输出 query trace 调试信息

### 11.4 通过标准

- 模块2已形成可独立调用的完整预处理与查询闭环

### 11.5 当前实际完成情况（本轮代码开发同步）

截至当前阶段，批次4已完成正式闭环，具体包括：

#### 已落地对象与过程

1. `SceneQuery`
   - 已形成模块2对模块4暴露的统一查询门面；
   - 已可提供：
     - `QueryClosestFaceHit`
     - `QueryAllFaceHits`
     - `QueryFaceHitsInRange`
     - `IsOccluded`
     - `IsVisible`
     - `QueryCandidateWedges`

2. `FaceQueryContext / VisibilityQueryContext / WedgeQueryContext`
   - 已形成第一版局部查询约束对象；
   - 已显式承载忽略面元、起点自碰撞抑制、可见性偏移、楔边候选过滤等基础控制能力；

3. `SceneCacheMeta / SceneCacheContent / SceneCache`
   - 已形成第一版缓存元信息与缓存内容结构；
   - 已支持 SceneCache 首次构建与二次命中；
   - 已支持基于源文件、规则文件、材料库、预处理配置与算法版本的 cache 有效性判断；

4. debug / production preprocess mode
   - 已进入 `AppConfig.scene_preprocess`；
   - 已通过 `preprocess_mode` 与 cache meta 中的 debug 标志进行区分；

5. `SceneBatch4Reporter`
   - 已输出 cache 命中/失效摘要；
   - 已输出 query self-check 调试信息；

#### 当前实际验证结果

已完成并通过：

1. VS2022 / MSBuild 编译通过；
2. cache miss 运行通过；
3. cache hit 运行通过；
4. query self-check 通过；
5. 日志中已出现：
   - `Batch4 query facade and scene cache closed loop completed.`

#### 当前结论

一句话结论：

> **批次4已完成“SceneQuery 可用 + SceneCache 可生成并命中 + preprocess mode 可区分”的正式闭环。模块2已形成可独立调用的完整预处理与查询底座，可进入批次5。**

### 11.6 当前仍可继续优化但不阻塞批次5的内容

1. `closest hit` 当前通过 `all hits` 结果取最近项，后续仍可优化为专用最近命中遍历；
2. Face BVH 当前采用中位分裂主线，后续仍可增强为更成熟的 SAH/桶式分裂；
3. SceneCache 当前为第一版工程内二进制格式，后续仍可增强兼容性与校验粒度；
4. `SceneQuery` 的批量查询、trace 粒度与 transmission 多层边界分析仍可继续增强。

---

## 12. 批次5：模块4核心结构与 SearchEngine 骨架

### 12.1 开发文件

- `InteractionType.h`
- `PathNode.h`
- `GeometricPath.h`
- `PathState.h`
- `PathSearchContext.h`
- `SearchEngine.h/.cpp`
- `StateSignatureBuilder.cpp`
- `PathSignatureBuilder.cpp`

### 12.2 闭环目标

1. 初始状态构造成功
2. DFS 主循环可运行
3. 状态级预检查、状态级去重、路径级去重框架可用
4. 仅 LOS 情况可完成 `Tx -> Rx` 路径搜索

### 12.3 检测方式

- 小型 free-space 场景
- 打印/导出 PathState trace
- 输出状态数、去重数、路径数

### 12.4 通过标准

- SearchEngine 骨架稳定
- 最基础 LOS 闭环成立

---

## 13. 批次6：模块4 Reflection / Transmission / Diffraction 扩展器

### 13.1 开发文件

- `ReflectionExpander.cpp`
- `TransmissionExpander.cpp`
- `DiffractionExpander.cpp`
- `GeometryValidity.cpp`
- `ResolveMediumTransition.cpp`

### 13.2 闭环目标

1. Reflection 可稳定扩展
2. Transmission 可稳定扩展并正确切换介质状态
3. 单次 Diffraction 可稳定扩展
4. mixed path toy scene 可运行

### 13.3 检测方式

- 单反射/单透射/单绕射场景
- mixed path toy scene
- 输出扩展器失败原因统计
- 输出路径可视化 polyline 和命中点 JSON

### 13.4 通过标准

- 模块4能输出几何真实且语义完整的 `GeometricPath`

---

## 14. 批次7：模块5基础物理求值主链

### 14.1 开发文件

- `EMSolverInput.h`
- `FieldAccumulator.h`
- `EMPathResult.h`
- `PreparePathForEM.cpp`
- `InitializeTxField.cpp`
- `ApplyFreeSpaceSegment.cpp`
- `ApplyReflectionInteraction.cpp`
- `ApplyTransmissionInteraction.cpp`
- `ApplyDiffractionInteraction.cpp`
- `FinalizeAtReceiver.cpp`

### 14.2 闭环目标

1. 单路径 LOS 可得到 `EMPathResult`
2. 单反射/单透射/单绕射路径可得到稳定结果
3. `PreciseEM` 主链可运行

### 14.3 检测方式

- 输出路径级 `EMPathResult`
- 检查时延、功率、相位、极化状态是否合理
- 与理论趋势或成熟实现趋势对比

### 14.4 通过标准

- 模块5可独立对单条路径求解并返回可信结果

---

## 15. 批次8：模块5多路径汇总与双模式求值

### 15.1 开发文件

- `BuildCIR.cpp`
- `BuildPDP.cpp`
- `BuildAPS.cpp`
- `BuildChannelStatistics.cpp`
- `BuildCoverageResult.cpp`
- `BuildISACFeatureSet.cpp`
- `PreciseEMProfile.cpp`
- `CoverageEMProfile.cpp`

### 15.2 闭环目标

1. `CIR / PDP / APS / Statistics / Coverage / ISACFeatureSet` 可生成
2. `PreciseEM / CoverageEM` 双 profile 可切换

### 15.3 检测方式

- 比较 precise 与 coverage 的输出差异
- 检查 coverage 功率趋势
- 检查 PDP/APS 合理性

### 15.4 通过标准

- 模块5输出层闭环完成

---

## 16. 批次9：模块6结果表达、验证与回归闭环

### 16.1 开发文件

- `ExportBundle.h/.cpp`
- `ValidationReport.h/.cpp`
- `RegressionReport.h/.cpp`
- `ExportPaths.cpp`
- `ExportChannel.cpp`
- `ExportCoverage.cpp`
- `ExportISAC.cpp`
- `ExportVisualization.cpp`

### 16.2 闭环目标

1. 结构化结果文件完整导出
2. 目录结构稳定
3. 验证报告、回归报告可生成
4. 可视化辅助文件可直接给脚本用

### 16.3 检测方式

- JSON/CSV 文件解析测试
- 导出数量与内存对象一致性检查
- 使用可视化脚本检查路径线和命中点
- regression 报告摘要是否合理

### 16.4 通过标准

- 模块6形成完整结果表达与验证闭环

---

## 17. 模块级 debug 与闭环检查方式总表

### 17.1 模块1 debug 方式

- 配置校验结果
- 错误码输出
- 日志级别切换
- 版本号输出

### 17.2 模块2 debug 方式

- SceneMeta 输出
- SceneDiagnostics 输出
- 对象级材料绑定记录输出
- Face/Wedge 调试 JSON
- BVH vs brute-force 抽样比对

### 17.3 模块4 debug 方式

- 单射线路径 trace
- 扩展器失败原因统计
- 状态级/路径级去重统计
- 路径 polyline / hit points 可视化文件

### 17.4 模块5 debug 方式

- 单路径 `EMPathResult`
- 各交互求值中间量（可选 debug 模式）
- CIR/PDP/APS 摘要
- precise / coverage 对照摘要

### 17.5 模块6 debug 方式

- ExportBundle 内容完整性检查
- 文件导出一致性检查
- Validation/Regression 摘要检查

---

## 18. 当前项目开发状态与建议执行方式

### 18.1 当前状态

根据当前文档收口结果：

1. 模块1：已完成批次0 / 1，已形成正式闭环，可直接作为后续模块底座
2. 模块2：结构已冻结，可正式进入开发
2. 模块3：接口已冻结，可先实现简化版
3. 模块4：核心架构、扩展器主线、双模式和去重体系已冻结，可进入实现
4. 模块5：主链、双 profile、结果结构已冻结，可进入实现
5. 模块6：结构、目录、验证与回归路线已冻结，可进入实现

### 18.2 是否已足够支撑 AI 编程工具继续开发

结论：

> **是。**

当前文档已经足以支持任意新会话中的 AI 编程工具：

1. 理解项目目标；
2. 理解当前已冻结架构；
3. 理解模块边界与接口；
4. 理解具体开发顺序；
5. 理解每一批开发如何闭环验证；
6. 继续后续开发或优化。

### 18.3 后续推荐动作

正式建议后续进入：

1. 对本开发文档做最终规整（章节去重、统一命名、补目录）；
2. 按批次 0~9 顺序正式进入代码实现；
3. 每完成一批，必须补充：
   - 实际已完成文件
   - 已通过测试
   - 遗留问题
   - 下批次风险

### 18.4 本文档的正式用途结论

一句话总结：

> **本文档可以作为《RT算法正式开发文档 v1》的直接工作底稿，并可在任意后续时期提供给新的 AI 编程工具作为从结构到细节、从目标到进展、从模块职责到测试闭环的统一开发依据。**

### 18.5 防误开发补充说明

为了避免 AI 因主文档较草案更精炼而误开发，正式要求：

1. **主文档不是草案替代品，而是开发主导航文档**；
2. **草案不是过时讨论，而是详细设计字典**；
3. 任何 AI 若要开始编码，必须同时满足：
   - 已读取本主文档对应模块章节；
   - 已按主文档提示查阅草案的对应细节索引；
4. 若当前任务涉及：
   - 公式实现
   - 字段级含义
   - 交互细节
   - QueryContext
   - profile 参数
   则必须进一步查阅草案，不得只看主文档直接拍脑袋写代码。

这条要求是为了保证：

> 主文档负责“不会走错方向”，草案负责“不会丢掉关键细节”。

---

## 19. 术语表（统一词汇，避免歧义）

本节用于统一本项目中的核心术语，保证后续人工开发与 AI 开发使用一致词汇。

### 19.1 场景与几何相关术语

#### `Scene`
模块2输出的统一静态场景对象，包含：

1. 几何（点、面）
2. 语义（对象类型、材料绑定）
3. 拓扑（边、楔边）
4. 加速结构（BVH、候选结构）
5. 诊断信息

#### `Face`
场景中的三角面元，是 reflection / transmission 主交互对象。

#### `Edge`
面元之间的唯一拓扑边，是构造 `Wedge` 的基础对象。

#### `Wedge`
由边及其相邻面构成的绕射楔边，是 diffraction 的主几何对象。

#### `SceneAcceleration`
模块2输出的高频查询基础设施集合，至少包含：

1. Face BVH
2. WedgeAcceleration
3. 加速结构诊断信息

#### `SceneQuery`
模块2对模块4暴露的统一几何查询门面，模块4不能直接操作 BVH。

### 19.2 路径与搜索相关术语

#### `SBR`
Shooting and Bouncing Rays，射线发射与弹跳主干。当前新RT模块4保留其发射主线思想。

#### `PathState`
模块4运行时状态对象，不是最终结果，负责描述当前几何扩展状态。

#### `PathNode`
模块4最终输出路径中的单个交互节点，表示一跳 reflection / transmission / diffraction / Tx / Rx。

#### `GeometricPath`
模块4输出给模块5的几何路径对象，是几何真实且语义完整的路径契约。

#### `SearchEngine`
模块4统一调度器，负责 DFS-SBR 状态机搜索。

#### `Expander`
模块4中对某一类传播机制进行状态扩展的组件，如：

- ReflectionExpander
- TransmissionExpander
- DiffractionExpander

#### `StateSignature`
模块4用于状态级去重的签名。

#### `PathSignature`
模块4用于路径级去重的签名。

### 19.3 电磁求解相关术语

#### `FieldAccumulator`
模块5的路径级场状态载体，承载复场分量、累计相位、累计长度、当前介质等。

#### `EMPathResult`
模块5输出的单路径电磁结果对象。

#### `PreciseEM`
模块5中的高保真求值模式，保留复场、相位、极化等细节。

#### `CoverageEM`
模块5中的覆盖求值模式，保留基础物理链，但在接收机与结果汇总层做受控简化。

#### `CIR / PDP / APS`

- `CIR`：冲激响应
- `PDP`：时延功率谱
- `APS`：角度功率谱

#### `ISACFeatureSet`
面向通感研究的基础特征集合，不等于终态感知性能评价指标。

### 19.4 验证与工程相关术语

#### `SceneCache`
模块2生成的场景预处理缓存容器，用于复用静态场景处理结果。

#### `ValidationReport`
模块6输出的统一验证报告。

#### `RegressionReport`
模块6输出的统一回归报告。

#### `ExportBundle`
模块6统一导出容器，承载路径、电磁、统计、验证与元信息结果。

---

## 20. 模块对象速查表（面向新会话快速定位）

本节用于让 AI 或开发者快速判断：某个对象属于哪个模块、谁生产、谁消费、是否已冻结。

### 20.1 模块1对象速查表

| 对象 | 所属模块 | 谁生产 | 谁消费 | 当前状态 |
|---|---|---|---|---|
| `AppConfig` | 模块1 | 模块1 | 模块2/3/4/5/6 | 已冻结 |
| `ConfigValidationResult` | 模块1 | 模块1 | app/模块6 | 已冻结 |
| `Logger` | 模块1 | 模块1 | 所有模块 | 已冻结 |
| `RtError` | 模块1 | 模块1 | 所有模块 | 已冻结 |
| `NumericToleranceConfig` | 模块1 | 模块1 | 模块2/4/5 | 已冻结 |
| `VersionInfo` | 模块1 | 模块1 | 模块2/5/6 | 已冻结 |

### 20.2 模块2对象速查表

| 对象 | 所属模块 | 谁生产 | 谁消费 | 当前状态 |
|---|---|---|---|---|
| `Scene` | 模块2 | 模块2 | 模块4/5/6 | 已冻结 |
| `Face` | 模块2 | 模块2 | 模块2内部/模块5回查 | 已冻结 |
| `Edge` | 模块2 | 模块2 | 模块2内部 | 已冻结 |
| `Wedge` | 模块2 | 模块2 | 模块4/5 | 已冻结 |
| `SceneDiagnostics` | 模块2 | 模块2 | 模块6 | 已冻结 |
| `SceneMaterialBinding` | 模块2 | 模块2 | 模块2内部/调试 | 已冻结 |
| `SceneAcceleration` | 模块2 | 模块2 | 模块2/4 | 已冻结 |
| `FaceQueryRecord` | 模块2 | 模块2 | 模块2查询层 | 已冻结 |
| `WedgeQueryRecord` | 模块2 | 模块2 | 模块2查询层 | 已冻结 |
| `SceneQuery` | 模块2 | 模块2 | 模块4 | 已冻结 |
| `SceneCacheMeta/Content` | 模块2 | 模块2 | 模块2/app | 已冻结 |

### 20.3 模块3对象速查表

| 对象 | 所属模块 | 谁生产 | 谁消费 | 当前状态 |
|---|---|---|---|---|
| `AntennaModel` | 模块3 | 模块3 | 模块4/5 | 接口冻结 |
| `AntennaArrayModel` | 模块3 | 模块3 | 模块5（预留） | 接口冻结 |
| 方向图/极化外部接口字段 | 模块3 | 模块3 | 模块5 | 接口冻结 |

### 20.4 模块4对象速查表

| 对象 | 所属模块 | 谁生产 | 谁消费 | 当前状态 |
|---|---|---|---|---|
| `PathState` | 模块4 | 模块4 | 模块4内部 | 已冻结 |
| `PathNode` | 模块4 | 模块4 | 模块5/6 | 已冻结 |
| `GeometricPath` | 模块4 | 模块4 | 模块5/6 | 已冻结 |
| `SearchEngine` | 模块4 | 模块4 | app | 已冻结 |
| `StateSignature` | 模块4 | 模块4 | 模块4内部 | 已冻结 |
| `PathSignature` | 模块4 | 模块4 | 模块4/6 | 已冻结 |

### 20.5 模块5对象速查表

| 对象 | 所属模块 | 谁生产 | 谁消费 | 当前状态 |
|---|---|---|---|---|
| `FieldAccumulator` | 模块5 | 模块5 | 模块5内部 | 已冻结 |
| `EMPathResult` | 模块5 | 模块5 | 模块6 | 已冻结 |
| `CIRResult` | 模块5 | 模块5 | 模块6 | 已冻结 |
| `PDPResult` | 模块5 | 模块5 | 模块6 | 已冻结 |
| `APSResult` | 模块5 | 模块5 | 模块6 | 已冻结 |
| `ChannelStatistics` | 模块5 | 模块5 | 模块6 | 已冻结 |
| `CoverageResult` | 模块5 | 模块5 | 模块6 | 已冻结 |
| `ISACFeatureSet` | 模块5 | 模块5 | 模块6/后续分析 | 已冻结 |

### 20.6 模块6对象速查表

| 对象 | 所属模块 | 谁生产 | 谁消费 | 当前状态 |
|---|---|---|---|---|
| `ExportBundle` | 模块6 | 模块6组装 | 模块6内部 | 已冻结 |
| `ValidationReport` | 模块6 | 模块6组装 | 人工/AI/回归工具 | 已冻结 |
| `RegressionReport` | 模块6 | 模块6组装 | 人工/AI/回归工具 | 已冻结 |

---

## 21. 模块接口速查表（最关键的上下游契约）

### 21.1 模块1 → 模块2/3/4/5/6

| 输出对象 | 作用 | 说明 |
|---|---|---|
| `AppConfig` | 全局配置 | 所有模块唯一高层配置来源 |
| `Logger` | 统一日志 | 所有模块日志必须统一经过它 |
| `RtError` | 统一错误 | 所有模块错误统一结构 |
| `NumericToleranceConfig` | 容差控制 | 统一几何/电磁阈值 |
| `VersionInfo` | 版本控制 | cache 与结果追踪必需 |

### 21.2 模块2 → 模块4

| 输出对象 | 作用 | 说明 |
|---|---|---|
| `Scene` | 场景真源 | 模块4不再重建场景语义 |
| `SceneQuery` | 查询门面 | 模块4只能通过它做几何查询 |
| `SceneDiagnostics` | 诊断信息 | 可辅助调试与验证 |

### 21.3 模块4 → 模块5

| 输出对象 | 作用 | 说明 |
|---|---|---|
| `GeometricPath` | 路径几何真源 | 模块5不再重建几何 |
| `PathNode` | 单节点交互语义 | reflection/transmission/diffraction 必需 |

### 21.4 模块5 → 模块6

| 输出对象 | 作用 | 说明 |
|---|---|---|
| `EMPathResult` | 路径级电磁结果 | 模块6直接导出与可视化 |
| `CIR/PDP/APS` | 通道结果 | 模块6组织输出 |
| `ChannelStatistics` | 统计量 | 报告与回归比较 |
| `CoverageResult` | 覆盖结果 | 覆盖图与功率统计 |
| `ISACFeatureSet` | 通感基础特征 | 后续分析与结果导出 |

---

## 22. 开发前检查清单（任何新会话必须执行）

在任何新对话中，AI 或开发者开始写代码前，必须先逐项确认：

### 22.1 文档与上下文检查

- [ ] 已读取 `RT算法正式开发文档 v1.md`
- [ ] 已确定当前要开发的是哪个模块
- [ ] 已根据主文档索引回查草案对应章节
- [ ] 已确认当前模块的冻结边界与不可回退项

### 22.2 开发目标检查

- [ ] 已明确当前属于哪个开发批次
- [ ] 已明确本批次的闭环目标
- [ ] 已明确检测方式与通过标准
- [ ] 已明确是否会影响上下游接口

### 22.3 代码约束检查

- [ ] 不跨模块职责污染
- [ ] 不新增平行结构替代已冻结对象
- [ ] 所有新配置进入 `AppConfig`
- [ ] 所有新日志进入 `Logger`
- [ ] 所有新错误走 `RtError`
- [ ] 所有新阈值进入 `NumericToleranceConfig` 或 profile 配置

### 22.4 验证准备检查

- [ ] 已准备最小测试场景/输入
- [ ] 已明确本次要输出哪些 debug 信息
- [ ] 已明确通过后要记录什么内容

---

## 23. 每批次开发结束记录模板

为了保证任意时刻都可被新 AI 会话接续，建议每批次开发结束后按如下模板记录：

### 23.1 建议记录模板

```text
批次编号：
本次目标：

已实现文件：
- 
- 

已实现对象/函数：
- 
- 

本次闭环测试：
- 测试输入：
- 输出结果：
- 是否通过：

主要 debug/日志/可视化证据：
- 
- 

尚未完成内容：
- 

已知风险与注意事项：
- 

是否影响后续模块接口：是/否
若是，影响说明：
```

### 23.2 记录模板的正式作用

1. 让人工快速了解当前进度；
2. 让新 AI 会话快速接续；
3. 让回归与问题定位有清晰阶段边界；
4. 防止“看似写了很多代码，但没人知道当前闭环到哪一步”。

---

## 24. 当前版本文档的最终使用建议

### 24.1 正式建议

从现在开始，任何后续开发应按以下方式进行：

1. **先以本正式开发文档为主导航**
2. **再根据索引到草案中查询具体细节**
3. **按批次开发**
4. **每批次形成明确闭环并记录**

### 24.2 对 AI 编程工具的正式结论

> 经过本轮补充后，`RT算法正式开发文档 v1.md` 已经不再只是“简短摘要”，而是具备了：
>
> 1. 模块冻结边界
> 2. 文档体系优先级
> 3. 草案索引映射
> 4. 模块对象与接口速查表
> 5. 分批开发流程
> 6. 闭环测试标准
> 7. 开发前检查清单
> 8. 批次结束记录模板
>
> 因而已经可以作为新对话中 AI 编程工具继续本项目开发的正式主入口文档。

### 24.3 最终一句话结论

> **当前文档体系（主开发文档 + 详细设计草案字典）已经足以支持在任意新对话中，让 AI 编程工具从结构到细节理解本项目的目标、进展、极化、电磁求解、覆盖模式、验证闭环与后续开发路线，并继续安全推进开发或优化。**
