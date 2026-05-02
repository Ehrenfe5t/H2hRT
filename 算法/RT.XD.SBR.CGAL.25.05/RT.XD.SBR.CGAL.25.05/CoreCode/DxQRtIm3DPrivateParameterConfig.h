#pragma once

#include"QzQDiffuseScatteringParameter.h"

namespace RtIm3DPrivateParameterConfigStd {

	class RtIm3DPrivateParameterConfig
	{
	public:

		/// <summary>
		/// 漫散射的相关参数
		/// </summary>
		DiffuseScatteringParameterStd::DiffuseScatteringParameter diffuseScatteringParameter;

		/// <summary>
		/// 是否允许反射和绕射的组合
		/// </summary>
		bool isDiffractionReflection;

		/// <summary>
		/// 计算面元漫散射时的最大离散边长
		/// </summary>
		double maxDiscreteSideLength;

		/// <summary>
		/// 反向射线跟踪中，查找点的可见面时场景的离散点数量由这个值绝对，这个值越大，离散约精细，计算速度越慢，建议默认值100；
		/// </summary>
		double mumberOfDiscreteBoundingBox;



		RtIm3DPrivateParameterConfig();
		~RtIm3DPrivateParameterConfig();

	private:

	};


}
