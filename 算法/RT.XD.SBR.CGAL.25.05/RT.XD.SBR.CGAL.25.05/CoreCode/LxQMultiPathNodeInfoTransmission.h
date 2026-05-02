#pragma once


#include"LxQMultiPathNodeInfo.h"

namespace MultiPathNodeInfoStd {

	class MultiPathNodeInfoTransmission : public MultiPathNodeInfo
	{
	public:

		/// <summary>
		/// 上侧物质类型，-1表示未初始化
		/// </summary>
		int upObjectType;
		/// <summary>
		/// 下侧物质类型，-1表示未初始化
		/// </summary>
		int downObjectType;
		/// <summary>
		/// 入射角
		/// </summary>
		double thetai;

		Point3DStd::Point3D normalVector;
		MultiPathNodeInfoTransmission(
			int upObjectType, int downObjectType, double thetai,
			const Point3DStd::Point3D& location,
			const Point3DStd::Point3D& normalVector);
		MultiPathNodeInfoTransmission();
		~MultiPathNodeInfoTransmission();

	private:

	};

}