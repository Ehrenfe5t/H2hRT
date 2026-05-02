
#include"DxQRtIm3DParameterConfigJsonFileOperate.h"
#include"DxQRtIm3DParameterConfigJsonOperate.h"
#include"QzQJsonFileOperateBase.h"
#include"QzQFileBase.h"

namespace RtIm3DParameterConfigStd {

    std::string RtIm3DParameterConfigToJsonString(const RtIm3DParameterConfigStd::RtIm3DParameterConfig& rtIm3DParameterConfig) {
        nlohmann::json jf;
        to_json(jf, rtIm3DParameterConfig);
        return jf.dump(4);
    }

    /// <summary>
    /// 读取配置文件
    /// </summary>
    /// <param name="path"></param>
    /// <param name="object"></param>
    bool ReadRtIm3DParameterConfigByJsonFile(const char* path, RtIm3DParameterConfigStd::RtIm3DParameterConfig& rtIm3DParameterConfig) {

        if (!FileOperateStd::ExistFile(path)) {

            return false;
        }

        std::ifstream ifs(path);
        nlohmann::json jf = nlohmann::json::parse(ifs);
        from_json(jf, rtIm3DParameterConfig);
        ifs.close();

        return true;
    }


    /// <summary>
    /// 写入到json文件
    /// </summary>
    /// <param name="path"></param>
    /// <param name="object"></param>
    void WriteRtIm3DParameterConfigToJsonFile(const char* path, const RtIm3DParameterConfigStd::RtIm3DParameterConfig& rtIm3DParameterConfig) {
        nlohmann::json jf;
        to_json(jf, rtIm3DParameterConfig);
        JsonFileOperateBaseStd::WriteJsonStringToJsonFile(path, jf);
    }


}
