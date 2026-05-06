// 文件目标：
// - 实现模块5批次7的路径预整理逻辑。
//
// 主要功能：
// - 校验输入指针、路径有效性和最小节点数量；
// - 为后续自由空间段传播和交互更新建立统一前置条件；
// - 确保模块5不反向重建几何语义。

#include "PreparePathForEM.h"

namespace rt {

/// <summary>
/// 对路径做进入模块5前的最小预检查。
/// </summary>
/// <param name="input">电磁求解输入。</param>
/// <returns>true 表示路径可进入模块5主链；false 表示应阻断。</returns>
bool PreparePathForEM(const EMSolverInput& input)
{
    if (input.config == nullptr || input.scene == nullptr || input.path == nullptr)
    {
        return false;
    }
    if (!input.path->valid)
    {
        return false;
    }
    if (input.path->nodes.size() < 2)
    {
        return false;
    }

    bool transmissionSemanticComplete = true;
    int firstTransmissionMediumIn = -1;
    int firstTransmissionMediumOut = -1;
    bool firstTransmissionCaptured = false;

    for (const PathNode& node : input.path->nodes)
    {
        if (node.interaction_type != InteractionType::Transmission)
        {
            continue;
        }

        if (!node.transmission_semantic_complete)
        {
            transmissionSemanticComplete = false;
            break;
        }

        if (node.medium_in_id < 0 || node.medium_out_id < 0)
        {
            transmissionSemanticComplete = false;
            break;
        }

        if (node.medium_in_id == node.medium_out_id)
        {
            transmissionSemanticComplete = false;
            break;
        }

        if (!firstTransmissionCaptured)
        {
            firstTransmissionMediumIn = node.medium_in_id;
            firstTransmissionMediumOut = node.medium_out_id;
            firstTransmissionCaptured = true;
        }
    }

    const_cast<EMSolverInput&>(input).transmission_semantic_complete = transmissionSemanticComplete;
    const_cast<EMSolverInput&>(input).first_transmission_medium_in_id = firstTransmissionMediumIn;
    const_cast<EMSolverInput&>(input).first_transmission_medium_out_id = firstTransmissionMediumOut;

    if (!transmissionSemanticComplete)
    {
        return false;
    }

    return true;
}

} // namespace rt
