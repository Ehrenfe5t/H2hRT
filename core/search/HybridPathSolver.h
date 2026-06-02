// v10 Iter4: 混合路径精确求解器
// 级联镜像法 (纯反射 O(k)) + BFGS 参数化优化 (混合路径)
// 参考文献: Eertmans et al. EuCAP 2026, Nocedal & Wright 2006

#pragma once

#include "../common/math/Vec3.h"
#include "../path/PathNode.h"
#include "../path/InteractionType.h"
#include "../scene/Scene.h"

#include <vector>
#include <functional>

namespace rt {

class SceneQuery;
class MaterialDatabase;

/// 交互点的参数化描述
struct ParameterizedInteraction {
    int face_id = -1;              // 约束面元 (-1 表示楔边)
    int wedge_id = -1;             // 楔边索引 (-1 表示面元)
    InteractionType type = InteractionType::Reflection;

    // 参数化: 世界坐标 x = b + A * t
    // 面元 2D: A[0..5] = [u_x, v_x, u_y, v_y, u_z, v_z], t[0..1]
    // 楔边 1D: A[0..5] = [e_x, 0, e_y, 0, e_z, 0], t[0] only
    double A[6] = {0};
    double b[3] = {0};

    double t[2] = {0, 0};          // 参数化坐标 (变量)

    double n_before = 1.0;         // 入射侧折射率
    double n_after = 1.0;          // 出射侧折射率

    int num_params() const { return (type == InteractionType::Diffraction) ? 1 : 2; }

    /// 从参数坐标计算世界坐标
    Point3 WorldPos() const {
        return MakeVec3(
            b[0] + A[0]*t[0] + A[1]*t[1],
            b[1] + A[2]*t[0] + A[3]*t[1],
            b[2] + A[4]*t[0] + A[5]*t[1]);
    }
};

/// BFGS 求解器配置
struct BfgsSolverConfig {
    int max_iterations = 30;
    double gradient_tol = 1e-8;
    double line_search_tol = 1e-6;
    double constraint_penalty = 1e6;  // 罚函数系数
};

/// 混合路径精确求解器
class HybridPathSolver {
public:
    explicit HybridPathSolver(const BfgsSolverConfig& cfg = BfgsSolverConfig{})
        : config_(cfg) {}

    /// 求解给定面元序列的精确交互点位置
    /// @return true 如果 BFGS 收敛且所有硬约束满足
    bool Solve(
        const std::vector<int>& face_ids,
        const std::vector<int>& wedge_ids,
        const std::vector<InteractionType>& types,
        const Point3& tx, const Point3& rx,
        const Scene& scene, const SceneQuery& query,
        std::vector<PathNode>& out_nodes);

private:
    /// 纯反射子链: 级联镜像法 O(k)
    bool SolveReflectionChain(
        const std::vector<int>& face_ids,
        int start_idx, int end_idx,
        const Point3& source, const Point3& target,
        const Scene& scene, SceneQuery& query,
        std::vector<Point3>& out_points);

    /// BFGS 核心优化
    bool BfgsMinimize(
        std::vector<ParameterizedInteraction>& params,
        const Point3& tx, const Point3& rx);

    /// 光程损失函数 L(T) = Σ n_{before,i}·|x_i - x_{i-1}|
    double OpticalPathLength(
        const std::vector<ParameterizedInteraction>& params,
        const Point3& tx, const Point3& rx) const;

    /// 解析梯度 ∇L(T)
    void ComputeGradient(
        const std::vector<ParameterizedInteraction>& params,
        const Point3& tx, const Point3& rx,
        std::vector<double>& grad) const;

    /// BFGS 逆 Hessian 更新
    static void UpdateInverseHessian(
        std::vector<double>& H_inv, int n,
        const std::vector<double>& s,
        const std::vector<double>& y);

    /// 固定点线搜索 (Eertmans 2026)
    double FixedPointLineSearch(
        const std::vector<ParameterizedInteraction>& params,
        const std::vector<double>& direction,
        const Point3& tx, const Point3& rx) const;

    /// 将参数化交互展平为1D向量
    void FlattenParams(const std::vector<ParameterizedInteraction>& params, std::vector<double>& t) const;
    void UnflattenParams(std::vector<ParameterizedInteraction>& params, const std::vector<double>& t) const;

    BfgsSolverConfig config_;
};

} // namespace rt
