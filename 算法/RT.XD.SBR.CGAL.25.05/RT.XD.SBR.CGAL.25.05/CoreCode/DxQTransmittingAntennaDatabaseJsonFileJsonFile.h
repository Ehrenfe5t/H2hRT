#pragma once

#include"QzQJson.hpp"
#include"DxQTransmittingAntennaDatabaseJsonFile.h"
namespace TransmittingAntennaDatabaseJsonFileStd {



    /// <summary>
    /// 从json字符串获取点
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void from_json(const nlohmann::json& j, TransmittingAntennaDatabaseJsonFile& obj);

    /// <summary>
    /// 将点对象转化为json字符串
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void to_json(nlohmann::json& j, const TransmittingAntennaDatabaseJsonFile& obj);

    /// <summary>
    /// 读取配置文件
    /// </summary>
    /// <param name="path"></param>
    /// <param name="object"></param>
    bool ReadTransmittingAntennaDatabaseJsonFileByJsonFile(const char* path, TransmittingAntennaDatabaseJsonFile& rtSbr3DForRay3DParameterConfig);

    /// <summary>
    /// 写入到json文件
    /// </summary>
    /// <param name="path"></param>
    /// <param name="object"></param>
    void WriteTransmittingAntennaDatabaseJsonFileToJsonFile(const char* path, const TransmittingAntennaDatabaseJsonFile& rtSbr3DForRay3DParameterConfig);



}