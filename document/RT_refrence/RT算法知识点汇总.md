# RT 电磁传播仿真系统 — 算法知识点汇总 (v7)

> 面向：华为通信算法岗面试准备
> 系统定位：自主开发的室内 ISAC 多维域信道建模 RT 仿真系统，C++20 + MSVC，~1700 行核心算法
> 覆盖：物理理论 → 信道特征 → 代码实现 → 加速优化

---

## 1. 系统架构总览

### 1.1 六模块分层架构

```
┌─────────────────────────────────────────────────────┐
│  app/        主流水线 RtPipeline                    │
│              Batch0→6 顺序调度                       │
├─────────────────────────────────────────────────────┤
│  core/search   寻径引擎     core/em     电磁计算     │
│  Image Method  SbrEngine    Fresnel/UTD  FSPL/PDP  │
├─────────────────────────────────────────────────────┤
│  core/query 空间加速      core/scene  场景数据结构   │
│  BVH遍历     RxHashGrid   Face/Edge/Wedge           │
├─────────────────────────────────────────────────────┤
│  preprocess/  场景预处理                             │
│  OBJ导入 材质绑定 BVH构建 楔边拓扑 场景缓存          │
├─────────────────────────────────────────────────────┤
│  core/common  基础设施                               │
│  Vec3 Complex Matrix3x3 MaterialDatabase Logger     │
└─────────────────────────────────────────────────────┘
```

### 1.2 双模式设计

| 维度 | Precise 模式 | SBR Coverage 模式 |
|------|------------|------------------|
| 算法 | Image Method + 优先级队列 | Shooting & Bouncing Ray + Monte Carlo |
| 路径 | 确定性几何精确路径 | 统计采样（200K rays × 1.25M Rx） |
| 极化 | Jones 矢量 TE/TM 相干合成 | 标量 Fresnel 功率反射 |
| 绕射 | UTD 8点 Gauss-Legendre 数值积分 | Keller 锥 4 射线 + 三因子简化 UTD |
| 透射 | Snell 折射 + Fresnel 相干系数 | Monte Carlo 概率选择（不分裂） |
| 产出 | 路径级：复振幅/时延/AoA/AoD/极化 | 覆盖级：Rx 网格功率热力图 |
| 规模 | ~10³ 路径（受签名去重限制） | 10⁵ 射线 × 10⁶ Rx 体素 |
| 并行 | 逐路径 OpenMP | 逐射线 OpenMP 28核 |

---

## 2. 物理层：电磁理论实现

### 2.1 Fresnel 反射与透射系数

**理论基础**：Balanis "Advanced Engineering Electromagnetics" Ch.5；Born & Wolf §1.5

**复介电常数**（`CalcEpsC`）：
```
ε_c(f) = ε_r(f) - j·σ(f) / (ω·ε₀)
```
- ω = 2πf，ε₀ = 8.854×10⁻¹² F/m
- 虚部符号约定：e^{-jωt} 时谐 → ℑ(ε_c) = -σ/(ωε₀)
- 频率相关性查询 ITU-R P.2040 材质数据库

**反射系数**（`FresnelTE`, `FresnelTM`）：
```
Γ_TE = (cosθ_i - √(ε_c - sin²θ_i)) / (cosθ_i + √(ε_c - sin²θ_i))
Γ_TM = (ε_c·cosθ_i - √(ε_c - sin²θ_i)) / (ε_c·cosθ_i + √(ε_c - sin²θ_i))
```
- cosθ_i = |k̂_inc·n̂|，n̂ 为面元法线
- √取复平方根主分支
- 极限行为：垂直入射 Γ_TE = -Γ_TM；Brewster 角 Γ_TM→0

**透射系数**（`FresnelTE_T`, `FresnelTM_T`）：
```
T_TE = 2·cosθ_i / (cosθ_i + √(ε_c - sin²θ_i))
T_TM = 2·√ε_c·cosθ_i / (ε_c·cosθ_i + √(ε_c - sin²θ_i))
```
- **关键修复 (v7 C1)**：TM 透射分子使用 √ε_c 而非 ε_c。E-field convention 一致性。
- 功率守恒验证：|Γ|² + (n₂/n₁)·(cosθ_t/cosθ_i)·|T|² = 1 ✓

**代码位置**：`ApplyReflectionInteraction.cpp:21-33`，`ApplyTransmissionInteraction.cpp:19-35`

### 2.2 UTD 一致性绕射理论

**理论基础**：Kouyoumjian & Pathak (1974)；Balanis Ch.12

这是本系统**学术贡献最显著**的模块。

**绕射系数 D**（`ComputeUTD_D`）：
```
D = -exp(-jπ/4) / (2n·√(2πk)·sinβ₀) · [T₁+T₂ ± T₃ ± T₄]
```
- 硬边界(Neumann/TM)取 +T₃+T₄；软边界(Dirichlet/TE)取 -T₃-T₄

**四个 cotangent 项**（完整 K&P Table 1）：
```
T₁ = cot((π + (φ-φ'))/(2n)) · F(kL a⁺(φ-φ'))
T₂ = cot((π - (φ-φ'))/(2n)) · F(kL a⁻(φ-φ'))
T₃ = cot((π + (φ+φ'))/(2n)) · F(kL a⁺(φ+φ'))
T₄ = cot((π - (φ+φ'))/(2n)) · F(kL a⁻(φ+φ'))
```

**Fresnel 过渡函数 F(x)**（`FresnelTransitionNumerical`）：
```
F(x) = 2j√x·e^{jx} · ∫_{√x}^{∞} e^{-jτ²} dτ
```
- 数值求解：8-point Gauss-Legendre × 16 区间 × step=0.5
- 自适应积分范围 [√x, √x+8.0]
- 阴影边界过渡：F(x→0)→0, F(x→∞)→1
- 精度：128 采样点覆盖 8 rad，每周期 ~6.4 采样点 > Nyquist(2) ✓

**距离参数 L**（`s₁`, `s₂`）：
```
L = s₁·s₂·sin²β₀ / (s₁+s₂)    （球面波入射）
```
- **v7 C3 修复**：添加 sin²β₀ 因子（原缺失，仅垂直入射对）

**边缘固定坐标系**（**v7 C2 修复**）：
- ê_z = 楔边方向
- ê_x = Cross(n_0face, ê_z) — 从 0-face 面法线构建参考方向
- ê_y = ê_z × ê_x
- φ, φ' 均从 0-face 测量（非出射方向）
- **关键**：φ+φ' 在参考系旋转下不守恒 → 必须对齐楔面

**Keller 锥**：
```
cosβ₀ = |k̂·ê_z|    （入射角=绕射角）
```
- 绕射射线位于以 ê_z 为轴、β₀ 为半角的锥面上

**代码位置**：`ApplyDiffractionInteraction.cpp:69-92`（ComputeUTD_D），`21-58`（FresnelIntegral）

### 2.3 Jones 矢量极化

**理论基础**：Born & Wolf Ch.1；IEEE Std 149

**表示**（`FieldAccumulator`）：
```
E = A · (p_re + j·p_im)
```
- A = 标量复振幅（|A| = 场强幅值，arg(A) = 全局相位）
- p_re, p_im = Jones 矢量实部/虚部（3D 单位复矢量，|p_re|² + |p_im|² = 1）

**TE/TM 复投影**：
```
A_TE = A · <p, e_TE>           ← 复投影（非实投影）
A_TM = A · <p, e_TM>
```
- e_TE = Normalize(k̂_inc × n̂) — 垂直于入射面
- e_TM = Normalize(e_TE × k̂_inc) — 在入射面内

**交互后重构**（**v7 C5 修复**）：
```
E_out = Γ_TE·A_TE·e_TE + Γ_TM·A_TM·e_TM    ← 矢量相加（非标量）
|A_new| = √(|Γ_TE·A_TE|² + |Γ_TM·A_TM|²)   ← 正交分量功率和
Jones_new = (Re(E_out) + j·Im(E_out)) / |A_new|
```
- **关键**：TE ⟂ TM 正交，不能做标量 `A_TE + A_TM` 加法
- Jones 矢量归一化使 |Jones| = 1，幅值信息全部存储在 A 中

**代码位置**：`ApplyReflectionInteraction.cpp:58-95`，`FieldAccumulator.h:37-38`

### 2.4 自由空间路径损耗 (FSPL)

**Friis 传输公式**（`FinalizeAtReceiver`）：
```
A_fspl = λ / (4πd)
```
- λ = c₀/f，d = 总路径几何长度
- **施加时机**：仅在路径末端 FinalizeAtReceiver 施加**一次**（非每段累乘）
- 远场假设：d > 2D²/λ ≈ 4m @ 2.4GHz（对短 LOS 路径近似性已文档化）

**介质衰减**：
```
A_media = exp(-Σ α_i·d_i)
```
- α_i = 各段介质衰减常数（Np/m），在 ApplyFreeSpaceSegment 中累积

**代码位置**：`FinalizeAtReceiver.cpp:29`，`ApplyFreeSpaceSegment.cpp:29-36`

---

## 3. 寻径层：双模式路径搜索

### 3.1 Precise 模式 — Image Method + 优先级队列

**算法**（`SearchEngine`）：
```
1. 初始状态：Tx 位置 + 剩余预算 (R,T,D) + 空路径签名
2. 优先级队列 (最佳优先搜索)：
   cost = 候选面元数 × 10.0 - depth × 0.5  （倾向深路径）
3. 对每个状态出队：
   a. 交互展开器展开候选路径
   b. uint64 状态签名去重（防止重复面序列）
   c. 几何合法性检查（同面不重复交互、透射介质切换合法）
   d. 到达 Rx 的路径加入结果集
4. 终止：队列空 或 所有预算耗尽
```

**交互展开器**：
- `ReflectionExpander`：镜像法（Image Method）。虚 Rx = Reflect(Rx, face)，从当前点向虚 Rx 发射线，命中面=face 则有效
- `TransmissionExpander`：Snell 折射方向 + 介质侧语义完整性检查
- `DiffractionExpander`：Fermat 黄金分割搜索楔边上最短路径点 + Keller 锥验证

**uint64 路径签名**：各面元/wedge ID × 交互类型 的 FNV-1a hash → 碰撞概率 ~10⁶/2⁶⁴ ≈ 5×10⁻¹¹（可忽略）

**代码位置**：`SearchEngine.cpp:244-340`，`ReflectionExpander.cpp:131-180`

### 3.2 SBR Coverage 模式 — Monte Carlo 射线追踪

**射线生成**（`GenerateFibonacciRays`）：
- Fibonacci 球面均匀分布：φ_gold = π(3-√5) ≈ 2.4 rad
- y = 1 - 2i/(N-1)，r = √(1-y²)，θ = φ_gold·i
- 比经纬度网格更优的均匀性（无极点过密）
- 功率归一化：每射线 P = P_tx / N

**射线-场景交互循环**（`SbrEngine::Run`）：
```
While (depth ≤ maxDepth AND power > threshold):
    1. BVH 射线-三角面求交 (Möller-Trumbore)
    2. RxHashGrid 接收球命中检测
    3. 绕射 (概率采样)：Keller 锥 4 方向 + 三因子 UTD 功率模型
    4. 透射 (Monte Carlo)：概率 |Γ|² 选反射 / (1-|Γ|²) 选透射
    5. 反射：Fresnel 功率反射系数 |Γ|² 衰减
```

**关键修复 (v7 C6)**：Keller 锥绕射射线方向 y/z 分量添加缺失的 Cross(ê_D, p̂_B) 项

**代码位置**：`SbrEngine.cpp:20-41`（Fibonacci），`196-291`（主循环）

### 3.3 RxHashGrid — 空间哈希加速

**设计**（`RxHashGrid`）：
- cell_size = 2 × sphere_radius（保证相邻 cell 不漏检）
- 27 邻域查询：每个射线步检查 3×3×3 邻域 cell
- uint64 编码：(cx << 42) | (cy << 21) | cz
- **限制**：长射线步用中点查询，极长步可能跨越多 cell → 多点采样缓解

**代码位置**：`SbrEngine.cpp:57-88`

### 3.4 路径几何合法性

**`GeometryValidity`**：
- 同面不重复交互检测
- 透射介质侧语义完整性（medium_in ≠ medium_out）
- 交互次数不超过预算

**代码位置**：`GeometryValidity.cpp`

---

## 4. 空间加速层：BVH 与场景结构

### 4.1 SAH BVH 构建

**Surface Area Heuristic**（`FaceBVHBuilder`）：
```
SAH cost = (leftArea × leftCount + rightArea × rightCount) / parentArea
```
- 16-bin 分桶：面元 centroid 沿轴线性映射 → 15 个候选分裂点
- 三轴择优（X/Y/Z）：取 cost 最小
- 小节点回退：nTris ≤ leafSize×2 → median-split
- leaf_size 默认 16（自适应公式：max(4, min(32, N_faces/200))）

**遍历**（`SceneQuery`）：
- 递归遍历 BVH 节点（当前无近-远排序 → v7 M6）
- Möller-Trumbore 射线-三角面求交

**代码位置**：`FaceBVHBuilder.cpp:139-193`

### 4.2 楔边拓扑构建

**边提取**（`EdgeBuilder`）：
- 共享边查找：二面角计算 + 共面/非流形/边界分类
- 边方向一致性：vertex_index₀ < vertex_index₁

**楔边构建**（`WedgeBuilder`）：
- 楔角 = 180° - 二面角（外角 → 内角转换）
- 过滤：非流形边、共面边、楔角 < 10° 或 > 170°（UTD 有效范围）
- positive/negative face material 分配

**代码位置**：`EdgeBuilder.cpp`，`WedgeBuilder.cpp`

### 4.3 场景缓存

**序列化**（`SceneCache`）：
- 二进制格式：顶点 → 面元 → 楔边
- 缓存签名：FNV-1a hash of 配置参数 → 缓存失效检测
- **已知限制**：size_t 跨平台不兼容（64→32 bit），无魔数/版本号 → v7 H7/H8

**代码位置**：`SceneCache.cpp`

---

## 5. 信道特征计算

### 5.1 信道冲激响应 (CIR)

**复基带 CIR**：
```
h(τ) = Σ α_i · e^{jφ_i} · δ(τ - τ_i)
```
- α_i = |A_i|·λ/(4πd_i)（FSPL 衰减后的复振幅幅值）
- φ_i = arg(A_i)（累积相位 + 交互相位跳变）
- τ_i = d_i/c₀（传播时延）

**代码位置**：`BuildCIR.cpp`，`FinalizeAtReceiver.cpp`

### 5.2 关键信道参数

| 参数 | 公式 | 物理意义 |
|------|------|---------|
| 路径损耗 | PL = -10·log₁₀(Σ|α_i|²) dB | 发射→接收总功率衰减 |
| RMS 时延扩展 | στ = √(ΣP_i·(τ_i-μ_τ)²/ΣP_i) | 多径时延散布（室内~5-20ns） |
| K 因子 | K = 10·log₁₀(P_LOS/P_NLOS) | LOS 分量主导度 |
| AoD/AoA | 球坐标 (θ, φ) 从路径几何提取 | 3D 角度域信道特征 |
| 极化态 | Jones 矢量 (p_re+j·p_im) | 交叉极化鉴别率 XPD |

**代码位置**：`BuildChannelStatistics.cpp`，`rt_utils.py:compute_channel_params`

### 5.3 ISAC 特征

- 感知 SINR：通信干扰 + 感知检测概率联合评估
- PDP 功率时延谱 / APS 角度功率谱
- 覆盖热力图：path_gain / RSS / SINR 空间分布

**代码位置**：`BuildPDP.cpp`，`BuildAPS.cpp`，`BuildISACFeatureSet.cpp`

---

## 6. 加速优化

### 6.1 OpenMP 多线程并行

**SBR 射线并行**：
```cpp
#pragma omp parallel for schedule(dynamic) num_threads(28)
for (int ri = 0; ri < N; ++ri) { /* 每条射线独立追踪 */ }
```
- 每线程独立缓冲区（tp[tid], th[tid]）：无锁归并
- 每射线独立 XorShift32 随机数种子：ri × 2654435761 + 1
- 加速比：28 核 ~20×（meeting_cov_hires: 200K rays ≈ 8min）

**逐路径 EM 求解并行**：
```cpp
#pragma omp parallel for schedule(dynamic)
for (int i = 0; i < nPaths; ++i) {
    EMPathResult result;
    SolveSinglePathEM(config, scene, paths[i], result, materialDb);
    #pragma omp critical
    set.results.push_back(result);
}
```

**代码位置**：`SbrEngine.cpp:196-197`，`RtRealChainRunner.cpp:126-128`

### 6.2 算法级优化

| 优化 | 方法 | 效果 |
|------|------|------|
| 射线功率阈值早停 | P_cur < P_thr → break | 裁剪不可检测路径 |
| Fresnel 缓存 | 32 mat × 20 cosθ 档 | 避免重复 ε_c 计算 |
| 绕射降频 | 仅奇数步(stepIdx&1)执行 | 绕射开销减半 |
| 楔边随机采样 | nSample 个候选 → 按距离取前 K | O(N_sample) 替代 O(N_wedges) |
| uint64 路径签名 | FNV-1a hash | O(1) 去重 vs O(N) 遍历 |

### 6.3 可扩展加速方向 (v8+)

- **SIMD (AVX2)**：Fresnel 积分点并行计算（4 doubles/__m256d）
- **GPU (CUDA/OptiX)**：OptiX 内置 BVH + CUDA kernel 批量射线
- **分层空间索引**：粗-细二级 Rx 哈希（针对 10⁷ 级 Rx）
- **射线方向锥剔除**：Rx 可见方向锥预计算，跳过不可见 Rx
- **异步日志**：消除大场景 IO 瓶颈

---

## 7. 面试常见追问与回答要点

### Q1：Image Method vs SBR 的优劣和适用场景？

- **Image Method**：精确几何路径，无遗漏（在给定深度内）。但组合爆炸 O(B^D)，大场景不可行
- **SBR**：采样统计，线性可扩展。但可能遗漏罕见路径，接收球半径引入空间模糊
- **本系统"双模互补"**：precise 验证物理正确性，SBR 做大规模覆盖

### Q2：UTD 绕射为什么用数值积分而不用 Luebbers 近似？

- Luebbers 近似是对 F(x) 的有理函数拟合，在阴影边界过渡区误差可达 2-3 dB
- 本系统使用 8-point Gauss-Legendre × 16 区间直接数值积分，精度 < 0.1%
- 8 点 GL 对 e^{-jτ²} 振荡被积函数在每周期 ≥ 6.4 采样点，满足 Nyquist

### Q3：为什么 Fresnel 反射/透射要做 TE/TM 分解？

- 任意极化入射场可分解为 TE（⟂入射面）和 TM（∥入射面）分量
- Γ_TE ≠ Γ_TM（除垂直入射外），两者独立作用
- 反射/透射后 TE 和 TM 分量相位差产生椭圆极化
- 本系统使用 Jones 矢量复投影 + 矢量振幅重构（v7 C5 修复）

### Q4：如何处理路径组合爆炸？

- uint64 签名去重：同一面序列不重复展开
- 优先级队列最佳优先：先探索最有希望的路径
- 功率阈值早停：弱路径不继续展开
- 交互次数限制：max_reflection/transmission/diffraction_count

### Q5：SBR 的接收球半径如何选择？

- 太小：漏检（射线恰好从 Rx 旁边经过）
- 太大：重复计数（同一条射线多次命中同 Rx）→ hitList 去重
- balanced：通常取 √(3)·grid_step/2 ≈ 0.15m for 0.1m grid
- **限制**：引入空间模糊，时延精度受限于 sphereR/c₀

### Q6：为什么要用 Jones 矢量而不是 Stokes 参数？

- Jones 矢量：保持完整相位信息，支持相干合成 → precise 模式需要
- Stokes 参数：仅含强度信息，丢失相位 → 不能做相干叠加
- ISAC 场景：交叉极化鉴别率（XPD）需要完整极化态 → Jones 必须

### Q7：材质数据库 ITU-R P.2040 的关键参数？

- ε_r(f)：相对介电常数（混凝土~5-7，玻璃~6-7，木材~2-3）
- σ(f)：电导率（S/m）
- 频率依赖：幂律模型 ε_r ∝ f^a（非简单线性插值）
- 复介电常数转换：ε_c = ε_r - jσ/(ωε₀)

---

## 8. 系统核心指标

| 指标 | 数值 |
|------|------|
| 代码规模 | ~1700 行核心 C++ 算法 |
| 支持交互类型 | 反射/透射/绕射 任意组合混合路径 |
| 最大路径深度 | 12（precise）/ 8（SBR） |
| SBR 吞吐 | 200K rays × 1.25M Rx 网格 ≈ 8 min (28核) |
| precise 路径数 | 26~664（取决于交互开关和场景复杂度） |
| 频率范围 | 1~28 GHz（ITU-R P.2040 频点） |
| 极化 | Jones 矢量 TE/TM 相干合成 |
| 绕射 | UTD 8点 GL 数值积分（非近似） |
| 材质 | ITU-R P.2040 数据库 + Fresnel 幂律插值 |

---

## 9. 参考文献

- Balanis, C. A. "Advanced Engineering Electromagnetics" (2nd Ed.), Wiley, 2012. Ch.5, Ch.12
- Born, M. & Wolf, E. "Principles of Optics" (7th Ed.), Cambridge, 1999. §1.5
- Kouyoumjian, R. G. & Pathak, P. H. "A Uniform Geometrical Theory of Diffraction for an Edge in a Perfectly Conducting Surface," Proc. IEEE, vol. 62, pp. 1448-1461, 1974
- Friis, H. T. "A Note on a Simple Transmission Formula," Proc. IRE, vol. 34, pp. 254-256, 1946
- Seidel, S. Y. & Rappaport, T. S. "Site-Specific Propagation Prediction for Wireless In-Building Personal Communication System Design," IEEE Trans. Veh. Technol., vol. 43, pp. 879-891, 1994
- ITU-R P.2040-1 "Effects of Building Materials and Structures on Radiowave Propagation Above About 100 MHz," 2015
- 3GPP TR 38.901 "Study on Channel Model for Frequencies from 0.5 to 100 GHz," v16.1.0, 2019
- Wald, I. "On Fast Construction of SAH-Based Bounding Volume Hierarchies," Eurographics, 2007
- Möller, T. & Trumbore, B. "Fast, Minimum Storage Ray-Triangle Intersection," J. Graphics Tools, 1997
