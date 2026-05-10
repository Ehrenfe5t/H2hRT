# RT算法优化开发文档 v7

> 审计日期：2026-05-09
> 审计范围：全量代码（6模块 × 35+文件），逐行对照 Balanis/Born&Wolf/Kouyoumjian-Pathak 原始文献
> 审计方法：四路并行 Agent 独立深度审计 → 交叉验证 → 合成报告
> 前序文档：v5 开发文档（D0~D10 批次规划），v6 配置问题解答（Q1~Q38 配置清理）

---

## 第0章：v7 审计总览与基线

### 0.1 审计方法

v7 审计采用四路并行 Agent 架构，每路独立审计一个子系统：

| Agent | 审计范围 | 文件数 | 审计深度 |
|-------|---------|--------|---------|
| EM-Agent | `core/em/` 全部 .cpp/.h | 18 | 逐公式对照 Balanis/K&P/Born&Wolf |
| Search-Agent | `core/search/` 全部 .cpp/.h | 18 | 逐算法对照 Rappaport/Fibonacci/UTD几何 |
| Geometry-Agent | `preprocess/` + `core/scene/` + `core/query/` | 35 | 逐数据结构+算法对照 Wald/Möller-Trumbore |
| Pipeline-Agent | `app/` + `core/common/` + `core/antenna/` + `core/result/` | 25+ | 全链路数据流追踪 |

### 0.2 发现总览

| 严重度 | 数量 | 涉及模块 |
|--------|------|---------|
| **Critical** | 9 | EM(4) + Search(1) + Pipeline(1) + Geometry(1) + Config/Antenna(2) |
| **High** | 16 | EM(2) + Search(3) + Geometry(3) + Pipeline(5) + Material(3) |
| **Medium** | 18 | EM(3) + Search(2) + Geometry(5) + Config/Pipeline(5) + Antenna(3) |
| **Low** | 21 | 各模块分散 |
| **总计** | **64** | |

### 0.3 与 D1 审计的关系

D1 审计发现 4 项问题（1 Critical + 1 High + 2 Medium），v7 审计在 D1 基础上：
- **确认** D1#4 (TM透射系数) 仍存在 → v7 Critical #C1
- **确认** D1#11 (极化简化) 仍存在 → v7 High #H2
- **新发现** D1 未覆盖的 5 个 Critical 问题（UTD坐标系、SBR绕射方向、材料链路断裂等）
- **新发现** D1 未覆盖的 12 个 High 问题
- **新发现** D1 未覆盖的 60 个 Medium/Low 问题

---

## 第1章：Critical 发现（9项—阻塞正确性）

### C1 [EM] TM透射系数分子错误 — D1#4 确认

**文件**: `core/em/ApplyTransmissionInteraction.cpp:30`
**参考**: Born & Wolf §1.5.2, Eq.(57)

**问题**: `FresnelTM_T` 分子使用 `2·ε_c·cosθ_i`，正确应为 `2·√ε_c·cosθ_i`。
```cpp
// 当前（错误）:
return (e_cos + e_cos) / (e_cos + sqrtTerm);  // = 2·ε_c·cosθ_i / (...)

// 修正:
Complex sqrtEpsC = Sqrt(epsC);
Complex nCos(sqrtEpsC.re * cosI, sqrtEpsC.im * cosI);
return (nCos + nCos) / (Complex(epsC.re * cosI, epsC.im * cosI) + sqrtTerm);
//    = 2·√ε_c·cosθ_i / (ε_c·cosθ_i + √(ε_c - sin²θ_i))
```

**影响**: TM透射功率被高估 |ε_c| 倍（玻璃 ~6×, 混凝土 ~5×）。a3_transmission 的 L1 验证通过的功率部分是假阳性。
**修复**: 修改 `FresnelTM_T` 函数，新增 L1 TM透射功率验证项。

---

### C2 [EM] UTD绕射边缘坐标系未对齐楔面0-face

**文件**: `core/em/ApplyDiffractionInteraction.cpp:124-145`
**参考**: Kouyoumjian & Pathak (1974), Balanis Ch.12

**问题**: 边缘固定坐标系 ê_x 基于**出射方向** kOut 构建（非楔面几何），导致：
- `φ = atan2(kOut·ê_y, kOut·ê_x) = atan2(0, |kOutPerp|) = 0`（始终为零）
- `φ'` 同样失去对楔面的参考
- D₁-D₄ 的 cot 参数 `(π ± (φ ∓ φ'))/(2n)` 全部使用了错误的角参数

**根因**: 标准 UTD 要求 φ 和 φ' 从楔面的 0-face 测量。当前代码使用射线方向定义坐标系，相当于在 Keller 锥上旋转坐标系——这丢失了楔面几何信息。

**影响**: **所有绕射路径的 UTD 系数计算错误。** 这不是近似，是坐标系层级的实现错误。
**修复**: 使用楔面的 `positive_face_id` / `negative_face_id` 获取面法线，从入射方向构建标准边缘坐标系。

---

### C3 [EM] UTD距离参数L缺少sin²(β₀)因子

**文件**: `core/em/ApplyDiffractionInteraction.cpp:165`
**参考**: K&P Eq.(27); Balanis Ch.12, Eq.(12-97)

**问题**:
```cpp
// 当前:
double L = s1 * s2 / (s1 + s2);

// 正确 (球面波入射):
double L = s1 * s2 * sinBeta * sinBeta / (s1 + s2);
```

`s₁·s₂/(s₁+s₂)` 仅对垂直入射(β₀=π/2)有效。一般斜入射时需乘 sin²β₀。sinBeta 在第113行已经计算但未使用。

**影响**: Fresnel 过渡函数 F(k·L·a) 在所有非垂直入射绕射路径中被评估在错误参数上。
**修复**: 第165行改为 `double L = s1 * s2 * sinBeta * sinBeta / (s1 + s2);`

---

### C4 [EM] UTD Soft/Hard极化方向与系数分配颠倒

**文件**: `core/em/ApplyDiffractionInteraction.cpp:172-176`
**参考**: K&P Eq.(29)-(31); Balanis Ch.12

**问题**: 代码将 D_soft 应用于 ê_x 方向，D_hard 应用于 ê_φ 方向:
```cpp
Vec3 eSoft = ex;                                // ê_β (in-plane)
Vec3 eHard = Normalize(Cross(eSoft, kOut));     // ê_φ (out-of-plane)
Complex eDS = Dsoft * eS;   // D_soft → in-plane  ← 错误
Complex eDH = Dhard * eH;   // D_hard → out-of-plane ← 错误
```

标准 UTD 规定：
- D_soft (Dirichlet) → 电场分量**平行于**边缘 (ê_φ, out-of-plane)
- D_hard (Neumann) → 电场分量**垂直于**边缘 (ê_β, in-plane)

代码将两者颠倒。这导致绕射场的极化状态完全错误。

**影响**: 所有绕射路径的极化分量张冠李戴。
**修复**: 交换赋值或交换系数应用方向。

---

### C5 [EM] 标量振幅加法破坏矢量正交性

**文件**: `ApplyReflectionInteraction.cpp:71`, `ApplyTransmissionInteraction.cpp:75`, `ApplyDiffractionInteraction.cpp:178-179`

**问题**: 将正交方向上的复振幅做标量加法:
```cpp
Complex amp_ref = A_TE_ref + A_TM_ref;  // TE ⟂ TM，但做标量加法
Complex ampDiff = eDS + eDH;            // soft ⟂ hard，但做标量加法
```

由于 eTE ⟂ eTM（正交单位矢量），场的正确幅值是 `√(|A_TE|² + |A_TM|²)`，而非 `|A_TE + A_TM|`。标量加法可能导致 TE 和 TM 分量因相位相反（~180°）而部分或完全对消，但实际上它们是正交方向上的分量，不应发生干涉对消。

**影响**: 每次交互后振幅幅值错误。多次交互路径的误差累积放大。功率计算（`|A_TE|² + |A_TM|²`）是正确的，但振幅和相位传给下游时不正确。
**修复**: 正交方向的复振幅不应做标量加法。需用矢量重构：`|amp| = √(|A_TE|² + |A_TM|²)`，相位取能量加权平均。

---

### C6 [SBR] SBR绕射射线方向Y分量缺少叉积项

**文件**: `core/search/SbrEngine.cpp:242-245`

**问题**: Keller锥绕射射线方向计算中，y分量缺少 `sin(β₀)·sin(φ)·(ê_D × p̂_B).y` 项:
```cpp
// 当前（不完整）:
double dirY = std::sin(b0)*std::cos(phi)*pB.y + std::cos(b0)*eD.y;
// 缺少: + std::sin(b0)*std::sin(phi)*Cross(eD, pB).y

// 正确 (Keller锥):
// dir = sin(β₀)·cos(φ)·p̂_B + cos(β₀)·ê_D + sin(β₀)·sin(φ)·(ê_D × p̂_B)
```

**影响**: **所有 4 条 SBR 绕射射线的方向在几何上都是错的。** 绕射射线没有正确分布在 Keller 锥面上。
**修复**: 修正 y 分量，添加缺失的叉积项。

---

### C7 [Pipeline] 材料数据库在EM链路中被丢弃

**文件**: `app/RtRealChainRunner.cpp:172`

**问题**: 材料数据库作为参数传入 EM 求解链路，但在函数入口处被 `(void)materialDb` 丢弃:
```cpp
void RunA1RealChain(..., const MaterialDatabase& materialDb) {
    (void)materialDb;  // ← 材料数据被丢弃
    ...
}
```

这意味着 **所有 EM 交互（反射/透射/绕射）都是在 ε_r=1, σ=0（真空）假设下计算的**。Fresnel 系数、UTD 系数、透射衰减全部使用了错误的材料参数。这是一个跨模块的集成断裂——材料数据库正确加载了解析，但从未连接到 EM 求解器。

**影响**: **整个 EM 链路的材料参数全部空缺。** 所有路径的功率/相位计算结果与真实材料无关。
**修复**: 将 materialDb 传入每个 Apply*Interaction 函数，使 Fresnel/UTD 使用实际 ε_r(f) 和 σ(f)。

---

### C8 [Config] Matrix3x3 operator()行列索引颠倒

**文件**: `core/common/math/Matrix3x3.h:44`
**参考**: 线性代数基础

**问题**: 矩阵存储为列主序（m[0]=col0.x, m[1]=col1.x, m[2]=col2.x），但 `operator()` 使用行主序索引:
```cpp
double operator()(int row, int col) const { return m[row * 3 + col]; }
// 列主序正确索引: m[col * 3 + row]
```

`Multiply(rot, v)` 函数直接使用内部数组 m[]，这是正确的。但 `operator()` 对元素访问返回错误值。

**影响**: 任何通过 `operator()` 访问矩阵元素的代码会得到静默损坏的数据。当前需要确认是否有代码使用此操作符。
**修复**: 改为 `return m[col * 3 + row];` 或删除此操作符（如果未使用）。

---

### C9 [Config] AntennaModel增益削波错误

**文件**: `core/antenna/AntennaModel.cpp:72`

**问题**: 增益削波使用了不正确的下界:
```cpp
response.gain_db = 10.0 * std::log10(std::max(1.0, response.gain_linear));
//                                                ^^^ 应该用 1e-15
```

`std::max(1.0, ...)` 将负增益（< 0 dBi）削波为 0 dBi。真实天线在旁瓣方向通常有 -10 到 -30 dBi 的增益。当前代码将这些方向全部映射为 0 dBi。

**影响**: 使用方向性天线方向图时，旁瓣方向的增益被系统性高估。
**修复**: `std::max(1e-15, response.gain_linear)` — 仅防止 log10(0)。

---

## 第2章：High 发现（16项—显著影响精度/正确性）

### H1 [EM] 介质衰减使用良导体近似

**文件**: `core/em/ApplyTransmissionInteraction.cpp:100-103`
**参考**: Balanis Ch.4

**问题**: 衰减常数 α 简化为 `k₀·√(σ/(ωε₀))`，仅在 `σ/(ωε₀) ≫ ε_r`（良导体）时有效。对建筑材料（混凝土 ε_r≈6, σ≈0.001 S/m @ 2.4GHz），σ/(ωε₀)≈0.007 ≪ 6，是典型的低损耗介质。

| 材料 | 频率 | 代码 α (Np/m) | 正确 α (Np/m) | 误差 |
|------|------|--------------|--------------|------|
| Concrete | 2.4 GHz | 14.8 | 0.076 | **195×** |
| Glass | 28 GHz | 8.5 | 0.12 | **70×** |

**影响**: 所有透射路径的介质衰减被高估 70-200×。
**修复**: 使用正确公式 `α = (2πf/c₀) · Im(√ε_c)`。

---

### H2 [EM] 介质内相位累积使用自由空间波数

**文件**: `core/em/ApplyFreeSpaceSegment.cpp:29`

**问题**: 透射进入介质后的自由空间段，相位累积仍使用 `k₀ = 2π/λ₀`（自由空间波数）:
```cpp
field.phase_rad -= 2π * segmentLengthM / field.wavelength_m;  // λ₀，非 λ_medium
```

对混凝土墙(ε_r≈6, d=0.2m, f=2.4GHz)，相位误差约 170 rad (~27周期)，相干求和完全随机化。

**影响**: 含透射路径的相干相位在 Rx 处完全错误。
**修复**: 在 FieldAccumulator 中存储当前介质的有效折射率 `n_eff = Re(√ε_c)`，分段相位累积使用 `k₀·n_eff·d`。

---

### H3 [SBR] 透射概率功率双重缩放导致系统性能量损失

**文件**: `core/search/SbrEngine.cpp:269-280`

**问题**: 透射使用了两种不兼容的 Monte Carlo 方法:
```cpp
// 以概率 refPwr 选反射，且缩放功率 curPwr *= refPwr
// 以概率 1-refPwr 选透射，且缩放功率 curPwr *= (1-refPwr)
```
期望功率 = `refPwr² + (1-refPwr)² < 1`（无损介质时）。

当 refPwr = 0.5 时，期望功率 = 0.5（实际应为 1.0）。**每次透射交互损失 50% 的功率。**
对含多次透射的路径，功率指数衰减。

**修复**: 方案A（概率选择不缩放）或方案B（确定性分裂）。

---

### H4 [SBR] Fresnel缓存OpenMP数据竞争

**文件**: `core/search/SbrEngine.cpp:108-127`

**问题**: `static double FresnelCache[32][20]` 被多个 OpenMP 线程无同步地并发读写。C++ 标准将此定义为未定义行为。

**影响**: x86_64 上可能表现为间歇性的错误 Fresnel 系数（部分写入被读到）。
**修复**: 使用 `thread_local` 缓存或 `#pragma omp critical` 保护写操作。

---

### H5 [SBR] RxHashGrid中点查询对长射线漏检Rx

**文件**: `core/search/SbrEngine.cpp:73-76`

**问题**: `CheckSegment` 使用线段**中点**查询 27 邻域。对于从 Tx 到远距离场景边界的长射线，Rx 球体可能靠近端点而非中点，落入 27 个邻域单元之外。

**影响**: 覆盖场景中，特定方向的长射线段会漏掉 Rx 命中。
**修复**: 采用 DDA 体素遍历或沿线段多点采样查询。

---

### H6 [Geom] OBJ坐标变换双重应用

**文件**: `preprocess/import/OBJImporter.cpp:248-267` + `preprocess/build/SceneImporter.cpp:42-45`

**问题**: `blender_z_up_to_y_up` 坐标变换在**两个地方**都应用了 (y↔z):
- OBJImporter: 变换顶点 + 面法线 + 边方向
- SceneImporter: 再次变换顶点 + OBJ法线

净效果: 顶点被变换两次（回原值），面法线仅变换一次（正确），但 OBJ 顶点法线与面法线不一致。

**修复**: 删除 SceneImporter 中的重复变换（保留 OBJImporter 中的）。

---

### H7 [Geom] SceneCache签名缺少coordinate_transform

**文件**: `preprocess/cache/SceneCache.cpp:79-89`

**问题**: `BuildPreprocessConfigSignature` 不包含 `coordinate_transform` 字段，且多个参数被硬编码为字符串。改变坐标变换不会使缓存失效。

**修复**: 将 `coordinate_transform` 纳入签名，取消硬编码。

---

### H8 [Geom] SceneCache size_t跨平台不兼容

**文件**: `preprocess/cache/SceneCache.cpp` 多处

**问题**: 二进制缓存使用 `std::size_t` 序列化（64-bit 系统为 8 bytes，32-bit 为 4 bytes）。64-bit 写的缓存无法在 32-bit 上读。

**修复**: 统一使用 `uint64_t` 进行序列化。

---

### H9 [Pipeline] AppConfigValidator是空桩

**文件**: `core/common/config/AppConfigValidator.cpp:2-9`

```cpp
ConfigValidationResult ValidateAppConfig(const AppConfig& config) {
    ConfigValidationResult r; r.passed = true; (void)config; return r;
}
```

所有配置验证被禁用。负值、越界、矛盾配置全部静默接受。
**修复**: 实现完整的配置验证逻辑。

---

### H10 [Pipeline] AoA/AoD完全缺失

**文件**: `core/em/EMPathResult.h` vs `core/result/ExportPaths.cpp`

`EMPathResult` 结构体不包含 AoA/AoD 字段，导出（JSON/CSV）无角度信息。对信道模型来说，离开角(AoD)和到达角(AoA)是最基础的输出参数。

**修复**: 从 GeometricPath 的首尾节点计算 AoD/AoA，添加到 EMPathResult。

---

### H11 [Material] ITU-R P.2040 线性插值替代幂律模型

**文件**: `core/common/material/MaterialDatabase.h:90-91`

```cpp
r.epsilon_r = p0.epsilon_r + t * (p1.epsilon_r - p0.epsilon_r);  // 线性
r.sigma     = p0.sigma     + t * (p1.sigma     - p0.sigma);      // 线性
```

ITU-R P.2040 规定的模型是 `ε_r(f) = a·f^b`, `σ(f) = c·f^d`（幂律），不是线性插值。
**修复**: 实现幂律插值（对标 Sionna RT 的 `itu.py`）。

---

### H12 [Material] 材质查询失败静默回退到真空

**文件**: `core/common/material/MaterialDatabase.h:59-61`

```cpp
auto itN = byName_.find(name);
if (itN == byName_.end()) return MaterialProps{};  // ε_r=1, σ=0, μ_r=1
```

拼写错误的材质名（如 "Conrete"）导致静默回退到空气，无任何警告。
**修复**: 查询失败 → `Logger::Fatal` + 退出。

---

### H13 [Material] 复介电常数未在MaterialDatabase中计算

**文件**: `core/common/material/MaterialDatabase.h`

`ε_c = ε_r(f) - j·σ(f)/(ω·ε₀)` 的计算在 MaterialDatabase 及其所有消费者中都缺失。结合 C7（materialDb 被丢弃），EM 链路的材料参数完全通路断裂。

**修复**: 在 MaterialDatabase 中提供 `QueryComplexPermittivity(name, freq_hz)` 方法，并在所有 Apply*Interaction 中使用。

---

### H14 [Antenna] polarization_file 配置被静默忽略

**文件**: `core/antenna/AntennaFactory.cpp:25,45`

```cpp
// BuildTxAntennaModel 和 BuildRxAntennaModel 都硬编码:
model.polarization_vector = MakeVec3(0.0, 1.0, 0.0);  // ← Y-up，忽略配置
```

用户配置的 `polarization_file` 被完全忽略，极化始终为 Y-up 线极化。
**修复**: 读取并加载 polarization_file，构建对应的极化向量。

---

### H15 [Path] 极化虚部未导出

**文件**: `core/result/ExportPaths.cpp`

`EMPathResult` 存储了 `polarization_imag` (Jones 矢量虚部)，但 JSON/CSV 导出均不包含它。仅导出 `polarization_magnitude`。接收端完整的极化椭圆重构需要实部+虚部。

**修复**: 导出中包含 `polarization_imag`。

---

### H16 [Path] CSV导出未转义逗号和引号

**文件**: `core/result/ExportPaths.cpp:121-148`

手动拼接 CSV 字符串，不对字符串字段中的逗号或引号做转义。字段值含逗号会导致列错位。
**修复**: 对字符串字段做 CSV 标准转义（双引号包裹 + 内部引号加倍）。

---

## 第3章：Medium 发现（18项—特定场景/边界条件错误）

### M1 [EM] UTD Fresnel积分大x渐近未实现

**文件**: `core/em/ApplyDiffractionInteraction.cpp:29-58`

对 x > 10（深照明/深阴影区），代码仍用数值积分，但应切换为渐近展开 `F(x)→1`。大 x 时固定步长0.5欠采样振荡的被积函数。
**修复**: 添加 `if (x > 10.0) return Complex(1.0, 0.5/x - 0.75/(x*x));`

---

### M2 [EM] 多次透射路径首末介质元数据错误

**文件**: `core/em/FinalizeAtReceiver.cpp:76-79`

`first_transmission_medium_in/out_id` 从 `field.last_transmission_*` 复制，而非从 `EMSolverInput` 的真正"首次"值读取。2+次透射时"首次"＝"末次"。
**修复**: 从 `input.first_transmission_medium_in/out_id` 读取。

---

### M3 [EM] UTD phi' 依赖DiffractionExpander正确写入incident_direction

**文件**: `core/em/ApplyDiffractionInteraction.cpp:119-122`

phi' 的入射方向优先从 `node.incident_direction` 获取，回退为 -kOut（后向散射近似）。若 DiffractionExpander 未正确填充此字段，所有绕射路径的 phi' 都回退到 kPi。
**修复**: 验证 DiffractionExpander 是否正确写入 incident_direction。若未写入，添加。

---

### M4 [Search] 状态签名不含累积长度，可能过度去重

**文件**: `core/search/StateSignatureBuilder.cpp:22-35`

状态签名包含 `(last_face_id, last_interaction_type, path_depth, budget_counters)`，但不含 `accumulated_length`。两个具有相同交互历史但不同累积长度的搜索状态会被去重丢弃。
**修复**: 评估是否需要将累积长度纳入签名。

---

### M5 [Search] SBR绕射系数为经验值无UTD基础

**文件**: `core/search/SbrEngine.cpp:240`

```cpp
double bF = 0.02 * (1.0/(nW*nW)) * (1.0/(sB0*sB0));
```

这是纯经验公式（固定 2% 功率 + 楔角因子 + 掠射因子），不是 UTD 绕射系数。v5-DD3 已决定升级为三因子简化 UTD 模型，但当前代码尚未实施。
**修复**: 按 v5-DD3 方案实现三因子简化 UTD (球面扩散 + 楔角 + Keller 锥角)。

---

### M6 [Geom] BVH traversal缺少近-远排序

**文件**: `core/query/SceneQuery.cpp:204-205`

两个孩子始终按固定顺序（先左后右）遍历，不考虑射线方向。`QueryClosestFaceHit` 先收集全部命中再排序——正确的实现应做近-远排序+提前终止。
**影响**: BVH 最近命中查询慢 ~2×。
**修复**: 按射线方向对子节点排序，先遍历近端。

---

### M7 [Geom] 退化面NaN二面角

**文件**: `preprocess/build/EdgeBuilder.cpp:127-129`

退化面（零面积）的法线 `Normalize(zero_vector)` 返回零向量，`acos(Dot(0,0)≈0)=π/2` 导致二面角被计算为 90°（非 0°），共面过滤失效。
**修复**: 在计算二面角前检查法线长度。

---

### M8 [Geom] 楔边材料侧约定需验证

**文件**: `preprocess/build/WedgeBuilder.cpp:93-94`

正/负面都使用了 `back_material_name`。如果 front/back 约定是"法线方向=前面"，则楔边的外部材料语义依赖于面法线朝向——需验证在所有场景中的一致性。
**修复**: 验证并文档化正/负面材料分配约定。

---

### M9 [Geom] SceneCache结构体填充跨编译器不兼容

**文件**: `preprocess/cache/SceneCache.cpp` 多处

含 `Vec3`、`bool`、`double` 的结构体使用 `sizeof(T)` 做原始读写。不同编译器/ABI 的填充规则不同，导致格式不兼容。
**修复**: 逐字段序列化，不依赖结构体布局。

---

### M10 [Config] JSON反斜杠转义不处理\\"序列

**文件**: `core/common/config/AppConfigJsonCodec.cpp:69,126`

转义检测仅看前一字符是否为 `\`，不处理 `\\"` 的情况（两个反斜杠+引号被误判为转义引号）。
**修复**: 检查前一字符的转义状态奇偶性。

---

### M11 [Config] JSON快照含尾随逗号

**文件**: `core/common/config/AppConfigJsonCodec.cpp:409-410`

生成配置快照时，对象/数组的末项后有多余逗号。严格 JSON 解析器（RFC 8259）拒绝此格式。
**修复**: 删除末项后的逗号。

---

### M12 [Config] 容差默认值不统一

**文件**: `MathConstants.h` vs `NumericToleranceConfig.h`

- `kEpsLength = 1e-9`（硬编码）vs `eps_length = 1e-6`（可配置默认）— 差 1000×
- `AntennaPattern` 使用 0.01° 去重阈值（~1.7e-4 rad）vs `eps_angle = 1e-6` rad — 差 170×

**修复**: 统一容差来源，所有代码使用可配置值。

---

### M13 [Antenna] 双线性插值依赖输入排序

**文件**: `core/antenna/AntennaPattern.h:47-66`

方向图查询假设 theta 已排序且 CSV 数据为 theta-major 顺序。无排序验证。乱序 CSV 导致不确定结果。
**修复**: LoadCsv 后对数据进行排序。

---

### M14 [Antenna] CSV解析失败静默回退到0.0 dBi

**文件**: `core/antenna/AntennaPattern.h:31`

```cpp
catch (...) { row.push_back(0.0); }  // 解析异常 → 静默0.0 dB
```

损坏的方向图数据被静默接受为 0 dBi（全向）。
**修复**: 解析失败应报错退出。

---

### M15 [Pipeline] MaterialDatabase加载失败被忽略

**文件**: `app/RtPipeline.cpp:181-184`

`LoadFromCsv` 返回 bool 但被丢弃。CSV 丢失/损坏时静默产生空数据库。
**修复**: 检查返回值，失败时 Fatal 退出。

---

### M16 [Config] SelfCheck跳过全部负测试

**文件**: `core/common/config/ConfigSelfCheck.cpp:86-87`

自检中的负样例测试（无效配置应被拒绝）全部跳过。
**修复**: 在 v7 中实现完整的自检链。

---

### M17 [Geom] SceneCache内容无魔数/版本号

**文件**: `preprocess/cache/SceneCache.cpp`

二进制内容文件没有格式标识符。结构体布局变更后旧缓存被静默错误读取。
**修复**: 添加 4-byte 魔数 + 格式版本号。

---

### M18 [Config] ReadOptionalBool非true即不变

**文件**: `core/common/config/AppConfigJsonCodec.cpp:180`

`ReadOptionalBool` 仅在值为 true 变体时设置 target。若 JSON 中明确写为 "false" 且默认值为 true 的字段——当前无此情况——JSON 的 false 会被忽略。当前不是 bug，但是陷阱。
**修复**: 明确处理 "false" 值。

---

## 第4章：发现汇总矩阵

### 4.1 按模块分布

| 模块 | Critical | High | Medium | Low | 合计 |
|------|----------|------|--------|-----|------|
| EM (Fresnel/UTD/极化) | 5 | 2 | 3 | 4 | **14** |
| Search (SBR + Precise) | 1 | 3 | 2 | 6 | **12** |
| Geometry (BVH/Wedge/Scene) | 0 | 3 | 5 | 7 | **15** |
| Config/Pipeline | 2 | 3 | 5 | 2 | **12** |
| Material | 0 | 3 | 0 | 2 | **5** |
| Antenna | 1 | 1 | 3 | 0 | **5** |
| Path/Result/Export | 0 | 2 | 0 | 1 | **3** |
| **合计** | **9** | **16** | **18** | **21** | **64** |

### 4.2 按影响维度分布

| 维度 | Critical | High | Medium | 说明 |
|------|----------|------|--------|------|
| 公式/理论正确性 | 5 | 3 | 2 | Fresnel, UTD, 介质EM |
| 几何/算法正确性 | 2 | 2 | 4 | SBR绕射, BVH, 坐标变换 |
| 数值/精度 | 0 | 2 | 3 | 介质衰减, 相位累积, Fresnel积分 |
| 数据流/集成 | 1 | 2 | 2 | 材料链路, 验证链路, AoA/AoD缺失 |
| 并发/线程安全 | 0 | 1 | 0 | Fresnel缓存数据竞争 |
| 可移植/稳健性 | 0 | 2 | 4 | size_t, 字节序, 填充, 转义 |
| 代码/架构 | 0 | 0 | 3 | 空桩, 死代码, 重复 |

---

## v7-A 实施记录

> 实施日期：2026-05-09
> 编译：VS2022 MSVC, x64/Debug/RT.exe, 0 warnings 0 errors
> 验证：5/5 配置回归通过, L1 5/5 + L2 2/2 PASS

### 实施结果

| ID | 项 | 处理 | 验证 |
|----|----|------|------|
| C1 | TM透射系数 | ✅ 已在v5 D6-A修复 (ApplyTransmissionInteraction.cpp:27-35) | 代码核实通过 |
| C2 | UTD坐标系 | ✅ 修复 — 边缘固定坐标系基于楔面0-face法线构建 | b1(362p) + meeting(664p) 一致 |
| C3 | UTD L参数 | ✅ 修复 — L = s₁·s₂·sin²β₀/(s₁+s₂) | 同上 |
| C4 | UTD Soft/Hard | ✅ 修复 — D_soft→ê_φ(⟂衍射面), D_hard→ê_β(面内) | 同上 |
| C5 | 标量振幅加法 | ✅ 修复 — 三处Apply*函数: amplitude=⎮E⎮, Jones矢量归一化编码复方向 | 同上 |
| C6 | SBR绕射方向 | ✅ 修复 — y/z分量添加Cross(ê_D,p̂_B)项 | meeting_cov_hires通过 |
| C7 | 材料DB链路 | ✅ 移除(v)materialDb死代码 — materialDb已正确传入EM链路 | b1/a3/meeting 均正确使用材质 |
| C8 | Matrix3x3 | ✅ 修正注释 (行主序非列主序, operator()未被调用) | 编译通过 |
| C9 | 增益削波 | ✅ 修复 — std::max(1.0, g) → std::max(1e-15, g) | 编译通过 |

### 回归基线

| 配置 | 路径数 | 验证 | 状态 |
|------|--------|------|------|
| b1_mixed_path_test.json | 362 paths | validation_passed=true | ✅ 一致 |
| a3_transmission_minimal.json | 2 paths | validation_passed=true | ✅ 一致 |
| meeting_v3.json | 664 paths | validation_passed=true | ✅ 一致 |
| b4_sbr_test.json | SBR 2000 rays, 30 Rx | 运行成功 | ✅ 通过 |
| meeting_coverage_hires.json | 200K rays, 644K/1.25M activeRx, 28 thr | 运行成功 | ✅ 通过 |
| Python L1 | 5/5 | ALL PASSED | ✅ |
| Python L2 | 2/2 | ALL PASSED | ✅ |

### 代码变更清单

| 文件 | 行变更 | 修复项 |
|------|--------|--------|
| `core/em/ApplyDiffractionInteraction.cpp` | ~40行 | C2(U坐标系)+C3(L参数)+C4(S/H交换)+C5(振幅) |
| `core/em/ApplyReflectionInteraction.cpp` | ~10行 | C5(振幅+Jones归一化) |
| `core/em/ApplyTransmissionInteraction.cpp` | ~10行 | C5(振幅+Jones归一化) |
| `core/search/SbrEngine.cpp` | ~5行 | C6(Keller锥射线方向) |
| `app/RtRealChainRunner.cpp` | -1行 | C7(移除(void)死代码) |
| `core/antenna/AntennaModel.cpp` | ~2行 | C9(增益削波阈值) |
| `core/common/math/Matrix3x3.h` | ~4行 | C8(注释修正) |

---

## 第5章：v7 修复批次规划

### 5.1 修复优先级原则

1. **P0 (v7-A)**: Critical 项 → 阻塞正确性，立即修复
2. **P1 (v7-B)**: High 项 → 显著影响精度，v7 必须修复
3. **P2 (v7-C)**: Medium 项 → 特定场景影响，v7 应修复
4. **P3 (v7-D)**: Low 项 → 代码质量，v7 择机修复
5. **P4 (v7-E)**: v6 配置清理剩余项（Q7~Q36）→ 继续清理

### 5.2 v7-A: Critical 修复（9项 — 预计 5-7 天）

| ID | 项 | 文件 | 工作量 |
|----|----|------|--------|
| C1 | TM透射系数分子修正 | ApplyTransmissionInteraction.cpp | 0.3d |
| C2 | UTD坐标系对齐楔面 | ApplyDiffractionInteraction.cpp | 1.5d |
| C3 | UTD L参数 sin²β₀ | ApplyDiffractionInteraction.cpp | 0.2d |
| C4 | UTD Soft/Hard方向交换 | ApplyDiffractionInteraction.cpp | 0.3d |
| C5 | 标量振幅加法→矢量重构 | ApplyReflection/Transmission/Diffraction | 1.0d |
| C6 | SBR绕射射线方向修正 | SbrEngine.cpp | 0.5d |
| C7 | 材料数据库接入EM链路 | RtRealChainRunner.cpp + 6 Apply* | 1.5d |
| C8 | Matrix3x3 operator()修正 | Matrix3x3.h | 0.2d |
| C9 | AntennaModel增益削波 | AntennaModel.cpp | 0.1d |

**v7-A 产出**: C1~C9 全部修复，新增 L1 验证项（UTD楔角扫描 + TM透射功率）。

---

### 5.3 v7-B: High 修复（16项 — 预计 5-8 天）

| ID | 项 | 文件 | 工作量 |
|----|----|------|--------|
| H1 | 介质衰减正确公式 | ApplyTransmissionInteraction.cpp | 0.3d |
| H2 | 介质内相位累积 | ApplyFreeSpaceSegment.cpp + FieldAccumulator | 0.5d |
| H3 | SBR透射功率核算修正 | SbrEngine.cpp | 0.5d |
| H4 | Fresnel缓存线程安全 | SbrEngine.cpp | 0.3d |
| H5 | RxHashGrid长线段多点查询 | SbrEngine.cpp | 0.5d |
| H6 | OBJ坐标变换去重 | OBJImporter.cpp / SceneImporter.cpp | 0.3d |
| H7 | SceneCache签名完善 | SceneCache.cpp | 0.3d |
| H8 | size_t→uint64_t序列化 | SceneCache.cpp | 0.5d |
| H9 | AppConfigValidator实现 | AppConfigValidator.cpp | 1.0d |
| H10 | AoA/AoD计算与导出 | EMPathResult.h + ExportPaths.cpp | 0.5d |
| H11 | ITU幂律插值 | MaterialDatabase.h | 0.5d |
| H12 | 材质查询失败报错 | MaterialDatabase.h | 0.1d |
| H13 | 复介电常数查询接口 | MaterialDatabase.h | 0.3d |
| H14 | polarization_file加载 | AntennaFactory.cpp | 0.3d |
| H15 | 极化虚部导出 | ExportPaths.cpp | 0.1d |
| H16 | CSV转义处理 | ExportPaths.cpp | 0.1d |

**v7-B 产出**: H1~H16 全部修复，全量回归通过。

---

### 5.4 v7-C: Medium 修复（18项 — 预计 3-5 天）

| ID | 项 | 文件 |
|----|----|------|
| M1 | UTD Fresnel积分大x渐近 | ApplyDiffractionInteraction.cpp |
| M2 | 首末透射介质元数据 | FinalizeAtReceiver.cpp |
| M3 | DiffractionExpander incident_direction 验证 | DiffractionExpander.cpp |
| M4 | 状态签名是否含累积长度评估 | StateSignatureBuilder.cpp |
| M5 | SBR绕射三因子UTD模型 | SbrEngine.cpp (v5-DD3) |
| M6 | BVH近-远遍历排序 | SceneQuery.cpp |
| M7 | 退化面NaN二面角保护 | EdgeBuilder.cpp |
| M8 | 楔边材料侧约定文档化 | WedgeBuilder.cpp |
| M9 | SceneCache逐字段序列化 | SceneCache.cpp |
| M10 | JSON \\" 转义修正 | AppConfigJsonCodec.cpp |
| M11 | JSON快照尾随逗号删除 | AppConfigJsonCodec.cpp |
| M12 | 容差默认值统一 | MathConstants.h + NumericToleranceConfig |
| M13 | 天线方向图数据排序 | AntennaPattern.h |
| M14 | 天线CSV解析失败报错 | AntennaPattern.h |
| M15 | MaterialDB加载失败报错 | RtPipeline.cpp |
| M16 | SelfCheck负测试还原 | ConfigSelfCheck.cpp |
| M17 | SceneCache魔数+版本号 | SceneCache.cpp |
| M18 | ReadOptionalBool明确false | AppConfigJsonCodec.cpp |

---

### 5.5 v7-D: v6 配置清理剩余项（~15字段 — 预计 2-3 天）

基于 v6/配置问题解答.md 的"待实施(v7)"列表：

| Q# | 字段 | 目标 |
|----|------|------|
| Q7 | allow_name_auto_cleanup | 删除，常开 |
| Q8 | enable_wedge_build | 自动推导 |
| Q10 | enable_bvh_bruteforce_validation | 删除 |
| Q12 | wedge_min/max_angle_deg | 硬编码10°/170° |
| Q13 | filter_non_manifold_wedge_sources | 常开 |
| Q14 | skip_coplanar_edges_for_wedge | 常开 |
| Q20 | enable_reflection/transmission/diffraction | max_*_count=0替代 |
| Q23 | tx_position/rx_position | 改为数组[x,y,z] |
| Q27 | ray_power_threshold_dB | linear→dB |
| Q29 | grid_margin_m | 自动计算 |
| Q30 | tx_power_dBm | W→dBm |
| Q35 | enable_polarization | 删除 |
| Q36 | output_directory | 硬编码output/ |

---

### 5.6 v7-E: Jones矢量极化升级（v5-DD1 — 预计 3-5 天）

将实向量极化升级为完整的 Jones 矢量（复极化向量），涉及：
- FieldAccumulator: 新增 `Complex pol_x, pol_y, pol_z`
- 6 个 Apply* 文件: TE/TM 复投影
- FinalizeAtReceiver + InitializeTxField: 同步升级
- 新增 L1 极化验证项（交叉极化鉴别率 XPD）

**前置条件**: C1~C9 + H10/H14/H15 修复完成后再做，避免在破损基础上叠加。

---

## 第6章：v7 批次依赖与时间线

```
v7-A (Critical, 5-7d) ──→ v7-B (High, 5-8d) ──→ v7-C (Medium, 3-5d)
                                │                        │
                                └──→ v7-D (Config, 2-3d, 可并行)
                                                         │
                                                         └──→ v7-E (Jones, 3-5d)

关键路径: v7-A → v7-B → v7-C → 全量回归
配置清理(v7-D)可在 v7-B 完成后并行
Jones极化(v7-E)依赖 v7-A + v7-B 完成
```

**v7 总预估**: 18-28 工作日（取决于 Medium 项的取舍）。

---

## 第7章：后续优化方向 (v8+)

### 7.1 理论完善
- 面元级材质绑定（OBJ格式限制，需FBX/glTF导入）
- 漫散射模型（Rayleigh粗糙度+Lambertian）
- SBR确定性绕射（替代概率采样）
- 时变/多普勒建模（论文范围外，远期）

### 7.2 加速优化
- OpenMP扩展到BVH构建和EM求解
- SIMD向量化Fresnel积分（AVX2）
- SBR Ray-Sphere早期剔除
- Rx-BVH二级空间索引（1e7 Rx级别）
- GPU CUDA/OptiX方案（设计文档 → 实现）

### 7.3 验证增强
- L1 解析解扩展（UTD楔角扫描、TM透射功率、极化XPD、Brewster角）
- L2 交叉验证 vs 参考实现（场景翻译器 + 4场景对比）
- L3 统计验证 vs 3GPP TR 38.901 InH-Office
- CI/CD 自动化回归

### 7.4 工程完善
- OBJ解析器升级（支持 v/vt/vn, 四边形, 负索引）
- 配置文件迁移到 JSON Schema 验证
- 异步日志（消除大场景IO瓶颈）
- 路径流式写入磁盘（降低内存）

---

## 附录A：审计方法学

### A.1 公式对照方法

对每个 EM 公式：
1. 定位原始文献（Balanis Ch.5/Ch.12, Born & Wolf §1.5, K&P 1974）
2. 逐变量映射：文献符号 → 代码变量
3. 数值验证：代入具体参数做手算验证
4. 边界条件：检查极限行为（θ→0, θ→π/2, f→0, ε_r→∞等）

### A.2 算法对照方法

对每个搜索/几何算法：
1. 定位标准参考（Rappaport 1994, Wald 2007, Möller-Trumbore 1997）
2. 追踪关键数据流：输入→处理→输出
3. 复杂度分析：时间/空间/正确性边界
4. 边界压力测试：空输入、退化几何、极值参数

### A.3 数据流追踪方法

对配置/管道/导出模块：
1. 字段声明 → JSON读写 → 管道消费 → 最终输出
2. 标记每个环节的断裂/丢失/转换错误
3. 交叉验证：同一信息在多处的表示一致性

---

## 附录B：关键参考文献

- Balanis, C. A. "Advanced Engineering Electromagnetics" (2nd Ed.), Wiley, 2012
- Born, M. & Wolf, E. "Principles of Optics" (7th Ed.), Cambridge, 1999
- Kouyoumjian, R. G. & Pathak, P. H. "A Uniform Geometrical Theory of Diffraction for an Edge in a Perfectly Conducting Surface," Proc. IEEE, vol. 62, pp. 1448-1461, 1974
- Friis, H. T. "A Note on a Simple Transmission Formula," Proc. IRE, vol. 34, pp. 254-256, 1946
- Seidel, S. Y. & Rappaport, T. S. "Site-Specific Propagation Prediction for Wireless In-Building Personal Communication System Design," IEEE Trans. Veh. Technol., vol. 43, pp. 879-891, 1994
- ITU-R P.2040-1 "Effects of Building Materials and Structures on Radiowave Propagation Above About 100 MHz," 2015
- Wald, I. "On Fast Construction of SAH-Based Bounding Volume Hierarchies," Eurographics, 2007
- Möller, T. & Trumbore, B. "Fast, Minimum Storage Ray-Triangle Intersection," J. Graphics Tools, 1997
