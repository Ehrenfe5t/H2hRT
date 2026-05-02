

#include"QzQDataOutputParameterConfigJsonOperate.h"


namespace DataOutputParameterConfigStd {


    /// <summary>
    /// 从json字符串获取点
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void from_json(const nlohmann::json& j, DataOutputParameterConfigStd::DataOutputParameterConfig& obj) {
        {
            auto jsonObject = j.at("outPutDirectoryPathName");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.outPutDirectoryPathName);
            }
        }
        {
            auto jsonObject = j.at("outPutLogTxtFileName");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.outPutLogTxtFileName);
            }
        }
        {
            auto jsonObject = j.at("switchOfBigChannelParameterInfo");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.switchOfBigChannelParameterInfo);
            }
        }
        {
            auto jsonObject = j.at("switchOfPathInfo");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.switchOfPathInfo);
            }
        }
        {
            auto jsonObject = j.at("switchOfSmallChannelParameterInfo");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.switchOfSmallChannelParameterInfo);
            }
        }
        {
            auto jsonObject = j.at("switchOfStatisticChannelParameterInfo");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.switchOfStatisticChannelParameterInfo);
            }
        }
        {
            auto jsonObject = j.at("switchOfMultipleSignalSourceSuperposition");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.switchOfMultipleSignalSourceSuperposition);
            }
        }
        
    }
    /// <summary>
    /// 将点对象转化为json字符串
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void to_json(nlohmann::json& j, const DataOutputParameterConfigStd::DataOutputParameterConfig& obj) {

        j["outPutDirectoryPathName"] = obj.outPutDirectoryPathName;
        j["outPutLogTxtFileName"] = obj.outPutLogTxtFileName;
        j["switchOfBigChannelParameterInfo"] = obj.switchOfBigChannelParameterInfo;
        j["switchOfPathInfo"] = obj.switchOfPathInfo;
        j["switchOfSmallChannelParameterInfo"] = obj.switchOfSmallChannelParameterInfo;
        j["switchOfStatisticChannelParameterInfo"] = obj.switchOfStatisticChannelParameterInfo;
        j["switchOfMultipleSignalSourceSuperposition"] = obj.switchOfMultipleSignalSourceSuperposition;
        
    }



}



