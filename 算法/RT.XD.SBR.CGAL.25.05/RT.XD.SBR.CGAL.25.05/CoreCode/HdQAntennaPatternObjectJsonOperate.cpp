

#include"HdQAntennaPatternObjectJsonOperate.h"

namespace AntennaPatternObjectStd {



    /// <summary>
    /// 从json字符串获取点
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void from_json(const nlohmann::json& j, AntennaPatternObjectStd::AntennaPatternObject& obj) {
        {
            auto jsonObject = j.at("radiationPatternId");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.radiationPatternId);
            }
        }


        {

            auto aaa = j.at("radiationPattern");
            auto martix = aaa.get<std::vector<std::vector<double>>>();

            // 从json对象中提取二维数组
            for (int i = 0; i < martix.size(); ++i) {
                for (int j = 0; j < martix[i].size(); ++j) {
                    obj.radiationPattern[i][j] = martix[i][j];
                }
            }
        }


    }
    /// <summary>
    /// 将点对象转化为json字符串
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void to_json(nlohmann::json& j, const AntennaPatternObjectStd::AntennaPatternObject& obj) {

        j["radiationPatternId"] = obj.radiationPatternId;

        
        j["radiationPattern"];
        for (int i = 0; i < AntennaPatternObjectStd::radiationPattern_rows; ++i) {
            nlohmann::json innerArray = nlohmann::json::array();
            for (int j = 0; j < AntennaPatternObjectStd::radiationPattern_cols; ++j) {
                innerArray.push_back(obj.radiationPattern[i][j]);
            }
            j["radiationPattern"].emplace_back(innerArray);
        }

    }



}