#pragma once

#include"LxQMultiPathNodeInfo.h"

namespace MultiPathNodeInfoStd {

	class MultiPathNodeInfoDiffraction : public MultiPathNodeInfo
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
		/// 入射线与棱线的夹角
		/// </summary>
		double beta;
		/// <summary>
		/// 拐角夹角,相对概念指的是入射线看不见的一侧
		/// </summary>
		double phiE;
		/// <summary>
		/// 入射线与拐角的0面的投影夹角
		/// </summary>
		double phi1;
		/// <summary>
		/// 入射线与拐角的n面的投影夹角
		/// </summary>
		double phi2;

		Point3DStd::Point3D normalVector;
		MultiPathNodeInfoDiffraction(
			int upObjectType, int downObjectType, 
			double beta, double phiE, double phi1, double phi2,
			const Point3DStd::Point3D& location,
			const Point3DStd::Point3D& normalVector);
		MultiPathNodeInfoDiffraction();
		~MultiPathNodeInfoDiffraction();

	private:

	};

}