#pragma once

#include"LxQPoint3D.h"

namespace AntennaPropertyStd {

	class AntennaProperty
	{
	public:

        /// <summary>
        /// 天线方向图id
        /// </summary>
        int radiationPatternId;

        /// <summary>
        /// 极化id,
        /// polarization3DModelId表示唯一编号,如果出现重复则后面的不起作用，只以第一次初始化为准;
        /// polarization3DModelId == -1表示非法，请不要设置它的值为 - 1;
        /// polarization3DModelId == 0表示全极化，不需要设置初始相位和极化方向以及能量占比;
        /// polarization3DModelId == 1表示默认的单一线极化, 初始相位为0，极化方向为(0, 0, 1), 能量占比为100 %;
        /// polarization3DModelId == 2表示默认的双线极化表示的一种左旋圆极化;
        /// 第1个线极化的初始相位为0，极化方向为(0, 0, 1)，能量占比为50 %;
        /// 第2个线极化的初始相位为0.5 * pi，极化方向为(0, 1, 0)，能量占比为50 %;
        ///  ======polarization3DModelId不能设置为 - 1，0，1，2.
        /// </summary>
        int polarization3DModelId;

        /// <summary>
        /// 频率，单位Hz
        /// </summary>
        std::vector<long long> frequencys;

        /// <summary>
        /// 天线坐标
        /// </summary>
        Point3DStd::Point3D location;
        AntennaProperty();
		~AntennaProperty();

	private:

	};


}