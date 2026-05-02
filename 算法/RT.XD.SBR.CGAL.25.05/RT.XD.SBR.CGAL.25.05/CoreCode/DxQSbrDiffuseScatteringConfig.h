#pragma once

#include"QzQDiffuseScatteringParameter.h"

namespace SbrDiffuseScatteringConfigStd {

	class SbrDiffuseScatteringConfig
	{
	public:

        DiffuseScatteringParameterStd::DiffuseScatteringParameter diffuseScatteringParameter;
        /// <summary>
        /// 计算面元漫散射时的最大离散边长
        /// </summary>
        double maxDiscreteSideLength;

        /// <summary>
        /// 离散漫散射的量，方位角
        /// </summary>
        double gapDiffuseScatteringAzimuth;

        /// <summary>
        /// 离散漫散射的量，俯仰角
        /// </summary>
        double gapDiffuseScatteringPitchAngle;

		SbrDiffuseScatteringConfig();
		~SbrDiffuseScatteringConfig();

	private:

	};


}