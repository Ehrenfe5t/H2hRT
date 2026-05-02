

#include"HdQAntennaPatternJsonObjectJsonFile.h"

#include"HdQAntennaPatternJsonObjectJsonOperate.h"
#include"QzQJsonFileOperateBase.h"
#include"QzQFileBase.h"

namespace AntennaPatternJsonObjectStd {


    std::string AntennaPatternJsonObjectToJsonString(const AntennaPatternJsonObjectStd::AntennaPatternJsonObject& antennaPatternJsonObject) {
        nlohmann::json jf;
        to_json(jf, antennaPatternJsonObject);
        return jf.dump(4);
    }

    /// <summary>
    /// 读取配置文件
    /// </summary>
    /// <param name="path"></param>
    /// <param name="object"></param>
    bool ReadAntennaPatternJsonObjectByJsonFile(const char* path, AntennaPatternJsonObjectStd::AntennaPatternJsonObject& antennaPatternJsonObject) {

        if (!FileOperateStd::ExistFile(path)) {

            return false;
        }

        std::ifstream ifs(path);
        nlohmann::json jf = nlohmann::json::parse(ifs);
        from_json(jf, antennaPatternJsonObject);
        ifs.close();

        return true;
    }


    /// <summary>
    /// 写入到json文件
    /// </summary>
    /// <param name="path"></param>
    /// <param name="object"></param>
    void WriteAntennaPatternJsonObjectToJsonFile(const char* path, const AntennaPatternJsonObjectStd::AntennaPatternJsonObject& antennaPatternJsonObject) {
        nlohmann::json jf;
        to_json(jf, antennaPatternJsonObject);
        JsonFileOperateBaseStd::WriteJsonStringToJsonFile(path, jf);
    }


}