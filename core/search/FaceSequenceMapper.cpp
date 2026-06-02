// v10: 面元序列映射器实现

#include "FaceSequenceMapper.h"
#include <algorithm>
#include <cstdio>
#include <cstdint>
#include <iterator>
#include <cmath>

namespace rt {

bool FaceInteractionSequence::IsPureReflection() const {
    for (auto t : types)
        if (t != InteractionType::Reflection) return false;
    return true;
}

bool FaceInteractionSequence::HasDiffraction() const {
    for (auto t : types)
        if (t == InteractionType::Diffraction) return true;
    return false;
}

// ═══════════════════════════════════════════════════════════
// 类型过滤辅助 (在使用前定义)
// ═══════════════════════════════════════════════════════════

static void FilterByType(std::vector<int>& faces, InteractionType it, const Scene& scene) {
    std::vector<int> filtered;
    for (int fid : faces) {
        if (fid < 0 || fid >= (int)scene.faces.size()) continue;
        const Face& f = scene.faces[fid];
        if (f.degenerate) continue;
        switch (it) {
            case InteractionType::Reflection:
                if (f.reflection_enabled) filtered.push_back(fid);
                break;
            case InteractionType::Transmission:
                if (f.transmission_enabled && f.dual_side_material_resolved) filtered.push_back(fid);
                break;
            default: break;
        }
    }
    faces.swap(filtered);
}

// ═══════════════════════════════════════════════════════════
// 主入口
// ═══════════════════════════════════════════════════════════

std::vector<FaceInteractionSequence> FaceSequenceMapper::Map(
    const std::vector<InteractionSequence>& type_seqs,
    const BidirectionalPVSResult& pvs_result,
    const Scene& scene,
    int max_combinations) const
{
    std::vector<FaceInteractionSequence> output;
    for (const auto& seq : type_seqs) {
        MapOneSequence(seq, pvs_result, scene, max_combinations, output);
    }
    std::fprintf(stderr, "[FaceSeqMapper] %zu type seqs -> %zu face seqs\n",
                 type_seqs.size(), output.size());
    return output;
}

// ═══════════════════════════════════════════════════════════
// 单个类型序列 -> 面元组合
// ═══════════════════════════════════════════════════════════

void FaceSequenceMapper::MapOneSequence(
    const InteractionSequence& seq,
    const BidirectionalPVSResult& pvs,
    const Scene& scene,
    int max_combinations,
    std::vector<FaceInteractionSequence>& output) const
{
    int D = seq.total_depth;
    if (D <= 0) return;

    std::vector<std::vector<int>> candidates(D);
    for (int i = 0; i < D; ++i) {
        InteractionType it = seq.steps[i].type;

        int fwdIdx = i + 1;
        int bwdIdx = D - i - 1;
        const auto& fwd_layer = (fwdIdx < (int)pvs.forward_layers.size())
            ? pvs.forward_layers[fwdIdx] : CandidateLayer{};
        const auto& bwd_layer = (bwdIdx < (int)pvs.backward_layers.size())
            ? pvs.backward_layers[bwdIdx] : CandidateLayer{};

        // 优先用正向层
        if (!fwd_layer.face_ids.empty()) {
            candidates[i] = fwd_layer.face_ids;
        } else if (!bwd_layer.face_ids.empty()) {
            candidates[i] = bwd_layer.face_ids;
        }

        // 如果 midpoints[i] 有交集数据, 用它缩减
        if (i < (int)pvs.midpoints.size() && !pvs.midpoints[i].face_ids.empty()) {
            std::vector<int> mid = pvs.midpoints[i].face_ids;
            if (!candidates[i].empty()) {
                std::sort(mid.begin(), mid.end());
                std::sort(candidates[i].begin(), candidates[i].end());
                std::vector<int> isect;
                std::set_intersection(candidates[i].begin(), candidates[i].end(),
                                      mid.begin(), mid.end(), std::back_inserter(isect));
                candidates[i] = std::move(isect);
            }
        }

        FilterByType(candidates[i], it, scene);

        if (candidates[i].empty()) return; // 该类型序列在当前场景不可能
    }

    ExpandCombinations(candidates, seq, max_combinations, output);
}

// ═══════════════════════════════════════════════════════════
// 层候选获取
// ═══════════════════════════════════════════════════════════

std::vector<int> FaceSequenceMapper::GetLayerCandidates(
    InteractionType type, int depth,
    const BidirectionalPVSResult& pvs,
    const Scene& scene) const
{
    const auto& mid_layer = (depth < (int)pvs.midpoints.size())
        ? pvs.midpoints[depth] : CandidateLayer{};
    std::vector<int> out = mid_layer.face_ids;
    FilterByType(out, type, scene);
    return out;
}

// ═══════════════════════════════════════════════════════════
// 笛卡尔积 (有限制)
// ═══════════════════════════════════════════════════════════

void FaceSequenceMapper::ExpandCombinations(
    const std::vector<std::vector<int>>& layer_candidates,
    const InteractionSequence& seq,
    int max_combinations,
    std::vector<FaceInteractionSequence>& output) const
{
    int D = seq.total_depth;

    // 如果组合数小, 完整展开; 否则随机采样
    size_t total = 1;
    for (int i = 0; i < D; ++i) {
        total *= layer_candidates[i].empty() ? 1 : layer_candidates[i].size();
        if (total > (size_t)max_combinations * 10) break;
    }

    auto addSeq = [&](const std::vector<int>& indices) {
        FaceInteractionSequence fseq;
        fseq.types.resize(D);
        fseq.face_ids.resize(D, -1);
        fseq.wedge_ids.resize(D, -1);
        fseq.type_sig = seq.type_signature;
        for (int i = 0; i < D; ++i) {
            fseq.types[i] = seq.steps[i].type;
            fseq.face_ids[i] = layer_candidates[i].empty() ? -1 : layer_candidates[i][indices[i]];
        }
        output.push_back(fseq);
    };

    if (total <= (size_t)max_combinations) {
        std::vector<int> indices(D, 0);
        for (size_t count = 0; count < total && (int)output.size() < max_combinations; ++count) {
            addSeq(indices);
            for (int i = D - 1; i >= 0; --i) {
                indices[i]++;
                if (indices[i] < (int)layer_candidates[i].size()) break;
                indices[i] = 0;
            }
        }
    } else {
        int per = std::max(1, (int)std::pow((double)max_combinations, 1.0 / D));
        for (int c = 0; c < max_combinations; ++c) {
            std::vector<int> indices(D, 0);
            for (int i = 0; i < D; ++i)
                indices[i] = c % std::min((int)layer_candidates[i].size(), per);
            addSeq(indices);
        }
    }
}

} // namespace rt
