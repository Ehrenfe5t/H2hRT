


#include"0.Solve1SPathByIm3DModule.Impl.Input.h"
#include"S1Ray3DIntersectTriangle3DBallBvhTree.h"

#include<iostream>

using namespace S1Ray3DIntersectTriangle3DBallBvhTreeStd;

namespace Solve1SPathByIm3DStd {

	struct TempTriangle3D {
		Point3D p1;
		Point3D p2;
		Point3D p3;
	};

	struct DiffuseScatteringFace
	{
		//属性：面积
		double Area = 0.0;
		//属性：三角形的三个顶点
		Point3D Center;
	};


	long long globalFrequency = (long long)3e9;
	double globalDiffuseScatteringRayleighRange = 8.0;
	double globalDiffuseScatteringAr = 2;
	double globalDiffuseScatteringCoefficient = 0.5;
	double globalDiffuseScatteringMaxDiscreteSideLength = 1.0;
	int globalTransmitterAntennaMaterialIndex = -1;
	std::vector<Triangle3DMaterial> globalTriangle3DMaterials;
	std::vector<bool> globalTriangle3DMaterialsInvalid;
	std::vector<Material> globalMaterials;
	std::vector<std::vector<DiffuseScatteringFace>> globalTriangle3DDiffuseScatteringFaces;
	std::vector<bool> globalTxCanSeeTriangle3Ds;
	std::vector<std::vector<bool>> globalRxCanSeeTriangle3Ds;

	void CalGlobalPoint3DCanSeeTriangle3Ds(const Point3D& o, std::vector<bool>& pointCanSeeTriangle3Ds) {
		for (int i = 0; i < globalTriangle3DMaterials.size(); i++) {
			Point3D po = SubPoint3DPoint3D(o, globalTriangle3DMaterials[i].scenarioTriangleP1);
			if (DotPoint3DPoint3D(globalTriangle3DMaterials[i].scenarioTriangleN, po) < Eps) {
				pointCanSeeTriangle3Ds[i] = false;
			}
		}
	}

	void CalGlobalTxCanSeeTriangle3Ds(const Point3D& o) {
		globalTxCanSeeTriangle3Ds.clear();
		globalTxCanSeeTriangle3Ds.resize(globalTriangle3DMaterials.size(), true);
		CalGlobalPoint3DCanSeeTriangle3Ds(o, globalTxCanSeeTriangle3Ds);
	}

	void CalGlobalRxCanSeeTriangle3Ds(int receiversCount, ReceiverAntenna* receivers) {
		globalRxCanSeeTriangle3Ds.clear();
		globalRxCanSeeTriangle3Ds.resize(receiversCount, std::vector<bool>(globalTriangle3DMaterials.size(), true));
		for (int i = 0; i < receiversCount; i++) {
			CalGlobalPoint3DCanSeeTriangle3Ds(receivers[i].location, globalRxCanSeeTriangle3Ds[i]);
		}
	}


	void GetDiffuseScatteringTriangles(const TempTriangle3D& rootTriangle, std::list<DiffuseScatteringFace>& tempTriangles) {
		double length1 = GetDistancePoint3DPoint3D(rootTriangle.p2, rootTriangle.p1);
		double length2 = GetDistancePoint3DPoint3D(rootTriangle.p3, rootTriangle.p2);
		double length3 = GetDistancePoint3DPoint3D(rootTriangle.p1, rootTriangle.p3);
		if (length1 <= globalDiffuseScatteringMaxDiscreteSideLength) {
			if (length2 <= globalDiffuseScatteringMaxDiscreteSideLength) {
				if (length3 <= globalDiffuseScatteringMaxDiscreteSideLength) {
					Point3D center;
					center.x = (rootTriangle.p1.x + rootTriangle.p2.x + rootTriangle.p3.x) / 3.0;
					center.y = (rootTriangle.p1.y + rootTriangle.p2.y + rootTriangle.p3.y) / 3.0;
					center.z = (rootTriangle.p1.z + rootTriangle.p2.z + rootTriangle.p3.z) / 3.0;
					DiffuseScatteringFace diffuseScatteringFace;
					double s = 0.5 * (length1 + length2 + length3);
					diffuseScatteringFace.Area = sqrt(s * (s - length1) * (s - length2) * (s - length3));
					diffuseScatteringFace.Center = center;
					tempTriangles.push_back(diffuseScatteringFace);
					return;
				}
			}
		}

		if (length1 >= length2 && length1 >= length3) {
			Point3D p4;
			p4.x = (rootTriangle.p1.x + rootTriangle.p2.x) * 0.5;
			p4.y = (rootTriangle.p1.y + rootTriangle.p2.y) * 0.5;
			p4.z = (rootTriangle.p1.z + rootTriangle.p2.z) * 0.5;
			TempTriangle3D childTriangle1;
			childTriangle1.p1 = rootTriangle.p1;
			childTriangle1.p2 = p4;
			childTriangle1.p3 = rootTriangle.p3;
			GetDiffuseScatteringTriangles(childTriangle1, tempTriangles);
			TempTriangle3D childTriangle2;
			childTriangle2.p1 = p4;
			childTriangle2.p2 = rootTriangle.p2;
			childTriangle2.p3 = rootTriangle.p3;
			GetDiffuseScatteringTriangles(childTriangle2, tempTriangles);
		}
		else if(length2 >= length2 && length2 >= length3){
			Point3D p4;
			p4.x = (rootTriangle.p2.x + rootTriangle.p3.x) * 0.5;
			p4.y = (rootTriangle.p2.y + rootTriangle.p3.y) * 0.5;
			p4.z = (rootTriangle.p2.z + rootTriangle.p3.z) * 0.5;
			TempTriangle3D childTriangle1;
			childTriangle1.p1 = rootTriangle.p1;
			childTriangle1.p2 = rootTriangle.p2;
			childTriangle1.p3 = p4;
			GetDiffuseScatteringTriangles(childTriangle1, tempTriangles);
			TempTriangle3D childTriangle2;
			childTriangle2.p1 = rootTriangle.p1;
			childTriangle2.p2 = p4;
			childTriangle2.p3 = rootTriangle.p3;
			GetDiffuseScatteringTriangles(childTriangle2, tempTriangles);
		}
		else {
			Point3D p4;
			p4.x = (rootTriangle.p3.x + rootTriangle.p1.x) * 0.5;
			p4.y = (rootTriangle.p3.y + rootTriangle.p1.y) * 0.5;
			p4.z = (rootTriangle.p3.z + rootTriangle.p1.z) * 0.5;
			TempTriangle3D childTriangle1;
			childTriangle1.p1 = rootTriangle.p1;
			childTriangle1.p2 = rootTriangle.p2;
			childTriangle1.p3 = p4;
			GetDiffuseScatteringTriangles(childTriangle1, tempTriangles);
			TempTriangle3D childTriangle2;
			childTriangle2.p1 = rootTriangle.p2;
			childTriangle2.p2 = rootTriangle.p3;
			childTriangle2.p3 = p4;
			GetDiffuseScatteringTriangles(childTriangle2, tempTriangles);
		}
		
	}

	void InitAccelerateStructDiffuseScatteringFace() {

		int DiffuseScatteringFacesNumber = 0;
		globalTriangle3DDiffuseScatteringFaces.clear();
		globalTriangle3DDiffuseScatteringFaces.resize(globalTriangle3DMaterials.size());
		for (int i = 0; i < globalTriangle3DDiffuseScatteringFaces.size(); ++i) {
			if (globalTriangle3DMaterialsInvalid[i]) {
				continue;
			}
			std::list<DiffuseScatteringFace> tempTriangles;
			TempTriangle3D rootTriangle;
			rootTriangle.p1 = globalTriangle3DMaterials[i].scenarioTriangleP1;
			rootTriangle.p2 = globalTriangle3DMaterials[i].scenarioTriangleP2;
			rootTriangle.p3 = globalTriangle3DMaterials[i].scenarioTriangleP3;
			//std::cout<<i<<std::endl;
			GetDiffuseScatteringTriangles(rootTriangle, tempTriangles);
			globalTriangle3DDiffuseScatteringFaces[i].clear();
			globalTriangle3DDiffuseScatteringFaces[i].resize(tempTriangles.size());
			int index = 0;
			for (auto& tempTriangle : tempTriangles) {
				globalTriangle3DDiffuseScatteringFaces[i][index] = tempTriangle;
				index++;
			}

			DiffuseScatteringFacesNumber+= (int)globalTriangle3DDiffuseScatteringFaces[i].size();
		}

		std::cout << "DiffuseScatteringFacesNumber=" << DiffuseScatteringFacesNumber << std::endl;
	}

	void InitAccelerateStructMaterial(const MaterialSet& materialSet) {
		globalMaterials.clear();
		globalMaterials.resize(materialSet.size);
		for (int i = 0; i < globalMaterials.size(); ++i) {
			globalMaterials[i] = materialSet.materials[i];
		}
	}

	int GetMaterialIndex(short materialTypeNumber) {

		for (int i = 0; i < globalMaterials.size(); i++) {
			if (globalMaterials[i].materialTypeNumber == materialTypeNumber
				&& globalMaterials[i].frequency == globalFrequency) {
				return i;
			}
		}

		std::cout << "GetMaterialIndex: not found materialTypeNumber=" << materialTypeNumber << std::endl;
		std::cout << "GetMaterialIndex: not found frequency=" << globalFrequency << std::endl;
		return -1;

	}

	void InitAccelerateStructTriangle3DMaterial(const Scenario3D& scenario) {

		globalTriangle3DMaterials.clear();
		globalTriangle3DMaterials.resize(scenario.trianglesCount);
		globalTriangle3DMaterialsInvalid.clear();
		globalTriangle3DMaterialsInvalid.resize(scenario.trianglesCount, false);

		for (int loop = 0; loop < scenario.trianglesCount; ++loop) {
			Point3D p1 = scenario.scenario_point3d_set[scenario.scenario_triangle3d_set[loop].triangleP1Index];
			Point3D p2 = scenario.scenario_point3d_set[scenario.scenario_triangle3d_set[loop].triangleP2Index];
			Point3D p3 = scenario.scenario_point3d_set[scenario.scenario_triangle3d_set[loop].triangleP3Index];
			Point3D n = CrossPoint3DPoint3D(SubPoint3DPoint3D(p2, p1), SubPoint3DPoint3D(p3, p1));
			double length_n = Length_Point3D(n);
			if (IsZeroAbs(length_n)) {
				std::cout << "Triangle3DMaterial的法向量长度为0" << std::endl;
				globalTriangle3DMaterialsInvalid[loop] = true;
				continue;
			}
			n = MulDoublePoint3D(1.0 / length_n, n);
			if (DotPoint3DPoint3D(n, scenario.scenario_triangle3d_set[loop].n) < 0.0) {
				auto temp = p2;
				p2 = p3;
				p3 = temp;
			}

			Point3D E1 = SubPoint3DPoint3D(p1, p2);
			Point3D E2 = SubPoint3DPoint3D(p1, p3);
			Point3D E1E2 = CrossPoint3DPoint3D(E1, E2);
			Point3D E2E1 = CrossPoint3DPoint3D(E2, E1);

			int materialIndex1 = GetMaterialIndex(scenario.scenario_triangle3d_set[loop].topMaterialTypeNumber);
			int materialIndex2 = GetMaterialIndex(scenario.scenario_triangle3d_set[loop].bottomMaterialTypeNumber);
			globalTriangle3DMaterials[loop].scenarioTriangleP1 = p1;
			globalTriangle3DMaterials[loop].scenarioTriangleP2 = p2;
			globalTriangle3DMaterials[loop].scenarioTriangleP3 = p3;
			globalTriangle3DMaterials[loop].scenarioTriangleN = scenario.scenario_triangle3d_set[loop].n;
			globalTriangle3DMaterials[loop].scenarioE1 = E1;
			globalTriangle3DMaterials[loop].scenarioE2 = E2;
			globalTriangle3DMaterials[loop].scenarioE1E2 = E1E2;
			globalTriangle3DMaterials[loop].scenarioE2E1 = E2E1;
			globalTriangle3DMaterials[loop].roughness = (double)scenario.scenario_triangle3d_set[loop].roughness;
			globalTriangle3DMaterials[loop].materialIndex1 = materialIndex1;
			globalTriangle3DMaterials[loop].materialIndex2 = materialIndex2;
		}
	}


	void InitScenarioAndMaterial(
		short transmitterAntennaMaterialTypeNumber,
		long long frequency,
		double diffuseScatteringMaxDiscreteSideLength,
		double diffuseScatteringAr,
		double diffuseScatteringCoefficient,
		double diffuseScatteringRayleighRange,
		const Scenario3D& scenario,
		const MaterialSet& materialSet) {
		globalFrequency = frequency;
		globalDiffuseScatteringRayleighRange = diffuseScatteringRayleighRange;
		globalDiffuseScatteringAr = diffuseScatteringAr;
		globalDiffuseScatteringCoefficient = diffuseScatteringCoefficient;
		globalDiffuseScatteringMaxDiscreteSideLength = diffuseScatteringMaxDiscreteSideLength;
		InitAccelerateStructMaterial(materialSet);

		globalTransmitterAntennaMaterialIndex = GetMaterialIndex(transmitterAntennaMaterialTypeNumber);
		InitAccelerateStructTriangle3DMaterial(scenario);
		InitAccelerateStructDiffuseScatteringFace();
	}

	void InitAntenna(
		const TransmitterAntenna& transmitterAntenna) {

		CalGlobalTxCanSeeTriangle3Ds(transmitterAntenna.location);
		CalGlobalRxCanSeeTriangle3Ds(transmitterAntenna.receiversCount, transmitterAntenna.receivers);
	}
}


namespace Solve1SPathByIm3DStd {

	bool CalRayleighCriterion(long long frequency, double roughness, double thetai, double rayleighRange) {
		double hc = C / frequency / (rayleighRange * cos(thetai));//波长/（8*cos(thetai)）
		if (hc < roughness) {
			return true;
		}
		return false;
	}

	void Solve1SPathByIm3DSISO_1s(
		int triangle_index_1, 
		int diffuse_scattering_index_1,
		const TransmitterAntenna& transmitterAntenna,
		const ReceiverAntenna& receiver,
		std::list<std::vector<ElectricFieldNode>>& result) {

		const Triangle3DMaterial& triangle3DMaterial_1 = globalTriangle3DMaterials[triangle_index_1];
		const DiffuseScatteringFace& diffuseScatteringFace_1 = globalTriangle3DDiffuseScatteringFaces[triangle_index_1][diffuse_scattering_index_1];

		Point3D node_location_1s = diffuseScatteringFace_1.Center;

		const Point3D& a = transmitterAntenna.location;
		const Point3D& b = receiver.location;

		Point3D temp1 = SubPoint3DPoint3D(node_location_1s, a);
		Point3D temp2 = SubPoint3DPoint3D(b, node_location_1s);
		if (DotPoint3DPoint3D(temp1, temp2) < Eps) {
			return;
		}
		const Point3D& n_1 = triangle3DMaterial_1.scenarioTriangleN;

		//if (DotPoint3DPoint3D(temp1, n_1) > -Eps) {
		//	return;
		//}
		//if (DotPoint3DPoint3D(temp2, n_1) < Eps) {
		//	return;
		//}

		Point3D vec_1;
		bool state_node_1 = CalculateCanSeeNode_Plus(
			Eps,
			a,
			node_location_1s,
			vec_1);
		if (!state_node_1) {
			return;
		}

		double thetai_1 = GetThetaI(vec_1, n_1);

		//瑞利散射
		if (!CalRayleighCriterion(globalFrequency, triangle3DMaterial_1.roughness, thetai_1, globalDiffuseScatteringRayleighRange)) {
			return;
		}

		Point3D vec_2;
		bool state_node_2 = CalculateCanSeeNode_Plus(
			Eps,
			node_location_1s,
			b,
			vec_2);
		if (!state_node_2) {
			return;
		}

		double thetar_1 = GetThetaI(vec_2, n_1);
		thetar_1 = abs(thetai_1 - thetar_1);
		int materialIndex1_1 = triangle3DMaterial_1.materialIndex1;
		int materialIndex2_1 = triangle3DMaterial_1.materialIndex2;

		double length1 = GetDistancePoint3DPoint3D(a, node_location_1s);
		double length2 = GetDistancePoint3DPoint3D(node_location_1s, b);

		std::vector<ElectricFieldNode> path(3);
		path[0].attachmentNumber = transmitterAntenna.transmitterAntennaId;
		path[0].location_x = transmitterAntenna.location.x;
		path[0].location_y = transmitterAntenna.location.y;
		path[0].location_z = transmitterAntenna.location.z;
		path[0].type = 0;
		path[0].pre_distance = 0.0;
		path[0].next_distance = length1;
		path[0].materialIndex1 = globalTransmitterAntennaMaterialIndex;
		path[0].materialIndex2 = path[0].materialIndex1;

		path[1].attachmentNumber = triangle_index_1;
		path[1].location_x = node_location_1s.x;
		path[1].location_y = node_location_1s.y;
		path[1].location_z = node_location_1s.z;

		path[1].r_t_s_d_thetai_beta0 = thetai_1;
		path[1].d_phi1_s_thetar = thetar_1;
		path[1].d_phi2_s_ar = globalDiffuseScatteringAr;
		path[1].d_phiE_s_s = globalDiffuseScatteringCoefficient;
		path[1].materialIndex1 = materialIndex1_1;
		path[1].materialIndex2 = materialIndex2_1;
		path[1].n_x = n_1.x;
		path[1].n_y = n_1.y;
		path[1].n_z = n_1.z;

		path[1].type = 5;
		path[1].pre_distance = length1;
		path[1].next_distance = length2;

		path[2].attachmentNumber = receiver.receiverAntennaId;
		path[2].location_x = receiver.location.x;
		path[2].location_y = receiver.location.y;
		path[2].location_z = receiver.location.z;
		path[2].type = 1;
		path[2].next_distance = 0.0;
		path[2].pre_distance = length1 + length2;
		path[2].materialIndex1 = path[0].materialIndex1;
		path[2].materialIndex2 = path[0].materialIndex2;

		result.emplace_back(path);

	}

	void Solve1SPathByIm3DSISO(
		int rx_index,
		const TransmitterAntenna& transmitterAntenna,
		const ReceiverAntenna& receiver,
		std::list<std::vector<ElectricFieldNode>>& result) {

		for (int triangle_index_1 = 0; triangle_index_1 < globalTriangle3DMaterials.size(); ++triangle_index_1) {
			if (globalTriangle3DMaterialsInvalid[triangle_index_1]) {
				continue;
			}
			if (!globalTxCanSeeTriangle3Ds[triangle_index_1]) {
				continue;
			}
			if (!globalRxCanSeeTriangle3Ds[rx_index][triangle_index_1]) {
				continue;
			}

			for (int diffuse_scattering_index_1 = 0; diffuse_scattering_index_1 < globalTriangle3DDiffuseScatteringFaces[triangle_index_1].size(); ++diffuse_scattering_index_1) {
				Solve1SPathByIm3DSISO_1s(triangle_index_1, diffuse_scattering_index_1, transmitterAntenna, receiver, result);
			}
		}
	}

	void Solve1SPathByIm3D(
		double diffuseScatteringMaxDiscreteSideLength,
		double diffuseScatteringAr,
		double diffuseScatteringCoefficient,
		double diffuseScatteringRayleighRange,
		const TransmitterAntenna& transmitterAntenna,
		const Scenario3D& scenario,
		const MaterialSet& materialSet,
		std::vector<std::list<std::vector<ElectricFieldNode>>>& result) {

		std::cout << "进行1次漫散射射计算.开始..." << std::endl;
		//这个模块可以完全独立运行

		//初始化数据
		InitScenarioAndMaterial(transmitterAntenna.materialTypeNumber, transmitterAntenna.frequencyBandwidth.frequencys[0],
			diffuseScatteringMaxDiscreteSideLength,
			diffuseScatteringAr,
			diffuseScatteringCoefficient,
			diffuseScatteringRayleighRange, 
			scenario, materialSet);
		InitAntenna(transmitterAntenna);
		//初始化场景的加速结构,这里只需要三角形的加速结构即可
		InitializeScenario3D(false, 0.001, transmitterAntenna.frequencyBandwidth.frequencys[0], scenario, materialSet);

		result.clear();
		result.resize(transmitterAntenna.receiversCount);

		int printf_jg = transmitterAntenna.receiversCount / 30;
		if (printf_jg < 2) {
			printf_jg = 2;
		}
		for (int i = 0; i < transmitterAntenna.receiversCount; i++) {

			Solve1SPathByIm3DSISO(i, transmitterAntenna, transmitterAntenna.receivers[i], result[i]);

			if (i % printf_jg == 0) {
				std::cout << "已完成" << ((100.00 * i) / transmitterAntenna.receiversCount) << "%计算." << std::endl;
			}
		}

		std::cout << "进行1次漫散射射计算.结束..." << std::endl;

	}

}