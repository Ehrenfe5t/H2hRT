#pragma once

#include"HdQBigChannelParameter.h"
#include<string>
#include<vector>

namespace AverageAdditionalDelayStd {


    void WriteVectorBigChannelParameterToTxtFile_AverageAdditionalDelay(
        const std::string& directoryPathName,
        const std::vector<ChannelDataDoubleStd::ChannelDataDouble>& bigChannelParameters);

}

