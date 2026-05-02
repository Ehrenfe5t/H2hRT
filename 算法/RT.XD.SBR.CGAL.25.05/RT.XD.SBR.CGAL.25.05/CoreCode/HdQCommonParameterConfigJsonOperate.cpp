
#include"HdQCommonParameterConfigJsonOperate.h"

namespace CommonParameterConfigStd {

    /// <summary>
    /// 从json字符串获取点
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void from_json(const nlohmann::json& j, CommonParameterConfigStd::CommonParameterConfig& obj) {
        {
            auto jsonObject = j.at("airSubstanceType");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.airSubstanceType);
            }
        }
        {
            auto jsonObject = j.at("electricFieldCalculationMode");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.electricFieldCalculationMode);
            }
        }
        {
            auto jsonObject = j.at("energyOutputMode");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.energyOutputMode);
            }
        }
        {
            auto jsonObject = j.at("powerThreshold");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.powerThreshold);
            }
        }
        {
            auto jsonObject = j.at("rebuildEdge");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.rebuildEdge);
            }
        }
        {
            auto jsonObject = j.at("deduplicateRadius");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.deduplicateRadius);
            }
        }

    }
    /// <summary>
    /// 将点对象转化为json字符串
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void to_json(nlohmann::json& j, const CommonParameterConfigStd::CommonParameterConfig& obj) {

        j["airSubstanceType"] = obj.airSubstanceType;
        j["electricFieldCalculationMode"] = obj.electricFieldCalculationMode;
        j["energyOutputMode"] = obj.energyOutputMode;
        j["powerThreshold"] = obj.powerThreshold;
        j["rebuildEdge"] = obj.rebuildEdge;
        j["deduplicateRadius"] = obj.deduplicateRadius;

    }

}
