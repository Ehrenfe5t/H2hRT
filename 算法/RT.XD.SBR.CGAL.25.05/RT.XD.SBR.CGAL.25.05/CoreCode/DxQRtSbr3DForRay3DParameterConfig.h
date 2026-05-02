#pragma once

#include"HdQCommonParameterConfig.h"
#include"QzQDataInputCsvFileParameterConfig.h"
#include"QzQDataOutputParameterConfig.h"
#include"QzQGeometricSpaceAccelerateParameterConfig.h"
#include"LxQMultithreadParameterConfig.h"
#include"LxQNumericalMethodParameterConfig.h"
#include"DxQRayEjectionParameterConfig.h"
#include"DxQRtSbr3DForRay3DPrivateParameterConfig.h"

namespace RtSbr3DForRay3DParameterConfigStd {

	class RtSbr3DForRay3DParameterConfig
	{
	public:
        /// <summary>
        /// 常用配置
        /// </summary>
        CommonParameterConfigStd::CommonParameterConfig commonParameterConfig;

        /// <summary>
        /// 输入文件
        /// </summary>
        DataInputCsvFileParameterConfigStd::DataInputCsvFileParameterConfig dataInputCsvFileParameterConfig;

        /// <summary>
        /// 输出文件配置
        /// </summary>
        DataOutputParameterConfigStd::DataOutputParameterConfig dataOutputParameterConfig;

        /// <summary>
        /// 空间加速配置
        /// </summary>
        GeometricSpaceAccelerateParameterConfigStd::GeometricSpaceAccelerateParameterConfig geometricSpaceAccelerateParameterConfig;

        /// <summary>
        /// 多线程配置
        /// </summary>
        MultithreadParameterConfigStd::MultithreadParameterConfig multithreadParameterConfig;

        /// <summary>
        /// 数值法配置
        /// </summary>
        NumericalMethodParameterConfigStd::NumericalMethodParameterConfig numericalMethodParameterConfig;

        /// <summary>
        /// 射线弹射次数设置
        /// </summary>
        RayEjectionParameterConfigStd::RayEjectionParameterConfig rayEjectionParameterConfig;

        /// <summary>
        /// 正向向射线跟踪的特有参数
        /// </summary>
        RtSbr3DForRay3DPrivateParameterConfigStd::RtSbr3DForRay3DPrivateParameterConfig rtSbr3DForRay3DPrivateParameterConfig;
		RtSbr3DForRay3DParameterConfig();
		~RtSbr3DForRay3DParameterConfig();

	private:

	};



}