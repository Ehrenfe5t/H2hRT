

#include"LxQNumericalMethodParameterConfigJsonOperate.h"


namespace NumericalMethodParameterConfigStd {


    /// <summary>
    /// 从json字符串获取点
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void from_json(const nlohmann::json& j, NumericalMethodParameterConfigStd::NumericalMethodParameterConfig& obj) {
        
        {
            auto jsonObject = j.at("numericalMethodMaxLevelOfGuess");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.numericalMethodMaxLevelOfGuess);
            }
        }
        {
            auto jsonObject = j.at("numericalMethodNumbersOfGuess");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.numericalMethodNumbersOfGuess);
            }
        }

    }
    /// <summary>
    /// 将点对象转化为json字符串
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void to_json(nlohmann::json& j, const NumericalMethodParameterConfigStd::NumericalMethodParameterConfig& obj) {

        j["numericalMethodMaxLevelOfGuess"] = obj.numericalMethodMaxLevelOfGuess;
        j["numericalMethodNumbersOfGuess"] = obj.numericalMethodNumbersOfGuess;

    }



}


