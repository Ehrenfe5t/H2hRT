#pragma once

#include"LxQMultiPathNodeInfo.h"

namespace MultiPathNodeInfoStd {

	class MultiPathNodeInfoDiffuseScattering : public MultiPathNodeInfo
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
		/// 漫散射波瓣的宽度系数
		/// </summary>
		int widthCoefficientOfDiffuseLobe;
		/// <summary>
		/// 表面粗糙度，0表示光滑
		/// </summary>
		float roughness;
		/// <summary>
		/// 入射角,注意这里的入射角应该是和反射的入射角统一概念，即与面元的法相夹角
		/// </summary>
		double thetai;
		/// <summary>
		/// 漫散射路径与镜面反射方向之间的夹角
		/// </summary>
		double beta;

		/// <summary>
		/// 散射面积
		/// </summary>
		double area;

		/// <summary>
		/// 散射系数，0-1.0
		/// </summary>
		double scatteringCoefficient;


		Point3DStd::Point3D normalVector;
		MultiPathNodeInfoDiffuseScattering(
			int upObjectType, int downObjectType,
			int widthCoefficientOfDiffuseLobe, float roughness,double thetai, double beta, double area, double scatteringCoefficient,
			const Point3DStd::Point3D& location,
			const Point3DStd::Point3D& normalVector);
		MultiPathNodeInfoDiffuseScattering();
		~MultiPathNodeInfoDiffuseScattering();

	private:

	};

}