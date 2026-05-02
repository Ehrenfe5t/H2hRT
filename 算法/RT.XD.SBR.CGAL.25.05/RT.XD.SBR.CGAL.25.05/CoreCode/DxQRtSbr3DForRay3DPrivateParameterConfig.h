#pragma once

#include"QzQDiffuseScatteringParameter.h"

namespace RtSbr3DForRay3DPrivateParameterConfigStd {

	class RtSbr3DForRay3DPrivateParameterConfig
	{
	public:
		/// <summary>
		/// 是否是真实折射，true表示带偏折，false表示不带偏折
		/// </summary>
		bool realWorldRefraction;

		/// <summary>
		/// 采用圆柱法还是圆锥法,true圆柱形管,false圆锥射线管
		/// </summary>
		bool cylindricalTube;

		/// <summary>
		/// 棱边的接收半径
		/// </summary>
		double radiusCorner;

		/// <summary>
		/// 接收球的接收半径/接收角
		/// </summary>
		double radiusRx;

		/// <summary>
		/// 射线数量
		/// </summary>
		size_t rayNumber;

		/// <summary>
		/// 棱边绕射的离散角度
		/// </summary>
		double gapDiffractionRad;

		/// <summary>
		/// 漫散射的离散角度，方位角
		/// </summary>
		double gapDiffuseScatteringAzimuth;

		/// <summary>
		/// 漫散射的离散角度，俯仰角
		/// </summary>
		double gapDiffuseScatteringPitchAngle;

		/// <summary>
		/// 漫散射的相关参数
		/// </summary>
		DiffuseScatteringParameterStd::DiffuseScatteringParameter diffuseScatteringParameter;


		RtSbr3DForRay3DPrivateParameterConfig();
		~RtSbr3DForRay3DPrivateParameterConfig();

	private:

	};



}