
#include"DxQRtIm3DPrivateParameterConfigJsonOperate.h"
#include"QzQDiffuseScatteringParameterJsonOperate.h"

namespace RtIm3DPrivateParameterConfigStd {

    /// <summary>
    /// 从json字符串获取点
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void from_json(const nlohmann::json& j, RtIm3DPrivateParameterConfigStd::RtIm3DPrivateParameterConfig& obj) {
        {
            auto jsonObject = j.at("diffuseScatteringParameter");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.diffuseScatteringParameter);
            }
        }
        {
            auto jsonObject = j.at("isDiffractionReflection");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.isDiffractionReflection);
            }
        }
        {
            auto jsonObject = j.at("maxDiscreteSideLength");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.maxDiscreteSideLength);
            }
        }
        {
            auto jsonObject = j.at("mumberOfDiscreteBoundingBox");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.mumberOfDiscreteBoundingBox);
            }
        }

    }
    /// <summary>
    /// 将点对象转化为json字符串
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void to_json(nlohmann::json& j, const RtIm3DPrivateParameterConfigStd::RtIm3DPrivateParameterConfig& obj) {

        j["diffuseScatteringParameter"] = obj.diffuseScatteringParameter;
        j["isDiffractionReflection"] = obj.isDiffractionReflection;
        j["maxDiscreteSideLength"] = obj.maxDiscreteSideLength;
        j["mumberOfDiscreteBoundingBox"] = obj.mumberOfDiscreteBoundingBox;

    }


}