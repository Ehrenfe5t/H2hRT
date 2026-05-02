

#include"DxQRayEjectionParameterConfigJsonOperate.h"


namespace RayEjectionParameterConfigStd {


    /// <summary>
    /// 从json字符串获取点
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void from_json(const nlohmann::json& j, RayEjectionParameterConfigStd::RayEjectionParameterConfig& obj) {
        {
            auto jsonObject = j.at("ejectionsMaxTotalNumber");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.ejectionsMaxTotalNumber);
            }
        }
        {
            auto jsonObject = j.at("ejectionsOfDiffractionMaxNumber");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.ejectionsOfDiffractionMaxNumber);
            }
        }
        {
            auto jsonObject = j.at("ejectionsOfDiffuseScatteringMaxNumber");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.ejectionsOfDiffuseScatteringMaxNumber);
            }
        }
        {
            auto jsonObject = j.at("ejectionsOfReflectionMaxNumber");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.ejectionsOfReflectionMaxNumber);
            }
        }
        {
            auto jsonObject = j.at("ejectionsOfTransmissionMaxNumber");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.ejectionsOfTransmissionMaxNumber);
            }
        }
        {
            auto jsonObject = j.at("switchOfLos");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.switchOfLos);
            }
        }

    }
    /// <summary>
    /// 将点对象转化为json字符串
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void to_json(nlohmann::json& j, const RayEjectionParameterConfigStd::RayEjectionParameterConfig& obj) {

        j["ejectionsMaxTotalNumber"] = obj.ejectionsMaxTotalNumber;
        j["ejectionsOfDiffractionMaxNumber"] = obj.ejectionsOfDiffractionMaxNumber;
        j["ejectionsOfDiffuseScatteringMaxNumber"] = obj.ejectionsOfDiffuseScatteringMaxNumber;
        j["ejectionsOfReflectionMaxNumber"] = obj.ejectionsOfReflectionMaxNumber;
        j["ejectionsOfTransmissionMaxNumber"] = obj.ejectionsOfTransmissionMaxNumber;
        j["switchOfLos"] = obj.switchOfLos;

    }



}


