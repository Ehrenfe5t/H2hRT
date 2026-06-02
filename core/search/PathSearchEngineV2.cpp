// v10 Iter7: PathSearchEngineV2 集成实现 — 四阶段管线骨架

#include "PathSearchEngineV2.h"
#include "BidirectionalPVS.h"
#include "InteractionStateMachine.h"
#include "HybridPathSolver.h"
#include "CascadeImageMethod.h"
#include "PathValidator.h"
#include "../query/SceneQuery.h"
#include <cstdio>

namespace rt {

SearchEngineResult PathSearchEngineV2::Run(const PathSearchContext& context) const
{
    SearchEngineResult result;
    result.uses_real_scene_query = true;

    if (!context.config || !context.scene || !context.scene_query) {
        result.trace_lines.push_back("V2 aborted: incomplete context");
        return result;
    }

    const auto& cfg = context.config->path_search;
    std::fprintf(stderr, "[V2] PathSearchEngineV2 starting: R<=%d T<=%d D<=%d depth<=%d\n",
                 cfg.max_reflection_count, cfg.max_transmission_count,
                 cfg.max_diffraction_count, cfg.max_path_depth);

    // ── Stage 0: 候选生成 ──
    // BidirectionalPVS (主通道)
    BidirectionalPVSContraction pvs_contractor;
    BidirectionalPVSResult pvs_result = pvs_contractor.Contract(
        context.tx_point, context.rx_point, cfg, *context.scene, *context.scene_query);

    // SBR 辅助池 (可选, 默认关闭)
    // if (cfg.enable_sbr_auxiliary_pool) { ... }

    if (!pvs_result.valid) {
        result.trace_lines.push_back("V2: BidirectionalPVS failed — falling back to V1");
        return result;  // caller should fallback to SearchEngine
    }

    // ── Stage 1: 交互类型序列枚举 ──
    InteractionStateMachine ism;
    std::vector<InteractionSequence> sequences = ism.Enumerate(cfg);

    std::fprintf(stderr, "[V2] PVS layers: %zu forward, %zu backward, %zu midpoint\n",
                 pvs_result.forward_layers.size(),
                 pvs_result.backward_layers.size(),
                 pvs_result.midpoints.size());
    std::fprintf(stderr, "[V2] Interaction sequences: %zu\n", sequences.size());

    // ── Stage 2: 面元序列映射 + Stage 3: 精确求解 ──
    // (完整实现需 FaceSequenceMapper + 从 PVS 层映射类型序列→面元序列)
    // 当前为集成骨架 — 面元序列映射和求解在后续迭代中完善

    // ── Stage 4: 虚拟验证 ──
    // PathValidator validator;
    // ...

    result.generated_state_count = static_cast<int>(sequences.size());
    result.trace_lines.push_back("V2: Stage 0-3 skeleton executed, Stage 4 pending");
    result.succeeded = true;
    return result;
}

} // namespace rt
