#pragma once

#include"Scenario3D.h"

#include <vector>

/// <summary>
/// 接收天线结构体
/// </summary>
struct ReceiverAntenna {

	/// <summary>
	/// 接收天线id
	/// </summary>
	int receiverAntennaId;

	/// <summary>
	/// 天线坐标
	/// </summary>
	Point3D location;

};


/// <summary>
/// 带宽信息
/// </summary>
struct FrequencyBandwidth {

	int size;
	/// <summary>
	/// 多频点
	/// </summary>
	long long* frequencys;

};


/// <summary>
/// 暂时没有考虑天线方向图和天线极化
/// </summary>
struct TransmitterAntenna {
	/// <summary>
	/// 天线处在的材质类型
	/// </summary>
	short materialTypeNumber;
	/// <summary>
	/// 发射天线编号
	/// </summary>
	int transmitterAntennaId;
	int polarization3DModelId;
	int radiationPatternId;
	/// <summary>
	/// 发射功率,单位是w
	/// </summary>
	double transmitPower;
	/// <summary>
	/// 频率相关
	/// </summary>
	FrequencyBandwidth frequencyBandwidth;
	/// <summary>
	/// 天线坐标
	/// </summary>
	Point3D location;
	int receiversCount;       // 接收机数量
	/// <summary>
	/// 接收机
	/// </summary>
	ReceiverAntenna* receivers;
};





/// <summary>
/// 天线极化3D模型
/// </summary>
struct AntennaLinearPolarization3DObject
{

	/// <summary>
	/// 初始相位
	/// </summary>
	double phi0;

	/// <summary>
	/// 天线极化方向
	/// </summary>
	Point3D vec;
};


//一个天线线极化模型
struct OneAntennaLinearPolarization3D
{
	double weight;
	AntennaLinearPolarization3DObject linearPolarization3DObject;
};


//一个天线极化模型
struct AntennaPolarization3DModel
{
	/// <summary>
	/// 唯一编号
	/// </summary>
	int polarization3DModelId;

	int size;
	/// <summary>
	/// 包含的多个线极化模型
	/// </summary>
	OneAntennaLinearPolarization3D* multiLinearPolarization3D;
};



//一个天线远场辐射图模型
struct AntennaRadiationPattern3DModel
{
	/// <summary>
	/// 唯一编号
	/// </summary>
	int radiationPatternId;
	int rows;    // 行数（360）
	int columns; // 列数（181）

	/// <summary>
	/// 必须是360*181的矩阵，表示每个角度的辐射强度
	/// </summary>
	double* radiationPattern;
};
