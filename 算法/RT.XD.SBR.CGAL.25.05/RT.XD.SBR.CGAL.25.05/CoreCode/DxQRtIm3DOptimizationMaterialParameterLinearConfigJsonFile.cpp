
#include"DxQRtIm3DOptimizationMaterialParameterLinearConfigJsonFile.h"
#include"DxQRtIm3DOptimizationMaterialParameterLinearConfigJsonOperate.h"
#include"QzQJsonFileOperateBase.h"
#include"QzQFileBase.h"

namespace RtIm3DOptimizationMaterialParameterLinearConfigStd {

    std::string RtIm3DOptimizationMaterialParameterLinearConfigToJsonString(const RtIm3DOptimizationMaterialParameterLinearConfigStd::RtIm3DOptimizationMaterialParameterLinearConfig& rtIm3DOptimizationMaterialParameterLinearConfig) {
        nlohmann::json jf;
        to_json(jf, rtIm3DOptimizationMaterialParameterLinearConfig);
        return jf.dump(4);
    }

    /// <summary>
    /// 读取配置文件
    /// </summary>
    /// <param name="path"></param>
    /// <param name="object"></param>
    bool ReadRtIm3DOptimizationMaterialParameterLinearConfigByJsonFile(const char* path, RtIm3DOptimizationMaterialParameterLinearConfigStd::RtIm3DOptimizationMaterialParameterLinearConfig& rtIm3DOptimizationMaterialParameterLinearConfig) {

        if (!FileOperateStd::ExistFile(path)) {

            return false;
        }

        std::ifstream ifs(path);
        nlohmann::json jf = nlohmann::json::parse(ifs);
        from_json(jf, rtIm3DOptimizationMaterialParameterLinearConfig);
        ifs.close();

        return true;
    }


    /// <summary>
    /// 写入到json文件
    /// </summary>
    /// <param name="path"></param>
    /// <param name="object"></param>
    void WriteRtIm3DOptimizationMaterialParameterLinearConfigToJsonFile(const char* path, const RtIm3DOptimizationMaterialParameterLinearConfigStd::RtIm3DOptimizationMaterialParameterLinearConfig& rtIm3DOptimizationMaterialParameterLinearConfig) {
        nlohmann::json jf;
        to_json(jf, rtIm3DOptimizationMaterialParameterLinearConfig);
        JsonFileOperateBaseStd::WriteJsonStringToJsonFile(path, jf);
    }


}