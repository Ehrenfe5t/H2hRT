

#include"HdQAntennaPatternJsonObjectJsonOperate.h"

#include"HdQAntennaPatternObjectJsonOperate.h"

namespace AntennaPatternJsonObjectStd {


    /// <summary>
    /// 从json字符串获取点
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void from_json(const nlohmann::json& j, AntennaPatternJsonObjectStd::AntennaPatternJsonObject& obj) {
        

        {
            for (auto& object : j["database"]) {
                AntennaPatternObjectStd::AntennaPatternObject value;
                from_json(object, value);
                obj.antennaPatternObjects.emplace_back(value);
            }
        }

        //{
        //
        //    for (auto& object : j["database"]) {
        //
        //        auto aaa = object.at("radiationPattern");
        //        martix = aaa.get<std::vector<std::vector<double>>>();
        //        //for (const auto& row : martix) {
        //        //    for (const auto& item : row) {
        //        //        std::cout << item << ' ';
        //        //    }
        //        //    std::cout << std::endl;
        //        //}
        //        AntennaPatternObjectStd::AntennaPatternObject value;
        //        for (int i = 0; i < martix.size(); ++i) {
        //            for (int j = 0; j < martix[i].size(); ++j) {
        //                value.radiationPattern[i][j] = martix[i][j];
        //            }
        //        }
        //        obj.antennaPatternObjects.emplace_back(value);
        //    }
        //
        //
        //    
        //
        //}

    }
    /// <summary>
    /// 将点对象转化为json字符串
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void to_json(nlohmann::json& j, const AntennaPatternJsonObjectStd::AntennaPatternJsonObject& obj) {

        j["database"];
        for (auto& value : obj.antennaPatternObjects) {
            nlohmann::json jf;
            to_json(jf, value);
            j["database"].emplace_back(jf);
        }
    }



}