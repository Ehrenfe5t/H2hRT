
#include"DxQSpaceSubregionParameterConfigJsonOperate.h"

#include"LxQPixelParameterConfigJsonOperate.h"
#include"HdQBvhParameterConfigJsonOperate.h"

namespace SpaceSubregionParameterConfigStd {
    /// <summary>
    /// 从json字符串获取点
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void from_json(const nlohmann::json& j, SpaceSubregionParameterConfig& obj) {
        j.at("type").get_to(obj.type);
        j.at("pixelParameterConfig").get_to(obj.pixelParameterConfig);
        j.at("bvhParameterConfig").get_to(obj.bvhParameterConfig);
        
    }
    /// <summary>
    /// 将点对象转化为json字符串
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void to_json(nlohmann::json& j, const SpaceSubregionParameterConfig& obj) {
        j = nlohmann::json{
        {"type",obj.type },
        {"pixelParameterConfig",obj.pixelParameterConfig },
        {"bvhParameterConfig",obj.bvhParameterConfig }
        };
    }
}