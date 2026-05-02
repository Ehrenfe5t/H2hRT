#pragma once

#include "ElectricFieldPath.h"

/// <summary>
/// 这个仅仅生成冲激响应信息文件，会生成多个文件，因此传入的是一个文件夹的名称
/// </summary>
/// <param name="pRtoiOutputInformation"></param>
/// <param name="directoryPathName"></param>
extern "C" _declspec(dllexport) void RtoiOutputImpactResponseInformationToCsvFile(const RtoiOutputInformation& rtoiOutputInformation, const std::string& directoryPathName);




/// <summary>
/// 这个仅仅生成大尺度信息文件，会生成多个文件，因此传入的是一个文件夹的名称
/// </summary>
/// <param name="pRtoiOutputInformation"></param>
/// <param name="directoryPathName"></param>
extern "C" _declspec(dllexport) void RtoiOutputPowerInformationToCsvFile(const RtoiOutputInformation& rtoiOutputInformation, const std::string& directoryPathName);




/// <summary>
/// 将RtoiOutputInformation转换为json文件
/// </summary>
/// <param name="pRtoiOutputInformation"></param>
/// <param name="jsonFileName"></param>
/// <returns></returns>
extern "C" _declspec(dllexport) void RtoiOutputInformationToJsonFile(const RtoiOutputInformation & rtoiOutputInformation, const std::string & jsonFileName);


extern "C" _declspec(dllexport) void ElectricFieldAllPathToJsonFile(const ElectricFieldAllPath & electricFieldAllPath, const std::string & jsonFileName);

