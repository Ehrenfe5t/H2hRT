

#include"DxQRootMeanSquareDelayExtension.h"
#include"QzQFileBase.h"
namespace RootMeanSquareDelayExtensionStd {



    void WriteVectorBigChannelParameterToTxtFile_RootMeanSquareDelayExtension(
        const std::string& directoryPathName,
        const std::vector<ChannelDataDoubleStd::ChannelDataDouble>& bigChannelParameters) {

        auto context = ChannelDataDoubleStd::ChannelDataDouble_To_string(bigChannelParameters);
        std::string csv_context;
        csv_context.append("tx_id,tx_x,tx_y,tx_z,rx_id,rx_x,rx_y,rx_z,rootMeanSquareDelayExtension(ns)\n");
        csv_context.append(context);

        std::string csv_fileName = directoryPathName + "\\BigChannelParameter.RootMeanSquareDelayExtension.csv";
        std::string txt_fileName = directoryPathName + "\\BigChannelParameter.RootMeanSquareDelayExtension.txt";

        FileOperateStd::WriteStringToTxtFile(csv_fileName.c_str(), csv_context);
        FileOperateStd::WriteStringToTxtFile(txt_fileName.c_str(), context);

    }

}