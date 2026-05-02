
#include"DxQRtIm3DOptimizationMaterialParameterLinearConfigJsonOperate.h"
#include"DxQRtIm3DParameterConfigJsonOperate.h"
#include"LxQOptimizationMaterialParameterLinearConfigJsonOperate.h"



namespace RtIm3DOptimizationMaterialParameterLinearConfigStd {


    /// <summary>
    /// 从json字符串获取点
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void from_json(const nlohmann::json& j, RtIm3DOptimizationMaterialParameterLinearConfigStd::RtIm3DOptimizationMaterialParameterLinearConfig& obj) {
        {
            auto jsonObject = j.at("optimizationMaterialParameterLinearConfig");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.optimizationMaterialParameterLinearConfig);
            }
        }
        {
            auto jsonObject = j.at("rtIm3DParameterConfig");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.rtIm3DParameterConfig);
            }
        }
    }

    /// <summary>
    /// 将点对象转化为json字符串
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void to_json(nlohmann::json& j, const RtIm3DOptimizationMaterialParameterLinearConfigStd::RtIm3DOptimizationMaterialParameterLinearConfig& obj) {

        j["optimizationMaterialParameterLinearConfig"] = obj.optimizationMaterialParameterLinearConfig;
        j["rtIm3DParameterConfig"] = obj.rtIm3DParameterConfig;

    }



}