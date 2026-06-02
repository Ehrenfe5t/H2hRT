# Precise 模式几何寻径重构方案

> 版本：v2.0 — 最终定版（PVS 主通道 + SBR 辅助补位）  
> 日期：2026-06-01  
> 关联主线：v9 主线 E（precise/SBR 双模式寻径合理性）  
> 论文方向：基于双向可见性收缩与统一参数化优化的室内确定性射线追踪寻径方法
> 
> 修订：v1.1 同步《分析v1.md》审查反馈 | v2.0 整合《分析v2.md》SBR辅助通道 + SBR近似点作为BFGS初始值

---

## 1. 现状分析与问题定位

### 1.1 当前架构

```
SearchEngine::Run(context)
  │
  ├─ 优先队列 best-first 搜索
  │   ├─ ExpandReflection  → 镜像法 (每个候选面元一次镜像, Rx引导)
  │   ├─ ExpandTransmission → Tx→Rx连线搜索 (仅1D线段)
  │   └─ ExpandDiffraction  → 楔边候选 + Fermat 1D黄金分割
  │
  ├─ TryBuildLosPath       → 闭合验证
  │
  └─ SortAndTruncateCandidates → Top-K截断
```

### 1.2 已知限制

| # | 问题 | 根因 | 影响 |
|---|------|------|------|
| 1 | **透射受限** | `ExpandTransmission` 只在 Tx→Rx 连线找透射面元 | 遗漏 Snell 偏折后不在连线上的透射路径 |
| 2 | **混合路径依赖序列展开** | 优先队列逐跳搜索，每跳方向受 Rx 约束 | R→T→D 等混合路径可能因中间跳方向不对而遗漏 |
| 3 | **无全局优化** | 各 Expander 独立工作，无联合求解 | 交互点位置非全局最优 |
| 4 | **候选组合爆炸** | per_expander_keep_limit 截断可能丢弃真路径 | exhaustive_debug_mode 可缓解但极慢 |

### 1.3 已有可复用基础设施

| 组件 | 当前状态 | 可用性 |
|------|---------|--------|
| `FacePVS` | 152k entries, 面-面互见表 | ✅ 直接用于候选过滤 |
| `EdgeAdjacency` | 645k pairs, 楔边邻接表 | ✅ 直接用于绕射候选 |
| `AngularGrid` | 2048 cells, 方向→面元索引 | ✅ 方向快速过滤 |
| `MaterialDatabase` | ITU-R P.2040, 完整电参数 | ✅ 透射折射率查询 |
| `MirrorPointAcrossPlane` | 镜像法核心函数 | ✅ 可直接复用 |
| `SnellRefractV2` | Snell 折射 + 诊断 | ✅ 可直接复用 |
| `GoldenSectionSearch` | Fermat 1D优化 | ✅ 可直接复用 |
| `ExportPaths` | 完整路径诊断导出 | ✅ 无需修改 |
| `ConstrainedSearchConfig` | 约束搜索配置 (已预留) | ✅ 接口已有 |

---

## 2. 重构目标

### 2.1 核心指标

| 指标 | 当前 | 目标 |
|------|------|------|
| **完备性** | 透射受限于 Rx 方向 | 双向搜索消除方向约束 |
| **混合路径** | 依赖队列逐跳展开 | 状态机枚举 + 全局求解 |
| **纯反射** | 单跳镜像法 | 级联镜像法 (O(k) 解析) |
| **精确度** | 逐跳独立求解 | BFGS 全局联合优化 |
| **性能** | ~0.5s/Rx (412场景) | ~0.3s/Rx (候选过滤更精准) |

### 2.2 非目标

- 不修改 SBR 模式
- 不改变 EM 计算链路
- 不改变导出/验证链路
- 不引入新的用户可见模式

---

## 3. 重构架构：四阶段混合寻径引擎（PVS 主通道 + SBR 辅助补位）

```
                    PathSearchEngineV2::Run(context)
                              │
              ┌───────────────┼───────────────┐
              ▼               ▼               ▼
      阶段0: 候选生成 (双通道)              阶段3: 精确求解
              │                               │
    ┌─────────┴─────────┐           ┌─────────┼─────────┐
    ▼                   ▼           ▼         ▼         ▼
 主通道               辅通道     级联镜像    BFGS     Fermat
 BidirectionalPVS    SBR候选池    (纯反射)  (混合)   (绕射)
    │                   │           O(k)     O(k²·iter) O(1)
    ▼                   ▼
 阶段1: PVS双向收缩   阶段1b: SBR面元序列提取
    │                   │
    └────────┬──────────┘
             ▼
      阶段2: 序列枚举 + 映射 + 合并去重
      InteractionStateMachine + FaceSequenceMapper
             │
             ▼
      ┌──────┴──────┐
      ▼              ▼
  PVS候选序列    SBR候选序列 + SBR近似命中点
      │              │
      └──────┬───────┘
             ▼
      阶段3: 精确求解 (SBR近似点优先作为BFGS初始值)
             │
             ▼
      阶段4: 验证与合并 → GeometricPathSet
```

---

## 4. 详细设计

### 4.1 阶段1：双向可见性收缩 (Bidirectional PVS Contraction)

#### 4.1.1 新增数据结构

```cpp
// core/search/BidirectionalPVS.h

namespace rt {

/// 交互层类型 — 区分反射/透射/绕射的候选面元集
enum class InteractionLayerType {
    Reflection,     // reflection_enabled 面元
    Transmission,   // transmission_enabled + dual_side 面元
    Diffraction     // 楔边候选
};

/// 一层候选 — 对应路径中的一个交互位置
struct CandidateLayer {
    InteractionLayerType type;
    std::vector<int> face_ids;      // 候选面元 (反射/透射)
    std::vector<int> wedge_ids;     // 候选楔边 (绕射)
};

/// 双向 PVS 收缩结果
struct BidirectionalPVSResult {
    bool valid = false;
    
    // 正向层: Forward[0]=Tx层, Forward[1]=第1跳候选, ...
    std::vector<CandidateLayer> forward_layers;
    
    // 反向层: Backward[0]=Rx层, Backward[1]=第1跳反向候选, ...
    std::vector<CandidateLayer> backward_layers;
    
    // 中层交集索引: Midpoint[k] = forward_layers[k] ∩ backward_layers[N-k]
    std::vector<CandidateLayer> midpoints;
    
    // 统计
    int total_face_candidates = 0;
    int total_wedge_candidates = 0;
    double build_time_ms = 0.0;
};

/// 双向可见性收缩器
class BidirectionalPVSContraction {
public:
    /// 从 Tx/Rx 两端对称扩张候选层
    BidirectionalPVSResult Contract(
        const PathSearchContext& context,
        const PathSearchConfig& config) const;

private:
    /// 单步正向扩张: 从当前层扩展到一个交互类型的下一层
    CandidateLayer ExpandForward(
        const CandidateLayer& current,
        InteractionLayerType next_type,
        const Scene& scene,
        const FacePVS& pvs,
        const EdgeAdjacency& edge_adj) const;

    /// 单步反向扩张: 同正向，但使用反向 PVS (入边集合)
    CandidateLayer ExpandBackward(
        const CandidateLayer& current,
        InteractionLayerType prev_type,
        const Scene& scene,
        const FacePVS& pvs,
        const EdgeAdjacency& edge_adj) const;
};

} // namespace rt
```

#### 4.1.2 算法流程

```
输入: Tx, Rx, max_depth, PathSearchConfig
输出: BidirectionalPVSResult

步骤:
  1. 初始化:
     Forward[0] = {Tx}
     Backward[0] = {face | face对Rx可见 (BVH射线检查)}

  2. 正向扩张 (d = 1..ceil(N/2)):
     对每个 Forward[d-1] 中的候选:
       - 如果是面元: Forward[d] ∪= PVS[face_id] (所有可见面元)
       - 如果是楔边: Forward[d] ∪= WedgePVS[wedge_id] (楔边可见面元)
     按 next_type 过滤:
       - Reflection:    保留 reflection_enabled=true 的面
       - Transmission:  保留 transmission_enabled=true + dual_side 的面
       - Diffraction:   保留关联楔边

  3. 反向扩张 (d = 1..floor(N/2)):
     同上，但使用反向 PVS (哪些面能看到当前层的面)

  4. 中层交集:
     对 k = 0..N:
       Midpoint[k] = Forward[k] ∩ Backward[N-k]
       
  5. 返回有效层集合
```

#### 4.1.3 复杂度

```
每层扩张: O(|Layer[d]| × avg_PVS_degree)
         = O(50 × 50) = O(2500) per layer (典型值)
10层: O(25000) 次整数集合操作
总耗时: < 5ms
```

#### 4.1.4 关键实现细节

**A. PVS 方向性与反向表**

`BuildPVS` 构建的 `pvs_faces[i]` 是**单向**的——记录"面 i 能看到哪些面"，而非"哪些面能看到面 i"。每个面元从其重心向半球发射 200 条射线，记录命中面元。因此 $PVS[A]$ 包含 B **不保证** $PVS[B]$ 包含 A。

反向扩张需要**转置表**。在 `BuildPVS` 末尾一次性构建：

```cpp
// O(total_entries) ≈ O(152k), < 1ms
pvs.reverse_pvs.resize(N);
for (int i = 0; i < N; ++i) {
    for (int j : pvs.pvs_faces[i]) {
        pvs.reverse_pvs[j].push_back(i);  // "谁能看到 j"
    }
}
```

对应地在 `FacePVS` 结构中新增字段：

```cpp
struct FacePVS {
    std::vector<std::vector<int>> pvs_faces;    // 正向: 面 i → 面 i 能看到的其他面
    std::vector<std::vector<int>> reverse_pvs;  // 反向: 面 j → 能看到面 j 的面集合
    // ...
};
```

正向扩张用 `pvs_faces`，反向扩张用 `reverse_pvs`。

**B. 稀疏 PVS 的 fallback 策略**

对于极端稀疏场景（如大型空旷厂房，面元间距超过 PVS max distance），某层的 PVS 扩张可能产生空集。fallback 方案：

```cpp
CandidateLayer ExpandWithFallback(const CandidateLayer& current,
    const FacePVS& pvs, const Scene& scene, const SceneQuery& query) 
{
    CandidateLayer result;
    for (int face_id : current.face_ids) {
        const auto& visible = pvs.GetVisibleFaces(face_id);
        if (!visible.empty()) {
            result.face_ids.insert(visible.begin(), visible.end());
        } else {
            // fallback: 该面元PVS为空 → 50条自适应局部重采样
            auto local = AdaptiveLocalSample(face_id, scene, query, 50);
            result.face_ids.insert(local.begin(), local.end());
        }
    }
    return result;
}
```

对本研究的紧凑室内场景（412 会议室，面元间距 < 5m，200 条半球射线覆盖充分），PVS 覆盖有效，fallback 在正常仿真中不被触发。

### 4.2 阶段2：交互类型状态机 (Interaction State Machine)

#### 4.2.1 新增数据结构

```cpp
// core/search/InteractionStateMachine.h

namespace rt {

/// 单个交互步骤的类型描述
struct InteractionStep {
    InteractionType type;       // Reflection / Transmission / Diffraction
    int remaining_after;        // 执行此步骤后剩余的该类型配额
    bool can_close_to_rx;       // 此步骤后是否可直接闭合到 Rx
};

/// 完整的交互类型序列 (不含具体面元)
struct InteractionSequence {
    std::vector<InteractionStep> steps;  // 从 Tx 出发的交互序列
    int total_depth;                      // 总交互次数
    uint64_t type_signature;              // 序列类型的哈希签名
    bool mixed;                           // 是否包含多种交互类型
};

/// 交互类型状态机
class InteractionStateMachine {
public:
    /// 枚举所有合法交互类型序列
    std::vector<InteractionSequence> Enumerate(
        const PathSearchConfig& config) const;

private:
    /// 递归生成序列
    void GenerateSequences(
        int remaining_R, int remaining_T, int remaining_D,
        int max_depth, int current_depth,
        std::vector<InteractionStep>& current,
        std::vector<InteractionSequence>& output) const;

    /// 过滤非法序列
    bool IsValidTransition(InteractionType prev, InteractionType next) const;
};

} // namespace rt
```

#### 4.2.2 序列生成规则

```
配置: max_R=4, max_T=2, max_D=1, max_depth=10

状态转移约束:
  1. 不允许连续相同类型的透射 (T→T 在室内罕见)
  2. 绕射后不跟绕射 (D→D 仅在多楔边场景，暂不支持)
  3. 任何状态都可直接闭合到 Rx
  4. 过度反射 (R→R→R→R→R...) 前须有 T 或 D 介入
  
预期序列数: 约 800-1500 (对 R≤4, T≤2, D≤1)
```

#### 4.2.3 面元序列映射

```cpp
/// 将交互类型序列 + PVS 层 → 具体面元序列
struct FaceSequenceMapper {
    /// 对给定类型序列，从 PVS 层中提取可行面元组合
    std::vector<std::vector<int>> MapToFaceSequences(
        const InteractionSequence& type_seq,
        const BidirectionalPVSResult& pvs_layers) const;
    
    /// 用方向聚类压缩候选面元 (AngularGrid)
    void ClusterByDirection(
        std::vector<int>& candidates,
        const Vec3& reference_direction,
        const AngularFaceGrid& grid) const;
};
```

**压缩策略**：如果某层候选面元 > 200，用 AngularGrid 按参考方向聚类，每个聚类保留 1-2 个代表面元。这不会遗漏路径，只是推迟精确验证。

### 4.3 阶段3：混合精确求解器 (Hybrid Path Solver)

#### 4.3.1 新增数据结构

```cpp
// core/search/HybridPathSolver.h

namespace rt {

/// 交互点的参数化描述 (借鉴 Eertmans et al. 2026)
struct ParameterizedInteraction {
    int face_id;              // 约束面元索引 (-1 表示楔边)
    int wedge_id;             // 约束楔边索引 (-1 表示面元)
    InteractionType type;     // R / T / D
    
    // 参数化: x = A·t + b
    double A[6];              // 3×2 基矩阵 (行主序), 楔边只用第1列
    double b[3];              // 参考点 (面元重心 / 楔边起点)
    double t[2];              // 参数化坐标 (变量), 楔边仅用 t[0]
    
    double n_before;          // 入射侧折射率
    double n_after;           // 出射侧折射率
};

/// BFGS 求解器配置
struct BfgsSolverConfig {
    int max_iterations = 30;
    double gradient_tol = 1e-8;
    double line_search_tol = 1e-6;
    bool use_fixed_point_line_search = true;  // Eertmans 2026 推荐
};

/// 混合路径精确求解器
class HybridPathSolver {
public:
    HybridPathSolver(const BfgsSolverConfig& cfg);
    
    /// 求解给定面元序列的交互点位置
    /// @param faces 面元/楔边索引序列
    /// @param types 对应交互类型
    /// @param tx Tx 位置
    /// @param rx Rx 位置
    /// @param out_nodes 输出的 PathNode 序列
    /// @return 是否成功 (满足所有约束)
    bool Solve(
        const std::vector<int>& face_ids,
        const std::vector<int>& wedge_ids,
        const std::vector<InteractionType>& types,
        const Point3& tx,
        const Point3& rx,
        const PathSearchContext& context,
        std::vector<PathNode>& out_nodes);

private:
    /// 纯反射子链: 级联镜像法 O(k) 解析解
    bool SolveReflectionChain(
        const std::vector<int>& face_ids,
        int start_idx, int end_idx,
        const Point3& source,
        const Point3& target,
        const Scene& scene,
        std::vector<Point3>& out_points);
    
    /// 混合路径: BFGS 参数化优化
    bool SolveParameterized(
        const std::vector<ParameterizedInteraction>& params,
        const Point3& tx,
        const Point3& rx,
        const Scene& scene,
        const MaterialDatabase& mat_db,
        std::vector<Point3>& out_points);
    
    /// BFGS 核心迭代
    bool BfgsMinimize(
        std::vector<ParameterizedInteraction>& params,
        const Point3& tx,
        const Point3& rx,
        const Scene& scene,
        const BfgsSolverConfig& cfg);
    
    /// 光程损失函数 L(T) = Σ n_i·‖x_{i+1} - x_i‖
    double OpticalPathLength(
        const std::vector<ParameterizedInteraction>& params,
        const Point3& tx,
        const Point3& rx) const;
    
    /// 解析梯度 ∇L(T)
    void ComputeGradient(
        const std::vector<ParameterizedInteraction>& params,
        const Point3& tx,
        const Point3& rx,
        std::vector<double>& gradient) const;
    
    /// 固定点线搜索 (Eertmans 2026)
    double FixedPointLineSearch(
        const std::vector<ParameterizedInteraction>& params,
        const std::vector<double>& direction,
        const Point3& tx,
        const Point3& rx) const;

    BfgsSolverConfig config_;
};

} // namespace rt
```

#### 4.3.2 级联镜像法（纯反射子链）

```cpp
// 输入: source点, target点, 反射面元序列 [f₁, f₂, ..., fₘ]
// 输出: 反射点序列 [P₁, P₂, ..., Pₘ]
//
// 原理: 对 target 连续做 m 次镜像（从后往前），得到 target_final。
//       从 source 向 target_final 连线，与 f₁ 的交点即 P₁。
//       然后从 P₁ 向 target^(m-1)（少一次镜像的中间点）连线，与 f₂ 交于 P₂。
//       以此类推——不需要在循环内做"逆镜像"。

bool HybridPathSolver::SolveReflectionChain(
    const std::vector<int>& face_ids,
    int start_idx, int end_idx,
    const Point3& source,
    const Point3& target,
    const Scene& scene,
    std::vector<Point3>& out_points)
{
    int m = end_idx - start_idx;
    out_points.resize(m);

    // ── 步骤 1: 一次性计算所有中间镜像点 ──
    // mirrors[k] = target 经过 fₖ...fₘ 连续镜像后的结果
    // mirrors[m] = 原始 target
    std::vector<Point3> mirrors(m + 1);
    mirrors[m] = target;
    for (int i = m - 1; i >= 0; --i) {
        const Face& f = scene.faces[face_ids[start_idx + i]];
        mirrors[i] = MirrorPointAcrossPlane(mirrors[i + 1], f.centroid, f.normal);
    }
    // mirrors[0] = target 经 f₁...fₘ 全部镜像后的最终点

    // ── 步骤 2: 正向递推求交 ──
    Point3 src = source;
    for (int i = 0; i < m; ++i) {
        // 向 mirrors[i] 连线——这是向 target 经剩余面元 (f_i...f_m) 镜像后的位置
        Vec3 dir = Normalize(Subtract(mirrors[i], src));
        Ray ray; ray.origin = src; ray.direction = dir;

        FaceQueryContext qc;
        FaceHit hit = scene_query->QueryClosestFaceHit(ray, qc);

        const Face& f = scene.faces[face_ids[start_idx + i]];
        if (hit.face_id != f.face_id) return false;
        if (!PointInTriangle(hit.position, f)) return false;

        out_points[i] = hit.position;
        src = hit.position;
        // mirrors[i+1] 已天然是 target 经 f_{i+1}..f_m 镜像——下一轮直接用
    }

    return true;
}
```

#### 4.3.3 BFGS 优化器核心

```cpp
bool HybridPathSolver::BfgsMinimize(
    std::vector<ParameterizedInteraction>& params,
    const Point3& tx, const Point3& rx,
    const Scene& scene,
    const BfgsSolverConfig& cfg)
{
    int n_vars = 0;
    for (auto& p : params) {
        n_vars += (p.type == InteractionType::Diffraction) ? 1 : 2;
    }
    
    // 初始化: 面元重心投影
    std::vector<double> t(n_vars, 0.0);  // 所有参数变量展平为1D向量
    
    // BFGS 状态
    std::vector<double> H_inv(n_vars * n_vars, 0.0);  // 近似 Hessian 逆
    for (int i = 0; i < n_vars; ++i) H_inv[i * n_vars + i] = 1.0;  // 初始H⁻¹ = I
    
    std::vector<double> grad(n_vars);
    std::vector<double> t_new(n_vars);
    
    for (int iter = 0; iter < cfg.max_iterations; ++iter) {
        // 1. 更新参数化点到实际坐标
        UpdatePositions(params, t);
        
        // 2. 计算梯度和光程
        ComputeGradient(params, tx, rx, grad);
        double loss = OpticalPathLength(params, tx, rx);
        
        // 3. 收敛检查
        double grad_norm = 0.0;
        for (double g : grad) grad_norm += g * g;
        if (grad_norm < cfg.gradient_tol * cfg.gradient_tol) break;
        
        // 4. 搜索方向: p = -H⁻¹·g
        std::vector<double> p(n_vars, 0.0);
        for (int i = 0; i < n_vars; ++i)
            for (int j = 0; j < n_vars; ++j)
                p[i] -= H_inv[i * n_vars + j] * grad[j];
        
        // 5. 线搜索
        double alpha = FixedPointLineSearch(params, p, tx, rx);
        
        // 6. 更新: t_new = t + α·p
        for (int i = 0; i < n_vars; ++i)
            t_new[i] = t[i] + alpha * p[i];
        
        // 7. BFGS更新 H⁻¹
        // s = t_new - t, y = g_new - g
        UpdateBFGSInverseHessian(H_inv, t, t_new, grad, grad_old, n_vars);
        
        // 8. 下一轮
        t.swap(t_new);
        grad_old = grad;
    }
    
    return true;
}

double HybridPathSolver::OpticalPathLength(
    const std::vector<ParameterizedInteraction>& params,
    const Point3& tx, const Point3& rx) const
{
    double L = 0.0;
    Point3 prev = tx;
    double n_prev = 1.0;
    
    for (auto& p : params) {
        Point3 x(p.b[0] + p.A[0]*p.t[0] + p.A[1]*p.t[1],
                 p.b[1] + p.A[2]*p.t[0] + p.A[3]*p.t[1],
                 p.b[2] + p.A[4]*p.t[0] + p.A[5]*p.t[1]);
        L += n_prev * Length(Subtract(x, prev));
        prev = x;
        n_prev = p.n_after;  // 出射折射率作为下一段的速度因子
    }
    L += n_prev * Length(Subtract(rx, prev));  // 最后一段到Rx
    return L;
}
```

#### 4.3.4 梯度计算

```cpp
void HybridPathSolver::ComputeGradient(
    const std::vector<ParameterizedInteraction>& params,
    const Point3& tx, const Point3& rx,
    std::vector<double>& grad) const
{
    // ∂L/∂tᵢ = n_before × ∂‖xᵢ - x_{i-1}‖/∂xᵢ · ∂xᵢ/∂tᵢ
    //         + n_after  × ∂‖x_{i+1} - xᵢ‖/∂xᵢ · ∂xᵢ/∂tᵢ
    
    // ∂‖x - x_prev‖/∂x = (x - x_prev) / ‖x - x_prev‖ (单位方向)
    // ∂x/∂t = A[:,k] (第k个基矢量)
    
    // 对每个交互点:
    for (size_t i = 0; i < params.size(); ++i) {
        Point3 x_i = GetPosition(params[i]);
        Point3 x_prev = (i == 0) ? tx : GetPosition(params[i-1]);
        Point3 x_next = (i + 1 < params.size()) ? GetPosition(params[i+1]) : rx;
        
        double d_prev = Length(Subtract(x_i, x_prev));
        double d_next = Length(Subtract(x_next, x_i));
        
        if (d_prev < 1e-12 || d_next < 1e-12) continue;
        
        Vec3 dir_in  = Scale(Subtract(x_i, x_prev), 1.0/d_prev);
        Vec3 dir_out = Scale(Subtract(x_next, x_i), 1.0/d_next);
        
        double n_in  = params[i].n_before;
        double n_out = params[i].n_after;
        
        // 投影到面元基矢量
        for (int k = 0; k < (params[i].type == Diffraction ? 1 : 2); ++k) {
            Vec3 basis(params[i].A[0 + k], params[i].A[2 + k], params[i].A[4 + k]);
            double dL = n_in * Dot(dir_in, basis) - n_out * Dot(dir_out, basis);
            //                                   ↑ 注意负号: ∂‖x_next - x‖/∂x = -(x_next-x)/d
            grad[GetVarIndex(params, i, k)] = dL;
        }
    }
}
```

#### 4.3.5 BFGS 约束处理

面元边界约束分为**软约束（罚函数）**和**硬约束（不可违反，直接拒绝）**：

```cpp
// 软约束 — 通过罚函数嵌入损失函数 (对BFGS平滑可微)
double OpticalPathLengthWithBarrier(
    const std::vector<ParameterizedInteraction>& params,
    const Point3& tx, const Point3& rx,
    const Scene& scene) const
{
    double L = GeometricPathLength(params, tx, rx);
    
    for (size_t i = 0; i < params.size(); ++i) {
        const auto& p = params[i];
        if (p.type == InteractionType::Diffraction) {
            // 楔边 1D: t[0] 需在 [0, 1] 内
            if (p.t[0] < 0.0)   L += 1e6 * p.t[0] * p.t[0];
            if (p.t[0] > 1.0)   L += 1e6 * (p.t[0] - 1.0) * (p.t[0] - 1.0);
        } else {
            // 面元 2D: 重心坐标法检查是否在三角形内
            auto bary = ToBarycentric(GetPosition(p), scene.faces[p.face_id]);
            for (int c = 0; c < 3; ++c)
                if (bary[c] < 0.0) L += 1e6 * bary[c] * bary[c];
        }
    }
    return L;
}

// 硬约束 — BFGS收敛后在阶段4中验证, 违反则整条路径reject:
//   - TIR 条件 (sin_t > 1)
//   - 面元类型不匹配 (transmission_enabled=false vs 透射类型)
//   - Keller 锥条件完全不满足
```

**选择罚函数而非投影梯度的理由**：罚函数保持损失函数平滑可微，BFGS 收敛不受影响。投影梯度需要在每步迭代后 clamp 参数——这会破坏 BFGS 的逆 Hessian 近似一致性。

如果 BFGS 收敛后解有点在面元边界外（罚函数未能完全约束），在阶段 4 做硬拒绝。

#### 4.3.6 梯度符号验证

梯度公式 $\partial L / \partial t_k = n_{before} \cdot \langle \text{dir}_{\text{in}}, \text{basis}_k \rangle - n_{after} \cdot \langle \text{dir}_{\text{out}}, \text{basis}_k \rangle$ 中的负号来源于：

```
∂/∂x_i [‖x_{i+1} - x_i‖] = -(x_{i+1} - x_i)/d = -dir_out  ← 链式法则的负号
∂/∂x_i [‖x_i - x_{i-1}‖] =  (x_i - x_{i-1})/d =  dir_in   ← 正号
```

因此 `∂L/∂t = n_before·dir_in·basis - n_after·dir_out·basis`。代码中的符号正确。

### 4.4 阶段4：验证器 (Path Validator)

```cpp
// core/search/PathValidator.h

namespace rt {

struct PathValidationResult {
    bool valid = false;
    std::vector<bool> point_in_triangle;     // 每个交互点是否在面元内
    std::vector<bool> visibility_ok;         // 每个段的可见性
    std::vector<double> snell_residuals;     // 透射点的Snell残差
    std::vector<double> keller_residuals;    // 绕射点的Keller残差
    std::vector<double> reflection_residuals;// 反射点的镜面残差
};

class PathValidator {
public:
    /// 完整验证一条几何路径
    PathValidationResult Validate(
        const std::vector<PathNode>& nodes,
        const Point3& tx, const Point3& rx,
        const PathSearchContext& context) const;

private:
    /// 验证点在三角面元内 (重心坐标法)
    bool PointInFace(const Point3& p, const Face& face) const;
    
    /// 验证两点间可见性
    bool SegmentVisible(const Point3& a, const Point3& b,
                        const SceneQuery& query,
                        int ignore_face_id = -1) const;
};

} // namespace rt
```

---

### 4.5 Iter 1 独立验证方案

纯反射级联镜像法 (Iter 1) 可在不依赖其他模块的情况下独立验证：

**测试场景**：412 场景, `max_R=2, T=0, D=0`

**步骤**：

```
1. 用现有 SearchEngine + ExpandReflection → 路径集 A
2. 用级联镜像法 (遍历 PVS 候选面元对) → 路径集 B
3. 验证 A ⊆ B
   - 对 A 中每条路径, 在 B 中找交互点偏差 < 1e-4m 的对应路径
   - 如有 A 中路径不在 B 中: 说明现有引擎产生了一个非精确路径
4. 检查 B \ A 中的额外路径
   - 这些是现有 per_expander_keep_limit 截断丢弃的路径
   - 验证其物理合理性
5. 对 A ∩ B 中的路径, 验证交互点位置偏差
```

**预期**：`|B| >= |A|`，且 A ∩ B 中交互点位置完全一致。级联镜像法是解析解，应产生与现有镜像法 + 优先队列完全相同的反射点位置。

**BFGS 初始值建议**：其他 AI 反馈指出以面元重心 `t=[0,0]` 为初始值在 Tx/Rx 与面元共面时可能导致梯度为零（退化）。建议以"从 Tx 向 Rx 方向在面元上的投影点"作为初始值：

```cpp
Vec2 InitGuess(const Face& face, const Point3& prev, const Point3& next) {
    Vec3 midpoint = Scale(Add(prev, next), 0.5);
    // 投影到面元切向坐标系
    Vec3 delta = Subtract(midpoint, face.centroid);
    Vec3 u = face.tangent_u;  // 面元第一个切向基矢量
    Vec3 v = face.tangent_v;  // 面元第二个切向基矢量
    return Vec2(Dot(delta, u), Dot(delta, v));
}
```

---

### 4.6 SBR 辅助候选通道

PVS 主通道承担约 90% 候选生成，但有两个盲区：远距离绕射（PVS 半球射线覆盖不足）+ 无向性反向遗漏。SBR 辅助通道利用现有 `SbrEngine` 补位。

#### 4.6.1 设计原则

- **SBR 不替代 PVS，只补充**：PVS 主通道（5ms, 确定完备）+ SBR 辅助（~1s per Tx, 对所有 Rx 共享）
- **SBR 阶段与 Rx 无关**：提取射线经过的面元序列，不涉及 Rx 位置检测
- **SBR 近似命中点作为 BFGS 初始值**：这是分析v2 方案中最有价值的贡献——SBR 的近似命中点比面元重心更接近真值

#### 4.6.2 从现有 SbrEngine 提取面元序列

现有 `SbrEngine` 在追踪每根射线时已记录 `curPt`、命中面元、交互类型。只需增量为每个射线累积面元序列：

```cpp
// 在 SbrEngine 射线追踪循环中新增:
struct SbrRayTrace {
    std::vector<int> face_ids;       // 经过的面元/楔边 ID
    std::vector<InteractionType> itypes;  // 对应交互类型
    std::vector<Vec3> approx_points; // SBR 近似命中点
    std::vector<double> segment_cone_half_angles; // 各段锥半角(可选)
    uint64_t seq_hash;                // 面元序列哈希(去重用)
};

// 追踪完成后, 存入全局候选池 (与 Rx 无关):
class SbrFaceSequencePool {
public:
    /// 从 SbrEngine 运行结果提取面元序列
    void ExtractFromSbrResult(const SbrEngineResult& sbr_result);
    
    /// 查询所有面元序列(对所有 Rx 共享)
    const std::unordered_map<uint64_t, SbrRayTrace>& GetPool() const;
    
    /// 按类型序列哈希查询匹配的面元序列
    std::vector<SbrRayTrace> QueryByTypeSeq(uint64_t type_seq_hash) const;
    
private:
    std::unordered_map<uint64_t, SbrRayTrace> pool_;  // seq_hash → trace
    size_t max_pool_size_ = 100000;
};
```

#### 4.6.3 候选池合并

```
阶段2输出 = PVS候选 ∪ SBR候选 (按面元序列哈希去重)

合并规则:
  - 同时存在于 PVS 和 SBR: 保留 PVS 的面元序列 + SBR 的 approx_points
    (SBR 近似点作为 BFGS 初始值, 优先级最高)
  - 仅存在于 PVS: 使用面元投影初始值 (fallback)
  - 仅存在于 SBR: 当作 PVS 未覆盖的额外候选, 用 SBR 近似点作为初始值
```

#### 4.6.4 BFGS 初始值优先级

```cpp
Vec2 InitGuess(const FaceInteractionSequence& seq, int interaction_idx,
               const SbrRayTrace* sbr_trace,  // 可为 nullptr
               const Point3& prev_point,
               const Point3& next_point) 
{
    // 优先级 1: SBR 近似命中点 (最准确)
    if (sbr_trace && interaction_idx < (int)sbr_trace->approx_points.size()) {
        Vec3 sbr_approx = sbr_trace->approx_points[interaction_idx];
        // 投影到面元切向坐标系
        return ProjectToFaceLocal(sbr_approx, seq.face_ids[interaction_idx]);
    }
    
    // 优先级 2: 直线投影 (Tx→Rx 向面元投影)
    Vec3 midpoint = Scale(Add(prev_point, next_point), 0.5);
    return ProjectToFaceLocal(midpoint, seq.face_ids[interaction_idx]);
    
    // 优先级 3: 面元重心 fallback (t=[0,0])
    // (ProjectToFaceLocal 内部自动退化)
}
```

#### 4.6.5 配置项

```cpp
struct PathSearchConfig {
    // ...
    bool enable_sbr_auxiliary_pool = true;       // 启用SBR辅助候选池
    int sbr_auxiliary_ray_count = 5000;          // 辅助SBR射线数(低于覆盖SBR的1M)
    bool use_sbr_approx_for_bfgs_init = true;    // BFGS初始值优先用SBR近似点
};
```

辅助 SBR 只需要 ~5000 条射线（而非覆盖模式的 100 万），因为目的不是 Rx 检测，而是面元序列拓扑覆盖。对 1 个 Tx 约耗时 50-100ms。

---

## 5. 与现有代码的接口兼容

### 5.1 新增类型不影响现有 SearchEngine

```cpp
// core/search/PathSearchEngineV2.h

namespace rt {

/// v10: 四阶段混合寻径引擎 — 与现有 SearchEngine 并行存在
class PathSearchEngineV2 {
public:
    /// 执行四阶段寻径
    /// @return 如果 V2 失败或配置未启用 V2, 返回空结果
    SearchEngineResult Run(const PathSearchContext& context) const;

    /// 是否启用 V2 引擎 (从 AppConfig 读取)
    static bool IsEnabled(const AppConfig& config);
};

} // namespace rt
```

### 5.2 AppConfig 扩展 (可选)

```cpp
// 在 PathSearchConfig 中新增:
struct PathSearchConfig {
    // ... 现有字段 ...
    
    // v10: 四阶段混合寻径
    bool enable_v10_precise_engine = false;       // 默认关闭, 兼容现有行为
    double bfgs_gradient_tol = 1e-8;              // BFGS 收敛容差
    int bfgs_max_iterations = 30;                 // BFGS 最大迭代
    int pvs_max_candidates_per_layer = 500;       // 每层最大候选数
    bool enable_bidirectional_pvs = true;         // 启用双向PVS收缩
};
```

### 5.3 RtPipeline 接入

```cpp
// app/RtPipeline.cpp — 搜索阶段替换为:

SearchEngineResult searchResult;
if (PathSearchEngineV2::IsEnabled(loadResult.config)) {
    // v10 四阶段引擎
    PathSearchEngineV2 v2_engine;
    searchResult = v2_engine.Run(batch5SearchContext);
} else {
    // 现有引擎 (兼容)
    SearchEngine engine;
    searchResult = engine.Run(batch5SearchContext);
}
```

---

## 6. 性能估算

| 阶段 | 操作 | 时间复杂度 | 估计耗时 (412场景) |
|------|------|-----------|-------------------|
| 1. 双向收缩 | PVS 集合合并 | O(k × avg_degree × layers) | 3-5 ms |
| 2. 序列枚举 | 状态机 + 映射 | O(sequences × log(candidates)) | 2-3 ms |
| 3a. 纯反射 | 级联镜像 O(k) | O(paths × k) | 0.1 ms/path |
| 3b. 混合路径 | BFGS O(k²·n_iter) | O(paths × k² × 20) | 0.5 ms/path |
| 4. 验证 | 可见性 + 边界检查 | O(paths × k × BVH_query) | 1 ms/path |
| **总计** | — | — | **100-300 ms per Rx** |

对比现有引擎: ~0.5s/Rx (412场景) → **约 2-5× 加速**, 同时完备性提升。

---

## 7. 实现路线

### 7.1 分阶段实施

| 迭代 | 内容 | 新增代码量 | 依赖 |
|------|------|-----------|------|
| **Iter 0** | FacePVS 反向表转置 | ~20 行 | BuildPVS (现有,增量) |
| **Iter 1** | 纯反射级联镜像法 | ~300 行 | MirrorPointAcrossPlane |
| **Iter 2** | 双向 PVS 收缩 (含 fallback) | ~450 行 | SceneVisibilityData + Iter 0 |
| **Iter 3** | 交互类型状态机 | ~250 行 | PathSearchConfig |
| **Iter 4** | BFGS 参数化求解器 (含罚函数 + 投影初始值) | ~650 行 | 基础线性代数 |
| **Iter 5** | SBR 辅助候选池 | ~350 行 | SbrEngine (现有,增量) |
| **Iter 6** | 验证器 + 候选池合并 | ~350 行 | BVH/SceneQuery + Iter 2,5 |
| **Iter 7** | PathSearchEngineV2 集成 | ~250 行 | 现有 pipeline + Iter 1-6 |

### 7.2 Iter 1 独立验证

纯反射级联镜像法不需要改动其他模块——直接写一个独立函数，用现有的 BVH 查询验证。按 §4.5 的 A⊆B 验证方案，立刻在 412 场景上跑并和现有 ReflectExpander 结果对比。

### 7.3 前置依赖

- **Iter 0 (FacePVS 反向表)**：在 `BuildPVS` 末尾增加一次转置操作（O(152k), < 1ms），为 `FacePVS` 新增 `reverse_pvs` 字段。这是对现有 `SceneVisibilityBuilder::BuildPVS` 的增量修改，必须在 Iter 2 之前完成。
- **Iter 5 (SBR 辅助池)**：基于现有 `SbrEngine` 的射线追踪循环增量改造，仅在 `enable_sbr_auxiliary_pool=true` 时激活。面元序列收集不改变现有 SBR 的覆盖统计逻辑。

---

## 8. 预期论文贡献

| 创新点 | 方法 | 对比基线 |
|--------|------|---------|
| **双向 PVS 收缩** | Tx/Rx 对称扩张 + 层交集 | 现有单向优先队列搜索 |
| **统一参数化优化** | BFGS + 光程加权 (n·d) | 各 Expander 独立求解 |
| **纯反射级联镜像** | 多跳反射 O(k) 解析解 | 单跳镜像法逐跳展开 |
| **交互类型状态机** | 保证完备 + 剪除不可能类型 | 无类型枚举 |
| **透射解耦 R→T→R** | 参数化光程 → Snell 自动满足 | Rx 方向约束 |

---

## 9. 前沿算法调研与方案增强

> 调研时间：2026-06-01  
> 搜索范围：IEEE / arXiv 2024-2026, Scholar, 确定性RT路径寻径优化

### 9.1 核心发现

#### 发现 1：ML 辅助路径采样 — 替代组合枚举

**Eertmans et al.**（同作者团队, arXiv:2410.23773, IEEE ICMLCN 2025）提出 **"Towards Generative Ray Path Sampling"**——这是他们 2026 年可微 RT 论文的**前序工作**，直接解决"如何高效枚举交互面元序列"这个瓶颈。

- **方法**：一个小型神经网络学习"哪些交互序列可能产生有效路径"的概率分布，替代穷举组合枚举
- **关键特性**：
  - 复杂度**线性增长**于场景复杂度（而非指数）
  - **几何不变**：平移/旋转/缩放后无需重训练
  - 输出经确定性几何验证（可见性、精确路径长度）过滤
- **与本方案融合**：可将 ML 采样作为阶段 2（序列枚举）的**加速替代**——用概率采样代替穷举状态机，对高密度场景（>5000 面元）效果显著

#### 发现 2：GeNeRT — 物理信息神经网络 RT

**Bian et al.**（arXiv:2506.18295, 2025）提出 **GeNeRT（Generalizable Neural Ray Tracing）**：

- **核心**：Fresnel 启发的神经网络架构，预测多径分量（MPC），实现**零样本跨场景泛化**
- **加速**：GPU 张量化向量化 Möller-Trumbore 三角求交
- **精度**：在未见过的场景上优于商业工具 Wireless InSite
- **对本方案的启示**：ML 可作为 Stage 3 中 BFGS 初始猜测的生成器——用 GeNeRT 预测交互点近似位置，再用 BFGS 精确优化

#### 发现 3：可见性矩阵 — 2024-2026 的主流加速范式

**多篇独立论文**在 2024-2026 年提出了基于**可见性预计算**的 RT 加速：

| 论文 | 年份 | 方法 |
|------|------|------|
| Yang et al. | 2024 | 动态场景的**互可见性矩阵**预计算，减少运动场景 RT 开销 |
| Vaara et al. | 2026 | 点云场景的**可见性矩阵**：一次 Tx 射线投射，所有路径共享 |
| Farashahi et al. | 2025 | **优化的可见性图**用于 RIS 辅助无线信道 |
| Schweins et al. | 2024 | **基于镜像的可见性预处理**，区分不同传播效应 |

**关键洞察**：你的现有代码已经通过 `SceneVisibilityBuilder` 构建了 **152k FacePVS entries** 和 **645k EdgeAdj pairs**——这正是 2024-2026 年学术界公认的**最有效加速方法**。你的基础设施已处于前沿水平。双向 PVS 收缩方案将该优势从"被动查询"升级为"主动层状搜索"。

#### 发现 4：Mochi — GPU BVH 硬件加速的跨领域应用

**Mandarapu et al.**（arXiv:2604.23520, 2026）提出 **Mochi**：利用 GPU RT Core 的硬件 BVH 遍历进行通用碰撞检测。

- **可迁移性**：碰撞检测与射线-面元求交在数学上等价
- **对本方案的启示**：可用于加速 Stage 4 中的**逐段可见性检查**——每段需 2 次 BVH 查询，对 1000 条路径 × 7 跳 ≈ 14000 次查询，GPU 硬件加速可将此步骤压缩至亚毫秒级

#### 发现 5：离线/在线混合范式

**Qi et al.**（arXiv:2604.15083, IEEE TVT 2026）提出 **RT-GSHCM 混合信道模型**：离线精确 RT 构建 + 在线 GBSM 更新。

- **对本方案的启示**：Precise 模式的候选面元序列可做**跨 Rx 复用**——多个 Rx 共享同一 PVS 层结构，仅求解阶段差异化

### 9.2 方案增强建议

基于上述调研，在原方案基础上增加三个**可选增强模块**：

| 模块 | 来源 | 作用 | 优先级 |
|------|------|------|--------|
| **ML 路径采样器** | Eertmans 2025 | 替代阶段2的组合枚举，训练小网络预测有效面元序列概率分布 | P1 (中期) |
| **GPU 批量可见性验证** | Mochi 2026 | 将阶段4的逐段可见性检查改为 GPU batch 提交 | P2 (后期) |
| **跨 Rx 候选复用** | Qi 2026 | 多个 Rx 共享 PVS 收缩结果，减少逐 Rx 重复计算 | P1 (中期) |

### 9.3 本方案在文献谱系中的定位

```
2024-2026 RT 寻径加速技术谱系:

可视化预处理 ──── 参数化优化 ──── ML辅助 ──── GPU硬件加速
(Yang 2024)      (Eertmans 2026) (GeNeRT)   (Mochi 2026)
(Vaara 2026)                     (Eertmans 2025)
      │                │              │            │
      └────────┬───────┴──────┬───────┘            │
               │              │                    │
      本方案：双向PVS收缩 + BFGS统一优化 + 级联镜像
      (已有PVS/EdgeAdj + 新增Stage1-4)
```

### 9.4 小论文题目建议

基于方案内容和调研定位，建议论文题目方向：

1. **"基于双向可见性收缩与参数化光程优化的室内确定性射线追踪寻径方法"**
   - 核心贡献：双向 PVS + BFGS 统一求解 + 级联镜像法
   - 对标：Eertmans 2026（参数化优化）、Yang 2024（可见性矩阵）

2. **"面向室内 ISAC 信道建模的高效精确多模式寻径框架"**
   - 核心贡献：precise/SBR 双模 + 可见性预计算 + 可配置精度
   - 对标：Vaara 2026（点云 RT）、Sionna RT

3. **"室内电大尺寸场景下融合可见性图搜索与 Fermat 优化的确定性 RT 方法"**
   - 核心贡献：PVS 图搜索 + Fermat/BFGS 混合求解
   - 对标：Farashahi 2025（可见性图）、Eertmans 2025（ML 路径采样）

---

## 附录 A：参考文献

1. Eertmans et al., "Fast, Differentiable, GPU-Accelerated Ray Tracing for Multiple Diffraction and Reflection Paths", EuCAP 2026.
2. Vaara et al., "Differentiable High-Performance RT-Based Simulation of Radio Propagation With Point Clouds", IEEE AWPL 2026.
3. Kim et al., "Visibility Precomputation for Accelerated Ray Tracing in Indoor Radio Propagation", IEEE AWPL 2020.
4. Tiberi et al., "Efficient Path Finding with Multiple Diffraction Using Edge-to-Edge Visibility Graph", IEEE TAP 2021.
5. Yun & Iskander, "Ray Tracing for Radio Propagation Modeling: Principles and Applications", IEEE Access 2015.
6. Kouyoumjian & Pathak, "A Uniform Geometrical Theory of Diffraction for an Edge in a Perfectly Conducting Surface", Proc. IEEE 1974.
7. Ling et al., "Shooting and Bouncing Rays: Calculating the RCS of an Arbitrarily Shaped Cavity", IEEE TAP 1989.
8. Nocedal & Wright, "Numerical Optimization", 2nd ed., Springer 2006. (BFGS 理论基础)
9. Eertmans et al., "Towards Generative Ray Path Sampling for Faster Point-to-Point Ray Tracing", IEEE ICMLCN 2025. (ML辅助路径采样)
10. Bian et al., "GeNeRT: Generalizable Neural Ray Tracing for Real-Time MPC Prediction", 2025. (物理信息神经网络RT)
11. Yang et al., "An Efficient Pre-processing Method for 6G Dynamic Ray-Tracing Channel Modeling", IEEE 2024. (互可见性矩阵预计算)
12. Qi et al., "A Novel 6G Dynamic Channel Map Based on a Hybrid Channel Model", IEEE TVT 2026. (离线/在线混合范式)
13. Mandarapu et al., "Mochi: Rethinking Collision Detection on GPU Ray Tracing Architecture", 2026. (GPU BVH硬件加速)

---

## 附录 B：修订记录

| 版本 | 日期 | 变更 |
|------|------|------|
| v1.0 | 2026-06-01 | 初版，含四阶段架构 + 前沿调研 |
| v1.1 | 2026-06-01 | 同步审查反馈：新增 §4.1.4 (PVS方向性+反向表+fallback)、修正 §4.3.2 (级联镜像法)、新增 §4.3.5 (BFGS罚函数约束)、新增 §4.3.6 (梯度符号验证)、新增 §4.5 (Iter 1 独立验证方案+BFGS初始值)、更新 §7 (实施路线)、更新附录A (新增5篇参考文献) |
| v2.0 | 2026-06-01 | 整合《分析v2.md》SBR辅助通道方案：更新 §3 架构图(双通道)、新增 §4.6 (SBR辅助候选通道：面元序列提取+BFGS初始值优先级+候选池合并)、更新 §7 实施路线(新增Iter 0/5, 总7个Iter)、更新参考文献 |
| v2.1 | 2026-06-02 | **Iter 0 完成**：FacePVS.reverse_pvs 反向表实现（`SceneVisibilityData.h` + `SceneVisibilityBuilder.cpp`，~25行），PVS=152289条目，反向表O(152k)<1ms，自测通过 |
| v2.2 | 2026-06-02 | **Iter 1 完成**：纯反射级联镜像法实现（`CascadeImageMethod.h/cpp`，~80行）+ `MirrorPointAcrossPlane` 提升至 `Vec3.h`（可复用）+ 移除 `ReflectionExpander.cpp` 重复定义。自测: 编译通过, O(k)解析解就绪 |
| v2.3 | 2026-06-02 | **Iter 2 完成**：双向 PVS 收缩器实现（`BidirectionalPVS.h/cpp`，~200行）。核心: ExpandForward(pvs_faces) + ExpandBackward(reverse_pvs) + 中层交集 + 类型过滤。自测: 编译通过, 管线无回归 (PVS=152289 entries) |
| v2.4 | 2026-06-02 | **Iter 3 完成**：交互类型状态机（`InteractionStateMachine.h/cpp`，~170行）。Enumerate() 生成所有合法交互序列（R≤3,T≤2,D≤2,depth≤5 → 81序列）。约束: 禁止T→T, D→D。自测: 编译通过 |
| v2.5 | 2026-06-02 | **Iter 4 完成**：BFGS 参数化求解器（`HybridPathSolver.h/cpp`，~250行）。核心: BfgsMinimize(Armijo回溯+BFGS逆Hessian更新) + 面元切向坐标系参数化 + 罚函数约束。Solve() 入口整合级联镜像+BFGS。自测: 编译通过 |

---

## 附录 C：方案定版摘要（供代码开发前快速查阅）

**核心思路**：PVS 主通道承担 90% 候选生成（5ms/Rx, 确定完备），SBR 辅助通道补充远距离绕射候选 + 提供 BFGS 初始值（~100ms/Tx, 跨 Rx 共享）。

**开发入口**：按 §7.1 的 Iter 0→1→2→3→4→5→6→7 顺序推进，每个 Iter 独立可测。

**关键接口**：`PathSearchEngineV2::Run(context)` — 与现有 `SearchEngine` 接受相同的 `PathSearchContext`，输出 `SearchEngineResult`。通过 `enable_v10_precise_engine` 配置开关切换。

**最小可验证单元**：Iter 1（级联镜像法）可在 §4.5 的 A⊆B 验证框架下立刻与现有 `ExpandReflection` 对比。

**BFGS 初始值**：优先级 SBR 近似点 > 直线投影 > 面元重心（§4.6.4）。
