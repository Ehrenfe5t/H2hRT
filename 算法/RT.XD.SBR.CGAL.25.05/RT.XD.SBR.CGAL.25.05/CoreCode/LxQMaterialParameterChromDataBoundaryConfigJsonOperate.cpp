
#include"LxQMaterialParameterChromDataBoundaryConfigJsonOperate.h"
#include"LxQMaterialParameterChromDataBoundaryJsonOperate.h"


namespace MaterialParameterChromDataBoundaryConfigStd {

    /// <summary>
    /// 从json字符串获取点
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void from_json(const nlohmann::json& j, MaterialParameterChromDataBoundaryConfigStd::MaterialParameterChromDataBoundaryConfig& obj) {
        {
            auto jsonObject = j.at("frequency");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.frequency);
            }
        }
        {
            auto jsonObject = j.at("materialParameterChromDataBoundary");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.materialParameterChromDataBoundary);
            }
        }
        {
            auto jsonObject = j.at("typeNumber");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.typeNumber);
            }
        }

    }
    /// <summary>
    /// 将点对象转化为json字符串
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void to_json(nlohmann::json& j, const MaterialParameterChromDataBoundaryConfigStd::MaterialParameterChromDataBoundaryConfig& obj) {

        j["frequency"] = obj.frequency;
        j["materialParameterChromDataBoundary"] = obj.materialParameterChromDataBoundary;
        j["typeNumber"] = obj.typeNumber;
    }

}


