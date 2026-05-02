
#include"DxQRtIm3DOptimizationMaterialParameterGeneticAlgorithmConfigJsonFile.h"

#include"DxQRtIm3DOptimizationMaterialParameterGeneticAlgorithmConfigJsonOperate.h"
#include"QzQJsonFileOperateBase.h"

#include"QzQFileBase.h"
namespace RtIm3DOptimizationMaterialParameterGeneticAlgorithmConfigStd {


    std::string RtIm3DOptimizationMaterialParameterGeneticAlgorithmConfigToJsonString(const RtIm3DOptimizationMaterialParameterGeneticAlgorithmConfigStd::RtIm3DOptimizationMaterialParameterGeneticAlgorithmConfig& rtIm3DOptimizationMaterialParameterGeneticAlgorithmConfig) {
        nlohmann::json jf;
        to_json(jf, rtIm3DOptimizationMaterialParameterGeneticAlgorithmConfig);
        return jf.dump(4);
    }

    /// <summary>
    /// 读取配置文件
    /// </summary>
    /// <param name="path"></param>
    /// <param name="object"></param>
    bool ReadRtIm3DOptimizationMaterialParameterGeneticAlgorithmConfigByJsonFile(const char* path, RtIm3DOptimizationMaterialParameterGeneticAlgorithmConfigStd::RtIm3DOptimizationMaterialParameterGeneticAlgorithmConfig& rtIm3DOptimizationMaterialParameterGeneticAlgorithmConfig) {

        if (!FileOperateStd::ExistFile(path)) {

            return false;
        }

        std::ifstream ifs(path);
        nlohmann::json jf = nlohmann::json::parse(ifs);
        from_json(jf, rtIm3DOptimizationMaterialParameterGeneticAlgorithmConfig);
        ifs.close();

        return true;
    }


    /// <summary>
    /// 写入到json文件
    /// </summary>
    /// <param name="path"></param>
    /// <param name="object"></param>
    void WriteRtIm3DOptimizationMaterialParameterGeneticAlgorithmConfigToJsonFile(const char* path, const RtIm3DOptimizationMaterialParameterGeneticAlgorithmConfigStd::RtIm3DOptimizationMaterialParameterGeneticAlgorithmConfig& rtIm3DOptimizationMaterialParameterGeneticAlgorithmConfig) {
        nlohmann::json jf;
        to_json(jf, rtIm3DOptimizationMaterialParameterGeneticAlgorithmConfig);
        JsonFileOperateBaseStd::WriteJsonStringToJsonFile(path, jf);
    }

}

