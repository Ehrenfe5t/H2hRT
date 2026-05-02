#pragma once

#include "Point3D.h"
#include <vector>

#include<string>


/// <summary>
/// 仿真计算的输出类型
/// </summary>
enum class OutputTypeOfSimulationCalculation : uint8_t {
	/// <summary>
	/// 计算异常
	/// </summary>
	Null,
	Successfully,
	Unsuccessfully,
};


struct OutputNewsOfSimulationCalculation {

	OutputTypeOfSimulationCalculation type;
	std::string news;

};

struct ElectricFieldNode
{
	/// <summary>
	/// 0:Tx
	/// 1:Rx
	/// 2:反射
	/// 3:折射
	/// 4:绕射
	/// 5:漫散射
	/// </summary>
	short type;
	short type_ZW;
	/// <summary>
	/// 附属物编号
	/// </summary>
	int attachmentNumber;

	int materialIndex1;
	int materialIndex2;

	//double pre_location_x;
	//double pre_location_y;
	//double pre_location_z;
	
	double location_x;
	double location_y;
	double location_z;
	//
	//double next_location_x;
	//double next_location_y;
	//double next_location_z;
	//
	double n_x;
	double n_y;
	double n_z;

	/// <summary>
	/// 电波已经走过的距离
	/// </summary>
	double pre_distance;
	/// <summary>
	/// 从当前节点到下一个节点的距离
	/// </summary>
	double next_distance;

	double r_t_s_d_thetai_beta0;

	double d_phi1_s_thetar;
	double d_phi2_s_ar;
	//漫散射系数
	double d_phiE_s_s;

	double s_roughness;

};

struct ElectricFieldPath
{


	int node_size;
	int node_size_ZW;


	/// <summary>
	/// 时延，单位：ns
	/// </summary>
	double delay;

	/// <summary>
	/// 波发方位角，单位：弧度
	/// </summary>
	double aod_phi;

	/// <summary>
	/// 波发俯仰角，单位：弧度
	/// </summary>
	double aod_theta;

	/// <summary>
	/// 波达方位角，单位：弧度
	/// </summary>
	double aoa_phi;

	/// <summary>
	/// 波达俯仰角，单位：弧度
	/// </summary>
	double aoa_theta;

	/// <summary>
	/// 冲激响应,dBm，这里是这条路径的在接收位置的平均功率对应的dBm值
	/// </summary>
	double impactResponse;

	ElectricFieldNode* nodes;

};

/// <summary>
/// 单个接收机的输出信息，单频点
/// </summary>
struct ElectricFieldAllPath {

	int receiverAntennaId;

	int path_size;

	double pathLoss;

	/// <summary>
	/// 
	/// </summary>
	double power;


	ElectricFieldPath* paths;
	Point3D rx_location;

};


/// <summary>
/// 单个频率的多个接收机输出信息
/// </summary>
struct PropagationFrequencyInformation {

	int receivers_information_size;
	int receivers_information_size_ZW;
	//8-32
	double s_coefficient;
	long long frequency;
	ElectricFieldAllPath* receivers_information;

};

struct RtoiOutputInformation {

	int transmitterAntennaId;
	int frequency_size;
	PropagationFrequencyInformation* frequencys_information;
	Point3D tx_location;

};

