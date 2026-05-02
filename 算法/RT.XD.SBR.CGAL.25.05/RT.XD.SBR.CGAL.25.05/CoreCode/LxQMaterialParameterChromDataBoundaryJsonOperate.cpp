
#include"LxQMaterialParameterChromDataBoundaryJsonOperate.h"


namespace MaterialParameterChromDataBoundaryStd {


    /// <summary>
    /// 从json字符串获取点
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void from_json(const nlohmann::json& j, MaterialParameterChromDataBoundaryStd::MaterialParameterChromDataBoundary& obj) {
        {
            auto jsonObject = j.at("conductivityMin");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.conductivityMin);
            }
        }
        {
            auto jsonObject = j.at("conductivityMax");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.conductivityMax);
            }
        }
        {
            auto jsonObject = j.at("relativePermittivityMin");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.relativePermittivityMin);
            }
        }
        {
            auto jsonObject = j.at("relativePermittivityMax");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.relativePermittivityMax);
            }
        }

    }
    /// <summary>
    /// 将点对象转化为json字符串
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void to_json(nlohmann::json& j, const MaterialParameterChromDataBoundaryStd::MaterialParameterChromDataBoundary& obj) {

        j["conductivityMin"] = obj.conductivityMin;
        j["conductivityMax"] = obj.conductivityMax;
        j["relativePermittivityMin"] = obj.relativePermittivityMin;
        j["relativePermittivityMax"] = obj.relativePermittivityMax;

    }



}
