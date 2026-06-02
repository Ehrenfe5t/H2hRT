// v10 Iter5: SBR辅助候选池实现

#include "SbrAuxiliaryPool.h"
#include <algorithm>

namespace rt {

// ── 面元序列哈希 ──

uint64_t SbrAuxiliaryPool::HashFaceSequence(const std::vector<int>& face_ids) {
    uint64_t h = 0;
    for (int fid : face_ids) {
        h = h * 31 + static_cast<uint64_t>(fid + 1);
    }
    return h;
}

// ── 插入 ──

void SbrAuxiliaryPool::Insert(const SbrRayTrace& trace) {
    if (!trace.valid) return;
    if (pool_.size() >= max_pool_size_) return;

    uint64_t hash = HashFaceSequence(trace.face_ids);
    // 如果已存在，保留较短路径的版本
    auto it = pool_.find(hash);
    if (it != pool_.end()) {
        if (trace.total_length < it->second.total_length) {
            it->second = trace;
            it->second.face_seq_hash = hash;
        }
    } else {
        SbrRayTrace t = trace;
        t.face_seq_hash = hash;
        pool_[hash] = t;
    }
}

// ── 查询 ──

const SbrRayTrace* SbrAuxiliaryPool::QueryByHash(uint64_t hash) const {
    auto it = pool_.find(hash);
    return (it != pool_.end()) ? &it->second : nullptr;
}

const SbrRayTrace* SbrAuxiliaryPool::QueryBySequence(
    const std::vector<int>& face_ids,
    const std::vector<int>& /*wedge_ids*/) const
{
    return QueryByHash(HashFaceSequence(face_ids));
}

// ── 从 SBR 射线提取 ──

void SbrAuxiliaryPool::ExtractFromSbrRays(
    const std::vector<std::vector<std::pair<int, Vec3>>>& sbr_rays,
    const Scene& /*scene*/)
{
    for (const auto& ray_hits : sbr_rays) {
        if (ray_hits.empty()) continue;

        SbrRayTrace trace;
        trace.valid = true;
        trace.total_length = 0.0;

        for (size_t i = 0; i < ray_hits.size(); ++i) {
            int hit_face = ray_hits[i].first;
            Vec3 hit_pt = ray_hits[i].second;
            trace.face_ids.push_back(hit_face);
            trace.approx_points.push_back(hit_pt);

            // 推断交互类型 — 简化: 默认为 Reflection
            // (完整实现需从SBR引擎获取具体的交互类型)
            trace.types.push_back(InteractionType::Reflection);
        }

        Insert(trace);
    }
}

} // namespace rt
