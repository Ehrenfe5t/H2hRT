#pragma once


namespace CommonParameterConfigStd {

	class CommonParameterConfig
	{
	public:

        /// <summary>
        /// 自由空间对应的物质类型，该参数与环境信息中的材质参数应该保持一致。
        /// </summary>
        int airSubstanceType;

        /// <summary>
        /// 电场计算模式:
        /// 0表示标量叠加计算.计算的有效功率(平均功率);
        /// 1表示矢量叠加计算.计算的有效功率(平均功率);
        /// 2表示矢量瞬时场计算.计算的瞬时功率;
        /// </summary>
        int electricFieldCalculationMode;

        /// <summary>
        /// 能量输出模式:0表示计算接收功率;1表示计算路径损耗。
        /// </summary>
        int energyOutputMode;

        /// <summary>
        /// 最低能量阈值
        /// </summary>
        double powerThreshold;

        /// <summary>
        /// 是否重构边
        /// </summary>
        bool rebuildEdge;

        /// <summary>
        /// 寻找中心簇的去重半径
        /// </summary>
        double deduplicateRadius;

		CommonParameterConfig();
		~CommonParameterConfig();

	private:

	};


}