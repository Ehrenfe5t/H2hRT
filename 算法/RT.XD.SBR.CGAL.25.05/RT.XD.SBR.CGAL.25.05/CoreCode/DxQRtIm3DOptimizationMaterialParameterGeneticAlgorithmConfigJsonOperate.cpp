
#include"DxQRtIm3DOptimizationMaterialParameterGeneticAlgorithmConfigJsonOperate.h"

#include"DxQRtIm3DParameterConfigJsonOperate.h"
#include"LxQOptimizationMaterialParameterGeneticAlgorithmConfigJsonOperate.h"

namespace RtIm3DOptimizationMaterialParameterGeneticAlgorithmConfigStd {


    /// <summary>
    /// 从json字符串获取点
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void from_json(const nlohmann::json& j, RtIm3DOptimizationMaterialParameterGeneticAlgorithmConfigStd::RtIm3DOptimizationMaterialParameterGeneticAlgorithmConfig& obj) {
        {
            auto jsonObject = j.at("optimizationMaterialParameterGeneticAlgorithmConfig");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.optimizationMaterialParameterGeneticAlgorithmConfig);
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
    void to_json(nlohmann::json& j, const RtIm3DOptimizationMaterialParameterGeneticAlgorithmConfigStd::RtIm3DOptimizationMaterialParameterGeneticAlgorithmConfig& obj) {

        j["optimizationMaterialParameterGeneticAlgorithmConfig"] = obj.optimizationMaterialParameterGeneticAlgorithmConfig;
        j["rtIm3DParameterConfig"] = obj.rtIm3DParameterConfig;

    }
}
