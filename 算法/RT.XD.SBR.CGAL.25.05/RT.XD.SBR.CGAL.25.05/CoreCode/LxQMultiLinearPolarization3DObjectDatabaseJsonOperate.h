#pragma once

#include"LxQMultiLinearPolarization3DObjectDatabaseJson.h"
#include<string>
namespace MultiLinearPolarization3DObjectDatabaseJsonOperateStd {



    /// <summary>
    /// 读取配置文件
    /// </summary>
    /// <param name="path"></param>
    /// <param name="object"></param>
    void ReadMultiLinearPolarization3DObjectDatabaseJsonByJsonFile(const char* path, MultiLinearPolarization3DObjectDatabaseJsonStd::MultiLinearPolarization3DObjectDatabaseJson& object);


    /// <summary>
    /// 写入到json文件
    /// </summary>
    /// <param name="path"></param>
    /// <param name="object"></param>
    void WriteMultiLinearPolarization3DObjectDatabaseJsonToJsonFile(const char* path, const MultiLinearPolarization3DObjectDatabaseJsonStd::MultiLinearPolarization3DObjectDatabaseJson& object);
    
    void WriteMultiLinearPolarization3DDatabaseToJsonFile(const std::string& multiLinearPolarization3DDatabaseFileName);

    void InitMultiLinearPolarization3DDatabaseByJsonFile(const std::string& multiLinearPolarization3DDatabaseFileName);

}