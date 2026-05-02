#pragma once


#include"HdQConfig.h"
#include"HdQAntennaMIMOPath.h"

namespace CalAntennaPathElectricFieldInterfaceStd{

	INTERFACE_API void CalantennaMIMOPathElectricField(bool modeType, const AntennaMIMOPathStd::AntennaMIMOPath& antennaMIMOPath);

	INTERFACE_API void CalBigChannelParameter_MIMO(
		int electricFieldCalculationMode,
		int energyOutputMode,
		double powerThreshold,
		std::vector<int>& transmittingAntennaIds,
		std::vector<Point3DStd::Point3D>& tx_locations,
		std::vector<int>& receiverAntennaIds,
		std::vector<Point3DStd::Point3D>& rx_locations,
		std::vector<double>& bigChannelParameters);
}

namespace AntennaMIMOPathElectricFieldFileOutputStd {

	/// <summary>
	/// electricFieldCalculationMode;
	/// 0表示标量叠加计算.计算的有效功率(平均功率);
	/// 1表示矢量叠加计算.计算的有效功率(平均功率);
	/// 2表示矢量瞬时场计算.计算的瞬时功率;
	/// energyOutputMode;
	/// 0表示计算接收功率;1表示计算路径损耗。
	/// </summary>
	/// <param name="string"></param>
	/// <param name=""></param>
	/// <param name=""></param>
	/// <param name=""></param>
	/// <param name=""></param>
	/// <param name=""></param>
	/// <param name="antennaMIMOPathElectricField"></param>
	/// <param name="outPutDirectoryPathName"></param>
	INTERFACE_API void AntennaMIMOPathElectricFieldFileOutput(
		int electricFieldCalculationMode,
		int energyOutputMode,
		bool switchOfBigChannelParameterInfo,
		bool switchOfSmallChannelParameterInfo,
		bool switchOfStatisticChannelParameterInfo,
		const std::string& outPutDirectoryPathName,
		double powerThreshold);


	INTERFACE_API void MultipleSignalSourceSuperpositionOutput(
		int electricFieldCalculationMode,
		int energyOutputMode,
		bool switchOfMultipleSignalSourceSuperposition,
		const std::string& outPutDirectoryPathName,
		double powerThreshold);

}