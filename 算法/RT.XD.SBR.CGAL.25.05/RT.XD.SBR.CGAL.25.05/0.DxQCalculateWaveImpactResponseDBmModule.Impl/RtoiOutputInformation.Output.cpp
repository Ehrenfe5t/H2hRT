

#include "Input.h"

#include<fstream>
#include<iostream>

#include<Windows.h>
namespace OutputFileOperateStd {

    /// <summary>
    /// 检查文件是否存在
    /// </summary>
    /// <param name="path">文件路径</param>
    /// <returns>存在返回true</returns>
    bool ExistFile(const char* path) {
        std::filesystem::path filePath = path;
        return std::filesystem::exists(filePath);
    }

    std::string doubleToString(double value) {
        std::array<char, 64> buffer{}; // 64字节足够存储所有double值
        auto [ptr, ec] = std::to_chars(buffer.data(), buffer.data() + buffer.size(), value);
        if (ec == std::errc()) {
            return std::string(buffer.data(), ptr);
        }
        return "";
    }

    std::string intToStr(int num) {
        std::array<char, 16> buffer{}; // 足够存储32位整数

        // 执行转换
        auto [ptr, ec] = std::to_chars(buffer.data(),
            buffer.data() + buffer.size(),
            num);

        if (ec == std::errc()) {
            return std::string(buffer.data(), ptr);
        }
        return "";
    }

    /// <summary>
    /// 点转化为字符串
    /// </summary>
    /// <param name="p"></param>
    /// <param name="midGap"></param>
    /// <param name="endGap"></param>
    /// <returns></returns>
    std::string Point3DToString(const Point3DStd::Point3D& p, const std::string& midGap, const std::string& endGap, int doubleSize) {
        std::string res = "";
        //res.append(StructureToStringStd::DoubleToStringByStringStream(p.x, doubleSize)); res.append(midGap);
        //res.append(StructureToStringStd::DoubleToStringByStringStream(p.y, doubleSize)); res.append(midGap);
        //res.append(StructureToStringStd::DoubleToStringByStringStream(p.z, doubleSize)); res.append(endGap);

        res.reserve(66);
        res.append(doubleToString(p.x)); res.append(midGap);
        res.append(doubleToString(p.y)); res.append(midGap);
        res.append(doubleToString(p.z)); res.append(endGap);
        return res;
    }


    const char* ConvertCharToLPWSTR(const char* szString)
    {
        return szString;
    }
    /// <summary>
    /// 分割字符串
    /// </summary>
    /// <param name="str"></param>
    /// <param name="pattern"></param>
    /// <returns></returns>
    std::vector<std::string> SplitWithStl(const std::string& str, const std::string& pattern)
    {
        std::vector<std::string> resVec;

        if ("" == str)
        {
            return resVec;
        }
        //方便截取最后一段数据
        std::string strs = str + pattern;

        size_t pos = strs.find(pattern);
        size_t size = strs.size();

        while (pos != std::string::npos)
        {
            std::string x = strs.substr(0, pos);
            resVec.push_back(x);
            strs = strs.substr(pos + 1, size);
            pos = strs.find(pattern);
        }

        return resVec;
    }
    /// <summary>
    /// 创建一个目录，如果是多级目录，则依次创建多级目录
    /// </summary>
    /// <param name="folderPath"></param>
    void CreateNewDirectorys(const std::string& folderPath) {
        std::vector<std::string>folders = SplitWithStl(folderPath, "\\");
        if (folders.size() < 1) {
            return;
        }
        auto curFolderPath = folders[0];

        if (!std::filesystem::exists(curFolderPath)) {
            CreateDirectory(ConvertCharToLPWSTR(curFolderPath.c_str()), NULL);
        }
        for (int loop = 1; loop < folders.size(); loop++) {
            curFolderPath.append("\\");
            curFolderPath.append(folders[loop]);
            if (!std::filesystem::exists(curFolderPath)) {
                CreateDirectory(ConvertCharToLPWSTR(curFolderPath.c_str()), NULL);
            }
        }
    }

    /// <summary>
    /// 将字符串写入txt类型的文件
    /// </summary>
    /// <param name="path"></param>
    /// <param name="jf"></param>
    void WriteStringToTxtFile(const char* path, const std::string& jf) {
        if (ExistFile(path)) {
            remove(path);
        }
        std::ofstream fileOpen; //定义ofstream 对象
        //fileOpen.open(path, std::ofstream::app);//追加
        fileOpen.open(path);
        if (!fileOpen)
        {
            std::cout << "没有打开文件！" << std::endl;
            fileOpen.close();
            return;
        }
        fileOpen << jf;
        fileOpen.close();
    }



    void WriteJsonStringToJsonFile(const char* path, const nlohmann::json& jf) {
        if (ExistFile(path)) {
            remove(path);
        }

        std::ofstream fileOpen; //定义ofstream 对象
        //fileOpen.open(path, std::ofstream::app);//追加
        fileOpen.open(path);
        if (!fileOpen)
        {
            std::cout << ("[" + std::string(path) + "]文件没有打开!") << std::endl;
            fileOpen.close();
            return;
        }

        //std::cout << jf;
        fileOpen << jf.dump(4);
        fileOpen.close();
    }



    std::string FileOutput202501ChannelParameterInfoBaseBig_To_string(
        const std::vector<int>& transmittingAntennaIds,
        const std::vector<Point3DStd::Point3D>& tx_locations,
        const std::vector<int>& receiverAntennaIds,
        const std::vector<Point3DStd::Point3D>& rx_locations,
        const std::vector<double>& bigChannelParameters_power,
        const std::vector<double>& bigChannelParameters_pathloss) {

        if (transmittingAntennaIds.size() < 1) {
            return "";
        }

        std::string pre_line;
        pre_line.append(intToStr(transmittingAntennaIds[0])); pre_line.append(",");
        pre_line.append(Point3DToString(tx_locations[0], ",", "", 16)); pre_line.append(",");

        std::string context;
        context.reserve(transmittingAntennaIds.size() * 200);
        for (int i = 0; i < (int)transmittingAntennaIds.size() - 1; i++) {
            context.append(pre_line);
            context.append(intToStr(receiverAntennaIds[i])); context.append(",");
            context.append(Point3DToString(rx_locations[i], ",", "", 16)); context.append(",");
            //context.append(StructureToStringStd::DoubleToStringByStringStream(bigChannelParameters_power[i], 16)); context.append(",");
            //context.append(StructureToStringStd::DoubleToStringByStringStream(bigChannelParameters_pathloss[i], 16)); context.append(",");
            context.append(doubleToString(bigChannelParameters_power[i])); context.append(",");
            context.append(doubleToString(bigChannelParameters_pathloss[i])); context.append(",");

            context.append("\n");
        }
        if (transmittingAntennaIds.size() > 0) {
            int i = (int)transmittingAntennaIds.size() - 1;
            context.append(pre_line);
            context.append(intToStr(receiverAntennaIds[i])); context.append(",");
            context.append(Point3DToString(rx_locations[i], ",", "", 16)); context.append(",");
            //context.append(StructureToStringStd::DoubleToStringByStringStream(bigChannelParameters_power[i], 16)); context.append(",");
            //context.append(StructureToStringStd::DoubleToStringByStringStream(bigChannelParameters_pathloss[i], 16)); context.append(",");
            context.append(doubleToString(bigChannelParameters_power[i])); context.append(",");
            context.append(doubleToString(bigChannelParameters_pathloss[i])); context.append(",");
        }
        context.shrink_to_fit();
        return context;

    }

    std::string FileOutput202501ChannelParameterInfoBaseSmall_To_string(
        const std::vector<double>& totalPathLength,
        const std::vector<double>& timeDelay,
        const std::vector<double>& impulseResponse,
        const std::vector<double>& waveEmissionAzimuth,
        const std::vector<double>& waveEmissionElevationAngle,
        const std::vector<double>& waveReachingAzimuth,
        const std::vector<double>& waveReachingElevationAngle) {

        if (totalPathLength.size() < 1) {
            return "";
        }


        std::string context;
        context.reserve(2 * totalPathLength.size() * 133);
        for (int i = 0; i < (int)totalPathLength.size() - 1; i++) {

            context.append(doubleToString(totalPathLength[i])); context.append(",");
            context.append(doubleToString(timeDelay[i])); context.append(",");
            context.append(doubleToString(impulseResponse[i])); context.append(",");
            context.append(doubleToString(waveEmissionAzimuth[i])); context.append(",");
            context.append(doubleToString(waveEmissionElevationAngle[i])); context.append(",");
            context.append(doubleToString(waveReachingAzimuth[i])); context.append(",");
            context.append(doubleToString(waveReachingElevationAngle[i]));

            context.append("\n");
        }
        if (totalPathLength.size() > 0) {
            int i = (int)totalPathLength.size() - 1;

            context.append(doubleToString(totalPathLength[i])); context.append(",");
            context.append(doubleToString(timeDelay[i])); context.append(",");
            context.append(doubleToString(impulseResponse[i])); context.append(",");
            context.append(doubleToString(waveEmissionAzimuth[i])); context.append(",");
            context.append(doubleToString(waveEmissionElevationAngle[i])); context.append(",");
            context.append(doubleToString(waveReachingAzimuth[i])); context.append(",");
            context.append(doubleToString(waveReachingElevationAngle[i]));
        }
        context.shrink_to_fit();
        return context;

    }


}



//转化为json字符串 start

void to_json(nlohmann::json& j, const Point3D& obj) {
    j = nlohmann::json{ { "x", obj.x },{ "y", obj.y },{ "z", obj.z } };
}

void to_json(nlohmann::json& j, const ElectricFieldNode& obj) {

    j["attachmentNumber"] = obj.attachmentNumber;
    j["d_phi1_s_thetar"] = obj.d_phi1_s_thetar;
    j["d_phi2_s_ar"] = obj.d_phi2_s_ar;
    j["d_phiE_s_s"] = obj.d_phiE_s_s;
    j["location_x"] = obj.location_x;
    j["location_y"] = obj.location_y;
    j["location_z"] = obj.location_z;
    j["materialIndex1"] = obj.materialIndex1;
    j["materialIndex2"] = obj.materialIndex2;
    j["n_x"] = obj.n_x;
    j["n_y"] = obj.n_y;
    j["n_z"] = obj.n_z;
    j["next_distance"] = obj.next_distance;
    j["pre_distance"] = obj.pre_distance;
    j["r_t_s_d_thetai_beta0"] = obj.r_t_s_d_thetai_beta0;
    j["s_roughness"] = obj.s_roughness;
    j["type"] = obj.type;

}

void to_json(nlohmann::json& j, const ElectricFieldPath& obj) {

    j["aoa_phi"] = obj.aoa_phi;
    j["aoa_theta"] = obj.aoa_theta;
    j["aod_phi"] = obj.aod_phi;
    j["aod_theta"] = obj.aod_theta;
    j["delay"] = obj.delay;
    j["impactResponse"] = obj.impactResponse;

    j["propagationNodeInformations"];

    for (int i = 0; i < obj.node_size; ++i) {
        auto& value = obj.nodes[i];
        nlohmann::json jf;
        to_json(jf, value);
        j["propagationNodeInformations"].push_back(jf);
    }

}

void to_json(nlohmann::json& j, const ElectricFieldAllPath& obj) {
    j["receiverAntennaId"] = obj.receiverAntennaId;
    j["pathLoss"] = obj.pathLoss;
    j["power"] = obj.power;

    j["propagationPathInformations"];
    for (int i = 0; i < obj.path_size; ++i) {
        auto& value = obj.paths[i];
        nlohmann::json jf;
        to_json(jf, value);
        j["propagationPathInformations"].push_back(jf);
    }


}

void to_json(nlohmann::json& j, const PropagationFrequencyInformation& obj) {
    j["frequency"] = obj.frequency;
    j["propagationReceiverInformations"];
    for (int i = 0; i < obj.receivers_information_size; ++i) {
        auto& value = obj.receivers_information[i];
        nlohmann::json jf;
        to_json(jf, value);
        j["propagationReceiverInformations"].push_back(jf);

    }
}

/// <summary>
/// 将点对象转化为json字符串
/// </summary>
/// <param name="j"></param>
/// <param name="p"></param>
void to_json(nlohmann::json& j, const RtoiOutputInformation& obj) {

    j["transmitterAntennaId"] = obj.transmitterAntennaId;

    j["propagationFrequencyInformations"];
    for (int i = 0; i < obj.frequency_size; ++i) {
        auto& value = obj.frequencys_information[i];
        nlohmann::json jf;
        to_json(jf, value);
        j["propagationFrequencyInformations"].push_back(jf);

    }

}


//转化为json字符串 end


void RtoiOutputInformationToJsonFile(const RtoiOutputInformation& rtoiOutputInformation, const std::string& jsonFileName) {

    nlohmann::json jf;
    to_json(jf, rtoiOutputInformation);
    OutputFileOperateStd::WriteJsonStringToJsonFile(jsonFileName.c_str(), jf);

}


void ElectricFieldAllPathToJsonFile(const ElectricFieldAllPath & electricFieldAllPath, const std::string & jsonFileName) {

    nlohmann::json jf;
    to_json(jf, electricFieldAllPath);
    OutputFileOperateStd::WriteJsonStringToJsonFile(jsonFileName.c_str(), jf);

}

void RtoiOutputPowerInformationToCsvFile_frequencys_information(
    int transmittingAntennaId,
    const Point3D& tx_location,
    const PropagationFrequencyInformation& propagationFrequencyInformation,
    const std::string& fileName) {


    std::vector<int> transmittingAntennaIds;
    std::vector<Point3DStd::Point3D> tx_locations;
    std::vector<int> receiverAntennaIds;
    std::vector<Point3DStd::Point3D> rx_locations;
    std::vector<double> bigChannelParameters_power;
    std::vector<double> bigChannelParameters_pathloss;

    size_t rx_size = propagationFrequencyInformation.receivers_information_size;
    transmittingAntennaIds.resize(rx_size);
    tx_locations.resize(rx_size);
    receiverAntennaIds.resize(rx_size);
    rx_locations.resize(rx_size);
    bigChannelParameters_power.resize(rx_size);
    bigChannelParameters_pathloss.resize(rx_size);

    for (int i = 0; i < rx_size; ++i) {
        auto siso = propagationFrequencyInformation.receivers_information[i];

        transmittingAntennaIds[i] = transmittingAntennaId;
        tx_locations[i].x = tx_location.x;
        tx_locations[i].y = tx_location.y;
        tx_locations[i].z = tx_location.z;
        receiverAntennaIds[i] = siso.receiverAntennaId;
        rx_locations[i].x = siso.rx_location.x;
        rx_locations[i].y = siso.rx_location.y;
        rx_locations[i].z = siso.rx_location.z;
        bigChannelParameters_power[i] = siso.power;
        bigChannelParameters_pathloss[i] = siso.pathLoss;
    }

    std::string excelHeader = "transmittingAntennaId,tx_x(m),tx_y(m),tx_z(m),receiverAntennaId,rx_x(m),rx_y(m),rx_z(m),power(dBm),pathloss(dB)\n";

    std::string context = OutputFileOperateStd::FileOutput202501ChannelParameterInfoBaseBig_To_string(
        transmittingAntennaIds, tx_locations, receiverAntennaIds, rx_locations, bigChannelParameters_power, bigChannelParameters_pathloss);

    OutputFileOperateStd::WriteStringToTxtFile(fileName.c_str(), excelHeader + context);

}

void RtoiOutputPowerInformationToCsvFile(const RtoiOutputInformation& rtoiOutputInformation, const std::string& directoryPathName) {

    std::string newDirectoryPathName = directoryPathName + "\\Tx[";
    {
        std::ostringstream oss;
        oss << rtoiOutputInformation.transmitterAntennaId;
        oss << "]";
        newDirectoryPathName.append(oss.str());
    }
    OutputFileOperateStd::CreateNewDirectorys(newDirectoryPathName);
    for (int i = 0; i < rtoiOutputInformation.frequency_size; i++) {

        long long frequency = rtoiOutputInformation.frequencys_information[i].frequency;
        std::string fileName = newDirectoryPathName + "\\frequency[";
        {
            std::ostringstream oss;
            oss << ((int)(frequency / 1e6));
            oss << "].MHz.csv";
            fileName.append(oss.str());
        }

        RtoiOutputPowerInformationToCsvFile_frequencys_information(
            rtoiOutputInformation.transmitterAntennaId, rtoiOutputInformation.tx_location,
            rtoiOutputInformation.frequencys_information[i], fileName);
    }

}


void RtoiOutputImpactResponseInformationToCsvFile_siso(
    const ElectricFieldAllPath& propagationReceiverInformation, const std::string& fileName) {


    std::string excelHeader = "totalPathLength(m),timeDelay(ns),impulseResponse(dBm),waveEmissionAzimuth(rad),waveEmissionElevationAngle(rad),waveReachingAzimuth(rad),waveReachingElevationAngle(rad)\n";

    auto paths_size = propagationReceiverInformation.path_size;

    std::vector<double> totalPathLength_vector(paths_size);
    std::vector<double> timeDelay_vector(paths_size);
    std::vector<double> impulseResponse_vector(paths_size);
    std::vector<double> waveEmissionAzimuth_vector(paths_size);
    std::vector<double> waveEmissionElevationAngle_vector(paths_size);
    std::vector<double> waveReachingAzimuth_vector(paths_size);
    std::vector<double> waveReachingElevationAngle_vector(paths_size);

    for (int i = 0; i < paths_size; ++i) {
        double timeDelay = propagationReceiverInformation.paths[i].delay;
        double totalPathLength = timeDelay * (GlobalConstantStd::C * 1e-9);
        double impulseResponse = propagationReceiverInformation.paths[i].impactResponse;
        double waveEmissionAzimuth = propagationReceiverInformation.paths[i].aod_phi;
        double waveEmissionElevationAngle = propagationReceiverInformation.paths[i].aod_theta;
        double waveReachingAzimuth = propagationReceiverInformation.paths[i].aoa_phi;
        double waveReachingElevationAngle = propagationReceiverInformation.paths[i].aoa_theta;

        totalPathLength_vector[i] = totalPathLength;
        timeDelay_vector[i] = timeDelay;
        impulseResponse_vector[i] = impulseResponse;
        waveEmissionAzimuth_vector[i] = waveEmissionAzimuth;
        waveEmissionElevationAngle_vector[i] = waveEmissionElevationAngle;
        waveReachingAzimuth_vector[i] = waveReachingAzimuth;
        waveReachingElevationAngle_vector[i] = waveReachingElevationAngle;

    }


    std::string context = OutputFileOperateStd::FileOutput202501ChannelParameterInfoBaseSmall_To_string(
        totalPathLength_vector, timeDelay_vector, impulseResponse_vector, waveEmissionAzimuth_vector, waveEmissionElevationAngle_vector, waveReachingAzimuth_vector, waveReachingElevationAngle_vector);

    OutputFileOperateStd::WriteStringToTxtFile(fileName.c_str(), excelHeader + context);


}

void RtoiOutputImpactResponseInformationToCsvFile_frequencys_information(
    const PropagationFrequencyInformation& propagationFrequencyInformation, const std::string& directoryPathName) {

    OutputFileOperateStd::CreateNewDirectorys(directoryPathName);

    for (int i = 0; i < propagationFrequencyInformation.receivers_information_size; i++) {

        int receiverAntennaId = propagationFrequencyInformation.receivers_information[i].receiverAntennaId;
        std::string fileName = directoryPathName + "\\Rx[";
        {
            std::ostringstream oss;
            oss << receiverAntennaId;
            oss << "].csv";
            fileName.append(oss.str());
        }

        RtoiOutputImpactResponseInformationToCsvFile_siso(propagationFrequencyInformation.receivers_information[i], fileName);

    }

}

void RtoiOutputImpactResponseInformationToCsvFile(const RtoiOutputInformation& rtoiOutputInformation, const std::string& directoryPathName) {

    std::string newDirectoryPathName = directoryPathName + "\\Tx[";
    {
        std::ostringstream oss;
        oss << rtoiOutputInformation.transmitterAntennaId;
        oss << "]";
        newDirectoryPathName.append(oss.str());
    }
    OutputFileOperateStd::CreateNewDirectorys(newDirectoryPathName);

    for (int i = 0; i < rtoiOutputInformation.frequency_size; i++) {

        long long frequency = rtoiOutputInformation.frequencys_information[i].frequency;
        std::string directoryPathName2 = newDirectoryPathName + "\\frequency[";
        {
            std::ostringstream oss;
            oss << ((int)(frequency / 1e6));
            oss << "].MHz";
            directoryPathName2.append(oss.str());
        }

        RtoiOutputImpactResponseInformationToCsvFile_frequencys_information(
            rtoiOutputInformation.frequencys_information[i], directoryPathName2);
    }
}