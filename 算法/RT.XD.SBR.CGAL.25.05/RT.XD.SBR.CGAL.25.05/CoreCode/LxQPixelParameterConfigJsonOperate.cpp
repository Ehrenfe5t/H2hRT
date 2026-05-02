
#include"LxQPixelParameterConfigJsonOperate.h"

namespace PixelParameterConfigStd {
    /// <summary>
    /// 从json字符串获取点
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void from_json(const nlohmann::json& j, PixelParameterConfig& obj) {
        j.at("stepLength").get_to(obj.stepLength);

    }
    /// <summary>
    /// 将点对象转化为json字符串
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void to_json(nlohmann::json& j, const PixelParameterConfig& obj) {
        j = nlohmann::json{
        {"stepLength",obj.stepLength }
        };
    }
}