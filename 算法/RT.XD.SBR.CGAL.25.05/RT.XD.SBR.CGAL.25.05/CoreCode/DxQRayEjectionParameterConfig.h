#pragma once


namespace RayEjectionParameterConfigStd {

	class RayEjectionParameterConfig
	{
	public:

        /// <summary>
        /// 射线最大弹射次数。
        /// </summary>
        int ejectionsMaxTotalNumber;

        /// <summary>
        /// 射线绕射最大弹射次数。
        /// </summary>
        int ejectionsOfDiffractionMaxNumber;

        /// <summary>
        /// 射线漫散射最大弹射次数。
        /// </summary>
        int ejectionsOfDiffuseScatteringMaxNumber;

        /// <summary>
        /// 射线反射最大弹射次数。
        /// </summary>
        int ejectionsOfReflectionMaxNumber;

        /// <summary>
        /// 射线折射最大弹射次数。
        /// </summary>
        int ejectionsOfTransmissionMaxNumber;

        /// <summary>
        /// 是否计算视距路径
        /// </summary>
        bool switchOfLos;

		RayEjectionParameterConfig();
		~RayEjectionParameterConfig();

	private:

	};


}
