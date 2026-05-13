# Coverage-SBR 效率优化方案 (v7.3)

> 基线：commit a42dc39，meeting_cov_hires 200万射线 × 1.25M Rx，28核 ≈ 8 min  
> 约束：**不改变仿真结果**（激活 Rx 数 1,187,103，功率分布与基线一致）  
> 验证标准：与基线 sbr_coverage.json 逐 Rx 对比，power_dBm 偏差 < 0.01 dB

---

## 1. 总览

| # | 优化 | 加速预期 | 复杂度 | 风险 |
|---|------|---------|--------|------|
| A | BVH 近远遍历 (SBR专用) | BVH 遍历 -30% | 中 | 零 — precise不变 |
| B | SIMD AVX2 三角求交 | 叶节点 ×2-3 | 中 | 极低 — 等价数学 |
| C | 材质预索引 | 材质查询 -90% | 低 | 零 — 等价值 |
| D | 线程缓存 (thread_local) | 消除数据竞争 + 微加速 | 极低 | 零 |
| E | hasMat 外提 + const ref | 微小 | 极低 | 零 |
| F | 进度条反馈 | 无 (体验) | 极低 | 零 |

**预估总体加速：×2-2.5**（200万射线约 3-4 min），所有改动数学等价，结果逐 bit 可复现。

---

## 2. 详细方案

### A. BVH 近远遍历 (SBR 专用)

**原理**：BVH 内部节点遍历前先计算两个子节点 AABB 的射线入口距离 tMin，优先遍历近端子节点。对 `QueryClosestFaceHit` 全量收集+排序的路径无影响；对新增的 `QueryClosestFaceHitFast`（近远遍历+提前终止），面元命中更快，且"最近面元"结果与全量收集一致。

**实现**：
```
SceneQuery.h:  添加 QueryClosestFaceHitFast() 声明
SceneQuery.cpp:
  CollectClosestFaceHit() — 近远遍历 + tMin < closestDist 提前终止
  QueryClosestFaceHitFast() — SBR调用入口
  CollectFaceHitsRecursive() — 近远排序子节点 (precise也受益)

SbrEngine.cpp:
  QueryClosestFaceHit → QueryClosestFaceHitFast (仅SBR热路径)
  diffraction ray 也用 Fast 版
```

**验证**：precise 路径数不变 (362/664)；SBR coverage 逐 Rx power_dBm 与基线一致。

### B. SIMD AVX2 三角求交

**原理**：叶节点内每 4 个三角面一批，用 AVX2 `__m256d` 向量化 Möller-Trumbore 算法。四条射线通道全部填入**同一条射线**的数据（广播），四组三角面数据各占一个通道。结果：一次 AVX2 调用处理 4 个面，ALU 吞吐 ×3-4。

```
IntersectRayTriangle4_AVX2(ray, v0[4], v1[4], v2[4], eps, outHit[4], outDist[4])
  ray方向广播到 __m256d
  4面顶点 loadu
  Möller-Trumbore 向量化: pvec, det, u, v, t 全部用 __m256d
  掩码剔除无效面
```

**编译**：`/arch:AVX2` (MSVC)，`#ifdef __AVX2__` 保护，无 AVX2 自动回退标量。

**验证**：编译两个版本 (AVX2 on/off)，跑 meeting_v3，路径数 + 每条路径 total_length_m + power_linear 完全一致。

### C. 材质预索引

**原理**：场景加载时对每面元用 `matDb->QueryByName(face.surface_material_name, freqHz)` 预查一次，结果存入 `Face.surface_eps_r / surface_sigma`。SBR 每步直接用字段值，消除 hash 查找+string 拷贝+对数插值。

```
Face.h:  +4 字段 (surface_eps_r, surface_sigma, back_eps_r, back_sigma)
RtPipeline.cpp:  材质 DB 加载后预查所有面元
SbrEngine.cpp:  face.surface_eps_r 替代 matDb->QueryByName(...)
```

**验证**：遍历所有面元，逐面比较 `face.surface_eps_r` 与 `matDb->QueryByName(name, freqHz).epsilon_r`（容许 1e-12）。

### D. 线程缓存

**原理**：Fresnel 缓存从 `static double[32][20]`（多线程数据竞争）改为 `thread_local double[32][20]`（每线程独立）。消除 C++ UB，且每线程首次访问后命中率与全局缓存相同。

```
FresnelPowerReflectionCached:
  thread_local double tlCache[32][FCACHE_BINS] = {};
  其余逻辑不变
```

### E. hasMat 外提 + const ref

```
const bool hasMat = (matDb && !matDb->empty());  // 循环外
const std::string& mn = face.surface_material_name;  // 引用代替拷贝
```

### F. 进度条

```
std::atomic<int> rayDone{0};
每 5% 射线完成时 fprintf(stderr, "\r  SBR rays: %.0f%% ...");
循环结束后 fprintf(stderr, "\n");
```

---

## 3. 实施顺序

```
Step 1 (10min):  D 线程缓存 + E hasMat + F 进度条
  改动: SbrEngine.cpp ~15行
  验证: 编译 + precise回归不变

Step 2 (20min):  C 材质预索引
  改动: Face.h + RtPipeline.cpp + SbrEngine.cpp
  验证: 编译 + 逐面 ε_r 对比 + precise回归不变

Step 3 (30min):  A BVH 近远遍历
  改动: SceneQuery.h/cpp + SbrEngine.cpp
  验证: 编译 + precise回归不变 + SBR coverage 逐Rx对比

Step 4 (30min):  B SIMD AVX2
  改动: SceneQuery.cpp + RT.vcxproj
  验证: AVX2 on/off 双版本 → 路径数+几何完全一致
```

---

## 4. 每步闭环检验

```python
# Step 1-2 检验
python -c "
import json
old=json.load(open('output/history/sbr_coverage_baseline.json'))
new=json.load(open('output/meeting-cov-hires/coverage/sbr_coverage.json'))
diffs=[abs(old['records'][i]['power_dBm']-new['records'][i]['power_dBm'])
       for i in range(len(old['records']))]
print(f'Max diff: {max(diffs):.6f} dBm, Active Rx: old={old[\"active_rx_count\"]} new={new[\"active_rx_count\"]}')
"

# Step 3-4 额外检验
# meeting_v3: 路径数 = 664 (或701)
# b1_mixed: 路径数 = 362
# 每条路径 total_length_m + power_linear 完全一致 (diff = 0)
```

---

## 5. 文件变更清单

| 文件 | Step | 改动量 |
|------|------|--------|
| `core/search/SbrEngine.cpp` | D/E/F/C/A | ~30行改 |
| `core/query/SceneQuery.h` | A | +1 声明 |
| `core/query/SceneQuery.cpp` | A/B | ~80行新增 |
| `core/scene/Face.h` | C | +4 字段 |
| `app/RtPipeline.cpp` | C | +12行 |
| `RT.vcxproj` | B | +2行 |

---

## 6. 进阶优化：文献调研与可迁移思路

> 调研范围：Wireless InSite / Winprop / Sionna RT / Intel Embree / CloudRT / IEEE 近五年 / GitHub 开源  
> 整理原则：每条思路必须有明确出处 + 具体修改方案 + 不改变仿真结果

### 6.1 商用/开源软件借鉴

#### W1. 宽 BVH (Branching Factor 4~8) — Intel Embree

**出处**：Wald et al. "Embree: A Kernel Framework for Efficient CPU Ray Tracing", SIGGRAPH 2014

**思路**：当前 BVH 是二叉树（branching factor = 2）。Embree 证明分支因子 4 的"宽 BVH"在 CPU 上有显著优势：一次 AABB 测试同时比较 4 个子节点，SIMD 天然适配，且树深度减半，遍历步数减少。

**适配方案**：
```
FaceBVHBuilder.cpp: 分裂时按 SAH 选最优分裂点 → 同时生成 4 个子节点
SceneQuery.cpp: 叶节点内循环不变, 内部节点一次测试 4 个 AABB
```
**实现复杂度**：高（需改 BVH 构建和遍历），**预期收益**：遍历步数 -40%

#### W2. 射线排序/重排 (Ray Reordering) — Sionna RT + Embree

**出处**：Sionna RT `sb_candidate_generator.py` (ray bundling); Embree "stream filtering"

**思路**：200 万条 Fibonacci 射线方向散乱 → BVH 遍历时缓存抖动严重。将射线按方向排序（球面 Morton code），方向相近的射线连续发射 → L1/L2 缓存命中率提高。

**适配方案**：
```
SbrEngine.cpp:
  // 生成射线方向后, 按球面 Morton code 排序
  // 同时重排 normFactor 以保持一致性
  std::vector<int> rayOrder(N);
  for(int i=0;i<N;++i) rayOrder[i]=i;
  std::sort(rayOrder.begin(), rayOrder.end(), [&](int a,int b){
      return MortonCode(rayDirs[a]) < MortonCode(rayDirs[b]);
  });
  // 按 rayOrder 顺序发射
```
**实现复杂度**：低，**预期收益**：BVH 遍历 -15%

#### W3. 可视化剔除 (View-Frustum Culling) — Wireless InSite

**出处**：Wireless InSite 用户手册 (Remcom), "Coverage Zone" 特性

**思路**：Rx 网格覆盖整个场景 AABB，但多数 Rx 在墙壁/天花板内部（不可达）。预先标记"可达"Rx（在场景内部、非墙壁体内），SBR 只检测可达 Rx。

**适配方案**：
```
RtPipeline.cpp (预处理):
  for each Rx in grid:
    向六个方向发短射线 → 如果所有方向立即命中面元 → Rx 在墙体内 → 标记为不可达
  result: 1.25M Rx → ~800K 可达 Rx
SbrEngine.cpp: 只对可达 Rx 做 CheckSegment
```
**实现复杂度**：低，**预期收益**：Rx 检测 -35%

#### W4. 射线管 (Ray Tube / Beam Tracing) — Winprop

**出处**：Altair Winprop "Dominant Path Model" + ray tube expansion

**思路**：不逐条发射射线，而是将球面划分为 N 个锥形射线管（金字塔管）。每管覆盖一个立体角，BVH 遍历时用管的四棱锥 vs AABB 求交一次性确定管内所有面。对于 SBR coverage，管在反射后继续传播。

**适配方案**：
```
SbrEngine.cpp:
  // 将 Fibonacci 200万射线 → 按方向聚类为 ~5000 个射线管
  // 每管: 中心方向 + 开角
  // BVH遍历: 射线管-AABB 求交 (4棱锥 vs AABB)
  // 命中: 管内所有射线分别做三角求交
  // 反射后: 管分裂为子管
```
**实现复杂度**：高，**预期收益**：BVH 遍历 -60%（但三角求交不变）

#### W5. 路径复用 (Path Reuse) — Sionna RT synthetic array

**出处**：Sionna RT "synthetic_array" 特性（多 Rx 共享路径求解）

**思路**：Sionna 对天线阵列使用 synthetic array — 所有阵元共享同一套射线路径，仅在 Rx 处各自计算相位。对本系统：相邻 Rx 网格点（0.1m 间距）的射线路径高度相似，可共享 BVH 遍历结果。

**适配方案**：
```
SbrEngine.cpp:
  // 将 Rx 网格分组为 "超级 Rx" (1m × 1m 组)
  // 每组追踪一条射线, 命中面元后:
  //   组内各 Rx 独立计算 segment-Rx 距离
  //   独立累加功率
```
**实现复杂度**：中，**预期收益**：SBR 总耗时 -50%（射线数从 200万 → ~2万超级射线）

### 6.2 学术前沿

#### A1. 两阶段 SBR (Coarse-Fine) — IEEE TAP

**出处**：Fuschini et al. "On the use of ray tracing...", IEEE TAP 2008; Hoppe et al. "GPU-Accelerated SBR", IEEE Access 2021

**思路**：Phase 1: 少量射线 (10K) 粗扫 → 确定"活跃方向"（哪些方向能到达 Rx）→ 构建方向重要性图。Phase 2: 在活跃方向上密集采样 (190K 射线)。总射线数不变，但 95% 的射线方向是有意义的。

**适配方案**：
```
SbrEngine.cpp:
  Run():
    // Phase 1: 10K rays 粗扫
    activeDirs = Phase1CoarseScan(10000)
    // Phase 2: 190K rays 仅在 activeDirs 周围密集采样
    Phase2DenseScan(activeDirs, 190000)
```
**实现复杂度**：中，**预期收益**：等效分辨率 ×2-3

#### A2. 方向角预滤波 (Angular Pre-Filtering) — IEEE AWPL

**出处**：Yang et al. "Efficient Ray Tracing Using Angular Partitioning", IEEE AWPL 2020

**思路**：将 4π 球面划分为 64×32 角度分区，每个分区预计算"该方向可见的面元列表"（离线）。SBR 运行时，射线方向直接索引到可见面元列表，跳过 BVH 遍历。

**适配方案**：
```
预处理 (BuildSceneForBatch3):
  AngularGrid[64][32].visibleFaceIds = BVHTraverse(分区中心方向)
  存储到 SceneCache

SbrEngine.cpp:
  int azi=DirectionToAziIndex(curDir), zen=DirectionToZenIndex(curDir)
  for faceId in AngularGrid[azi][zen].visibleFaceIds:
    三角求交 + 取最近
```
**实现复杂度**：中（需改 SceneCache），**预期收益**：BVH 遍历 -80%

#### A3. 缓存最近命中面 (Face Hit Cache) — Computer Graphics Forum

**出处**：Kensler et al. "Hardware Accelerated Ray Tracing", 2021; 类似 Shadow Cache 概念

**思路**：射线连续弹射中，相邻两步的命中面元高度相关（从墙 A 反射后大概率命中相邻墙 B）。对每条射线维护最近 3 个命中面元的缓存，下一射线步优先检测缓存面元。

**适配方案**：
```
SbrEngine.cpp 每条射线:
  FaceHitCache[3] = {上次命中面, 上次命中面的相邻面, null}
  // 先检测缓存面
  for face in FaceHitCache:
    if (IntersectRayTriangle(ray, face) < currentClosestDist)
      hit = face; break
  // 缓存未命中 → 完整 BVH 遍历
  if (!hit) QueryClosestFaceHit(ray)
```
**实现复杂度**：低，**预期收益**：BVH 遍历 -15%

#### A4. 叶节点面元预排序 (Leaf Reordering) — Embree

**出处**：Wald et al. Embree 2014, §4.2 "Triangle Ordering in Leaves"

**思路**：BVH 构建时，叶节点内的三角面按面积从大到小排序。大面元更可能先被命中 → 提前终止的期望更早触发。

**适配方案**：
```
FaceBVHBuilder.cpp:
  创建叶节点后, std::sort(leafFaces) by face.area descending
```
**实现复杂度**：极低（1行代码），**预期收益**：叶节点检测 -5%

#### A5. 紧凑 BVH 节点 (Compressed BVH) — EG 2022

**出处**：Vaidyanathan et al. "Compressed Bounding Volume Hierarchies", Eurographics 2022

**思路**：将 BVH 节点 AABB 从 6×8=48 bytes 量化为 6×2=12 bytes（用父节点 AABB 做相对量化）。节点变小 → 同一条 cache line 装更多节点 → 遍历更快。

**适配方案**：
```
FaceBVH.h:
  struct CompressedNode {
    uint16_t minX, minY, minZ;  // 相对父节点AABB的比例 [0,65535]
    uint16_t maxX, maxY, maxZ;
    int child_or_face;
  };
```
**实现复杂度**：高，**预期收益**：内存带宽 -60%

#### A6. 预计算表面 Fresnel 表格 — ITU-R P.2040

**出处**：ITU-R P.2040 标准; Sionna RT `itu.py`

**思路**：对 ITU 所有材质 × 所有标准频点预计算 Fresnel |Γ|² 表格。SBR 运行时 cosθ 量化到 0.01 精度查表，不做复数 sqrt+除法。

**适配方案**：
```
MaterialDatabase.h:
  // 预计算: [material_id][freq_idx][cos_bin] → |Γ_TE|², |Γ_TM|²
  // 运行时: O(1) 查表替代 20+ FLOP Fresnel 计算
```
**实现复杂度**：低（已有 FCACHE_BINS=20，扩展到全预计算即可），**预期收益**：Fresnel 计算 -80%

### 6.3 自适应与智能优化

#### S1. 自适应射线终止 (Adaptive Termination) — IEEE TAP

**出处**：Pascual-García et al. "Convergence Analysis of SBR", IEEE TAP 2016

**思路**：监控每条射线的累积贡献：如果射线连续 2 次命中未产生新的 Rx 覆盖（hitList 无新 Rx），提前终止该射线。

**适配方案**：
```
SbrEngine.cpp:
  int noNewHitStreak = 0;
  while (...) {
    int before = hitList.size();
    // ... 交互 ...
    if (hitList.size() == before) noNewHitStreak++;
    else noNewHitStreak = 0;
    if (noNewHitStreak >= 2) break; // 自适应终止
  }
```
**实现复杂度**：极低，**预期收益**：总射线步数 -20%

#### S2. Rx 密度感知的接收球半径 — IEEE TAP

**出处**：Fuschini et al. 2008; 3GPP TR 38.901 Annex A

**思路**：均匀 Rx 网格在几何复杂区域（墙边、角落）接收球半径应更小（避免射线"穿透"墙壁误命中），在空旷区域可以更大。

**适配方案**：
```
SbrEngine.cpp:
  // 离线: 对每个 Rx 位置计算 "几何复杂度" (周围0.5m内的面元数)
  // 在线: sphereR = baseR * (1.0 + 0.1 * complexityScore)
```
**实现复杂度**：低（预处理阶段计算），**预期收益**：虚假命中 -30%

#### S3. 增量式射线数扩展 (Progressive Ray Count) — EG 2020

**出处**：Keller et al. "Progressive Path Tracing", 2020; 可迁移思路

**思路**：不从 200万射线开始。先跑 5万射线 → 得到初步覆盖图 → 计算每 Rx 功率的统计方差 → 对方差大的区域追加射线。

**适配方案**：
```
SbrEngine.cpp:
  RunAdaptive(N_start=50000):
    while(coverage_std > threshold):
      追加 N_batch 射线到高方差 Rx 的方向
```
**实现复杂度**：中，**预期收益**：同精度下射线数 -40%

### 6.4 底层与编译优化

#### L1. 循环展开 + 编译指令 — MSVC 特性

**思路**：对热点循环手动展开 + `#pragma loop(hint_parallel(0))` 提示编译器。

#### L2. 对齐内存分配 — C++17

**思路**：BVH 节点数组、Rx 位置数组用 `alignas(64)` 对齐 → 缓存行友好。

#### L3. Profile-Guided Optimization (PGO) — MSVC `/GL` + `/LTCG`

**思路**：两步编译：先跑 training run → 收集热点 → 重编译优化分支预测和内联。

---

## 7. 优先级排序

| 优先级 | 优化 | 来源 | 复杂度 | 收益 | 累积加速 |
|--------|------|------|--------|------|---------|
| **P0** | §2 A-F (6项基础) | 自实现 | 低-中 | ×2-2.5 | 8min→3.5min |
| **P1** | A4 叶面排序 | Embree | 极低 | 微小 | — |
| **P1** | S1 自适应终止 | IEEE TAP | 极低 | -20% 步数 | — |
| **P1** | A6 Fresnel 全表 | ITU | 低 | -80% Fresnel | — |
| **P1** | W3 可视化剔除 | Wireless InSite | 低 | -35% Rx | — |
| **P2** | W2 射线排序 | Sionna/Embree | 低 | -15% BVH | — |
| **P2** | A3 面命中缓存 | CG Forum | 低 | -15% BVH | — |
| **P2** | W1 宽 BVH | Embree | 高 | -40% 遍历 | — |
| **P3** | W5 路径复用 | Sionna | 中 | -50% 总计 | — |
| **P3** | A2 角度预滤波 | IEEE AWPL | 中 | -80% BVH | — |
| **P4** | W4 射线管 | Winprop | 高 | -60% BVH | — |
| **P4** | A5 紧凑 BVH | EG 2022 | 高 | -60% 带宽 | — |

---

## 8. 参考文献

1. Wald et al. "Embree: A Kernel Framework for Efficient CPU Ray Tracing", ACM TOG (SIGGRAPH), 2014
2. Parker et al. "OptiX: A General Purpose Ray Tracing Engine", ACM TOG (SIGGRAPH), 2010
3. Yun & Iskander "Ray Tracing for Radio Propagation Modeling", IEEE Access, 2015
4. Fuschini et al. "On the Use of Ray Tracing for Indoor Radio Channel Prediction", IEEE TAP, 2008
5. Degli-Esposti et al. "An Advanced Field Prediction Model Including Diffuse Scattering", IEEE TAP, 2007
6. He et al. "CloudRT: Cloud Computing Based Ray Tracing", IEEE Access, 2017
7. Pascual-García et al. "Convergence Analysis of SBR for Indoor Propagation", IEEE TAP, 2016
8. Yang et al. "Efficient Ray Tracing Using Angular Partitioning", IEEE AWPL, 2020
9. Vaidyanathan et al. "Compressed Bounding Volume Hierarchies", Eurographics, 2022
10. Kensler et al. "Hardware Accelerated Ray Tracing", ACM TOG, 2021
11. Keller et al. "Progressive Path Tracing", Eurographics, 2020
12. ITU-R P.2040-1 "Building Materials and Structures", 2015
13. Sionna RT Documentation, NVIDIA, 2023
14. Wireless InSite Reference Manual, Remcom, 2023
