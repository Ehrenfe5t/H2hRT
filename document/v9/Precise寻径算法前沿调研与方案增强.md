# Precise 模式几何寻径 — 全网算法前沿调研与方案增强

> 调研日期：2026-06-02  
> 搜索范围：IEEE / arXiv / Semantic Scholar / NVIDIA Developer / 2024-2026  
> 关键词：RT path finding, image method, bidirectional visibility, BFGS optimization, GFlowNet, NeRF, GPU BVH

---

## 一、镜像法加速 — 可提升 Iter 1

### 核心发现：Anxel Beam Shrinkage（Kim et al., IEEE TAP 2024）

| 项目 | 内容 |
|------|------|
| **标题** | Anxel Beam Shrinkage Method and Heterogeneous Computing-Accelerated Full-Image Theory Method Ray Tracing Enabling Massive Outdoor Propagation Modeling |
| **来源** | *IEEE TAP*, Vol. 72, Issue 7, July 2024, pp. 5935–5949 |
| **作者** | Y. Kim, H. Yang, H. Kim, J. Jo, J. Oh (Seoul National University) |
| **DOI** | 10.1109/TAP.2024.3411793 |
| **核心贡献** | 651× 加速 over WinProp 镜像法求解器；支持 6 阶弹射的大规模城市场景；异构 CPU+GPU 并行 |

**关键方法**：
- **Anxel Beam Shrinkage（ABS）**：用光束收缩替代穷举镜像树遍历，大幅减少可见性树规模
- 不是对每个 RX 单独镜像，而是对所有观测点共享镜像树
- 与现有方案的关系：可增强 Iter 1 级联镜像法——不仅做多跳级联镜像，还加光束收缩剪枝

```
现有: 镜像树遍历 → 每个面元独立镜像 → O(N^k) 候选
ABS: 光束收缩 → 只追踪有效光束 → O(N·k) 候选
```

### 次要发现：RIS 加速镜像法（Wang et al., IEEE AP-S 2024）

| 项目 | 内容 |
|------|------|
| **标题** | An Acceleration Method for Ray-Tracing in Modeling Reconfigurable Intelligent Surface Propagation |
| **来源** | IEEE AP-S/URSI 2024 |
| **核心贡献** | 动态分区 RIS 子阵列减少射线数 |

---

## 二、混合图像法+射线发射 — 可增强 Iter 1 验证

### 核心发现：NimbusRT（Vaara et al., arXiv 2024）

| 项目 | 内容 |
|------|------|
| **标题** | Ray Launching-Based Computation of Exact Paths with Noisy Dense Point Clouds |
| **来源** | arXiv:2403.06648, 2024 |
| **作者** | N. Vaara, P. Sangi, M. Bordallo López, J. Heikkilä (University of Oulu) |
| **代码** | github.com/nvaara/NimbusRT |
| **核心贡献** | 射线发射→粗路径→精化为精确路径（多反射+绕射）；桥接镜像法与射线发射法 |

**关键方法**：
- 先做 Ray Launching（快速，适合多 RX）
- 提取粗路径 → 精化为精确路径（镜面反射+绕射）
- 对方案 Iter 1 的启示：可增加 RL 粗筛→镜像法精化的混合路径，减少纯镜像法候选

---

## 三、ML 辅助路径采样 — 可替代/增强 Iter 3 状态机

### 核心发现 1：Generative Ray Path Sampling（Eertmans et al., IEEE ICMLCN 2025）

| 项目 | 内容 |
|------|------|
| **标题** | Towards Generative Ray Path Sampling for Faster Point-to-Point Ray Tracing |
| **来源** | arXiv:2410.23773, Accepted at IEEE ICMLCN 2025 |
| **核心贡献** | GFlowNet 学习有效路径概率分布，替代穷举面元序列枚举 |

**关键方法**：
- **生成式流网络（GFlowNet）** 采样路径序列——复杂度**线性**增长（vs. 组合指数）
- 物理信息动作掩码——过滤不可能路径（不可见面元、类型不匹配）
- **几何不变性**：平移/旋转/缩放后无需重训练

```
现有 Iter 3: 状态机穷举 → ~800-1500 序列 → 面元映射 → 候选组合爆炸
GFlowNet:    概率采样    → top-K 序列    → 面元映射 → 可控候选数
```

### 核心发现 2：SANDWICH（IEEE ICMLCN 2025）

| 项目 | 内容 |
|------|------|
| **标题** | SANDWICH: Scene-Aware Neural Decision Wireless Channel Raytracing Hierarchy |
| **来源** | IEEE ICMLCN 2025 |
| **核心贡献** | 将路径生成 reformulate 为**序列决策问题**，用强化学习求解 |

**与本方案的融合**：GFlowNet/SANDWICH 可作为 Iter 3 的可选增强——默认用状态机（无需训练，开销小），高密度场景用 ML 采样。

---

## 四、神经辐射场（NeRF）→ 无线辐射场（WRF）

### 核心发现：GWRF（arXiv 2025.02）

| 项目 | 内容 |
|------|------|
| **标题** | GWRF: A Generalizable Wireless Radiance Field for Wireless Signal Propagation Modeling |
| **来源** | arXiv:2502.05708, 2025 |
| **核心贡献** | 将 NeRF 从光学扩展到无线 RF 域，跨场景泛化，无需逐场景重训练 |

**与本方案的潜在融合**：
- GWRF 可为 PVS 收缩提供**先验信息**——哪些面元对在给定 Tx 下更可能参与路径
- 不是替代 RT，而是辅助候选排序

### 次要发现：WRF-GS（IEEE INFOCOM 2025）

- 3D Gaussian Splatting 用于无线辐射场重建
- 从稀疏测量合成空间频谱——毫秒级

---

## 五、GPU 硬件加速 — 增强 Iter 6 验证

### 核心发现：V2V GPU RT（Hu et al., 2024）

| 项目 | 内容 |
|------|------|
| **标题** | A High-Performance GPU-Accelerated Ray-Tracing Method for Real-Time V2V Channel Modeling |
| **来源** | Semantic Scholar, 2024 |
| **核心贡献** | GPU RT 实时车联网信道建模 |

### 核心发现：Mochi（Mandarapu et al., arXiv 2026）

| 项目 | 内容 |
|------|------|
| **标题** | Mochi: Rethinking Collision Detection on GPU Ray Tracing Architecture |
| **来源** | arXiv:2604.23520, 2026 |
| **核心贡献** | GPU RT Core 的硬件 BVH 遍历用于通用碰撞检测 |

**对 Iter 6 的启示**：阶段 4 逐段可见性检查（~14000 次 BVH 查询）可用 GPU 批量提交替代 CPU 逐次查询。

---

## 六、方案增强建议（整合于现有重构方案）

基于上述全网调研，对原有 7-Iter 方案提出以下增强：

| 增强编号 | 影响 Iter | 来源 | 增强内容 | 优先级 |
|---------|----------|------|---------|:---:|
| **A1** | Iter 1 | Kim 2024 | 级联镜像法 + **光束收缩（Beam Shrinkage）** 预筛选，减少镜像树遍历 | P1 |
| **A2** | Iter 0 | Eertmans 2025 | ReversePVS 构建时同步计算**面元间距离加权**（为 GFlowNet 提供输入特征） | P2 |
| **A3** | Iter 3 | Eertmans 2025 | 交互序列枚举 **可选 GFlowNet 概率采样** 替代穷举状态机（高密度场景开关） | P1 |
| **A4** | Iter 6 | Mochi 2026 | 可见性批量验证 **GPU batch 提交**（OptiX RT Core 硬件 BVH 遍历） | P2 |
| **A5** | Iter 2 | GWRF 2025 | PVS 候选按 GWRF 评分排序（跨场景泛化先验）——留接口，不立即实现 | P3 |

### 增强后的架构图

```
PathSearchEngineV2::Run(context)
  │
  ├─ 阶段0: 候选生成 (3通道)
  │   ├─ 主通道: BidirectionalPVS (双向收缩 + BeamShrink预筛)
  │   ├─ 辅通道: SBR候选池 (面元序列 + SBR近似点)
  │   └─ ML通道(可选): GFlowNet概率采样 (高密度场景)
  │
  ├─ 阶段1: PVS双向收缩 + Beam Shrinkage
  │
  ├─ 阶段2: 序列枚举/采样
  │   ├─ 默认: 状态机穷举
  │   └─ 可选: GFlowNet概率采样
  │
  ├─ 阶段3: 精确求解
  │   ├─ 纯反射: 级联镜像法 (含Beam Shrinkage)
  │   ├─ 混合路径: BFGS
  │   └─ 绕射: Fermat
  │
  ├─ 阶段4: 验证
  │   ├─ CPU: 逐段BVH查询 (默认)
  │   └─ GPU: batch提交OptiX RT Core (可选)
  │
  └─ 输出: GeometricPathSet
```

---

## 七、推荐实施优先级（整合版）

| 顺序 | 内容 | 新增代码量 | 难度 | 依赖 |
|------|------|-----------|:---:|------|
| **Iter 0** | FacePVS 反向表转置 | ~20行 | 低 | BuildPVS |
| **Iter 1** | 纯反射级联镜像法 | ~300行 | 中 | Iter 0 |
| **Iter 2** | 双向 PVS 收缩 + BeamShrink | ~500行 | 高 | Iter 0 |
| **Iter 3** | 交互类型状态机 + GFlowNet接口 | ~350行 | 中 | Iter 2 |
| **Iter 4** | BFGS 参数化求解器 | ~650行 | 高 | 基础数 |
| **Iter 5** | SBR 辅助候选池 | ~350行 | 中 | SbrEngine |
| **Iter 6** | 验证器 + GPU batch接口 | ~350行 | 中 | Iter 2,5 |
| **Iter 7** | PathSearchEngineV2 集成 | ~250行 | 低 | Iter 1-6 |

---

## 八、关键新增参考文献

| # | 文献 | 来源 | 年份 | 用处 |
|---|------|------|------|------|
| 14 | Kim et al., "Anxel Beam Shrinkage + Heterogeneous Full-Image Theory RT" | *IEEE TAP* | 2024 | Iter 1 增强 |
| 15 | Vaara et al., "Ray Launching Exact Paths with Noisy Point Clouds" | arXiv:2403.06648 | 2024 | Iter 1 验证 |
| 16 | Eertmans et al., "Towards Generative Ray Path Sampling" | IEEE ICMLCN | 2025 | Iter 3 加速 |
| 17 | GWRF: "Generalizable Wireless Radiance Field" | arXiv:2502.05708 | 2025 | PVS 先验 |
| 18 | SANDWICH: "Scene-Aware Neural Decision Wireless Channel RT" | IEEE ICMLCN | 2025 | Iter 3 替代 |
| 19 | Hu et al., "GPU-Accelerated RT for Real-Time V2V Channel" | Semantic Scholar | 2024 | Iter 6 GPU |
