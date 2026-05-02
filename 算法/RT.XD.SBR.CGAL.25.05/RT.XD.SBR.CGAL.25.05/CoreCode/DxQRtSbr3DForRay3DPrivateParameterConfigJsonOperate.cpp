
#include"DxQRtSbr3DForRay3DPrivateParameterConfigJsonOperate.h"

#include"QzQDiffuseScatteringParameterJsonOperate.h"

namespace RtSbr3DForRay3DPrivateParameterConfigStd {



    /// <summary>
    /// 从json字符串获取点
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void from_json(const nlohmann::json& j, RtSbr3DForRay3DPrivateParameterConfigStd::RtSbr3DForRay3DPrivateParameterConfig& obj) {
        {
            auto jsonObject = j.at("cylindricalTube");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.cylindricalTube);
            }
        }
        {
            auto jsonObject = j.at("diffuseScatteringParameter");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.diffuseScatteringParameter);
            }
        }
        {
            auto jsonObject = j.at("gapDiffractionRad");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.gapDiffractionRad);
            }
        }
        {
            auto jsonObject = j.at("gapDiffuseScatteringAzimuth");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.gapDiffuseScatteringAzimuth);
            }
        }
        {
            auto jsonObject = j.at("gapDiffuseScatteringPitchAngle");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.gapDiffuseScatteringPitchAngle);
            }
        }
        {
            auto jsonObject = j.at("radiusCorner");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.radiusCorner);
            }
        }
        {
            auto jsonObject = j.at("radiusRx");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.radiusRx);
            }
        }
        {
            auto jsonObject = j.at("rayNumber");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.rayNumber);
            }
        }
        {
            auto jsonObject = j.at("realWorldRefraction");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.realWorldRefraction);
            }
        }

    }
    /// <summary>
    /// 将点对象转化为json字符串
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void to_json(nlohmann::json& j, const RtSbr3DForRay3DPrivateParameterConfigStd::RtSbr3DForRay3DPrivateParameterConfig& obj) {

        j["cylindricalTube"] = obj.cylindricalTube;
        j["diffuseScatteringParameter"] = obj.diffuseScatteringParameter;
        j["gapDiffractionRad"] = obj.gapDiffractionRad;
        j["gapDiffuseScatteringAzimuth"] = obj.gapDiffuseScatteringAzimuth;
        j["gapDiffuseScatteringPitchAngle"] = obj.gapDiffuseScatteringPitchAngle;
        j["radiusCorner"] = obj.radiusCorner;
        j["radiusRx"] = obj.radiusRx;
        j["rayNumber"] = obj.rayNumber;
        j["realWorldRefraction"] = obj.realWorldRefraction;

    }




}
