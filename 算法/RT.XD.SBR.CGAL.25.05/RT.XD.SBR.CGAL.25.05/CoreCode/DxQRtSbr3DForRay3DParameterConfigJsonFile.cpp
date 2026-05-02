
#include"DxQRtSbr3DForRay3DParameterConfigJsonFile.h"

#include"DxQRtSbr3DForRay3DParameterConfigJsonOperate.h"
#include"QzQJsonFileOperateBase.h"

#include"QzQFileBase.h"
namespace RtSbr3DForRay3DParameterConfigStd {


    std::string RtSbr3DForRay3DParameterConfigToJsonString(const RtSbr3DForRay3DParameterConfigStd::RtSbr3DForRay3DParameterConfig& rtSbr3DForRay3DParameterConfig) {
        nlohmann::json jf;
        to_json(jf, rtSbr3DForRay3DParameterConfig);
        return jf.dump(4);
    }

    /// <summary>
    /// 读取配置文件
    /// </summary>
    /// <param name="path"></param>
    /// <param name="object"></param>
    bool ReadRtSbr3DForRay3DParameterConfigByJsonFile(const char* path, RtSbr3DForRay3DParameterConfigStd::RtSbr3DForRay3DParameterConfig& rtSbr3DForRay3DParameterConfig) {

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
    void WriteRtSbr3DForRay3DParameterConfigToJsonFile(const char* path, const RtSbr3DForRay3DParameterConfigStd::RtSbr3DForRay3DParameterConfig& rtSbr3DForRay3DParameterConfig) {
        nlohmann::json jf;
        to_json(jf, rtSbr3DForRay3DParameterConfig);
        JsonFileOperateBaseStd::WriteJsonStringToJsonFile(path, jf);
    }

}
