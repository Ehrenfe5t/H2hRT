#pragma once


#include"LxQPoint3D.h"
#include<vector>
namespace ChannelDataDoubleStd {

	class ChannelDataDouble
	{
	public:
		int id_tx;
		int id_rx;
		Point3DStd::Point3D location_tx;
		Point3DStd::Point3D location_rx;
		double value;
		ChannelDataDouble();
		~ChannelDataDouble();

	private:

	};

	std::string ChannelDataDouble_To_string_MOMI(
		const std::vector<int>& receiverAntennaIds,
		const std::vector<Point3DStd::Point3D>& rx_locations,
		const std::vector<double>& bigChannelParameters);

	std::string ChannelDataDouble_To_string(const std::vector<ChannelDataDoubleStd::ChannelDataDouble>& bigChannelParameters);

	void WriteToTxtFile(const std::string& fileNameWithoutEnd,
		const  std::string& excelHeader, const std::vector<ChannelDataDoubleStd::ChannelDataDouble>& channelDataDoubles);

	std::vector<ChannelDataDoubleStd::ChannelDataDouble> GetVector(
		const std::vector<int>& transmittingAntennaIds,
		const std::vector<Point3DStd::Point3D>& tx_locations,
		const std::vector<int>& receiverAntennaIds,
		const std::vector<Point3DStd::Point3D>& rx_locations,
		const std::vector<double>& bigChannelParameters);

}