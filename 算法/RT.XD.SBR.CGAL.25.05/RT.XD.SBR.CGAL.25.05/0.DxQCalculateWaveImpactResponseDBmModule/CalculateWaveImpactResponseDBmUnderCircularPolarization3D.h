#pragma once

#include "ElectricFieldPath.h"
#include "Material.h"


/// <summary>
/// 计算一条路径的冲击响应，采用三维全极化，均为能量的标量叠加
/// </summary>
/// <param name="radiationPatternId">天线方向图编号</param>
/// <param name="transmitPowerW">发射功率</param>
/// <param name="frequency">频率，单位Hz</param>
/// <param name="s_coefficient">漫散射时使用，一般为8-32</param>
/// <param name="materialList">材质库信息</param>
/// <param name="path">路径信息，计算结果也在其中</param>
extern "C" _declspec(dllexport) 
void CalculateWaveImpactResponseDBmUnderCircularPolarization3D(
	int radiationPatternId,
	double transmitPowerW,
	long long frequency,
	double s_coefficient,
	const MaterialSet& materialList,
	ElectricFieldPath& path);

/// <summary>
/// 计算多条路径的冲击响应，采用三维全极化，均为能量的标量叠加
/// </summary>
/// <param name="radiationPatternId">天线方向图编号</param>
/// <param name="transmitPowerW">发射功率</param>
/// <param name="frequency">频率，单位Hz</param>
/// <param name="s_coefficient">漫散射时使用，一般为8-32</param>
/// <param name="materialList">材质库信息</param>
/// <param name="paths">多条路径</param>
/// <param name="prPathLossDB">计算结果，总的路径损耗</param>
/// <param name="prPowerDBm">计算结果，总的接收功率</param>
extern "C" _declspec(dllexport) void CalculateWaveImpactResponseDBmUnderCircularPolarization3Ds(
	int radiationPatternId,
	double transmitPowerW,
	long long frequency,
	double s_coefficient,
	const MaterialSet & materialList,
	int paths_size,
	ElectricFieldPath* paths,
	double& prPathLossDB,
	double& prPowerDBm);

extern "C" _declspec(dllexport) void CalculateWaveImpactResponseDBmUnderCircularPolarization3DsPrintf(
	int radiationPatternId,
	double transmitPowerW,
	long long frequency,
	double s_coefficient,
	const MaterialSet & materialList,
	int paths_size,
	ElectricFieldPath * paths,
	double& prPathLossDB,
	double& prPowerDBm);
