// v10 Iter5: SBR辅助候选池 — 轻量SBR提取面元序列 + 近似命中点
// 目的: 补充PVS盲区(远距离绕射) + 为BFGS提供优于面元重心的初始值
// 射线数: ~5000 (vs 覆盖模式1M), 跨Rx共享

#pragma once

#include "../common/config/AppConfig.h"
#include "../scene/Scene.h"
#include "../path/InteractionType.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace rt {

class SceneQuery;

/// SBR 单条射线的追踪记录
struct SbrRayTrace {
    std::vector<int> face_ids;            // 经过的面元ID序列
    std::vector<int> wedge_ids;           // 经过的楔边ID序列
    std::vector<InteractionType> types;   // 交互类型序列
    std::vector<Vec3> approx_points;      // SBR近似命中点 (BFGS初始值)
    uint64_t face_seq_hash = 0;           // 面元序列哈希 (去重用)
    double total_length = 0.0;            // 射线总长度
    bool valid = false;
};

/// SBR 面元序列池
class SbrAuxiliaryPool {
public:
    SbrAuxiliaryPool() = default;

    /// 清空池
    void Clear() { pool_.clear(); }

    /// 添加一条射线追踪记录
    void Insert(const SbrRayTrace& trace);

    /// 按面元序列哈希查找匹配的追踪记录
    /// @return 匹配的SBR近似点 (可为空)
    const SbrRayTrace* QueryByHash(uint64_t hash) const;

    /// 按面元序列查找 (精确匹配)
    const SbrRayTrace* QueryBySequence(const std::vector<int>& face_ids,
                                       const std::vector<int>& wedge_ids) const;

    /// 池大小
    size_t Size() const { return pool_.size(); }

    /// 从现有 SbrEngine 运行结果提取面元序列
    /// @param sbr_rays SBR引擎产生的射线命中数据
    /// @param scene 场景对象
    void ExtractFromSbrRays(
        const std::vector<std::vector<std::pair<int, Vec3>>>& sbr_rays,
        const Scene& scene);

    /// 计算面元序列哈希
    static uint64_t HashFaceSequence(const std::vector<int>& face_ids);

private:
    std::unordered_map<uint64_t, SbrRayTrace> pool_;
    size_t max_pool_size_ = 100000;
};

} // namespace rt
