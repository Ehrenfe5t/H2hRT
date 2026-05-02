


#include"DxQTransmittingAntennaDatabaseJsonFileJsonFile.h"

#include"QzQJsonFileOperateBase.h"
#include"QzQFileBase.h"

namespace TransmittingAntennaJsonFileStd {


    /// <summary>
    /// 从json字符串获取点
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void from_json(const nlohmann::json& j, TransmittingAntennaJsonFile& obj) {

        {
            auto jsonObject = j.at("center_location_x");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.center_location_x);
            }
        }
        {
            auto jsonObject = j.at("center_location_y");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.center_location_y);
            }
        }
        {
            auto jsonObject = j.at("center_location_z");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.center_location_z);
            }
        }
        {
            auto jsonObject = j.at("emissionPower");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.emissionPower);
            }
        }
        {
            auto jsonObject = j.at("frequencys");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.frequencys);
            }
        }
        {
            auto jsonObject = j.at("inputReceivingAntennaCsvFileName");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.inputReceivingAntennaCsvFileName);
            }
        }
        {
            auto jsonObject = j.at("polarization3DModelId");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.polarization3DModelId);
            }
        }
        {
            auto jsonObject = j.at("radiationPatternId");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.radiationPatternId);
            }
        }
        {
            auto jsonObject = j.at("transmittingAntennaId");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.transmittingAntennaId);
            }
        }
        {
            auto jsonObject = j.at("materialTypeNumber");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.materialTypeNumber);
            }
        }

    }
    /// <summary>
    /// 将点对象转化为json字符串
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void to_json(nlohmann::json& j, const TransmittingAntennaJsonFile& obj) {

        j["center_location_x"] = obj.center_location_x;
        j["center_location_y"] = obj.center_location_y;
        j["center_location_z"] = obj.center_location_z;
        j["emissionPower"] = obj.emissionPower;
        j["frequencys"] = obj.frequencys;
        j["inputReceivingAntennaCsvFileName"] = obj.inputReceivingAntennaCsvFileName;
        j["polarization3DModelId"] = obj.polarization3DModelId;
        j["radiationPatternId"] = obj.radiationPatternId;
        j["transmittingAntennaId"] = obj.transmittingAntennaId;
        j["materialTypeNumber"] = obj.materialTypeNumber;

    }
}

namespace TransmittingAntennaDatabaseJsonFileStd {



    /// <summary>
    /// 从json字符串获取点
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void from_json(const nlohmann::json& j, TransmittingAntennaDatabaseJsonFile& obj) {
        {
            auto jsonObject = j.at("inputAntennaPatternDatabaseJsonFileName");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.inputAntennaPatternDatabaseJsonFileName);
            }
        }
        {
            auto jsonObject = j.at("inputPolarization3DDatabaseJsonFileName");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.inputPolarization3DDatabaseJsonFileName);
            }
        }
        {
            auto jsonObject = j.at("transmittingAntennaJsonFiles");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.transmittingAntennaJsonFiles);
            }
        }

    }
    /// <summary>
    /// 将点对象转化为json字符串
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void to_json(nlohmann::json& j, const TransmittingAntennaDatabaseJsonFile& obj) {

        j["inputAntennaPatternDatabaseJsonFileName"] = obj.inputAntennaPatternDatabaseJsonFileName;
        j["inputPolarization3DDatabaseJsonFileName"] = obj.inputPolarization3DDatabaseJsonFileName;
        j["transmittingAntennaJsonFiles"];
        for (auto& value : obj.transmittingAntennaJsonFiles) {
            nlohmann::json jf;
            to_json(jf, value);
            j["transmittingAntennaJsonFiles"].emplace_back(jf);
        }
    }

    /// <summary>
    /// 读取配置文件
    /// </summary>
    /// <param name="path"></param>
    /// <param name="object"></param>
    bool ReadTransmittingAntennaDatabaseJsonFileByJsonFile(const char* path, TransmittingAntennaDatabaseJsonFile& rtSbr3DForRay3DParameterConfig) {

        if (!FileOperateStd::ExistFile(path)) {

            return false;
        }

        std::ifstream ifs(path);
        nlohmann::json jf = nlohmann::json::parse(ifs);
        from_json(jf, rtSbr3DForRay3DParameterConfig);
        ifs.close();

        return true;
    }


    /// <summary>
    /// 写入到json文件
    /// </summary>
    /// <param name="path"></param>
    /// <param name="object"></param>
    void WriteTransmittingAntennaDatabaseJsonFileToJsonFile(const char* path, const TransmittingAntennaDatabaseJsonFile& rtSbr3DForRay3DParameterConfig) {
        nlohmann::json jf;
        to_json(jf, rtSbr3DForRay3DParameterConfig);
        JsonFileOperateBaseStd::WriteJsonStringToJsonFile(path, jf);
    }



}