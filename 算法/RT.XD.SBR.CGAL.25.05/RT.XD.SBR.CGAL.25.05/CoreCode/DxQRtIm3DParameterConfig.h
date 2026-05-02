#pragma once

#include"HdQCommonParameterConfig.h"
#include"QzQDataInputCsvFileParameterConfig.h"
#include"QzQDataOutputParameterConfig.h"
#include"QzQGeometricSpaceAccelerateParameterConfig.h"
#include"LxQMultithreadParameterConfig.h"
#include"LxQNumericalMethodParameterConfig.h"
#include"DxQRayEjectionParameterConfig.h"
#include"DxQRtIm3DPrivateParameterConfig.h"

namespace RtIm3DParameterConfigStd {

	class RtIm3DParameterConfig
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
        /// 反向射线跟踪的特有参数
        /// </summary>
        RtIm3DPrivateParameterConfigStd::RtIm3DPrivateParameterConfig rtIm3DPrivateParameterConfig;

        RtIm3DParameterConfig();
		~RtIm3DParameterConfig();

	private:

	};


}