
#include"LxQOptimizationMaterialParameterLinearConfigJsonOperate.h"

#include"LxQOptimizationMaterialParameterConfigJsonOperate.h"

namespace OptimizationMaterialParameterLinearConfigStd {
    /// <summary>
    /// 从json字符串获取点
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void from_json(const nlohmann::json& j, OptimizationMaterialParameterLinearConfigStd::OptimizationMaterialParameterLinearConfig& obj) {
        {
            auto jsonObject = j.at("searchMaxNumber");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.searchMaxNumber);
            }
        }
        {
            auto jsonObject = j.at("optimizationMaterialParameterConfig");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.optimizationMaterialParameterConfig);
            }
        }
        {
            auto jsonObject = j.at("searchGap");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.searchGap);
            }
        }
        {
            auto jsonObject = j.at("searchRmseGoal");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.searchRmseGoal);
            }
        }
        {
            auto jsonObject = j.at("left_right_eps");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.left_right_eps);
            }
        }
    }

    /// <summary>
    /// 将点对象转化为json字符串
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void to_json(nlohmann::json& j, const OptimizationMaterialParameterLinearConfigStd::OptimizationMaterialParameterLinearConfig& obj) {

        j["searchMaxNumber"] = obj.searchMaxNumber;
        j["optimizationMaterialParameterConfig"] = obj.optimizationMaterialParameterConfig;
        j["searchGap"] = obj.searchGap;
        j["searchRmseGoal"] = obj.searchRmseGoal;
        j["left_right_eps"] = obj.left_right_eps;

    }
}