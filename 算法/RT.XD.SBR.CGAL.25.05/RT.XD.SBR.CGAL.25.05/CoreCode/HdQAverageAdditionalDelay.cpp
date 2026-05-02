
#include"HdQAverageAdditionalDelay.h"

#include"QzQFileBase.h"

namespace AverageAdditionalDelayStd {


    void WriteVectorBigChannelParameterToTxtFile_AverageAdditionalDelay(
        const std::string& directoryPathName,
        const std::vector<ChannelDataDoubleStd::ChannelDataDouble>& bigChannelParameters) {


        auto context = ChannelDataDoubleStd::ChannelDataDouble_To_string(bigChannelParameters);
        std::string csv_context;
        csv_context.append("tx_id,tx_x,tx_y,tx_z,rx_id,rx_x,rx_y,rx_z,averageAdditionalDelay(ns)\n");
        csv_context.append(context);

        std::string csv_fileName = directoryPathName + "\\BigChannelParameter.AverageAdditionalDelay.csv";
        std::string txt_fileName = directoryPathName + "\\BigChannelParameter.AverageAdditionalDelay.txt";

        FileOperateStd::WriteStringToTxtFile(csv_fileName.c_str(), csv_context);
        FileOperateStd::WriteStringToTxtFile(txt_fileName.c_str(), context);


    }

}