// v10 Iter4: 混合路径精确求解器 — BFGS 核心实现

#include "HybridPathSolver.h"
#include "CascadeImageMethod.h"
#include "../query/SceneQuery.h"
#include <cmath>
#include <cstdio>

namespace rt {

// ── 反射子链 ──
bool HybridPathSolver::SolveReflectionChain(
    const std::vector<int>& face_ids, int start_idx, int end_idx,
    const Point3& source, const Point3& target,
    const Scene& scene, SceneQuery& query,
    std::vector<Point3>& out_points)
{
    std::vector<int> sub(face_ids.begin() + start_idx, face_ids.begin() + end_idx);
    CascadeImageResult r = SolveCascadeReflection(source, target, sub, scene, query);
    if (!r.valid) return false;
    out_points = r.reflection_points;
    return true;
}

// ── 损失函数 ──
double HybridPathSolver::OpticalPathLength(
    const std::vector<ParameterizedInteraction>& params,
    const Point3& tx, const Point3& rx) const
{
    double L = 0.0; Point3 prev = tx; double np = 1.0;
    for (size_t i = 0; i < params.size(); ++i) {
        Point3 xi = params[i].WorldPos();
        L += np * Length(Subtract(xi, prev));
        for (int k = 0; k < params[i].num_params(); ++k) {
            double v = params[i].t[k];
            if (v < -1.0) L += config_.constraint_penalty * v * v;
            else if (v > 2.0) L += config_.constraint_penalty * (v-1.0)*(v-1.0);
        }
        prev = xi; np = params[i].n_after;
    }
    L += np * Length(Subtract(rx, prev));
    return L;
}

// ── 梯度 ──
void HybridPathSolver::ComputeGradient(
    const std::vector<ParameterizedInteraction>& params,
    const Point3& tx, const Point3& rx,
    std::vector<double>& grad) const
{
    int n = 0; for (auto& p : params) n += p.num_params();
    grad.assign(n, 0.0);
    int off = 0;
    for (size_t i = 0; i < params.size(); ++i) {
        const auto& p = params[i];
        Point3 xi = p.WorldPos();
        Point3 xp = (i == 0) ? tx : params[i-1].WorldPos();
        Point3 xn = (i+1 < params.size()) ? params[i+1].WorldPos() : rx;
        double dp = Length(Subtract(xi, xp));
        double dn = Length(Subtract(xn, xi));
        if (dp < 1e-12 || dn < 1e-12) { off += p.num_params(); continue; }
        Vec3 di = Scale(Subtract(xi, xp), 1.0/dp);
        Vec3 dout = Scale(Subtract(xn, xi), 1.0/dn);
        for (int k = 0; k < p.num_params(); ++k) {
            Vec3 basis = MakeVec3(p.A[0+k], p.A[2+k], p.A[4+k]);
            grad[off+k] = p.n_before * Dot(di, basis) - p.n_after * Dot(dout, basis);
        }
        off += p.num_params();
    }
}

// ── BFGS 逆 Hessian 更新 ──
void HybridPathSolver::UpdateInverseHessian(
    std::vector<double>& H, int n, const std::vector<double>& s, const std::vector<double>& y)
{
    double rden = 0.0; for (int i = 0; i < n; ++i) rden += y[i] * s[i];
    if (std::fabs(rden) < 1e-15) return;
    double rho = 1.0 / rden;
    std::vector<double> Hy(n, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            Hy[i] += H[i*n + j] * y[j];
    double yHy = 0.0; for (int i = 0; i < n; ++i) yHy += y[i] * Hy[i];
    double fac = rho * (rho * yHy + 1.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            H[i*n + j] += fac * s[i] * s[j] - rho * (s[i] * Hy[j] + Hy[i] * s[j]);
}

void HybridPathSolver::FlattenParams(const std::vector<ParameterizedInteraction>& params, std::vector<double>& t) const {
    int n = 0; for (auto& p : params) n += p.num_params();
    t.resize(n); int o = 0;
    for (auto& p : params) { for (int k = 0; k < p.num_params(); ++k) t[o+k] = p.t[k]; o += p.num_params(); }
}
void HybridPathSolver::UnflattenParams(std::vector<ParameterizedInteraction>& params, const std::vector<double>& t) const {
    int o = 0; for (auto& p : params) { for (int k = 0; k < p.num_params(); ++k) p.t[k] = t[o+k]; o += p.num_params(); }
}

// ── 回溯线搜索 (Armijo, 内联于 BfgsMinimize) ──
double HybridPathSolver::FixedPointLineSearch(
    const std::vector<ParameterizedInteraction>& /*params*/,
    const std::vector<double>& /*direction*/, const Point3& /*tx*/, const Point3& /*rx*/) const
{
    return 1e-3; // fallback: BfgsMinimize 内部已实现完整 Armijo 回溯
}

// ═══════════════════════════════════════════════════════════
// BFGS 核心优化
// ═══════════════════════════════════════════════════════════

bool HybridPathSolver::BfgsMinimize(
    std::vector<ParameterizedInteraction>& params,
    const Point3& tx, const Point3& rx)
{
    int n = 0; for (auto& p : params) n += p.num_params();
    if (n == 0) return true;

    // 初始化: t = 0 (面元重心)
    std::vector<double> t(n, 0.0);
    // 初始 Hessian 逆 = I
    std::vector<double> H_inv(n * n, 0.0);
    for (int i = 0; i < n; ++i) H_inv[i * n + i] = 1.0;

    std::vector<double> grad(n), grad_old(n);
    std::vector<double> s(n), y(n), p_dir(n);

    UnflattenParams(params, t);
    double f_old = OpticalPathLength(params, tx, rx);
    ComputeGradient(params, tx, rx, grad);
    grad_old = grad;

    for (int iter = 0; iter < config_.max_iterations; ++iter) {
        // 收敛检查
        double gn = 0.0; for (double g : grad) gn += g * g;
        if (gn < config_.gradient_tol * config_.gradient_tol) break;

        // 搜索方向: p = -H_inv * grad
        for (int i = 0; i < n; ++i) {
            p_dir[i] = 0.0;
            for (int j = 0; j < n; ++j) p_dir[i] -= H_inv[i * n + j] * grad[j];
        }

        // 线搜索: 简单 Armijo 回溯
        double alpha = 1.0, c = 1e-4, rho = 0.5;
        double gTd = 0.0; for (int i = 0; i < n; ++i) gTd += grad[i] * p_dir[i];
        if (gTd >= 0.0) { alpha = 0.0; }

        std::vector<double> t_new(n);
        for (int ls = 0; ls < 20 && alpha > config_.line_search_tol; ++ls) {
            for (int i = 0; i < n; ++i) t_new[i] = t[i] + alpha * p_dir[i];
            UnflattenParams(params, t_new);
            double f_new = OpticalPathLength(params, tx, rx);
            // Armijo condition: f_new <= f_old + c*alpha*gTd
            if (f_new <= f_old + c * alpha * gTd + 1e-15) break;
            alpha *= rho;
        }

        // 更新 t
        for (int i = 0; i < n; ++i) { s[i] = t_new[i] - t[i]; t[i] = t_new[i]; }
        UnflattenParams(params, t);

        // 新梯度
        ComputeGradient(params, tx, rx, grad);
        for (int i = 0; i < n; ++i) y[i] = grad[i] - grad_old[i];
        grad_old = grad;

        // BFGS 更新
        UpdateInverseHessian(H_inv, n, s, y);
    }

    return true;
}

// ── 主入口 ──

bool HybridPathSolver::Solve(
    const std::vector<int>& face_ids,
    const std::vector<int>& wedge_ids,
    const std::vector<InteractionType>& types,
    const Point3& tx, const Point3& rx,
    const Scene& scene, const SceneQuery& query,
    std::vector<PathNode>& out_nodes)
{
    int m = (int)types.size();
    if (m == 0) return true; // LOS only
    out_nodes.clear();

    // 构建参数化交互
    std::vector<ParameterizedInteraction> params(m);
    for (int i = 0; i < m; ++i) {
        params[i].type = types[i];
        params[i].face_id = (i < (int)face_ids.size()) ? face_ids[i] : -1;
        params[i].wedge_id = (i < (int)wedge_ids.size()) ? wedge_ids[i] : -1;
        params[i].n_before = 1.0; params[i].n_after = 1.0;

        if (types[i] != InteractionType::Diffraction && params[i].face_id >= 0) {
            const Face& f = scene.faces[params[i].face_id];
            // 构建面元切向坐标系
            Vec3 n = f.normal;
            Vec3 u, v;
            if (std::fabs(n.x) < 0.9) { u = Normalize(Cross(n, MakeVec3(1,0,0))); }
            else { u = Normalize(Cross(n, MakeVec3(0,1,0))); }
            v = Normalize(Cross(n, u));
            params[i].A[0]=u.x; params[i].A[1]=v.x; params[i].A[2]=u.y;
            params[i].A[3]=v.y; params[i].A[4]=u.z; params[i].A[5]=v.z;
            params[i].b[0]=f.centroid.x; params[i].b[1]=f.centroid.y; params[i].b[2]=f.centroid.z;
            params[i].t[0]=0.0; params[i].t[1]=0.0;
        } else if (types[i] == InteractionType::Diffraction && params[i].wedge_id >= 0) {
            const Wedge& w = scene.wedges[params[i].wedge_id];
            Vec3 e = w.direction;
            params[i].A[0]=e.x; params[i].A[2]=e.y; params[i].A[4]=e.z;
            params[i].b[0]=w.segment_start.x; params[i].b[1]=w.segment_start.y; params[i].b[2]=w.segment_start.z;
            params[i].t[0]=0.5; params[i].t[1]=0.0;
        }
    }

    // BFGS 优化
    BfgsMinimize(params, tx, rx);

    // 构建 PathNode
    Point3 prev = tx;
    for (int i = 0; i < m; ++i) {
        PathNode node;
        node.interaction_type = types[i];
        node.face_id = params[i].face_id;
        node.wedge_id = params[i].wedge_id;
        node.point = params[i].WorldPos();
        node.direction = (i+1 < m) ? Normalize(Subtract(params[i+1].WorldPos(), node.point))
                                    : Normalize(Subtract(rx, node.point));
        node.incident_direction = Normalize(Subtract(node.point, prev));
        node.segment_length_from_previous = Length(Subtract(node.point, prev));
        node.valid = true;
        out_nodes.push_back(node);
        prev = node.point;
    }

    return true;
}

} // namespace rt
