#pragma once
#include<string>

#include<vector>
#include"HdQChannelDataDouble.h"

namespace BigChannelParameterStd {

	void WriteVectorBigChannelParameterToTxtFile_0_electricFieldCalculationMode_power(const std::string& directoryPathName, const std::vector<ChannelDataDoubleStd::ChannelDataDouble>& bigChannelParameters);
	void WriteVectorBigChannelParameterToTxtFile_0_electricFieldCalculationMode_pathloss(const std::string& directoryPathName, const std::vector<ChannelDataDoubleStd::ChannelDataDouble>& bigChannelParameters);
	void WriteVectorBigChannelParameterToTxtFile_1_electricFieldCalculationMode_power(const std::string& directoryPathName, const std::vector<ChannelDataDoubleStd::ChannelDataDouble>& bigChannelParameters);
	void WriteVectorBigChannelParameterToTxtFile_1_electricFieldCalculationMode_pathloss(const std::string& directoryPathName, const std::vector<ChannelDataDoubleStd::ChannelDataDouble>& bigChannelParameters);
	void WriteVectorBigChannelParameterToTxtFile_2_electricFieldCalculationMode_power(const std::string& directoryPathName, const std::vector<ChannelDataDoubleStd::ChannelDataDouble>& bigChannelParameters);
	void WriteVectorBigChannelParameterToTxtFile_2_electricFieldCalculationMode_pathloss(const std::string& directoryPathName, const std::vector<ChannelDataDoubleStd::ChannelDataDouble>& bigChannelParameters);
	void WriteVectorBigChannelParameterToTxtFile_3_electricFieldCalculationMode_power(const std::string& directoryPathName, const std::vector<ChannelDataDoubleStd::ChannelDataDouble>& bigChannelParameters);
	void WriteVectorBigChannelParameterToTxtFile_3_electricFieldCalculationMode_pathloss(const std::string& directoryPathName, const std::vector<ChannelDataDoubleStd::ChannelDataDouble>& bigChannelParameters);
	void WriteVectorBigChannelParameterToTxtFile_4_electricFieldCalculationMode_power(const std::string& directoryPathName, const std::vector<ChannelDataDoubleStd::ChannelDataDouble>& bigChannelParameters);
	void WriteVectorBigChannelParameterToTxtFile_4_electricFieldCalculationMode_pathloss(const std::string& directoryPathName, const std::vector<ChannelDataDoubleStd::ChannelDataDouble>& bigChannelParameters);
	void WriteVectorBigChannelParameterToTxtFile_5_electricFieldCalculationMode_power(const std::string& directoryPathName, const std::vector<ChannelDataDoubleStd::ChannelDataDouble>& bigChannelParameters);
	void WriteVectorBigChannelParameterToTxtFile_5_electricFieldCalculationMode_pathloss(const std::string& directoryPathName, const std::vector<ChannelDataDoubleStd::ChannelDataDouble>& bigChannelParameters);

	void WriteVectorBigChannelParameterToTxtFile_power(int electricFieldCalculationMode, const std::string& directoryPathName, const std::vector<ChannelDataDoubleStd::ChannelDataDouble>& bigChannelParameters);
	void WriteVectorBigChannelParameterToTxtFile_pathloss(int electricFieldCalculationMode, const std::string& directoryPathName, const std::vector<ChannelDataDoubleStd::ChannelDataDouble>& bigChannelParameters);


}