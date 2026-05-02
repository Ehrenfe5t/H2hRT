
#include"QzQDiffuseScatteringParameterJsonOperate.h"

namespace DiffuseScatteringParameterStd {
    /// <summary>
    /// 从json字符串获取点
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void from_json(const nlohmann::json& j, DiffuseScatteringParameterStd::DiffuseScatteringParameter& obj) {
        
        {
            auto jsonObject = j.at("diffuseScatteringAr");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.diffuseScatteringAr);
            }
        }
        {
            auto jsonObject = j.at("diffuseScatteringCoefficient");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.diffuseScatteringCoefficient);
            }
        }
        {
            auto jsonObject = j.at("diffuseScatteringRayleighRange");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.diffuseScatteringRayleighRange);
            }
        }

    }
    /// <summary>
    /// 将点对象转化为json字符串
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void to_json(nlohmann::json& j, const DiffuseScatteringParameterStd::DiffuseScatteringParameter& obj) {

        j["diffuseScatteringAr"] = obj.diffuseScatteringAr;
        j["diffuseScatteringCoefficient"] = obj.diffuseScatteringCoefficient;
        j["diffuseScatteringRayleighRange"] = obj.diffuseScatteringRayleighRange;

    }

}