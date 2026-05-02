#pragma once



#include"LxQPixelParameterConfig.h"
#include"HdQBvhParameterConfig.h"
#include"DxQRayScenarioIntersectType.h"

namespace SpaceSubregionParameterConfigStd {


	class SpaceSubregionParameterConfig
	{
	public:
		/// <summary>
		/// 
		/// </summary>
		RayScenarioIntersectTypeStd::RayScenarioIntersectType type;

		/// <summary>
		/// Pixel3D,Pixel3D_SDF参数
		/// </summary>
		PixelParameterConfigStd::PixelParameterConfig pixelParameterConfig;

		/// <summary>
		/// Bvh参数
		/// </summary>
		BvhParameterConfigStd::BvhParameterConfig bvhParameterConfig;

		SpaceSubregionParameterConfig();
		~SpaceSubregionParameterConfig();

	private:

	};

}