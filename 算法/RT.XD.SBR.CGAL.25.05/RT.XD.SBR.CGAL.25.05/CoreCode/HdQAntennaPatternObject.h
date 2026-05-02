#pragma once

#include<vector>
#include"LxQPoint3D.h"

namespace AntennaPatternObjectStd {

	const int radiationPattern_rows = 360;
	const int radiationPattern_cols = 181;

	class AntennaPatternObject
	{
	public:
		AntennaPatternObject();
		~AntennaPatternObject();


		/// <summary>
		/// 方向图编号，radiationPatternId<0表示不使用天线方向图；radiationPatternId==0表示理想3维均匀方向图，即等半径球面上的能量完全相同；
		/// </summary>
		int radiationPatternId;
		/// <summary>
		/// 天线方向图，先方位角，再俯仰角，这里是考虑增益、旋转、极化后的天线方向图，即确定的天线远场辐射图
		/// </summary>
		double** radiationPattern;


		void UpdateData(const AntennaPatternObject& antennaPatternObject);

		void UpdateData1(int radiationPatternId, const std::vector<std::vector<double>>& radiationPattern);
		void UpdateData2(int radiationPatternId, double* radiationPattern);

	private:

	};

	double GetRadiationPatternByVector(const AntennaPatternObject& antennaPatternObject, const Point3DStd::Point3D& vector);


	void Free_All_radiationPattern_ptr();
}