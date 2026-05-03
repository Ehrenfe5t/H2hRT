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
    return true;
}

} // namespace rt
