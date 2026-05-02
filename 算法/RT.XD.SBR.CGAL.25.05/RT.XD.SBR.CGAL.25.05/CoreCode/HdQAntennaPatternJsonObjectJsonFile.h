#pragma once

#include"HdQAntennaPatternJsonObject.h"

#include<string>


namespace AntennaPatternJsonObjectStd {


    std::string AntennaPatternJsonObjectToJsonString(const AntennaPatternJsonObjectStd::AntennaPatternJsonObject& antennaPatternJsonObject);

    /// <summary>
    /// 读取配置文件
    /// </summary>
    /// <param name="path"></param>
    /// <param name="object"></param>
    bool ReadAntennaPatternJsonObjectByJsonFile(const char* path, AntennaPatternJsonObjectStd::AntennaPatternJsonObject& antennaPatternJsonObject);


    /// <summary>
    /// 写入到json文件
    /// </summary>
    /// <param name="path"></param>
    /// <param name="object"></param>
    void WriteAntennaPatternJsonObjectToJsonFile(const char* path, const AntennaPatternJsonObjectStd::AntennaPatternJsonObject& antennaPatternJsonObject);


}

