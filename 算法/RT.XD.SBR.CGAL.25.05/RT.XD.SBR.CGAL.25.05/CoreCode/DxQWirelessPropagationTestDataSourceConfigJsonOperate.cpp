

#include"DxQWirelessPropagationTestDataSourceConfigJsonOperate.h"

namespace WirelessPropagationTestDataSourceConfigStd {


    /// <summary>
    /// 从json字符串获取点
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void from_json(const nlohmann::json& j, WirelessPropagationTestDataSourceConfigStd::WirelessPropagationTestDataSourceConfig& obj) {
        {
            auto jsonObject = j.at("actualMeasurementDataTxtFileName");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.actualMeasurementDataTxtFileName);
            }
        }
        {
            auto jsonObject = j.at("inputTransmitterAntennaCsvFileName");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.inputTransmitterAntennaCsvFileName);
            }
        }
        {
            auto jsonObject = j.at("samplingIntervalForReceivingData");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.samplingIntervalForReceivingData);
            }
        }
    }


    /// <summary>
    /// 将点对象转化为json字符串
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void to_json(nlohmann::json& j, const WirelessPropagationTestDataSourceConfigStd::WirelessPropagationTestDataSourceConfig& obj) {

        j["actualMeasurementDataTxtFileName"] = obj.actualMeasurementDataTxtFileName;
        j["inputTransmitterAntennaCsvFileName"] = obj.inputTransmitterAntennaCsvFileName;
        j["samplingIntervalForReceivingData"] = obj.samplingIntervalForReceivingData;

    }

}