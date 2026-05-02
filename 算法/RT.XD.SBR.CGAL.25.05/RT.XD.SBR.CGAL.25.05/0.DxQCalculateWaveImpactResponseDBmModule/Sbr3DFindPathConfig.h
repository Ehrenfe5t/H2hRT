#pragma once


struct Sbr3DFindPathConfig
{

	/// <summary>
	/// 是否重构边
	/// </summary>
	bool rebuildEdge;

	bool multithreadConfigSwitchOfMultithread;

	bool switchOfLos;
	char ejectionsMaxTotalNumber;
	char ejectionsOfDiffractionMaxNumber;
	char ejectionsOfDiffuseScatteringMaxNumber;
	char ejectionsOfReflectionMaxNumber;
	char ejectionsOfTransmissionMaxNumber;

	/// <summary>
	/// 射线与环境交互加速模式
	/// </summary>
	int geometricSpaceAccelerateType;

	/// <summary>
	/// 定向散射波瓣系数
	/// </summary>
	int diffuseScatteringAr;

	double powerThreshold;

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
	double rayNumber;

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
	/// 散射系数
	/// </summary>
	double diffuseScatteringCoefficient;

	/// <summary>
	/// 毫米波瑞利判据范围8~32
	/// </summary>
	double diffuseScatteringRayleighRange;
};