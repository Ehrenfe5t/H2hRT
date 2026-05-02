
#include"HdQBigChannelParameter.h"

#include"QzQFileBase.h"
#include"QzQDirectoryOperate.h"
#include"LxQProjectDependencies.h"

namespace BigChannelParameterStd {


    void WriteVectorBigChannelParameterToTxtFile_electricFieldCalculationMode_power(
        const std::string& csv_fileName, 
        const std::string& txt_fileName,
        const std::vector<ChannelDataDoubleStd::ChannelDataDouble>& bigChannelParameters) {

        auto context = ChannelDataDoubleStd::ChannelDataDouble_To_string(bigChannelParameters);
        std::string csv_context;
        csv_context.append("tx_id,tx_x,tx_y,tx_z,rx_id,rx_x,rx_y,rx_z,power(dBm)\n");
        csv_context.append(context);

        FileOperateStd::WriteStringToTxtFile(csv_fileName.c_str(), csv_context);
        FileOperateStd::WriteStringToTxtFile(txt_fileName.c_str(), context);

    }

    void WriteVectorBigChannelParameterToTxtFile_electricFieldCalculationMode_pathloss(
        const std::string& csv_fileName,
        const std::string& txt_fileName,
        const std::vector<ChannelDataDoubleStd::ChannelDataDouble>& bigChannelParameters) {

        auto context = ChannelDataDoubleStd::ChannelDataDouble_To_string(bigChannelParameters);
        std::string csv_context;
        csv_context.append("tx_id,tx_x,tx_y,tx_z,rx_id,rx_x,rx_y,rx_z,pathloss(dB)\n");
        csv_context.append(context);

        FileOperateStd::WriteStringToTxtFile(csv_fileName.c_str(), csv_context);
        FileOperateStd::WriteStringToTxtFile(txt_fileName.c_str(), context);

    }

    void WriteVectorBigChannelParameterToTxtFile_0_electricFieldCalculationMode_power(
        const std::string& directoryPathName,
        const std::vector<ChannelDataDoubleStd::ChannelDataDouble>& bigChannelParameters) {

        std::string csv_fileName = directoryPathName + "\\BigChannelParameter.0.power.electricFieldCalculationMode.csv";
        std::string txt_fileName = directoryPathName + "\\BigChannelParameter.0.power.electricFieldCalculationMode.txt";

        WriteVectorBigChannelParameterToTxtFile_electricFieldCalculationMode_power(csv_fileName, txt_fileName, bigChannelParameters);

    }

    void WriteVectorBigChannelParameterToTxtFile_0_electricFieldCalculationMode_pathloss(
        const std::string& directoryPathName,
        const std::vector<ChannelDataDoubleStd::ChannelDataDouble>& bigChannelParameters) {

        std::string csv_fileName = directoryPathName + "\\BigChannelParameter.0.pathloss.electricFieldCalculationMode.csv";
        std::string txt_fileName = directoryPathName + "\\BigChannelParameter.0.pathloss.electricFieldCalculationMode.txt";

        WriteVectorBigChannelParameterToTxtFile_electricFieldCalculationMode_pathloss(csv_fileName, txt_fileName, bigChannelParameters);

    }

    void WriteVectorBigChannelParameterToTxtFile_1_electricFieldCalculationMode_power(
        const std::string& directoryPathName,
        const std::vector<ChannelDataDoubleStd::ChannelDataDouble>& bigChannelParameters) {

        std::string csv_fileName = directoryPathName + "\\BigChannelParameter.1.power.electricFieldCalculationMode.csv";
        std::string txt_fileName = directoryPathName + "\\BigChannelParameter.1.power.electricFieldCalculationMode.txt";

        WriteVectorBigChannelParameterToTxtFile_electricFieldCalculationMode_power(csv_fileName, txt_fileName, bigChannelParameters);

    }

    void WriteVectorBigChannelParameterToTxtFile_1_electricFieldCalculationMode_pathloss(
        const std::string& directoryPathName,
        const std::vector<ChannelDataDoubleStd::ChannelDataDouble>& bigChannelParameters) {

        std::string csv_fileName = directoryPathName + "\\BigChannelParameter.1.pathloss.electricFieldCalculationMode.csv";
        std::string txt_fileName = directoryPathName + "\\BigChannelParameter.1.pathloss.electricFieldCalculationMode.txt";

        WriteVectorBigChannelParameterToTxtFile_electricFieldCalculationMode_pathloss(csv_fileName, txt_fileName, bigChannelParameters);

    }

    void WriteVectorBigChannelParameterToTxtFile_2_electricFieldCalculationMode_power(
        const std::string& directoryPathName,
        const std::vector<ChannelDataDoubleStd::ChannelDataDouble>& bigChannelParameters) {

        std::string csv_fileName = directoryPathName + "\\BigChannelParameter.2.power.electricFieldCalculationMode.csv";
        std::string txt_fileName = directoryPathName + "\\BigChannelParameter.2.power.electricFieldCalculationMode.txt";

        WriteVectorBigChannelParameterToTxtFile_electricFieldCalculationMode_power(csv_fileName, txt_fileName, bigChannelParameters);

    }

    void WriteVectorBigChannelParameterToTxtFile_2_electricFieldCalculationMode_pathloss(
        const std::string& directoryPathName,
        const std::vector<ChannelDataDoubleStd::ChannelDataDouble>& bigChannelParameters) {

        std::string csv_fileName = directoryPathName + "\\BigChannelParameter.2.pathloss.electricFieldCalculationMode.csv";
        std::string txt_fileName = directoryPathName + "\\BigChannelParameter.2.pathloss.electricFieldCalculationMode.txt";

        WriteVectorBigChannelParameterToTxtFile_electricFieldCalculationMode_pathloss(csv_fileName, txt_fileName, bigChannelParameters);

    }

    void WriteVectorBigChannelParameterToTxtFile_3_electricFieldCalculationMode_power(
        const std::string& directoryPathName,
        const std::vector<ChannelDataDoubleStd::ChannelDataDouble>& bigChannelParameters) {

        std::string csv_fileName = directoryPathName + "\\BigChannelParameter.3.power.electricFieldCalculationMode.csv";
        std::string txt_fileName = directoryPathName + "\\BigChannelParameter.3.power.electricFieldCalculationMode.txt";

        WriteVectorBigChannelParameterToTxtFile_electricFieldCalculationMode_power(csv_fileName, txt_fileName, bigChannelParameters);

    }

    void WriteVectorBigChannelParameterToTxtFile_3_electricFieldCalculationMode_pathloss(
        const std::string& directoryPathName,
        const std::vector<ChannelDataDoubleStd::ChannelDataDouble>& bigChannelParameters) {

        std::string csv_fileName = directoryPathName + "\\BigChannelParameter.3.pathloss.electricFieldCalculationMode.csv";
        std::string txt_fileName = directoryPathName + "\\BigChannelParameter.3.pathloss.electricFieldCalculationMode.txt";

        WriteVectorBigChannelParameterToTxtFile_electricFieldCalculationMode_pathloss(csv_fileName, txt_fileName, bigChannelParameters);

    }

    void WriteVectorBigChannelParameterToTxtFile_4_electricFieldCalculationMode_power(
        const std::string& directoryPathName,
        const std::vector<ChannelDataDoubleStd::ChannelDataDouble>& bigChannelParameters) {

        std::string csv_fileName = directoryPathName + "\\BigChannelParameter.4.power.electricFieldCalculationMode.csv";
        std::string txt_fileName = directoryPathName + "\\BigChannelParameter.4.power.electricFieldCalculationMode.txt";

        WriteVectorBigChannelParameterToTxtFile_electricFieldCalculationMode_power(csv_fileName, txt_fileName, bigChannelParameters);

    }

    void WriteVectorBigChannelParameterToTxtFile_4_electricFieldCalculationMode_pathloss(
        const std::string& directoryPathName,
        const std::vector<ChannelDataDoubleStd::ChannelDataDouble>& bigChannelParameters) {

        std::string csv_fileName = directoryPathName + "\\BigChannelParameter.4.pathloss.electricFieldCalculationMode.csv";
        std::string txt_fileName = directoryPathName + "\\BigChannelParameter.4.pathloss.electricFieldCalculationMode.txt";

        WriteVectorBigChannelParameterToTxtFile_electricFieldCalculationMode_pathloss(csv_fileName, txt_fileName, bigChannelParameters);

    }

    void WriteVectorBigChannelParameterToTxtFile_5_electricFieldCalculationMode_power(
        const std::string& directoryPathName,
        const std::vector<ChannelDataDoubleStd::ChannelDataDouble>& bigChannelParameters) {

        std::string csv_fileName = directoryPathName + "\\BigChannelParameter.5.power.electricFieldCalculationMode.csv";
        std::string txt_fileName = directoryPathName + "\\BigChannelParameter.5.power.electricFieldCalculationMode.txt";

        WriteVectorBigChannelParameterToTxtFile_electricFieldCalculationMode_power(csv_fileName, txt_fileName, bigChannelParameters);

    }

    void WriteVectorBigChannelParameterToTxtFile_5_electricFieldCalculationMode_pathloss(
        const std::string& directoryPathName,
        const std::vector<ChannelDataDoubleStd::ChannelDataDouble>& bigChannelParameters) {

        std::string csv_fileName = directoryPathName + "\\BigChannelParameter.5.pathloss.electricFieldCalculationMode.csv";
        std::string txt_fileName = directoryPathName + "\\BigChannelParameter.5.pathloss.electricFieldCalculationMode.txt";

        WriteVectorBigChannelParameterToTxtFile_electricFieldCalculationMode_pathloss(csv_fileName, txt_fileName, bigChannelParameters);

    }



    void WriteVectorBigChannelParameterToTxtFile_power(int electricFieldCalculationMode, const std::string& directoryPathName, const std::vector<ChannelDataDoubleStd::ChannelDataDouble>& bigChannelParameters) {
        FileOperateStd::CreateNewDirectorys(directoryPathName);
        switch (electricFieldCalculationMode)
        {
        case 0:
            return WriteVectorBigChannelParameterToTxtFile_0_electricFieldCalculationMode_power(directoryPathName, bigChannelParameters);
        case 1:
            return WriteVectorBigChannelParameterToTxtFile_1_electricFieldCalculationMode_power(directoryPathName, bigChannelParameters);
        case 2:
            return WriteVectorBigChannelParameterToTxtFile_2_electricFieldCalculationMode_power(directoryPathName, bigChannelParameters);
        case 3:
            return WriteVectorBigChannelParameterToTxtFile_3_electricFieldCalculationMode_power(directoryPathName, bigChannelParameters);
        case 4:
            return WriteVectorBigChannelParameterToTxtFile_4_electricFieldCalculationMode_power(directoryPathName, bigChannelParameters);
        case 5:
            return WriteVectorBigChannelParameterToTxtFile_5_electricFieldCalculationMode_power(directoryPathName, bigChannelParameters);

        default:
            {
                std::ostringstream oss;
                oss   << "electricFieldCalculationMode为" << electricFieldCalculationMode << "未来实现";
                ProjectDependenciesStd::DisplayPromptOrReason(oss.str(), true, __FILE__, __LINE__);
            }
            break;
        }

    }


    void WriteVectorBigChannelParameterToTxtFile_pathloss(int electricFieldCalculationMode, const std::string& directoryPathName, const std::vector<ChannelDataDoubleStd::ChannelDataDouble>& bigChannelParameters) {
        FileOperateStd::CreateNewDirectorys(directoryPathName);
        switch (electricFieldCalculationMode)
        {
        case 0:
            return WriteVectorBigChannelParameterToTxtFile_0_electricFieldCalculationMode_pathloss(directoryPathName, bigChannelParameters);
        case 1:                                                                           
            return WriteVectorBigChannelParameterToTxtFile_1_electricFieldCalculationMode_pathloss(directoryPathName, bigChannelParameters);
        case 2:                                                                           
            return WriteVectorBigChannelParameterToTxtFile_2_electricFieldCalculationMode_pathloss(directoryPathName, bigChannelParameters);
        case 3:                                                                           
            return WriteVectorBigChannelParameterToTxtFile_3_electricFieldCalculationMode_pathloss(directoryPathName, bigChannelParameters);
        case 4:                                                                           
            return WriteVectorBigChannelParameterToTxtFile_4_electricFieldCalculationMode_pathloss(directoryPathName, bigChannelParameters);
        case 5:                                                                           
            return WriteVectorBigChannelParameterToTxtFile_5_electricFieldCalculationMode_pathloss(directoryPathName, bigChannelParameters);

        default:
        {
            std::ostringstream oss;
            oss << "electricFieldCalculationMode为" << electricFieldCalculationMode << "未来实现";
            ProjectDependenciesStd::DisplayPromptOrReason(oss.str(), true, __FILE__, __LINE__);
        }
        break;
        }

    }



}