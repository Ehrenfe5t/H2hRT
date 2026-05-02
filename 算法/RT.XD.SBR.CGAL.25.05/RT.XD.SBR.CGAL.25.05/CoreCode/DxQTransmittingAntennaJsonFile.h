#pragma once


#include<string>
#include<vector>

namespace TransmittingAntennaJsonFileStd {

	class TransmittingAntennaJsonFile
	{
	public:

		/// <summary>
		/// 天线所在的材质类型
		/// </summary>
		short materialTypeNumber;
		/// <summary>
		/// 发射天线Id，用于区别于其他的发射天线
		/// </summary>
		int transmittingAntennaId;

		/// <summary>
		/// 天线方向图映射Id，修正的是天线的电场的幅值
		/// </summary>
		int radiationPatternId;

		/// <summary>
		/// 天线极化映射Id，修正的是天线的电场的相位以及电场的方向
		/// </summary>
		int polarization3DModelId;

		/// <summary>
		/// 发射功率(w)
		/// </summary>
		double emissionPower;

		/// <summary>
		/// 位置x坐标
		/// </summary>
		double center_location_x;
		/// <summary>
		/// 位置y坐标
		/// </summary>
		double center_location_y;
		/// <summary>
		/// 位置z坐标
		/// </summary>
		double center_location_z;
		/// <summary>
		/// 这里进行多频点计算，其中以第一个频率作为计算路径的频点，单位(MHz)
		/// </summary>
		std::vector<long long> frequencys;

		/// <summary>
		/// 发射机对于的接收机文件,这做就支持了从属关系，点、多点仿真也支持的更好。
		/// </summary>
		std::string inputReceivingAntennaCsvFileName;
		TransmittingAntennaJsonFile();
		~TransmittingAntennaJsonFile();

		void AddFrequency(double frequency_);

		void AddFrequencys(const std::vector<double>& frequencys_);

	private:
		bool init_frequency;

	};


}