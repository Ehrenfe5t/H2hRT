

#include"QzQGeometricSpaceAccelerateParameterConfigJsonOperate.h"


namespace GeometricSpaceAccelerateParameterConfigStd {


    /// <summary>
    /// 从json字符串获取点
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void from_json(const nlohmann::json& j, GeometricSpaceAccelerateParameterConfigStd::GeometricSpaceAccelerateParameterConfig& obj) {
        {
            auto jsonObject = j.at("geometricSpaceAccelerateType");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.geometricSpaceAccelerateType);
            }
        }
        {
            auto jsonObject = j.at("lengthOfPixel");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.lengthOfPixel);
            }
        }

    }
    /// <summary>
    /// 将点对象转化为json字符串
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void to_json(nlohmann::json& j, const GeometricSpaceAccelerateParameterConfigStd::GeometricSpaceAccelerateParameterConfig& obj) {

        j["geometricSpaceAccelerateType"] = obj.geometricSpaceAccelerateType;
        j["lengthOfPixel"] = obj.lengthOfPixel;
    }



}


