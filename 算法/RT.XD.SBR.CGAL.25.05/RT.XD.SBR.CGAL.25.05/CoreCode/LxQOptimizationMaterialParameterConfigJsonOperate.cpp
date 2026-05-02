
#include"LxQOptimizationMaterialParameterConfigJsonOperate.h"

#include"DxQWirelessPropagationTestDataSourceConfigJsonOperate.h"

namespace OptimizationMaterialParameterConfigStd {

    /// <summary>
    /// 从json字符串获取点
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void from_json(const nlohmann::json& j, OptimizationMaterialParameterConfigStd::OptimizationMaterialParameterConfig& obj) {
        {
            auto jsonObject = j.at("inputMaterialTableCsvFileName");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.inputMaterialTableCsvFileName);
            }
        }
        {
            auto jsonObject = j.at("outputDirectoryFileName");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.outputDirectoryFileName);
            }
        }

        {
            for (auto& object : j["dataSource"]) {
                WirelessPropagationTestDataSourceConfigStd::WirelessPropagationTestDataSourceConfig value;
                from_json(object, value);
                obj.dataSource.emplace_back(value);
            }
        }

    }

    /// <summary>
    /// 将点对象转化为json字符串
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void to_json(nlohmann::json& j, const OptimizationMaterialParameterConfigStd::OptimizationMaterialParameterConfig& obj) {

        j["inputMaterialTableCsvFileName"] = obj.inputMaterialTableCsvFileName;
        j["outputDirectoryFileName"] = obj.outputDirectoryFileName;

        j["dataSource"];
        for (auto& value : obj.dataSource) {
            nlohmann::json jf;
            to_json(jf, value);
            j["dataSource"].emplace_back(jf);
        }

    }

}