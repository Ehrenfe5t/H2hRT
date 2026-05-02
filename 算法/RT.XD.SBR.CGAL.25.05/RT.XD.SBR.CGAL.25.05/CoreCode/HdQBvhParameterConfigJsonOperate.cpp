
#include"HdQBvhParameterConfigJsonOperate.h"

namespace BvhParameterConfigStd {
    /// <summary>
    /// 从json字符串获取点
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void from_json(const nlohmann::json& j, BvhParameterConfig& obj) {
        j.at("maxLevel").get_to(obj.maxLevel);

    }
    /// <summary>
    /// 将点对象转化为json字符串
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void to_json(nlohmann::json& j, const BvhParameterConfig& obj) {
        j = nlohmann::json{
        {"maxLevel",obj.maxLevel }
        };
    }
}