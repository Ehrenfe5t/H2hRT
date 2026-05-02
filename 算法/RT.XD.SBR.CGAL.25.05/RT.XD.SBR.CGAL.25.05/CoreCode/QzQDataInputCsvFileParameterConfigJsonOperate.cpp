

#include"QzQDataInputCsvFileParameterConfigJsonOperate.h"

namespace DataInputCsvFileParameterConfigStd {


    /// <summary>
    /// 从json字符串获取点
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void from_json(const nlohmann::json& j, DataInputCsvFileParameterConfigStd::DataInputCsvFileParameterConfig& obj) {
        {
            auto jsonObject = j.at("inputMaterialTableCsvFileName");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.inputMaterialTableCsvFileName);
            }
        }
        {
            auto jsonObject = j.at("inputScenarioCorner3DCsvFileName");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.inputScenarioCorner3DCsvFileName);
            }
        }
        {
            auto jsonObject = j.at("inputScenarioPoint3DCsvFileName");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.inputScenarioPoint3DCsvFileName);
            }
        }
        {
            auto jsonObject = j.at("inputScenarioTriangle3DCsvFileName");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.inputScenarioTriangle3DCsvFileName);
            }
        }
        {
            auto jsonObject = j.at("inputTransmittingAntennaDatabaseJsonFileName");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.inputTransmittingAntennaDatabaseJsonFileName);
            }
        }

    }
    /// <summary>
    /// 将点对象转化为json字符串
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void to_json(nlohmann::json& j, const DataInputCsvFileParameterConfigStd::DataInputCsvFileParameterConfig& obj) {

        j["inputMaterialTableCsvFileName"] = obj.inputMaterialTableCsvFileName;
        j["inputTransmittingAntennaDatabaseJsonFileName"] = obj.inputTransmittingAntennaDatabaseJsonFileName;
        j["inputScenarioCorner3DCsvFileName"] = obj.inputScenarioCorner3DCsvFileName;
        j["inputScenarioPoint3DCsvFileName"] = obj.inputScenarioPoint3DCsvFileName;
        j["inputScenarioTriangle3DCsvFileName"] = obj.inputScenarioTriangle3DCsvFileName;

    }



}

