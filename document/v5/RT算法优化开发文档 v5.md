# RT算法优化开发文档 v5

## 0. 文档定位与使用方式

本文档是 RT 系统在 v4 第四轮开发闭环后，进入第五轮开发时的**正式优化开发文档 v5**。

### 0.1 与前版本文档的关系

- `RT算法正式开发文档 v1.md`：第一轮正式开发方案，批次 0~9。已冻结。
- `RT算法优化开发文档 v2.md`：第二轮优化方案，批次 A0~A8。已冻结。
- `RT算法优化开发文档 v3.md`：第三轮开发方案，批次 B0~B10。已冻结。
- `RT算法优化开发文档 v4.md`：第四轮开发主文档，C0~C9。**8/10完全完成，2/10部分完成**。
- **本文档 v5**：第五轮开发主文档。目标：理论审计 + 开题对照 + 对标验证 + 代码规范 + 加速优化。

### 0.2 v5 总体基调

1. **证明正确性 > 增加新功能** — 核心产出是论文"算法验证"章节的证据链
2. **审计优先于编码** — D0~D2 全部是只读分析，D3~D4 搭建基准测试，D6 才开始修改代码
3. **论文导向** — 每个批次必须可追溯到论文某章某节的具体需求
4. **不推翻架构** — 六模块分层不变，函数签名尽量不动

### 0.3 v4 基线回顾

v4 实际完成度：

| 批次 | 主题 | 状态 |
|------|------|------|
| C0 | v3 冻结基线认定 + 问题清单 | ✅ 完成 |
| C1 | EM 精度修复（相干极化 / 动态材质 / FSPL） | ✅ 完成 |
| C2 | SBR 大规模覆盖重构（固定球 / Rx哈希 / OpenMP） | ✅ 完成 |
| C3 | 完整 UTD（Fresnel 积分数值解 + 精确 phi' + s2） | ✅ 完成 |
| C4 | 场景处理升级（SAH BVH / 坐标变换 / SceneCache） | ⚠️ SAH✅ / 坐标变换❌ / Cache✅ |
| C5 | 全中文规范化（日志 + 注释 + 错误信息） | ✅ 完成（app/除外） |
| C6 | 三级验证体系（L1+L2+L3） | ✅ 完成（9/9 PASS） |
| C7 | SBR 绕射 + 漫散射 | ⚠️ 概率绕射✅ / 漫散射❌ |
| C8 | 覆盖可视化 + PDP/APS 图表 | ✅ 完成 |
| C9 | v4 全量回归 + 开题对照 + 论文实验模板 | ✅ 完成 |

### 0.4 v4 → v5 遗留移交

| # | 遗留项 | v4来源 | v5去向 |
|---|--------|--------|--------|
| L1 | OBJ 坐标变换选项 | C4-B 未实现 | D6-A |
| L2 | 面元级材质绑定 | C0-Q2 推迟 | 论文局限性（OBJ格式限制） |
| L3 | Jones 矢量极化 | C1 已知限制 | D6-A / 论文局限性 |
| L4 | 漫散射 | C7 未实现 | 论文未来工作 |
| L5 | SBR 概率→确定性绕射 | C7 部分完成 | 论文已知限制 |
| L6 | app/ 目录中文注释 | C5 未覆盖 | D5 |
| L7 | preprocess/ 内联注释 | C5 英文残留 | D5 |

---

## 第1章：v5 总目标、边界与批次规划

### 1.1 v5 核心目标

1. **理论正确性证明**（需求1）：14项理论审计，对照 Balanis/Born&Wolf/Kouyoumjian-Pathak 原始文献
2. **开题报告对齐**（需求2）：13项需求逐项对照，标记缺口，明确论文范围
3. **三级对标验证**（需求3）：vs 参考实现 + vs Sionna RT + vs 3GPP 统计模型
4. **代码规范化**（需求4）：全量中文注释覆盖 + v4遗留补全
5. **加速优化**（需求5）：OpenMP 使能 → SIMD 向量化 → GPU 设计

### 1.2 v5 不做什么

1. 不推翻六模块架构
2. 不引入新的寻径算法（如 Metropolis 光传输、BDPT）
3. 不实现时变信道/多普勒（论文明确范围外）
4. 不做实测数据对照（缺少实测数据）
5. P2 加速只出设计文档不强制编码
6. 不引入新的外部依赖（保持纯 C++20 + MSVC）

### 1.3 v5 批次总览

| 批次 | 阶段 | 对应需求 | 主题 | 优先级 |
|------|------|---------|------|--------|
| D0 | 审计前置 | 基线冻结 | v4 基线冻结 + v5 变更边界划定 | P0 |
| D1 | 审计前置 | 需求1 | 全量理论检查与代码审计（14项） | P0 |
| D2 | 审计前置 | 需求2 | 开题报告全量逐项对照（13项） | P0 |
| D3 | 对标验证 | 需求3 | 三级对标验证体系搭建 | P0 |
| D4 | 对标验证 | 需求3 | 三级对标验证执行与数据分析 | P0 |
| D5 | 整理补齐 | 需求4 | 全量中文注释规范化 | P1 |
| D6 | 整理补齐 | 需求4+遗留 | v4 遗留补全 + 论文实验数据生成 | P1 |
| D7 | 加速优化 | 需求5-P0 | OpenMP 使能 + SAH 阈值精调 | P1 |
| D8 | 加速优化 | 需求5-P1 | SIMD 向量化 + Ray-Sphere 提前剔除 | P2 |
| D9 | 加速优化 | 需求5-P2 | GPU CUDA/OptiX + 场景LOD（设计文档） | P2 |
| D10 | 收口 | 全部 | v5 全量回归 + 论文验证章节定稿 | P1 |

### 1.4 批次依赖关系

```
D0 ──→ D1 ──→ D2 ──→ D3 ──→ D4 ──→ D6 ──→ D10    ← 关键路径
  │                        │              │
  │                        │              └──→ D7 ──→ D8 ──→ D9    ← 加速链（可并行）
  │                        │
  └──→ D5（贯穿并行）─────┘
```

- **D1 + D3** 可并行（都只读/基础设施）
- **D4 + D5** 可并行（跑数据 + 改注释互不干扰）
- **D7** 尽早做（加速影响后续所有实验数据生成速度）
- **D8 + D9** 为非关键路径，可裁剪

---

## 第2章：D0 — v4 基线冻结与 v5 变更边界

### 2.1 批次定位

D0 是 v5 的前置条件。在动任何代码之前，必须冻结 v4 基线并划定 v5 允许修改的文件边界。**D0 全部只读，不修改任何代码。**

### 2.2 v4 回归基线

运行全部 5 个 v4 回归配置，记录基线指标：

| 配置 | 预期路径数 | 预期时间 | 验证项 |
|------|-----------|---------|--------|
| b1_mixed_path_test.json | 362 paths | ~0.3s | 混合路径 precise |
| a3_transmission_minimal.json | 2 paths | ~0.1s | 单面透射 |
| meeting_v3.json | 664 paths | ~2.4s | 全机制 precise |
| meeting_coverage.json | 19 precise + 8/8 SBR | ~5s | SBR覆盖 |
| b4_sbr_test.json | SBR activeRx | ~0.5s | SBR小场景 |

**通过标准**：5/5 配置全部运行通过，路径数与 v4 基线一致（偏差=0），日志无 ERROR/FATAL。

### 2.3 v5 代码变更边界

**允许修改**（v5 将触碰的文件）：

| 文件 | 修改原因 | 批次 |
|------|---------|------|
| `preprocess/import/OBJImporter.cpp/.h` | 新增坐标变换选项 | D6 |
| `core/em/FieldAccumulator.h` | 可选 Jones 矢量升级 | D6 |
| `core/em/ApplyReflectionInteraction.cpp` | 可选极化升级 | D6 |
| `core/em/ApplyTransmissionInteraction.cpp` | 可选极化升级 | D6 |
| `core/em/ApplyDiffractionInteraction.cpp` | 可选极化升级 + SIMD(D8) | D6/D8 |
| `core/em/FinalizeAtReceiver.cpp` | 可选极化升级 | D6 |
| `core/em/InitializeTxField.cpp` | 可选极化升级 | D6 |
| `core/search/SbrEngine.cpp` | D7/D8 加速 | D7/D8 |
| `core/search/SbrEngine.h` | 加速相关接口 | D7/D8 |
| `preprocess/accel/FaceBVHBuilder.cpp` | SAH 阈值精调 + OpenMP | D7 |
| `core/common/config/AppConfig.h` | 新增 v5 配置字段 | D6/D7 |
| `core/common/config/AppConfigJsonCodec.cpp` | 新增字段读写 | D6/D7 |
| `RT.vcxproj` | 编译开关调整 | D7/D8 |
| `app/` (7文件) | 中文注释规范化 | D5 |
| `core/path/`, `core/scene/`, `core/query/`, `core/result/`, `preprocess/` | 中文注释规范化 | D5 |

**禁止修改**（v5 不触碰的稳定底座）：

| 文件/目录 | 原因 |
|-----------|------|
| `core/common/math/Vec3.h, Complex.h, Matrix3x3.h, MathConstants.h` | 数学基础库，已稳定 |
| `core/common/config/` (除 AppConfig 相关) | 配置体系稳定 |
| `core/common/log/`, `core/common/error/`, `core/common/material/` | 基础设施稳定 |
| `core/antenna/` | 天线模块稳定 |
| `core/search/SearchEngine.cpp/.h` | precise 搜索引擎稳定 |
| `core/search/ReflectionExpander.cpp/.h` | 反射扩展器稳定 |
| `core/search/TransmissionExpander.cpp/.h` | 透射扩展器稳定 |
| `core/search/DiffractionExpander.cpp/.h` | 绕射扩展器稳定（除 D5 注释） |
| `core/search/GeometryValidity.cpp/.h` | 几何合法性检查稳定 |
| `core/scene/Face.h, Edge.h, Wedge.h, Scene.h` | 场景数据结构稳定（除 D5 注释） |
| `preprocess/build/EdgeBuilder.cpp, WedgeBuilder.cpp` | 拓扑构建器稳定 |
| `app/legacy/` | 历史资产，不编译 |

### 2.4 D0 待确认问题

**Q1：v4 回归基线是否全部通过？**
运行全部 5 配置，确认路径数/功率/时延与 v4 记录值一致。

**Q2：Git 仓库状态**
当前是否有未提交更改？v4 最终基线是否需要 commit？

**Q3：x64/Debug/RT.exe 是否可用？**
确认编译环境就绪。

### 2.5 D0 状态

**当前状态：✅ 已完成。** 2026-05-07 执行。

**回归结果（5/5 ALL PASSED）**：

| 配置 | 路径数 | 验证 | 时间 |
|------|--------|------|------|
| b1_mixed_path_test.json | 362 paths | validation_passed=true | ~0.1s |
| a3_transmission_minimal.json | 2 paths | validation_passed=true | ~0.1s |
| meeting_v3.json | 664 paths | validation_passed=true | ~2s |
| b4_sbr_test.json | 362 precise + SBR 15/15 activeRx | validation_passed=true | ~0.5s |
| meeting_coverage.json | 19 precise + SBR 8/8 activeRx | validation_passed=true | ~14s |

**Python 验证（9/9 ALL PASSED）**：L1 5/5 + L2 2/2 + L3 2/2

**Git 状态**：
- HEAD: `4e174a8 v4 全量闭环: C0~C9 全部完成`
- 未提交修改: `app/RtPipeline.cpp`, `document/RT算法优化开发文档 v4.md`
- 未跟踪: `document/RT算法优化开发文档 v5.md`
- 建议: v5 正式启动后在 `document/RT算法优化开发文档 v4.md` 定稿时做 git tag `v4-final-baseline`

**编译环境**：VS2022 + MSVC, `x64/Debug/RT.exe` 可用。

---

## 第3章：D1 — 全量理论检查与代码审计（需求1）

### 3.1 批次定位

D1 是 v5 最核心的只读分析批次。逐模块逐行审查 EM 链路的理论正确性，对照 Balanis/Born&Wolf/Kouyoumjian-Pathak 原始文献。**D1 不修改任何代码。**

### 3.2 审计方法

对每个审计项执行以下步骤：
1. 阅读 C++ 实现（逐行）
2. 定位原始文献中的对应公式
3. 逐变量追溯：实现 vs 文献
4. 分类判定：**(a) 精确匹配 (b) 合理近似 (c) 无依据简化 (d) 潜在bug**
5. 严重度标注：Critical / High / Medium / Low / Informational

### 3.3 审计项全集（14项）

#### 审计1：Fresnel 反射 TE 系数

**文件**: `core/em/ApplyReflectionInteraction.cpp`
**参考**: Balanis "Advanced Engineering Electromagnetics" Ch.5, Born & Wolf Sec.1.5

检查要点：
- Γ_TE = (cosθ_i - √(ε_c - sin²θ_i)) / (cosθ_i + √(ε_c - sin²θ_i))
- 复数 ε_c = ε_r - jσ/(ωε₀) 是否正确计算
- cosθ_i 的符号约定（入射角定义）
- √取主分支是否正确
- 垂直入射极限 Γ→-1 是否正确

#### 审计2：Fresnel 反射 TM 系数

**文件**: 同上
**参考**: 同上

检查要点：
- Γ_TM = (ε_c·cosθ_i - √(ε_c - sin²θ_i)) / (ε_c·cosθ_i + √(ε_c - sin²θ_i))
- Brewster 角附近的行为
- ε_c 为大值（金属）时的极限行为

#### 审计3：Fresnel 透射 TE 系数

**文件**: `core/em/ApplyTransmissionInteraction.cpp`
**参考**: Balanis Ch.5

检查要点：
- T_TE = 2cosθ_i / (cosθ_i + √(ε_c - sin²θ_i))
- 功率守恒：|Γ|² + (n_t·cosθ_t / n_i·cosθ_i)·|T|² = 1
- Snell 折射角一致性

#### 审计4：Fresnel 透射 TM 系数

**文件**: 同上
**参考**: 同上

检查要点：
- T_TM = 2ε_c·cosθ_i / (ε_c·cosθ_i + √(ε_c - sin²θ_i))
- "2×cosθ_i" vs "cosθ_i+cosθ_i" 写法验证

#### 审计5：FSPL 自由空间路径损耗

**文件**: `core/em/FinalizeAtReceiver.cpp`, `core/em/ApplyFreeSpaceSegment.cpp`
**参考**: Friis (1946), Balanis Ch.4

检查要点：
- FSPL = λ/(4πd)，不是 λ/(4πd_total_accumulated) 
- 确认 FSPL 在 FinalizeAtReceiver 中恰好施加一次（非每段累乘）
- 波长-频率关系：λ = c/f
- 远场假设 d > 2D²/λ 是否在代码中注明

#### 审计6：UTD 绕射 D1-D4 系数

**文件**: `core/em/ApplyDiffractionInteraction.cpp`
**参考**: Kouyoumjian & Pathak (1974), Balanis Ch.12

检查要点：
- D1 = -exp(-jπ/4) / (2n√(2πk)) · cot( (π + (φ-φ'))/(2n) ) · F(k L a⁺)
- D2 = 同上 cot( (π - (φ-φ'))/(2n) )
- D3 = 同上 cot( (π + (φ+φ'))/(2n) )
- D4 = 同上 cot( (π - (φ+φ'))/(2n) )
- n = (2π - α)/π = 外角/π
- cot 函数参数在阴影边界（φ = φ'±π 或 φ = -φ'±π）时的奇异性
- 阴影边界过渡处理是否正确（F(x) 平滑过渡）

#### 审计7：UTD Fresnel 积分数值解 F(x)

**文件**: `core/em/ApplyDiffractionInteraction.cpp`（FresnelTransitionNumerical 函数）
**参考**: K&P Eq.25-28

检查要点：
- F(x) = 2j√x exp(jx) ∫_{√x}^∞ exp(-jτ²) dτ
- 8-point Gauss-Legendre 精度评估：对典型 x∈(0, 10) 误差 < 1e-6?
- 自适应区间 [tau0, tau0+8.0] 覆盖是否足够？NIntervals=16 是否最优？
- 小 x 极限（x→0, F(x)→0）
- 大 x 极限（x→∞, F(x)→1）

#### 审计8：UTD phi' 入射方位角

**文件**: `core/em/ApplyDiffractionInteraction.cpp`
**参考**: K&P Eq.14-17

检查要点：
- phi' = atan2(k_i_perp·ê_y, k_i_perp·ê_x)
- 边缘固定坐标系 (ê_x, ê_y, ê_z) 的构建是否正确
- grazing 回退 (phi'=kPi) 在实际路径中是否会被触发？
- phi' 范围 [0, 2π) 的正确性

#### 审计9：UTD s2 距离参数

**文件**: `core/em/ApplyDiffractionInteraction.cpp`
**参考**: 路径几何

检查要点：
- s2 = |Rx - 绕射点| 从 path->nodes.back() 获取
- path->nodes 为空时的回退逻辑是否安全
- s2 < 1e-9 时置为 1.0 是否合理

#### 审计10：EM 初始化（Tx 场）

**文件**: `core/em/InitializeTxField.cpp`
**参考**: Balanis Ch.2

检查要点：
- 发射功率 → 电场幅值换算
- 天线方向图注入（如果有）
- 极化向量初始化方向
- 相位初始化（通常为 0）

#### 审计11：极化处理

**文件**: `core/em/FieldAccumulator.h`, 所有 Apply*Interaction.cpp
**参考**: Born & Wolf Ch.1, IEEE Std 149

检查要点：
- 入射场 TE/TM 投影使用实向量 Dot 而非复投影
- 反射/透射后极化重构仅用实部（丢失虚部信息）
- TE/TM 相位差 = 椭圆极化 → 当前实向量无法描述
- 相位取平均 (γTE.Arg() + γTM.Arg())/2 在 TE≫TM 或 TM≫TE 时失真
- 量化：室内材料 tanδ < 0.1 时 TE/TM 相位差 < 20° → 椭圆极化效应有限

#### 审计12：SBR 发射模型

**文件**: `core/search/SbrEngine.cpp`
**参考**: Fibonacci 球面均匀分布

检查要点：
- Fibonacci 球面采样是否产生均匀方向分布
- 射线数 N 与角分辨率的关系
- 发射功率归一化（每射线功率 = P_tx / N）
- 射线是否从 Tx 点正确发射

#### 审计13：SBR 接收球

**文件**: `core/search/SbrEngine.cpp`
**参考**: Seidel & Rappaport (1994) "Site-Specific Propagation Prediction"

检查要点：
- 固定半径 default=0.3m 的物理依据
- RxHashGrid cell_size=2×radius：是否保证跨 cell 边界的命中不漏检
- CheckSegment 使用线段中点查询 27 邻域：长线段是否可能跨越多于 27 个 cell
- 收发同点（LOS 距离=0）的处理

#### 审计14：BVH SAH 正确性

**文件**: `preprocess/accel/FaceBVHBuilder.cpp`
**参考**: MacDonald & Booth (1990), Wald (2007)

检查要点：
- SAH cost = (leftArea×leftCount + rightArea×rightCount) / parentArea
- 16-bin 分桶逻辑：面元按 centroid 落入正确的 bin
- 遍历顺序（near-far）是否正确
- 小节点 median-split 回退的阈值 (leafSize×2) 是否合理

### 3.4 D1 关键问题清单

以下问题需在审计中重点回答：

1. **8点 Gauss-Legendre 是否足够？** 在阴影边界过渡区 (x≈0.1~5) 精度是否满足 < 0.5dB 要求？
2. **grazing 回退被触发过吗？** phi'=kPi 回退在 meeting_v3 664 path 中出现多少次？
3. **D1-D4 cot 参数有无符号错误？** 逐项对照 K&P 原文 Table 1
4. **FSPL 施加次数** — 在包含反射+透射+绕射的混合路径中，FSPL 是否精确施加一次？
5. **RxHashGrid 不漏检吗？** 27 邻域查询是否覆盖所有可能命中 Rx 的 cell？
6. **ε_c 虚部符号** — ℑ(ε_c) = -σ/(ωε₀)（负号约定是否与 Fresnel 公式一致）？

### 3.5 D1 产出

`document/v5/audit_D1_理论审计报告.md`，包含：
- 14项审计详情（每项: 实现代码片段 + 文献公式 + 对比结论 + 严重度）
- 发现清单（Critical/High/Medium/Low 分级）
- Critical/High 项的修复方案
- 可证明正确项汇总（可直接引用为论文"算法验证"证据）

### 3.6 D1 状态

**当前状态：待执行。**

---

## 第4章：D2 — 开题报告全量对照（需求2）

### 4.1 批次定位

D2 对照开题报告 (`西安电子科技大学硕士学位论文开题报告表v1.23.doc`) 中的每项 RT 算法需求，标记实现状态。D2 全部只读。

### 4.2 对照维度（13项）

| # | 开题需求 | 对照要点 |
|---|---------|---------|
| R1 | 室内ISAC多维域高保真信道建模 | 是否覆盖通信+感知双域参数 |
| R2 | 基带多径完整参数 | 时延/3D角度(AoA/AoD)/复振幅/极化态 → 四项是否全部输出 |
| R3 | Fresnel 反射/透射（材质+频率依赖, TE/TM） | 相干合成/动态材质/频率插值 |
| R4 | UTD 绕射（Fresnel积分数值解） | 非Luebbers近似，精确phi'+s2 |
| R5 | 混合路径 R/T/D 任意组合 | 同一条路径内反射→透射→绕射串行 |
| R6 | SBR 大规模覆盖仿真 | 10K Rx + 100K+ 射线 + OpenMP |
| R7 | 天线方向图外部导入+逐路径注入 | CSV加载+双线性插值+极化 |
| R8 | 多维输出（PDP/APS/路径级JSON/CSV） | 时延域/角度域/路径级 |
| R9 | 多域验证体系 | L1解析解+L2交叉+L3统计 |
| R10 | 仿真-实测/参考实现对比 | 对标框架就绪，实际对标数据待D4 |
| R11 | 3GPP TR 38.901 扩展场景 | InH-Office 路径损耗模型对比 |
| R12 | 时变/多普勒建模 | 论文明确范围外 |
| R13 | 场景导入(OBJ)+坐标系处理 | OBJ导入+材质绑定+坐标变换可选 |

### 4.3 每项判定标准

- **已满足** ✅：有代码实现 + 有验证证据 + 有输出
- **部分满足** ⚠️：有代码但存在已知限制（需在论文"局限性"中说明）
- **未满足** ❌：未实现但 v5/D6 计划实现
- **不可行** ✖️：超出论文范围，在"研究范围与局限"中声明

### 4.4 D2 关键产出：论文数据缺口清单

D2 的最终产物是精确的"论文实验数据缺口清单"——直接驱动 D6-B 的实验设计。

预判缺口（D2 完成后精确化）：

| 缺口 | 实验类型 | 驱动批次 |
|------|---------|---------|
| 无路径损耗 vs 距离曲线 | SBR多Tx-Rx距离扫描 | D6-B |
| 无 RMS 时延扩展分布 | meeting.obj 20+ Rx | D6-B |
| 无 SBR 收敛性曲线 | 1K~100K 射线扫描 | D6-B |
| 无绕射损耗 vs 楔角曲线 | 参数扫描 30°~150° | D6-B |
| 无透射损耗 vs 频率曲线 | 多频点 1~28 GHz | D6-B |
| 无极化鉴别率数据 | 交叉极化 Rx | D6-B |
| 无覆盖热力图 | SBR 高密度网格 | D6-B |
| 无 ISAC 特征（如需要） | 感知 SINR 等 | D6-B |

### 4.5 D2 产出

`document/v5/对照_D2_开题需求全量对照.md`，包含：
- 13项需求×4列（需求/状态/证据/缺口计划）矩阵
- 论文数据缺口清单（含具体实验参数）
- 论文范围声明（明确标注哪些是论文范围外）

### 4.6 D2 状态

**当前状态：待执行。**

---

## 第5章：D3 — 三级对标验证体系搭建（需求3）

### 5.1 批次定位

D3 搭建对标的工具链和基础设施。**D3 只搭建工具，不执行实际对标**（D4 执行）。

### 5.2 D3-A：vs 参考实现 (`算法/RT.XD.SBR.CGAL.25.05`)

#### 5.2.1 参考实现能力摸底

**已完成**（v5 启动前已摸底）：
- 代码规模：~27K 行 C++，243 .cpp + 276 .h
- 编译环境：VS2022，C++20，预编译 exe 可用
- 场景格式：CSV（vertices.csv + triangles.csv + material.csv + corners.csv）
- 输出格式：CSV（per-Rx 路径级：pathLength, timeDelay, impulseResponse, AoD, AoA）
- 输入配置：RtSbr3DForRay3D.Config.json
- 场景示例：ModelSci4/ 目录有完整室内场景
- 天线/极化数据库：JSON 格式，有独立的天线方向图和极化模型
- 关键差异：参考实现使用 CircularPolarization3D（圆极化基），自实现使用 TE/TM 线极化基

#### 5.2.2 场景翻译器设计

新建 `test/validate/cross_ref/scene_translator.py`：

```
输入: OBJ + scene_material_map.json + AppConfig.json
输出: 参考实现格式的 CSV 文件包

翻译映射:
  OBJ vertices       → ScenarioAcceleratePoint3D.csv (x,y,z)
  OBJ faces (tri)    → ScenarioAccelerateTriangle3D.csv (P1,P2,P3,upType,downType,roughness,nx,ny,nz)
  材料规则            → ScenarioMaterial.csv (id,name,TypeNumber,Freq,εr,σ,μr,σm,Color)
  Tx/Rx 坐标         → TransmittingAntenna*.json / ReceiverAntenna.csv
  仿真频率            → RtSbr3DForRay3D.Config.json
  楔边                → ScenarioAccelerateCorner3D.csv (P1,P2,Face0,FaceN)

材质类型编号映射 (ObjectType → reference impl TypeNumber):
  需要逆向参考实现的材料类型枚举
```

**关键挑战**：两个系统的物理模型不完全一致（参考实现用圆极化基，自实现用 TE/TM 基）。对比时需注意：
- 功率比较可行（标量）
- 极化比较需要基变换
- 时延比较直接可比
- 路径数受寻径算法差异影响（Image Method vs SBR 策略差异）

#### 5.2.3 cross_runner.py

```python
def run_both(scene_name, our_config, ref_config):
    """运行自实现和参考实现，返回统一格式的路径列表"""
    our_paths = run_rt_exe(our_config)    # 运行 RT.exe
    ref_paths = run_ref_exe(ref_config)   # 运行参考实现 exe
    our_normalized = normalize_paths(our_paths)  # 统一字段名
    ref_normalized = normalize_paths(ref_paths)
    return our_normalized, ref_normalized

def normalize_paths(paths):
    """将不同实现的输出统一为: path_id, length, delay, power_dBm, 
       aod_az, aod_el, aoa_az, aoa_el, interaction_types"""
```

#### 5.2.4 cross_compare.py

对比报告模板：
```json
{
  "scenario": "b1_mixed",
  "our_path_count": 362,
  "ref_path_count": 340,
  "path_count_deviation_pct": 6.4,
  "matched_paths": 310,
  "top5_power_deviation_dB": [0.8, 1.2, 0.5, 2.1, 1.7],
  "top5_delay_deviation_ns": [0.1, 0.2, 0.1, 0.3, 0.2],
  "overall_pass": true
}
```

### 5.3 D3-B：vs Sionna RT

Sionna RT 是 NVIDIA 开源的基于 TensorFlow 的射线追踪信道模拟器，使用 Image Method。

#### 5.3.1 对比策略

不追求等同场景的精确对比（Sionna 使用不同的场景格式和渲染管线），而是做**概念层级对比**：

1. 单墙反射：两个系统对平面反射的 PDP 峰值位置一致
2. 小房间：PDP 形状相似（峰值位置+相对功率分布）
3. RMS 时延扩展：同数量级

#### 5.3.2 脚本设计

新建 `test/validate/cross_sionna/sionna_compare.py`：

```
1. 在 Sionna RT 中创建等效简单场景（box, 单墙, 双墙）
2. 在本系统中创建相同几何场景
3. 分别运行，导出 PDP
4. PDP 互相关相似度分析
5. RMS DS 对比
```

### 5.4 D3-C：vs 统计模型

#### 5.4.1 3GPP TR 38.901 InH-Office

新建 `test/validate/cross_stats/path_loss_3gpp.py`：

```python
def inH_office_path_loss(d_3d, freq_GHz, los=True):
    """
    3GPP TR 38.901 Table 7.4.1-1
    InH-Office LOS: PL = 32.4 + 17.3*log10(d) + 20*log10(f_c)  [d in m, f_c in GHz]
    InH-Office NLOS: PL = 38.3*log10(d) + 17.3 + 24.9*log10(f_c)
    """
```

新建 `test/validate/cross_stats/rt_vs_3gpp.py`：

```
输入: SBR coverage JSON（多 Rx 的功率值）
输出: path_loss_vs_distance.png（叠加 3GPP 模型曲线 + RT 数据点）
计算: RT 数据的路径损耗指数 n（最小二乘拟合）
对比: n_RT vs n_3GPP (LOS:1.73, NLOS:3.19)
```

### 5.5 产出文件清单

```
test/validate/cross_ref/
├── scene_translator.py       # OBJ→参考实现CSV格式
├── cross_runner.py            # 双exe启动+输出归一化
├── cross_compare.py           # 对比报告生成
└── README.md                  # 使用说明

test/validate/cross_sionna/
├── sionna_compare.py          # Sionna RT 对比
└── README.md

test/validate/cross_stats/
├── path_loss_3gpp.py          # 3GPP TR 38.901 计算
├── rt_vs_3gpp.py              # 对比+图表
└── README.md
```

### 5.6 D3 状态

**当前状态：待执行。**

---

## 第6章：D4 — 三级对标验证执行与数据分析（需求3续）

### 6.1 批次定位

D4 使用 D3 搭建的工具链，执行全部对标验证，产出论文"算法验证"章节所需的所有图表和数据。**D4 主要运行仿真和 Python 分析脚本，不修改 RT 引擎代码。**

### 6.2 D4-A：参考实现交叉验证

#### 测试场景

| 场景 | 场景文件 | 配置 | 验证重点 |
|------|---------|------|---------|
| S1: 混合路径 | b1_mixed_path_test.obj | b1_mixed_path_test.json | 反射+绕射 precise |
| S2: 会议室反射 | meeting.obj | meeting_v3.json (仅反射,depth=4) | 纯反射大规模 |
| S3: PEC 90°楔 | 新建简单楔场景 | 单绕射 | UTD 绕射损耗 |
| S4: 玻璃透射 | a3_transmission_minimal.obj | a3_transmission_minimal.json | Fresnel 透射损耗 |

#### 对比指标与容许阈值

| 指标 | 阈值 | 说明 |
|------|------|------|
| 路径数偏差 | < ±15% | 寻径算法差异可解释 |
| 前5强路径功率偏差 | < 3 dB | 振幅核心指标 |
| 前5强路径长度偏差 | < 5 cm | 几何核心指标 |
| 前5强路径时延偏差 | < 0.5 ns | 时域核心指标 |
| 路径到达角偏差 | < 5° | 角度域指标 |

#### 差异分析分类

对每个超出阈值的差异：
1. **寻径算法差异**：参考实现的 SBR vs 自实现的 Image Method → 路径集合不同
2. **EM 模型差异**：圆极化基 vs TE/TM 基 → 功率可比但极化不可比
3. **材质参数差异**：ε_r 在不同频率的插值差异
4. **自实现 Bug**：需修复

### 6.3 D4-B：Sionna RT 概念对照

- 场景：单墙反射 + 小房间（3面墙）
- 输出：PDP 对比图，RMS DS 对比表
- 目标：PDP 峰值位置偏差 < 2ns，RMS DS 同数量级

### 6.4 D4-C：3GPP / 商业软件对比

- SBR 覆盖结果 vs 3GPP TR 38.901 InH-Office 模型
- 叠加图：路径损耗 vs log10(距离)
- 计算自实现的路径损耗指数 n，对比 3GPP 标准值

### 6.5 论文图表产出清单

| 图表 | 类型 | 用途 |
|------|------|------|
| 对比表格：路径数/功率/时延/角度 vs 参考实现 | 表格 | 算法验证 §4.2 |
| PDP 对比图：自实现 vs 参考实现 | 图 | 算法验证 §4.3 |
| 路径损耗 vs 距离（叠加 3GPP 模型） | 图 | 算法验证 §4.4 |
| RMS 时延扩展对比：自实现 vs Sionna vs 参考 | 表格 | 算法验证 §4.3 |
| SBR 收敛性：功率 std vs 射线数 | 图 | 算法验证 §4.5 |

### 6.6 D4 状态

**当前状态：待执行（依赖 D3 完成）。**

---

## 第7章：D5 — 全量中文注释规范化（需求4）

### 7.1 批次定位

D5 将中文注释覆盖率从当前 ~60% 提升至 100%。**D5 仅修改注释，不改变任何算法逻辑。** D5 贯穿 v5 全程，可与其他批次并行。

### 7.2 当前覆盖状况

| 目录 | 覆盖率 | 缺口 |
|------|--------|------|
| `core/search/` | ✅ 完成 | — |
| `core/em/` | ✅ 完成 | — |
| `core/common/` | ✅ 完成 | — |
| `core/antenna/` | ✅ 完成 | — |
| `core/path/` | ✅ 完成 | — |
| `core/scene/` | ✅ 完成 | — |
| `core/query/` | ✅ 完成 | — |
| `core/result/` | ✅ 完成 | — |
| `preprocess/` (头注释+///) | ✅ 完成 | 内联算法注释为英文残留 |
| `app/legacy/` | ✅ 完成 | — |
| **`app/` (顶层7文件)** | ❌ 0% | **全部英文** |
| `test/` Python | ~50% | 英文注释 |

### 7.3 子批次

#### D5-A：app/ 顶层文件（最高优先）

| 文件 | 行数 | 工作内容 |
|------|------|---------|
| `app/main.cpp` | ~30 | 文件头+注释 |
| `app/RtPipeline.h` | ~50 | 文件头+接口注释 |
| `app/RtPipeline.cpp` | ~250 | 文件头+流水线步骤注释（当前有3行中文，补齐其余） |
| `app/A1RealChainRunner.h` | ~40 | 文件头+接口注释 |
| `app/A1RealChainRunner.cpp` | ~150 | 文件头+步骤注释 |
| `app/Batch9ExportReporter.h` | ~30 | 文件头+接口注释 |
| `app/Batch9ExportReporter.cpp` | ~80 | 文件头+步骤注释 |

#### D5-B：preprocess/ 内联注释

所有 `.cpp` 文件中的英文内联算法注释 → 翻译为中文。约 29 个文件，每个文件 5~20 行英文注释。

#### D5-C：test/ Python 文件（~8文件）

- `test/rt_utils.py` (498行) — 工具库
- `test/rt_visual.py` (242行) — 可视化
- `test/plot_pdp_aps.py` (87行) — 图表
- `test/validate/run_all.py` — 验证入口
- `test/rt_grid_search.py` — 网格搜索

### 7.4 统一注释格式

```cpp
// ───────────────────────────────────────────────────────────────────
// 文件: Xxx.cpp
// 用途: [一句话功能描述，说明该文件在系统中的角色]
// 所属模块: 模块X (名称)
// 创建: v4 某批次 / v5 某批次
// ───────────────────────────────────────────────────────────────────

///<summary>
/// [函数名]: [功能说明]
/// 步骤: (1) xxx → (2) xxx → (3) xxx
/// 参考: [公式来源，如 Balanis Eq.5-xx]
///</summary>
```

### 7.5 D5 状态

**当前状态：待执行（可随时开始，贯穿全程）。**

---

## 第8章：D6 — v4 遗留补全 + 论文实验数据生成（需求4+遗留）

### 8.1 批次定位

D6 是 v5 中唯一涉及 C++ 编码的批次。分 A/B 两个子批次：
- **D6-A**：补齐 v4 遗留的功能缺口
- **D6-B**：生成论文"仿真结果"和"算法验证"章所需的全量实验数据

### 8.2 D6-A：v4 遗留补全

基于 C0 Q1~Q6 的决策和 v4 完成度审核，确定以下补全项：

#### 6A-1：OBJ 坐标变换选项（v4 C4-B）

**来源**：C4-B 未实现
**优先级**：中
**论文必要性**：中（方便不同建模工具的互操作）

**实现方案**：

在 `AppConfig.h` 的 `SceneImportConfig` 中新增字段：
```cpp
std::string coordinate_transform = "none"; // "none" | "blender_z_up_to_y_up"
```

在 `OBJImporter.cpp` 中，顶点导入后应用变换：
```cpp
if (transform == "blender_z_up_to_y_up") {
    // Blender (x,y,z) → 算法 (x,z,y)
    // 即: new_x = old_x, new_y = old_z, new_z = old_y
    for (auto& v : vertices) {
        double tmp = v.y;
        v.y = v.z;
        v.z = tmp;
    }
    // 法向同样变换
}
```

**注意**：Tx/Rx 坐标始终被视为"算法坐标系"中的值，不受此变换影响。

**涉及文件**：
- `core/common/config/AppConfig.h` — 新增字段
- `core/common/config/AppConfigJsonCodec.cpp` — 读写字段
- `preprocess/import/OBJImporter.cpp` — 应用变换

#### 6A-3：Jones 矢量极化（v4 C1 已知限制）

**来源**：C1 文档化限制
**优先级**：中
**论文必要性**：中（学术严谨性 vs 工程实用性权衡）

**⚠️ 此为关键设计决策，需用户确认。** 详见 §14.1 DD1。

**方案 A（实现）**：~3-5 天工作量

核心改动 `FieldAccumulator.h`：
```cpp
// 旧（v4）
double amplitude_real, amplitude_imag;  // 标量复振幅
Vec3 polarization_vector;               // 实向量极化方向

// 新（v5 D6-A）  
double amplitude_real, amplitude_imag;  // 标量复振幅（兼容）
Vec3 polarization_vector;               // 实向量（兼容）
Complex pol_x, pol_y, pol_z;            // Jones 矢量（新增）
```

所有 Apply* 函数需同步修改 TE/TM 投影为复投影：
```cpp
// 旧：实投影
double pTE = Dot(field.polarization_vector, eTE);

// 新：复投影
Complex pTE = Complex(Dot(field.polarization_vector, eTE), 
                      Dot(field.polarization_imag_vector, eTE));
```

**影响范围**：FieldAccumulator.h + 6个 Apply* 文件 + FinalizeAtReceiver + InitializeTxField

**方案 B（记录为局限性）**：0 天

论文中论证：室内低损耗材料 tanδ<0.1，Fresnel TE/TM 相位差通常 < 20°，椭圆极化效应在工程上不显著。此简化与 Wireless InSite 等商业软件同级。

**建议**：v5 启动时采用方案 B。D1 审计中若发现 TE/TM 相位差显著影响功率（>0.5dB 的场景），则升级为方案 A。

#### 6A-2/4/5/6：推迟/跳过项

| # | 项目 | 处理方式 |
|---|------|---------|
| 6A-2 | 面元级材质绑定 | 论文"局限性"记录：OBJ 格式限制 |
| 6A-4 | 漫散射 | 论文"未来工作" |
| 6A-5 | SBR确定性绕射 | 论文"已知限制"：概率采样已可用 |
| 6A-6 | 坐标自动检测 | 跳过：改进文档说明替代 |

### 8.3 D6-B：论文实验数据生成

基于 D2 确定的缺口清单，生成论文所需的全部实验数据。预判实验列表：

| # | 实验 | 方法 | 参数 | 产出 |
|---|------|------|------|------|
| E1 | 路径损耗 vs 距离 | SBR meeting.obj，多 Tx-Rx 对 1~20m | LOS+NLOS 各 8+ 距离点 | path_loss_vs_distance.png |
| E2 | RMS 时延扩展分布 | meeting.obj 20+ Rx 点 | precise 模式 full depth | rms_delay_spread_heatmap.png |
| E3 | SBR 收敛性分析 | 1K/2K/5K/10K/20K/50K/100K 射线 | 5 个代表性 Rx 位置 | convergence_power_vs_rays.png |
| E4 | 绕射损耗 vs 楔角 | 参数扫描 30°/60°/90°/120°/150° | PEC wedge, 单绕射 | diffraction_loss_vs_wedge.png |
| E5 | 透射损耗 vs 频率 | 1/2.4/3.5/6/28 GHz | Glass 6.31ε_r | transmission_loss_vs_freq.png |
| E6 | 极化鉴别 | Tx水平→Rx水平+垂直 | 单反射 Concrete | polarization_discrimination.csv |
| E7 | 覆盖热力图 | SBR 50K rays, 0.1m grid, 2.4GHz | meeting.obj 10×8m | coverage_heatmap.png |
| E8 | ISAC 特征 | 感知SINR+通信容量 | 如论文范围包含 | isac_features.csv |

每个实验配独立配置 JSON → 存入 `experiments/` 目录，含 config_snapshot 确保可复现。

### 8.4 涉及文件

| 文件 | 修改 |
|------|------|
| `preprocess/import/OBJImporter.cpp/.h` | D6-A 坐标变换 |
| `core/common/config/AppConfig.h` | 新增 `coordinate_transform` |
| `core/common/config/AppConfigJsonCodec.cpp` | 新增字段读写 |
| `core/em/FieldAccumulator.h` | D6-A Jones矢量（可选） |
| `core/em/ApplyReflectionInteraction.cpp` | D6-A Jones矢量（可选） |
| `core/em/ApplyTransmissionInteraction.cpp` | D6-A Jones矢量（可选） |
| `core/em/ApplyDiffractionInteraction.cpp` | D6-A Jones矢量（可选） |
| `core/em/FinalizeAtReceiver.cpp` | D6-A Jones矢量（可选） |
| `core/em/InitializeTxField.cpp` | D6-A Jones矢量（可选） |
| `experiments/*.json` (8+) | D6-B 新建实验配置 |

### 8.5 D6 状态

**当前状态：待执行（依赖 D2 确定精确缺口清单）。**

---

## 第9章：D7 — P0 加速优化（需求5-P0）

### 9.1 批次定位

D7 实现最高收益/最低风险的加速优化，直接影响 D6-B 实验数据生成的 wall-clock 时间。SBR 是瓶颈（50K 射线 + 多 Rx = 数分钟至数十分钟）。

### 9.2 D7-A：OpenMP 使能与验证

#### 当前状态

SbrEngine.cpp 已实现 `#pragma omp parallel for schedule(dynamic)`，但由 `RT_ENABLE_OPENMP` 宏保护。当前宏的状态未知。

#### 执行计划

1. **确认编译环境**：检查 VS2022 项目属性中 `/openmp` 开关
2. **启用 OpenMP**：在 Release 配置中添加 `RT_ENABLE_OPENMP` 预处理器定义
3. **测试加速比**：
   - 场景：meeting_coverage.json (20K rays, 8 Rx)
   - 线程数：1/2/4/8/16
   - 预期：8线程时 wall time ≤ 单线程的 25%
4. **检查线程安全**：
   - `omp critical` 保护的 Rx 写入路径
   - trace_lines 是否在并行区内被写入
   - 随机数生成器是否线程安全（如果是 shared 状态）
5. **扩展到 BVH 构建**：
   - SAH bin 计数在 FaceBVHBuilder 中可并行
   - 添加 `#pragma omp parallel for` 在 3-axis SAH 评估循环

#### 预期收益

| 场景 | v4 单线程 | D7 8线程 | 加速比 |
|------|----------|---------|--------|
| meeting_coverage (20K rays, 8 Rx) | ~5s | ~1.2s | ~4× |
| meeting_coverage (100K rays, 100 Rx) | ~60s | ~15s | ~4× |
| meeting_coverage (1M rays, 10K Rx) | ~600s | ~150s | ~4× |

### 9.3 D7-B：SAH 阈值精调

#### 参数扫描

对 `bvh_leaf_size` 进行参数扫描：{2, 4, 8, 12, 16, 24, 32, 48, 64}

对每个值测量：
- BVH 构建时间
- BVH 节点总数
- 平均射线-三角面求交次数（1000 条随机射线）
- 单条 SBR 射线平均 BVH 遍历步数

目标：找到 meeting.obj (~10K面) 和 b1 场景 (~12面) 的最优 leaf_size。

#### 自适应启发式

如果最优值因场景面数而异，实现自适应公式：
```cpp
int auto_leaf_size = std::max(4, std::min(32, total_faces / 200));
```

#### 预期收益

BVH 遍历次数相对 v4 基线减少 ~10%。

### 9.4 D7 状态

**当前状态：待执行（可在 D4 完成后并行开始）。**

---

## 第10章：D8 — P1 加速优化（需求5-P1）

### 10.1 批次定位

D8 实现中等收益的加速优化。P1 优先级：非阻塞，论文可引用也可不引用。

### 10.2 D8-A：SIMD 向量化 Fresnel 积分

**目标**：绕射计算 ~2× 加速

**位置**：`ApplyDiffractionInteraction.cpp` → `FresnelIntegralTailNumerical`

当前 8-point Gauss-Legendre 求和是标量 double 循环。可向量化：4 个积分点同时计算。

```cpp
#ifdef __AVX2__
#include <immintrin.h>
// AVX2: 4 doubles per __m256d
__m256d FresnelIntegralTail_AVX2(double tau0, double step);
#endif
```

**回退**：`#else` 保留当前标量实现，确保无 AVX2 的机器可编译运行。

### 10.3 D8-B：SBR Ray-Sphere 早期剔除

**目标**：SBR 总时间 ~1.2~1.5× 加速

两个优化：

**1. 射线功率阈值提前终止**
```cpp
// 每步后检查：剩余功率 * max_possible_gain < detection_threshold → break
if (rayPower * maxReflectionGain < cfg.ray_power_threshold_linear) break;
```

**2. Rx 方向锥剔除（可选）**
```cpp
// 为每个 Rx 预计算可见方向锥
// 如果射线方向与 Rx 锥无交集 → 跳过该 Rx 的球体检查
```

但 D8-B 仅当 profiling 显示 Rx 检查占总时间 > 10% 时才实现。

### 10.4 D8 状态

**当前状态：待执行（依赖 D7，非关键路径）。**

---

## 第11章：D9 — P2 加速优化（需求5-P2）

### 11.1 批次定位

D9 只出**设计文档**，不出代码（除非时间特别充裕）。论文"未来工作"章节直接引用 D9 的设计结论。

### 11.2 D9-A：GPU 加速架构设计

产出 `document/v5/design_D9_GPU_acceleration.md`：

1. **分析 SBR 数据并行模式**：射线独立、BVH 只读、Rx 网格只读
2. **设计 OptiX 方案**：
   - 用 OptiX 内置 BVH 替代 CPU FaceBVHBuilder
   - CUDA kernel：射线生成 + 反射/透射 spawn + Rx 捕获
   - 内存模型：场景数据一次上传 VRAM，射线批量流式处理
3. **设计 CUDA 方案**（如果无 OptiX 许可）：
   - 自实现 GPU BVH 遍历
   - Thrust 库加速
4. **估算加速比**：保守 50-100×（文献 + OptiX benchmark）

### 11.3 D9-B：场景 LOD 设计

产出 `document/v5/design_D9_scene_LOD.md`：

1. 边折叠简化（Garland & Heckbert 1997）用于远距离几何
2. BVH LOD：粗糙 BVH 用于远处射线
3. 材质简化：远处面使用平均材质属性

### 11.4 D9 状态

**当前状态：待执行（依赖 D7/D8 完成，非关键路径）。**

---

## 第12章：D10 — v5 全量回归 + 论文验证章节定稿

### 12.1 批次定位

D10 是 v5 最终收口，确保全部工作可验证、可交付、可直接支撑论文。

### 12.2 回归验证

运行全部配置（v4 回归 + v5 新增实验配置）：

| 类别 | 配置数 | 说明 |
|------|--------|------|
| v4 回归 | 5 | b1/a3/b4/meeting_v3/meeting_coverage |
| D6-B 实验 | 8+ | 论文实验数据生成配置 |
| D3/D4 对标 | 4+ | 参考实现对比场景 |
| **合计** | **17+** | |

### 12.3 L1/L2/L3 最终验证

运行 `test/validate/run_all.py` → 产出 `validate_report_v5.json`

| 级别 | 测试数 | 目标 |
|------|--------|------|
| L1 解析解 | 5 (基线) + 扩展 | 新增 UTD 楔 L1-4/L1-5（如 D1 审计发现需要） |
| L2 交叉验证 | 2 → 扩展到 4+ | 加入参考实现对比 L2-3 和 L2-4 |
| L3 统计验证 | 2 → 扩展到 4+ | 加入路径损耗指数 L3-3 和 PDP 相似度 L3-4 |

### 12.4 论文产出整理

```
document/
├── thesis/
│   ├── figures/                     # 论文级图表（300dpi, 规范字体）
│   │   ├── path_loss_vs_distance.png
│   │   ├── pdp_comparison.png
│   │   ├── convergence_analysis.png
│   │   ├── diffraction_loss.png
│   │   ├── transmission_loss.png
│   │   ├── coverage_heatmap.png
│   │   ├── rms_delay_spread.png
│   │   └── speedup_scaling.png
│   ├── tables/                      # 论文级表格
│   │   ├── comparison_ref_impl.tex
│   │   ├── requirement_status.tex
│   │   └── validation_summary.tex
│   └── chapter_outline.md           # 算法验证章+仿真结果章大纲
├── v5/
│   ├── audit_D1_理论审计报告.md
│   ├── 对照_D2_开题需求全量对照.md
│   ├── design_D9_GPU_acceleration.md
│   └── design_D9_scene_LOD.md
└── RT算法优化开发文档 v5.md         # 本文档（开发记录）
```

### 12.5 D10 状态

**当前状态：待执行（依赖所有前置批次完成）。**

---

## 第13章：风险矩阵

| 风险 | 概率 | 影响 | 缓解措施 |
|------|------|------|---------|
| D1 审计发现 Critical bug 需要重大重构 | 中 | 高 | 仅修 Critical；Medium 记录为论文局限性 |
| 参考实现无法用我们的场景格式运行 | 低 | 高 | 回退到概念对比（简单场景，按机制逐项对比） |
| Sionna RT 场景格式根本不兼容 | 中 | 低 | 概念对齐，不追求精确对比 |
| OpenMP 编译问题 | 低 | 高 | v4 C2 已测试，D7 仅启用已有宏 |
| D6 Jones 矢量实现级联影响过大 | 中 | 中 | 分阶段：先方案 B 记录局限性，时间允许再补方案 A |
| 实验数据生成时间超过 24h | 中 | 中 | 降低网格分辨率/射线数；论文注明参数 |
| GPU 原型需 CUDA 工具链与 VS2022 冲突 | 中 | 低 | D9 仅出设计文档 |
| v5 过程中 v4 回归衰退 | 低 | 高 | D0 基线记录 + 每个编码批次后重跑回归 |

---

## 第14章：关键设计决策

以下四个决策影响多个批次，需在 v5 启动前确认。

### 14.1 DD1：Jones 矢量极化 — ✅ 已确认：实现方案 A

**决策**：实现 Jones 矢量极化。ISAC 领域涉及交叉极化鉴别（XPD）和极化域分析，实向量近似不可接受。

**定量依据**（Concrete @ 2GHz, 45° 斜入射）：
- Γ_TE 幅度 0.51，相位 ~177°
- Γ_TM 幅度 0.26，相位 ~-6°
- TE/TM 幅度差 2×，相位差 ~180° → 强椭圆极化
- 多反射路径（depth≥2）在新 TE'/TM' 基上的投影显著失真

**实现范围**：FieldAccumulator + 6 个 Apply*/Finalize/Initialize 文件
**工作量**：~3-5 天
**批次归属**：D6-A

### 14.2 DD2：面元级材质绑定 — ✅ 已确认：保留 Object 级，推迟至 v6+

**决策**：保留 Object 级材质绑定。Face 结构体已支持逐面元材质字段（`front_material_name` 等），但绑定管线无逐面元覆写机制，且 OBJ 格式本身不支持逐面元材质。

**论文处理**：在"研究局限性"中记录为 OBJ 格式固有局限，如需逐面元材质需升级 FBX/glTF 导入（v6+）。

### 14.3 DD3：SBR 绕射 — ✅ 已确认：保留概率采样 + 替换功率模型

**决策**：保留概率采样框架（伪随机楔边 + Keller 锥 4 方向采样），但替换固定 5% 功率分配为三因子简化 UTD 功率模型。

**功率模型升级**（归入 D6-A）：
- 因子 1：球面扩散 `1/(s₁·s₂·(s₁+s₂))` — 主导项，Rx 间可差 100×
- 因子 2：楔角 `1/n²`，n = (360°-α)/180° — 次导项，5-10×
- 因子 3：Keller 锥角 `1/sin²β₀` — 掠射抑制
- 基准校准：PEC 90°楔 s₁=s₂=1m β₀=90° → 2%（precise 模式 UTD 验证值）
- 性能：~20 FLOP/命中，零额外查表

**论文处理**：precise 模式 = 确定性精确绕射，SBR 模式 = 统计覆盖。双模互补作为方法论优势论述。

### 14.4 DD4：坐标自动检测 — ✅ 已确认：跳过，采用建模规范约定方案

**决策**：不实现启发式自动检测。采用"约定优于检测"方案：
- Blender 导出时在 OBJ 文件头注释标注向上轴和前进轴
- `coordinate_transform` 字段默认为 `"none"`，用户显式配置
- 新建 `document/建模规范与导出清单.md` 标准化建模流程

**论文处理**：无需提及（不属于算法贡献）。

---

## 第15章：v5 全量总览与收口

### 15.1 批次全景图

```
D0  冻结基线 + 变更边界              ── ✅ P0 只读
│
├─ D1  全量理论审计 (14项)            ── ✅ P0 只读
├─ D2  开题报告全量对照 (13项)        ── ✅ P0 只读
│
├─ D3  三级对标验证体系搭建            ── ✅ P0 Python
├─ D4  三级对标验证执行与数据分析       ── ✅ P0 Python+仿真
│
├─ D5  全量中文注释规范化              ── ✅ P1 贯穿
├─ D6  v4遗留补全 + 论文实验数据       ── ✅ P1 C+++配置
│
├─ D7  OpenMP使能 + SAH阈值精调       ── ✅ P1 C++
├─ D8  SIMD向量化 + 提前剔除           ── P2 C++
├─ D9  GPU/LOD设计文档                ── P2 文档
│
└─ D10  全量回归 + 论文验证章定稿       ── ✅ P1 收口
```

### 15.2 v4 → v5 变化总览

| 维度 | v4 | v5 |
|------|-----|-----|
| 理论审计 | 无系统审计 | **14项逐行对照 Balanis/K&P** |
| 开题对齐 | 概略对照 | **13项逐项标记+缺口清单** |
| 对标验证 | 框架就绪，无实际数据 | **3级对标全部执行（参考实现+Sionna+3GPP）** |
| 中文注释 | ~60% | **100%** |
| 极化 | 实向量 | **实向量+论文局限性论证**（或 Jones 矢量） |
| 坐标变换 | 无 | **Blender Z-up→Y-up 可选** |
| 加速 | 单线程 | **OpenMP 4~8× + SAH 精调** |
| 论文支撑 | 间接 | **直接生成论文图表+验证证据** |

### 15.3 v5 文档最终说明

本文档（`RT算法优化开发文档 v5.md`）是第五轮开发的完整方案。D0~D10 共 11 个批次，覆盖理论审计、开题对照、对标验证、代码规范、加速优化。

**v5 文档版本：** v1.2（D7并行加速实施中，2026-05-07）

---

## 第16章：大规模场景效率分析（→ v6 定向优化方向）

### 16.1 压力测试场景定义

```
场景: 100K 三角面元, 复杂室内环境
Precise:  depth=12, 5R+5T+2D, 混合路径
Coverage: 1e8 射线, depth=12, 5R+5T+2D, 1e7 Rx 体素
```

### 16.2 逐模块复杂度缩放分析

#### A. 精确寻径 (SearchEngine) — 组合爆炸 (P0, v6 解决)

Image Method + 优先级队列: O(B^D)，B=每步候选面元, D=深度。

| 面元规模 | 每步候选 | D=8 理论路径数 | 实际(签名去重后) |
|---------|---------|--------------|----------------|
| 14 (b1) | ~5 | 5⁸≈390K | 362 |
| ~2K (meeting) | ~20 | 20⁸≈2.5e10 | 664 |
| **100K** | **~500** | **500⁸≈3.9e21** | **不可完成** |

**瓶颈**: 每步候选面元数随场景规模线性增长，导致路径空间指数爆炸。签名去重砍掉 >99.9% 但在 100K 面下仍不可行。

**v6 方向**:
- 候选面元空间过滤（可见性预计算、角度截断、FOV限制）
- 路径功率阈值早停
- 自适应深度限制（大场景浅搜索）

#### B. SBR Rx 哈希 — 1e7 Rx 下退化 (P0, v6 解决)

```
均匀哈希: cell_size = 2×sphereR
1e7 Rx / 场景体积1200m³ / cell_vol=0.216m³ → 每cell ~1800 Rx
每射线步: 27邻域 × 1800 = 48,600 次点-线段距离计算
总计算: 1e8射线 × 12深度 × 48K = 5.76e13 次 → ~16h+
```

**v6 方向**:
- 二级分层哈希（粗网格→细网格）
- Rx-BVH（对 Rx 点构建独立 BVH，射线步直接查询）
- 自适应 cell_size（高密度区域缩小 cell）

#### C. SBR 射线数 — 线性可扩展 (P1, v5-D7 解决)

1e8 射线 / 单核 → 数百小时。OpenMP 20 核 → 数十小时。v5-D7 解决基础并行化。

#### D. 精确寻径内存 — 千万路径内存 (P1, v6 解决)

100K 面场景可能产生 1000 万+ 条路径。每条路径 ~200B → 2GB+。
v6 方向: 路径流式写入磁盘、功率阈值裁剪。

### 16.3 v5→v6 过渡策略

v5 (当前): 处理 10K Rx + 1M 射线规模的正确性和基础加速（OpenMP并行化）
v6 (后续): 处理 1e7 Rx + 1e8 射线规模的大规模算法优化（分层索引、组合爆炸抑制）

v5 仅做：
1. OpenMP 并行（不动算法，纯并行化）
2. 线程私有归并（消除临界区瓶颈）
3. BVH/EM 等可并行循环的 OpenMP 化
4. 自适应核心数检测

不做：
- 算法重构
- 分层空间索引
- GPU 移植
- 候选面元过滤

---

## 附录 A：参考文献

- Balanis, C. A. "Advanced Engineering Electromagnetics" (2nd Ed.), Wiley, 2012. Ch.5, Ch.12
- Born, M. & Wolf, E. "Principles of Optics" (7th Ed.), Cambridge, 1999. Sec.1.5
- Kouyoumjian, R. G. & Pathak, P. H. "A Uniform Geometrical Theory of Diffraction for an Edge in a Perfectly Conducting Surface," Proc. IEEE, vol. 62, pp. 1448–1461, 1974
- Friis, H. T. "A Note on a Simple Transmission Formula," Proc. IRE, vol. 34, pp. 254–256, 1946
- Seidel, S. Y. & Rappaport, T. S. "Site-Specific Propagation Prediction for Wireless In-Building Personal Communication System Design," IEEE Trans. Veh. Technol., vol. 43, pp. 879–891, 1994
- ITU-R P.2040-1 "Effects of Building Materials and Structures on Radiowave Propagation Above About 100 MHz," 2015
- 3GPP TR 38.901 "Study on Channel Model for Frequencies from 0.5 to 100 GHz," v16.1.0, 2019
- MacDonald, J. D. & Booth, K. S. "Heuristics for Ray Tracing Using Space Subdivision," Visual Computer, vol. 6, pp. 153–166, 1990
- Wald, I. "On Fast Construction of SAH-Based Bounding Volume Hierarchies," Eurographics Symp. on Rendering, 2007

## 附录 B：文件地图

### 将被 D5 修改（注释规范化）
```
app/main.cpp, RtPipeline.h/.cpp, A1RealChainRunner.h/.cpp, Batch9ExportReporter.h/.cpp
preprocess/accel/FaceBVHBuilder.cpp, SceneAcceleration.cpp, WedgeAccelerationBuilder.cpp
preprocess/binding/MaterialRuleLoader.cpp, ResolveFaceDualSideMaterial.cpp
preprocess/build/EdgeBuilder.cpp, SceneBatch2Builder.cpp, SceneBatch3Builder.cpp, SceneBatch4Builder.cpp,
                SceneDiagnostics.cpp, SceneMaterialBinding.cpp, WedgeBuilder.cpp
preprocess/cache/SceneCache.cpp
preprocess/import/OBJImporter.cpp
core/path/ (5 files), core/scene/ (6 files), core/query/ (2 files), core/result/ (20 files)
test/rt_utils.py, rt_visual.py, plot_pdp_aps.py, validate/run_all.py, rt_grid_search.py
```

### 将被 D6-A 修改（功能变更）
```
preprocess/import/OBJImporter.cpp/.h         — 坐标变换
core/common/config/AppConfig.h               — coordinate_transform 字段
core/common/config/AppConfigJsonCodec.cpp    — 新字段读写
core/em/FieldAccumulator.h                   — Jones矢量（可选DD1）
core/em/ApplyReflectionInteraction.cpp       — Jones矢量（可选DD1）
core/em/ApplyTransmissionInteraction.cpp     — Jones矢量（可选DD1）
core/em/ApplyDiffractionInteraction.cpp      — Jones矢量（可选DD1）
core/em/FinalizeAtReceiver.cpp               — Jones矢量（可选DD1）
core/em/InitializeTxField.cpp                — Jones矢量（可选DD1）
```

### 将被 D7 修改（加速优化）
```
core/search/SbrEngine.cpp/.h                 — OpenMP验证+早期剔除
preprocess/accel/FaceBVHBuilder.cpp          — SAH精调+OpenMP
RT.vcxproj                                    — 编译开关
```

### 将被 D8 修改（加速优化）
```
core/em/ApplyDiffractionInteraction.cpp      — SIMD Fresnel
core/search/SbrEngine.cpp                    — Ray-Sphere提前剔除
core/common/math/Complex.h                   — SIMD复数运算（如需要）
```

### 新建文件（D3/D4/D6）
```
test/validate/cross_ref/scene_translator.py, cross_runner.py, cross_compare.py
test/validate/cross_sionna/sionna_compare.py
test/validate/cross_stats/path_loss_3gpp.py, rt_vs_3gpp.py
document/v5/audit_D1_理论审计报告.md
document/v5/对照_D2_开题需求全量对照.md
document/v5/design_D9_GPU_acceleration.md
document/v5/design_D9_scene_LOD.md
experiments/*.json (8+ 实验配置)
```
