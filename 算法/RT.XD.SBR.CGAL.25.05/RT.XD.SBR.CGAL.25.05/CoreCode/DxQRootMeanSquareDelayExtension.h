#pragma once


#include"HdQBigChannelParameter.h"
/// <summary>
/// 均方根时延扩展
/// </summary>
namespace RootMeanSquareDelayExtensionStd {



    void WriteVectorBigChannelParameterToTxtFile_RootMeanSquareDelayExtension(
        const std::string& directoryPathName,
        const std::vector<ChannelDataDoubleStd::ChannelDataDouble>& bigChannelParameters);

}