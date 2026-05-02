
#include"../Input.h"
#include"../Impl/CalOfElectricFieldLossUnderCircularPolarization3D.h"
#include"../Impl/CalculateWaveLossCoefficientConstant.h"
#include"../Impl/CalculateWaveLossCoefficientBase.h"

#include<stdio.h>


double CalculateWaveNodeLossUnderCircularPolarization3D_0(
	int radiationPatternId, const ElectricFieldNode&e1, const ElectricFieldNode&e2, long long frequency, ElectricFieldNode& node, const MaterialSet& materialList) {

	Point3DStd::Point3D normalizationDirectionVectorOfPropagation;
	normalizationDirectionVectorOfPropagation.x = e2.location_x - e1.location_x;
	normalizationDirectionVectorOfPropagation.y = e2.location_y - e1.location_y;
	normalizationDirectionVectorOfPropagation.z = e2.location_z - e1.location_z;
	node.pre_distance = 0.0;
	node.next_distance = Geometry3DOperateStd::Length_Point3D(normalizationDirectionVectorOfPropagation);
	normalizationDirectionVectorOfPropagation = Geometry3DOperateStd::MulDoublePoint3D(1.0/ node.next_distance,normalizationDirectionVectorOfPropagation);

	//double antennaRadiationCoefficient = AntennaPatternDatabaseStd::CalCoefficientByVector(radiationPatternId, normalizationDirectionVectorOfPropagation);
	double antennaRadiationCoefficient = 1.0;
	const Material* material1 = &materialList.materials[node.materialIndex1];
	double loss = CalOfElectricFieldLossUnderCircularPolarization3DStd::CalLossByAntennaRadiation( 
		frequency, node.next_distance, antennaRadiationCoefficient, material1->relativePermittivity, material1->conductivity);

	return loss;

}

double CalculateWaveNodeLossUnderCircularPolarization3D_1(long long frequency, const ElectricFieldNode& node, const MaterialSet& materialList) {

	return 0.0;
}

double CalculateWaveNodeLossUnderCircularPolarization3D_2(long long frequency, const ElectricFieldNode& node, const MaterialSet& materialList) {

	const Material* material1 = &materialList.materials[node.materialIndex1];
	const Material* material2 = &materialList.materials[node.materialIndex2];
	double loss = CalOfElectricFieldLossUnderCircularPolarization3DStd::CalLossByReflectionField(
		frequency, node.r_t_s_d_thetai_beta0,
		material1->relativePermittivity, material1->conductivity,
		material2->relativePermittivity, material2->conductivity);

	return loss;

}

double CalculateWaveNodeLossUnderCircularPolarization3D_3(long long frequency, const ElectricFieldNode& node, const MaterialSet& materialList) {

	const Material* material1 = &materialList.materials[node.materialIndex1];
	const Material* material2 = &materialList.materials[node.materialIndex2];
	double loss = CalOfElectricFieldLossUnderCircularPolarization3DStd::CalLossByTransmissionField(
		frequency, node.r_t_s_d_thetai_beta0, 
		material1->relativePermittivity, material1->conductivity,
		material2->relativePermittivity, material2->conductivity);

	return loss;

}

double CalculateWaveNodeLossUnderCircularPolarization3D_4(long long frequency, const ElectricFieldNode& node, const MaterialSet& materialList) {

	const Material* material1 = &materialList.materials[node.materialIndex2];
	double loss = CalOfElectricFieldLossUnderCircularPolarization3DStd::CalLossByDiffractionField(
		frequency, node.r_t_s_d_thetai_beta0, node.d_phi1_s_thetar, node.d_phi2_s_ar, node.d_phiE_s_s,
		material1->relativePermittivity, material1->conductivity);

	return loss;

}

double CalculateWaveNodeLossUnderCircularPolarization3D_5(long long frequency, double s_coefficient,const ElectricFieldNode& node, const MaterialSet& materialList) {

	const Material* material1 = &materialList.materials[node.materialIndex2];
	double loss = CalOfElectricFieldLossUnderCircularPolarization3DStd::CalLossByDiffuseScatteringField(
		node.s_roughness, frequency,
		node.r_t_s_d_thetai_beta0, node.d_phi1_s_thetar,
		node.d_phi2_s_ar, node.d_phiE_s_s, s_coefficient,
		material1->relativePermittivity, material1->conductivity);

	return loss;

}

double CalculateWavePathLossUnderCircularPolarization3D(
	int radiationPatternId, long long frequency, double s_coefficient, const ElectricFieldPath& path, const MaterialSet& materialList) {

	if (path.node_size < 2) {
		return CalculateWaveLossCoefficientStd::MAX_PATH_LOSS;
	}

	double pathLoss = 0.0;

	for (int i = 0; i < path.node_size; ++i) {
		
		double loss = 0.0;
		auto& node = path.nodes[i];
		if (path.nodes[i].type == 0 && i==0) {
			loss = CalculateWaveNodeLossUnderCircularPolarization3D_0(radiationPatternId, path.nodes[i], path.nodes[i+1], frequency, node, materialList);
		}
		else if (path.nodes[i].type == 1) {
			loss = CalculateWaveNodeLossUnderCircularPolarization3D_1(frequency, node, materialList);
		}
		else if (path.nodes[i].type == 2) {
			loss = CalculateWaveNodeLossUnderCircularPolarization3D_2(frequency, node, materialList);

			auto material = materialList.materials[node.materialIndex1];
			{
				//如果是在内部反射，

				Point3D p1;
				p1.x = path.nodes[i - 1].location_x;
				p1.y = path.nodes[i - 1].location_y;
				p1.z = path.nodes[i - 1].location_z;

				Point3D p2;
				p2.x = node.location_x;
				p2.y = node.location_y;
				p2.z = node.location_z;

				Point3D p3;
				p3.x = p2.x - p1.x;
				p3.y = p2.y - p1.y;
				p3.z = p2.z - p1.z;

				Point3D n;
				n.x = node.n_x;
				n.y = node.n_y;
				n.z = node.n_z;

				double dot_2 = (n.x* p3.x + n.y* p3.y + n.z* p3.z);
				if (dot_2 > 0.0) {
					material = materialList.materials[node.materialIndex2];
				}
			}

			{
				node.pre_distance = path.nodes[i - 1].pre_distance + path.nodes[i - 1].next_distance;

				auto temp_1_x = path.nodes[i + 1].location_x - node.location_x;
				auto temp_1_y = path.nodes[i + 1].location_y - node.location_y;
				auto temp_1_z = path.nodes[i + 1].location_z - node.location_z;

				node.next_distance = sqrt(temp_1_x * temp_1_x + temp_1_y * temp_1_y + temp_1_z * temp_1_z);
			}

			loss += CalOfElectricFieldLossUnderCircularPolarization3DStd::CalLossByDirectField(
				frequency,
				node.pre_distance, node.next_distance, material.relativePermittivity, material.conductivity);
		}
		else if (path.nodes[i].type == 3) {
			loss = CalculateWaveNodeLossUnderCircularPolarization3D_3(frequency, node, materialList);

			auto material = materialList.materials[node.materialIndex2];
			{
				//如果是在内部折射，

				Point3D p1;
				p1.x = path.nodes[i - 1].location_x;
				p1.y = path.nodes[i - 1].location_y;
				p1.z = path.nodes[i - 1].location_z;

				Point3D p2;
				p2.x = node.location_x;
				p2.y = node.location_y;
				p2.z = node.location_z;

				Point3D p3;
				p3.x = p2.x - p1.x;
				p3.y = p2.y - p1.y;
				p3.z = p2.z - p1.z;

				Point3D n;
				n.x = node.n_x;
				n.y = node.n_y;
				n.z = node.n_z;

				double dot_2 = (n.x * p3.x + n.y * p3.y + n.z * p3.z);
				if (dot_2 > 0.0) {
					material = materialList.materials[node.materialIndex1];
				}
			}
			{
				node.pre_distance = path.nodes[i - 1].pre_distance + path.nodes[i - 1].next_distance;

				auto temp_1_x = path.nodes[i + 1].location_x - node.location_x;
				auto temp_1_y = path.nodes[i + 1].location_y - node.location_y;
				auto temp_1_z = path.nodes[i + 1].location_z - node.location_z;

				node.next_distance = sqrt(temp_1_x * temp_1_x + temp_1_y * temp_1_y + temp_1_z * temp_1_z);
			}

			loss += CalOfElectricFieldLossUnderCircularPolarization3DStd::CalLossByDirectField(frequency,
				node.pre_distance, node.next_distance, material.relativePermittivity, material.conductivity);
		}
		else if (path.nodes[i].type == 4) {
			loss = CalculateWaveNodeLossUnderCircularPolarization3D_4(frequency, node, materialList);

			auto material = materialList.materials[node.materialIndex1];

			{
				node.pre_distance = path.nodes[i - 1].pre_distance + path.nodes[i - 1].next_distance;

				auto temp_1_x = path.nodes[i + 1].location_x - node.location_x;
				auto temp_1_y = path.nodes[i + 1].location_y - node.location_y;
				auto temp_1_z = path.nodes[i + 1].location_z - node.location_z;

				node.next_distance = sqrt(temp_1_x * temp_1_x + temp_1_y * temp_1_y + temp_1_z * temp_1_z);
			}

			loss += CalOfElectricFieldLossUnderCircularPolarization3DStd::CalLossByDirectField(frequency,
				node.pre_distance, node.next_distance, material.relativePermittivity, material.conductivity);
		}
		else if (path.nodes[i].type == 5) {
			loss = CalculateWaveNodeLossUnderCircularPolarization3D_5(frequency, s_coefficient, node, materialList);

			auto material = materialList.materials[node.materialIndex1];

			{
				node.pre_distance = path.nodes[i - 1].pre_distance + path.nodes[i - 1].next_distance;

				auto temp_1_x = path.nodes[i + 1].location_x - node.location_x;
				auto temp_1_y = path.nodes[i + 1].location_y - node.location_y;
				auto temp_1_z = path.nodes[i + 1].location_z - node.location_z;

				node.next_distance = sqrt(temp_1_x * temp_1_x + temp_1_y * temp_1_y + temp_1_z * temp_1_z);
			}

			loss += CalOfElectricFieldLossUnderCircularPolarization3DStd::CalLossByDirectField(frequency,
				node.pre_distance, node.next_distance, material.relativePermittivity, material.conductivity);
		}
		else {
			printf("Error: Unknown node type.\n");
			return CalculateWaveLossCoefficientStd::MAX_PATH_LOSS;
		}
		pathLoss += loss;
	}

	return pathLoss;	

}


void CalculateWaveImpactResponseDBmUnderCircularPolarization3D(
	int radiationPatternId,
	double transmitPowerW,
	long long frequency,
	double s_coefficient,
	const MaterialSet& materialList,
	ElectricFieldPath& path) {


	double pathLossDBm = CalculateWavePathLossUnderCircularPolarization3D(radiationPatternId, frequency, s_coefficient, path, materialList);
	double ptDBm = CalculateWaveLossCoefficientStd::GetDBmByW(transmitPowerW,-CalculateWaveLossCoefficientStd::MAX_PATH_LOSS);

	path.impactResponse = ptDBm - pathLossDBm;

}


void CalculateWaveImpactResponseDBmUnderCircularPolarization3Ds(
	int radiationPatternId, 
	double transmitPowerW, 
	long long frequency,
	double s_coefficient,
	const MaterialSet& materialList,
	int path_size,
	ElectricFieldPath* paths, 
	double& prPathLossDB, 
	double& prPowerDBm) {
	

	if (path_size < 1) {
		prPathLossDB = CalculateWaveLossCoefficientStd::MAX_PATH_LOSS;
		prPowerDBm = -CalculateWaveLossCoefficientStd::MAX_PATH_LOSS;
	}

	double ptDBm = CalculateWaveLossCoefficientStd::GetDBmByW(transmitPowerW, -CalculateWaveLossCoefficientStd::MAX_PATH_LOSS);

	double pathLossW = 0.0;

	for (int i = 0; i < path_size; ++i) {
		CalculateWaveImpactResponseDBmUnderCircularPolarization3D(
			radiationPatternId, transmitPowerW, frequency, s_coefficient,  materialList, paths[i]);
		double pathLoss1 = paths[i].impactResponse;
		if (pathLoss1 > -CalculateWaveLossCoefficientStd::MAX_PATH_LOSS) {
			//值大概率有效
			double pathLossW1 = CalculateWaveLossCoefficientStd::GetWByDBm(pathLoss1);
			pathLossW += pathLossW1;
		}
	}

	if (pathLossW > 0) {
		prPowerDBm = CalculateWaveLossCoefficientStd::GetDBmByW(pathLossW, -CalculateWaveLossCoefficientStd::MAX_PATH_LOSS);
		prPathLossDB = ptDBm - prPowerDBm;
	}
	else {
		prPathLossDB = CalculateWaveLossCoefficientStd::MAX_PATH_LOSS;
		prPowerDBm = -CalculateWaveLossCoefficientStd::MAX_PATH_LOSS;
	}
}


void CalculateWaveImpactResponseDBmUnderCircularPolarization3DsPrintf(
	int radiationPatternId,
	double transmitPowerW,
	long long frequency,
	double s_coefficient,
	const MaterialSet& materialList,
	int path_size,
	ElectricFieldPath* paths,
	double& prPathLossDB,
	double& prPowerDBm) {

	CalculateWaveImpactResponseDBmUnderCircularPolarization3Ds(
		radiationPatternId, transmitPowerW, frequency, s_coefficient,
		materialList, path_size, paths, prPathLossDB, prPowerDBm);

	PrintMaterialSet(materialList);
	ElectricFieldAllPath printfPath;
	printfPath.path_size = path_size;
	printfPath.paths = paths;
	ElectricFieldAllPathToJsonFile(printfPath, "test-printfPath.json");
}