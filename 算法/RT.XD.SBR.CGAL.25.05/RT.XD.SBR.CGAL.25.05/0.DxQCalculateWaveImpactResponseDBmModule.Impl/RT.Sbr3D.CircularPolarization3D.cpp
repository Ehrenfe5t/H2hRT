

#include "../0.DxQCalculateWaveImpactResponseDBmModule/CalculateWaveImpactResponseDBm.Output.h"

#include"../CoreCode/HdQAntennaPatternDatabase.h"
#include"../CoreCode/HdQAntennaSIMO.h"
#include"../CoreCode/HdQAntennaMIMO.h"
#include"../CoreCode/HdQAntennaMIMOPath.h"
#include"../CoreCode/HdQCommonParameterConfig.h"
#include"../CoreCode/HdQCalRunTime.h"

#include"../CoreCode/QzQGeometricSpaceAccelerateParameterConfig.h"
#include"../CoreCode/QzQGeometry3DOperate.Point3D.h"
#include"../CoreCode/QzQGeometry3DOperate.Equals.h"
#include"../CoreCode/QzQGeometry3DIntersect.h"
#include"../CoreCode/QzQGlobalConstant.h"



#include"../CoreCode/LxQMultiLinearPolarization3DDatabase.h"
#include"../CoreCode/LxQMaterialObjectDatabase.h"
#include"../CoreCode/LxQMultithreadParameterConfig.h"
#include"../CoreCode/LxQMultiPathNodeInfoOperate.Ptr.h"
#include"../CoreCode/LxQProjectDependencies.h"

#include"../CoreCode/DxQRayEjectionParameterConfig.h"
#include"../CoreCode/DxQRtSbr3DForRay3DPrivateParameterConfig.h"
#include"../CoreCode/DxQRtSbr3DForRay3DParameterConfig.h"
#include"../CoreCode/DxQRtProgramReadsDataAndPreprocessesData.h"
#include"../CoreCode/DxQRayScenarioIntersectType.h"
#include"../CoreCode/DxQRayTracingGeometricPathNodeSIMO.h"
#include"../CoreCode/DxQRayScenarioIntersect.h"
#include"../CoreCode/DxQSbrRayGeneratedByTransmission.h"
#include"../CoreCode/DxQSbrRayGeneratedByReflection.h"
#include"../CoreCode/DxQScenarioObject.h"
#include"../CoreCode/DxQScenarioDataInformation.h"
#include"../CoreCode/DxQSbrRayGeneratedByTransmittingAntenna.h"
#include"../CoreCode/DxQThreadPool.h"
#include"../0.Ray3DIntersectGeometry3DElementsModule/0.Ray3DIntersectGeometry3DElementsModule.Output.h"

#include"../0.SolveOneTimeDiffractionPathByEquationModule/0.SolveOneTimeDiffractionPathByEquationModule.Output.h"
#include"../0.Solve1SPathByIm3DModule/0.Solve1SPathByIm3DModule.Output.h"


#include <unordered_set>
#include <unordered_map>
#include <iostream>


namespace SIMOGlobalStd {


	RtSbr3DForRay3DPrivateParameterConfigStd::RtSbr3DForRay3DPrivateParameterConfig globalRtSbr3DForRay3DPrivateParameterConfig;
	RayEjectionParameterConfigStd::RayEjectionParameterConfig globalRayEjectionParameterConfig;

	void SetRtSbr3DForRay3DPrivateParameterConfig(
		const RtSbr3DForRay3DPrivateParameterConfigStd::RtSbr3DForRay3DPrivateParameterConfig& rtSbr3DForRay3DPrivateParameterConfig,
		const RayEjectionParameterConfigStd::RayEjectionParameterConfig& rayEjectionParameterConfig) {
		globalRtSbr3DForRay3DPrivateParameterConfig = rtSbr3DForRay3DPrivateParameterConfig;
		globalRayEjectionParameterConfig = rayEjectionParameterConfig;
	}

	//每次进行SIMO计算时都应该进行的赋值运算

	TriangleAccelerateStructDatabaseStd::TriangleAccelerateStructDatabase* globalTriangleAccelerateStructDatabase_ptr = NULL;
	TransmittingAntennaStd::TransmittingAntenna* globalTransmittingAntenna_ptr = NULL;
	int* globalReceiverAntennaId = NULL;
	Point3DStd::Point3D* globalReceiverAntennaLocation = NULL;

	size_t globalReceiverAntennas_size = 0;

	Point3DStd::Point3D* globalRayVecs = NULL;
	size_t globalRayVecs_size = 0;
	RayTracingGeometricPathNodeSIMOStd::RayTracingGeometricPathNodeSIMO* globalRayTracingGeometricPathNodeSIMO = NULL;
	ScenarioObjectStd::ScenarioObject* globalScenarioObject_ptr = NULL;



	void FreeSIMOGlobal_ReceiverAntenna() {
		if (globalReceiverAntennas_size == 0) {
			globalReceiverAntennas_size = 0;
			globalReceiverAntennaId = NULL;
			globalReceiverAntennaLocation = NULL;
			return;
		}
		delete globalReceiverAntennaId;
		delete globalReceiverAntennaLocation;
		globalReceiverAntennas_size = 0;
	}


	void FreeSIMOGlobal_RayVec() {
		if (globalRayVecs_size == 0) {
			globalRayVecs_size = 0;
			globalRayVecs = NULL;
			return;
		}
		delete globalRayVecs;
		globalRayVecs_size = 0;
	}

	void FreeSIMOGlobal() {
		FreeSIMOGlobal_ReceiverAntenna();
		FreeSIMOGlobal_RayVec();
	}

	bool SetSIMOGlobal_RayVec(const std::vector<Point3DStd::Point3D>& rayVecs) {

		FreeSIMOGlobal_RayVec();

		globalRayVecs_size = rayVecs.size();

		if (globalRayVecs_size < 1) {
			return false;
		}
		globalRayVecs = (Point3DStd::Point3D*)malloc(globalRayVecs_size * sizeof(Point3DStd::Point3D));
		if (globalRayVecs == NULL) {
			return false;
		}

		for (size_t i = 0; i < globalRayVecs_size; ++i) {
			globalRayVecs[i] = rayVecs[i];
		}

		return true;
	}


	bool SetSIMOGlobal_ReceiverAntenna(const std::vector<ReceiverAntennaStd::ReceiverAntenna>& receiverAntennas) {

		FreeSIMOGlobal_ReceiverAntenna();

		globalReceiverAntennas_size = receiverAntennas.size();

		if (globalReceiverAntennas_size < 1) {
			return false;
		}
		globalReceiverAntennaLocation = (Point3DStd::Point3D*)malloc(globalReceiverAntennas_size * sizeof(Point3DStd::Point3D));
		if (globalReceiverAntennaLocation == NULL) {
			return false;
		}
		globalReceiverAntennaId = (int*)malloc(globalReceiverAntennas_size * sizeof(int));
		if (globalReceiverAntennaId == NULL) {
			return false;
		}

		Point3DStd::Point3D min3D(GlobalConstantStd::BoundingBoxLength, GlobalConstantStd::BoundingBoxLength, GlobalConstantStd::BoundingBoxLength);
		Point3DStd::Point3D max3D(-GlobalConstantStd::BoundingBoxLength, -GlobalConstantStd::BoundingBoxLength, -GlobalConstantStd::BoundingBoxLength);
		std::vector<Point3DStd::Point3D> rx_location;
		for (size_t i = 0; i < globalReceiverAntennas_size; ++i) {
			globalReceiverAntennaId[i] = receiverAntennas[i].receiverAntennaId;
			globalReceiverAntennaLocation[i] = receiverAntennas[i].antennaProperty.location;
			rx_location.emplace_back(globalReceiverAntennaLocation[i]);

			min3D.x = std::min(globalReceiverAntennaLocation[i].x, min3D.x);
			min3D.y = std::min(globalReceiverAntennaLocation[i].y, min3D.y);
			min3D.z = std::min(globalReceiverAntennaLocation[i].z, min3D.z);


			max3D.x = std::max(globalReceiverAntennaLocation[i].x, max3D.x);
			max3D.y = std::max(globalReceiverAntennaLocation[i].y, max3D.y);
			max3D.z = std::max(globalReceiverAntennaLocation[i].z, max3D.z);
		}


		double cube_size = 0.05;
		{
			double max_len1 = max3D.x - min3D.x;
			double max_len2 = max3D.y - min3D.y;
			double max_len3 = max3D.z - min3D.z;

			double max_len = std::max(max_len1, std::max(max_len2, max_len3));

			cube_size = max_len / 99.0;
		}

		if (cube_size < globalRtSbr3DForRay3DPrivateParameterConfig.radiusRx) {
			cube_size = globalRtSbr3DForRay3DPrivateParameterConfig.radiusRx * 1.003;
		}

		return true;
	}

	bool SetSIMOGlobal(
		const std::vector<Point3DStd::Point3D>& rayVecs,
		ScenarioDataInformationStd::ScenarioDataInformation& scenarioDataInformation,
		AntennaSIMOStd::AntennaSIMO& antennaSIMO,
		RayTracingGeometricPathNodeSIMOStd::RayTracingGeometricPathNodeSIMO& rayTracingGeometricPathNodeSIMO) {

		globalTriangleAccelerateStructDatabase_ptr = &scenarioDataInformation.triangleAccelerateStructDatabase;
		globalScenarioObject_ptr = &scenarioDataInformation.scenarioObject;

		globalTransmittingAntenna_ptr = &antennaSIMO.transmittingAntenna;
		globalRayTracingGeometricPathNodeSIMO = &rayTracingGeometricPathNodeSIMO;

		if (!SetSIMOGlobal_ReceiverAntenna(antennaSIMO.receiverAntennas)) {
			return false;
		}

		if (!SetSIMOGlobal_RayVec(rayVecs)) {
			return false;
		}

		return true;

	}

}


namespace InteractionBetweenRayAndRxStd {


	void IsArriveReceiverAntennasByConeMethod_ShadowPoint_1(
		double rayScenarioIntersectResultDistance,
		const Ray3DStd::Ray3D& ray,
		std::vector<int>& indexRxList) {
		
		//std::cout << "[DEBUG] globalReceiverAntennas_size = " << SIMOGlobalStd::globalReceiverAntennas_size << std::endl;

		for (int i = 0; i < SIMOGlobalStd::globalReceiverAntennas_size; i++) {

			if (Geometry3DIntersectStd::ToDetermineWhetherTheReceiverCollidesWithTheRayByUsingTheCylindricalMethod(
				rayScenarioIntersectResultDistance,
				SIMOGlobalStd::globalRtSbr3DForRay3DPrivateParameterConfig.radiusRx,
				ray.o,
				ray.vec,
				SIMOGlobalStd::globalReceiverAntennaLocation[i])) {
				indexRxList.emplace_back(i);
			}
		}
	}


	/// <summary>
	/// 所有天线和当前这跟射线的交互结果
	/// </summary>
	/// <param name="rayScenarioIntersectResultDistance"></param>
	/// <param name="ray"></param>
	/// <param name="indexRxList"></param>
	void IsArriveReceiverAntennasByConeMethod_ShadowPoint(
		double rayScenarioIntersectResultDistance,
		const Ray3DStd::Ray3D& ray,
		std::vector<int>& indexRxList) {

		
		IsArriveReceiverAntennasByConeMethod_ShadowPoint_1(rayScenarioIntersectResultDistance, ray, indexRxList);

	}

	void InteractionBetweenRayAndRx_base(
		double rayScenarioIntersectResultDistance,
		const Ray3DStd::Ray3D& ray,
		const std::vector<RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode>& root) {

		if (!SIMOGlobalStd::globalRayEjectionParameterConfig.switchOfLos) {
			if (root.size() < 2) {
				return;
			}
		}

		//碰撞的接收天线索引
		std::vector<int> indexRxList;
		//std::vector<Point3DStd::Point3D> indexRxList_location;
		//3.计算射线是否被接收天线接收
		IsArriveReceiverAntennasByConeMethod_ShadowPoint( rayScenarioIntersectResultDistance, ray, indexRxList);

		for (int i = 0; i < indexRxList.size(); i++) {
			int rx_loop_index = indexRxList[i];
			Point3DStd::Point3D receiverAntenna_location = SIMOGlobalStd::globalReceiverAntennaLocation[rx_loop_index];
			int receiverAntenna_id = SIMOGlobalStd::globalReceiverAntennaId[rx_loop_index];

			RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode inforx(
				PropagationTypeStd::PropagationType::ReceiverAntenna, receiverAntenna_id, receiverAntenna_location);// (rx);

			//auto pre_location = root[(int)root.size() - 1].location;

			//double distance = Geometry3DOperateStd::GetDistancePoint3DPoint3D(receiverAntenna_location, pre_location);
			//Point3DStd::Point3D rx_ray_vec_old = Geometry3DOperateStd::SubPoint3DPoint3D(pre_location, receiverAntenna_location);
			//Point3DStd::Point3D rx_ray_vec;
			//if (!Geometry3DOperateStd::Normalization_Point3D_safe(rx_ray_vec_old, rx_ray_vec)) {
			//	continue;
			//}
			//
			//Ray3DStd::Ray3D rx_ray(receiverAntenna_location, rx_ray_vec);
			//Ray3DTriangle3DIntersectResultStd::Ray3DTriangle3DIntersectResult rx_ray3DTriangle3DIntersectResult = RayScenarioIntersectStd::RayScenarioTriangle3DIntersect(
			//	rx_ray, scenarioDataInformation.triangleAccelerateStructDatabase);
			//if (distance > GlobalConstantStd::Eps + rx_ray3DTriangle3DIntersectResult.distance) {
			//	continue;
			//}

			std::vector<RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode> path((int)root.size() + 1);
			for (size_t j = 0; j < root.size(); j++) {
				path[j] = root[j];
			}
			path[root.size()] = inforx;

			SIMOGlobalStd::globalRayTracingGeometricPathNodeSIMO->AddPath(rx_loop_index, path);
		}
	}


	//InteractionBetweenRayAndRxObject
	class InteractionBetweenRayAndRxObject
	{
	public:
		double rayScenarioIntersectResultDistance;
		Ray3DStd::Ray3D ray;
		std::vector<RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode> root;
		InteractionBetweenRayAndRxObject();
		~InteractionBetweenRayAndRxObject();

	private:

	};

	InteractionBetweenRayAndRxObject::InteractionBetweenRayAndRxObject()
	{
		this->rayScenarioIntersectResultDistance = GlobalConstantStd::BoundingBoxLength;
	}

	InteractionBetweenRayAndRxObject::~InteractionBetweenRayAndRxObject()
	{
	}


	std::vector<Ray3DStd::Ray3D> interactionBetweenRayAndRx_ray;
	std::vector<double> interactionBetweenRayAndRx_rayScenarioIntersectResultDistance;
	std::vector<std::vector<RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode>> interactionBetweenRayAndRx_nodes;


	std::mutex interactionBetweenRayAndRx_mtx;
	void InteractionBetweenRayAndRx(
		double rayScenarioIntersectResultDistance,
		const Ray3DStd::Ray3D& ray,
		const std::vector<RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode>& root) {

		//这里认为天线的路径如果没有被三角形遮挡，那么也不会被棱边遮挡

		{
			////这种思路消耗内存
			//interactionBetweenRayAndRx_mtx.lock();
			//
			//interactionBetweenRayAndRx_ray.emplace_back(ray);
			//interactionBetweenRayAndRx_rayScenarioIntersectResultDistance.emplace_back(rayScenarioIntersectResultDistance);
			//std::vector<RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode> nodes;
			//nodes.insert(nodes.end(), root.begin(), root.end());
			//interactionBetweenRayAndRx_nodes.emplace_back(nodes);
			//
			//interactionBetweenRayAndRx_mtx.unlock();
		}

		{
			//InteractionBetweenRayAndRxObject interactionBetweenRayAndRxObject;
			//interactionBetweenRayAndRxObject.ray = ray;
			//interactionBetweenRayAndRxObject.rayScenarioIntersectResultDistance = rayScenarioIntersectResultDistance;
			//interactionBetweenRayAndRxObject.root.insert(interactionBetweenRayAndRxObject.root.end(), root.begin(), root.end());
			//interactionBetweenRayAndRx_threadPool.submit(InteractionBetweenRayAndRx_Core, interactionBetweenRayAndRxObject);
		}


		//return;
		InteractionBetweenRayAndRx_base( rayScenarioIntersectResultDistance, ray, root);
	}
}

namespace RtSbr3DForRay3DFindPathSingleThreadStd {

	/// <summary>
	/// 计算入射角
	/// </summary>
	/// <returns></returns>
	double GetThetaI(const Point3DStd::Point3D& v1, const Point3DStd::Point3D& n1) {
		if (Geometry3DOperateStd::DotPoint3DPoint3D(n1, v1) > 0) {
			return Geometry3DOperateStd::GetAnglePoint3DPoint3D(v1, n1);
		}
		else {
			Point3DStd::Point3D v2(-v1.x, -v1.y, -v1.z);
			return Geometry3DOperateStd::GetAnglePoint3DPoint3D(v2, n1);
		}
	}


	void SbrFindGeometricPathSIMO(
		double rayPropagationDistance,
		const RayEjectionParameterConfigStd::RayEjectionParameterConfig& rayEjectionParameterConfig,
		const Ray3DStd::Ray3D& ray,
		std::vector<RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode>& root) {
		if (rayEjectionParameterConfig.ejectionsMaxTotalNumber < 1) {
			return;
		}
		bool transmission = true;
		if (rayEjectionParameterConfig.ejectionsOfTransmissionMaxNumber < 1) {
			transmission = false;
		}
		bool reflect = true;
		if (rayEjectionParameterConfig.ejectionsOfReflectionMaxNumber < 1) {
			reflect = false;
		}


		//计算射线的碰撞结果


		Point3D node_location;
		node_location.x = ray.o.x;
		node_location.y = ray.o.y;
		node_location.z = ray.o.z;

		Point3D current_ray_unit_vec;
		current_ray_unit_vec.x = ray.vec.x;
		current_ray_unit_vec.y = ray.vec.y;
		current_ray_unit_vec.z = ray.vec.z;

		Ray3DIntersectGeometry3DElementResult triangleIntersectResult;
		triangleIntersectResult.elementIndex = -1;
		Ray3DIntersectGeometry3DStd::CalculateRay3DIntersectTriangle3DFirst(
			node_location, current_ray_unit_vec, triangleIntersectResult);

		Ray3DIntersectGeometry3DElementResult cornerIntersectResult;
		cornerIntersectResult.elementIndex = -1;


		InteractionBetweenRayAndRxStd::InteractionBetweenRayAndRx(triangleIntersectResult.distance, ray, root);

		if (triangleIntersectResult.elementIndex == -1
			&& cornerIntersectResult.elementIndex == -1) {
			return;
		}

		if (triangleIntersectResult.elementIndex != -1) {


			Point3DStd::Point3D current_intersect_point3d;
			current_intersect_point3d.x = triangleIntersectResult.location.x;
			current_intersect_point3d.y = triangleIntersectResult.location.y;
			current_intersect_point3d.z = triangleIntersectResult.location.z;
			Point3DStd::Point3D curScenarioTriangleN = SIMOGlobalStd::globalTriangleAccelerateStructDatabase_ptr->triangleAccelerateStructs[triangleIntersectResult.elementIndex].scenarioTriangleN;



			if (reflect) {

				Ray3DStd::Ray3D newRay(Point3DStd::Point3D(0, 0, 0), Point3DStd::Point3D(0, 0, 0));
				double thetai;
				//根据当前的碰撞点计算反射信息
				if (SbrRayGeneratedByReflectionStd::BuildReflectionRay(
					ray.o,
					current_intersect_point3d,
					curScenarioTriangleN,
					newRay,
					thetai)) {
					RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode infoReflection(
						PropagationTypeStd::PropagationType::Reflection,
						triangleIntersectResult.elementIndex,
						current_intersect_point3d);

					root.emplace_back(infoReflection);

					RayEjectionParameterConfigStd::RayEjectionParameterConfig newRayEjectionParameterConfig;

					newRayEjectionParameterConfig.ejectionsMaxTotalNumber = rayEjectionParameterConfig.ejectionsMaxTotalNumber - 1;
					newRayEjectionParameterConfig.ejectionsOfDiffractionMaxNumber = rayEjectionParameterConfig.ejectionsOfDiffractionMaxNumber;
					newRayEjectionParameterConfig.ejectionsOfDiffuseScatteringMaxNumber = rayEjectionParameterConfig.ejectionsOfDiffuseScatteringMaxNumber;
					newRayEjectionParameterConfig.ejectionsOfReflectionMaxNumber = rayEjectionParameterConfig.ejectionsOfReflectionMaxNumber - 1;
					newRayEjectionParameterConfig.ejectionsOfTransmissionMaxNumber = rayEjectionParameterConfig.ejectionsOfTransmissionMaxNumber;
					newRayEjectionParameterConfig.switchOfLos = rayEjectionParameterConfig.switchOfLos;

					double newRayPropagationDistance = rayPropagationDistance + triangleIntersectResult.distance;


					{
						//增加针对反射条件下进行有限的绕射和漫散射
						if (newRayEjectionParameterConfig.ejectionsOfDiffractionMaxNumber > 2) {
							newRayEjectionParameterConfig.ejectionsOfDiffractionMaxNumber = 2;
						}
						if (newRayEjectionParameterConfig.ejectionsOfDiffuseScatteringMaxNumber > 2) {
							newRayEjectionParameterConfig.ejectionsOfDiffuseScatteringMaxNumber = 2;
						}
					}

					SbrFindGeometricPathSIMO(
						newRayPropagationDistance,
						newRayEjectionParameterConfig,
						newRay,
						root);
					root.pop_back();
				}
			}
			if (transmission) {

				auto triangleAccelerateStruct = SIMOGlobalStd::globalTriangleAccelerateStructDatabase_ptr->triangleAccelerateStructs[triangleIntersectResult.elementIndex];
				auto curScenarioTriangleN = triangleAccelerateStruct.scenarioTriangleN;

				int faceUpTypeNumber = triangleAccelerateStruct.upTypeNumber;
				int faceDownTypeNumber = triangleAccelerateStruct.downTypeNumber;

				Ray3DStd::Ray3D newRay(Point3DStd::Point3D(0, 0, 0), Point3DStd::Point3D(0, 0, 0));
				double thetai;
				//根据当前的碰撞点计算透射信息
				if (SbrRayGeneratedByTransmissionStd::BuildTransmissionRay_RayTracingGeometricPathNode(
					faceUpTypeNumber,
					faceDownTypeNumber,
					SIMOGlobalStd::globalTransmittingAntenna_ptr->antennaProperty.frequencys[0],
					ray.o,
					current_intersect_point3d,
					curScenarioTriangleN,
					newRay,
					thetai)) {

					RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode infoTransmission(
						PropagationTypeStd::PropagationType::Transmission,
						triangleIntersectResult.elementIndex,
						current_intersect_point3d);
					root.emplace_back(infoTransmission);

					RayEjectionParameterConfigStd::RayEjectionParameterConfig newRayEjectionParameterConfig;

					newRayEjectionParameterConfig.ejectionsMaxTotalNumber = rayEjectionParameterConfig.ejectionsMaxTotalNumber - 1;
					newRayEjectionParameterConfig.ejectionsOfDiffractionMaxNumber = rayEjectionParameterConfig.ejectionsOfDiffractionMaxNumber;
					newRayEjectionParameterConfig.ejectionsOfDiffuseScatteringMaxNumber = rayEjectionParameterConfig.ejectionsOfDiffuseScatteringMaxNumber;
					newRayEjectionParameterConfig.ejectionsOfReflectionMaxNumber = rayEjectionParameterConfig.ejectionsOfReflectionMaxNumber;
					newRayEjectionParameterConfig.ejectionsOfTransmissionMaxNumber = rayEjectionParameterConfig.ejectionsOfTransmissionMaxNumber - 1;
					newRayEjectionParameterConfig.switchOfLos = rayEjectionParameterConfig.switchOfLos;


					double newRayPropagationDistance = rayPropagationDistance + triangleIntersectResult.distance;
					SbrFindGeometricPathSIMO(
						newRayPropagationDistance,
						newRayEjectionParameterConfig,
						newRay,
						root);
					root.pop_back();
				}

			}
		}


	}




	void RtSbr3DForRay3DFindPathSingleThread_SIMO() {


		size_t index_gap_10 = (SIMOGlobalStd::globalRayVecs_size / 50);

		for (size_t loop_vec_index = 0; loop_vec_index < SIMOGlobalStd::globalRayVecs_size; ++loop_vec_index) {

			Ray3DStd::Ray3D ray(SIMOGlobalStd::globalTransmittingAntenna_ptr->antennaProperty.location, SIMOGlobalStd::globalRayVecs[loop_vec_index]);

			std::vector<RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode> root;
			RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode transmittingAntennaNode(
				PropagationTypeStd::PropagationType::TransmittingAntenna,
				SIMOGlobalStd::globalTransmittingAntenna_ptr->transmittingAntennaId,
				SIMOGlobalStd::globalTransmittingAntenna_ptr->antennaProperty.location);

			RayEjectionParameterConfigStd::RayEjectionParameterConfig newRayEjectionParameterConfig;
			newRayEjectionParameterConfig.ejectionsMaxTotalNumber = SIMOGlobalStd::globalRayEjectionParameterConfig.ejectionsMaxTotalNumber;
			newRayEjectionParameterConfig.ejectionsOfDiffractionMaxNumber = SIMOGlobalStd::globalRayEjectionParameterConfig.ejectionsOfDiffractionMaxNumber;
			newRayEjectionParameterConfig.ejectionsOfDiffuseScatteringMaxNumber = SIMOGlobalStd::globalRayEjectionParameterConfig.ejectionsOfDiffuseScatteringMaxNumber;
			newRayEjectionParameterConfig.ejectionsOfReflectionMaxNumber = SIMOGlobalStd::globalRayEjectionParameterConfig.ejectionsOfReflectionMaxNumber;
			newRayEjectionParameterConfig.ejectionsOfTransmissionMaxNumber = SIMOGlobalStd::globalRayEjectionParameterConfig.ejectionsOfTransmissionMaxNumber;
			newRayEjectionParameterConfig.switchOfLos = SIMOGlobalStd::globalRayEjectionParameterConfig.switchOfLos;

			root.emplace_back(transmittingAntennaNode);
			SbrFindGeometricPathSIMO(
				0.0,
				newRayEjectionParameterConfig,
				ray,
				root);
			root.pop_back();
			if (loop_vec_index % index_gap_10 == index_gap_10 - 1) {
				{
					std::ostringstream oss;
					oss << loop_vec_index << "/" << SIMOGlobalStd::globalRayVecs_size;
					ProjectDependenciesStd::DisplayPromptOrReason(oss.str(), false, __FILE__, __LINE__);
				}
			}
		}

	}

}

namespace RtSbr3DForRay3DFindPathMultiThreadStd {


	typedef struct DefMultiThread_RtSbr3DForRay3DFindPathMultiThread_SIMO {
		int start;
		int end;

	}MultiThread_RtSbr3DForRay3DFindPathMultiThread_SIMO;

	void RtSbr3DForRay3DFindPathMultiThread_SIMO_Core(MultiThread_RtSbr3DForRay3DFindPathMultiThread_SIMO& obj) {

		auto rayEjectionParameterConfig = SIMOGlobalStd::globalRayEjectionParameterConfig;

		for (size_t loop_vec_index = obj.start; loop_vec_index < SIMOGlobalStd::globalRayVecs_size && loop_vec_index < obj.end; ++loop_vec_index) {
			Ray3DStd::Ray3D ray(SIMOGlobalStd::globalTransmittingAntenna_ptr->antennaProperty.location, SIMOGlobalStd::globalRayVecs[loop_vec_index]);
			std::vector<RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode> root;
			RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode transmittingAntennaNode(
				PropagationTypeStd::PropagationType::TransmittingAntenna,
				SIMOGlobalStd::globalTransmittingAntenna_ptr->transmittingAntennaId,
				SIMOGlobalStd::globalTransmittingAntenna_ptr->antennaProperty.location);

			RayEjectionParameterConfigStd::RayEjectionParameterConfig newRayEjectionParameterConfig;
			newRayEjectionParameterConfig.ejectionsMaxTotalNumber = rayEjectionParameterConfig.ejectionsMaxTotalNumber;
			newRayEjectionParameterConfig.ejectionsOfDiffractionMaxNumber = rayEjectionParameterConfig.ejectionsOfDiffractionMaxNumber;
			newRayEjectionParameterConfig.ejectionsOfDiffuseScatteringMaxNumber = rayEjectionParameterConfig.ejectionsOfDiffuseScatteringMaxNumber;
			newRayEjectionParameterConfig.ejectionsOfReflectionMaxNumber = rayEjectionParameterConfig.ejectionsOfReflectionMaxNumber;
			newRayEjectionParameterConfig.ejectionsOfTransmissionMaxNumber = rayEjectionParameterConfig.ejectionsOfTransmissionMaxNumber;
			newRayEjectionParameterConfig.switchOfLos = rayEjectionParameterConfig.switchOfLos;

			root.emplace_back(transmittingAntennaNode);
			RtSbr3DForRay3DFindPathSingleThreadStd::SbrFindGeometricPathSIMO(
				0.0,
				newRayEjectionParameterConfig,
				ray,
				root);
			root.pop_back();
		}

	}


	void RtSbr3DForRay3DFindPathMultiThread_SIMO(
		const MultithreadParameterConfigStd::MultithreadParameterConfig& multithreadParameterConfig) {

		int indexGap = multithreadParameterConfig.multithreadConfigThreadOneCpuCalNum;

		std::vector<int> starts;
		for (int i = 0; i < SIMOGlobalStd::globalRayVecs_size; i = i + indexGap) {
			//if (i<809 ||i>810) {
			//	continue;
			//}
			starts.emplace_back(i);
		}


		ThreadPoolStd::ThreadPool threadPool;


		for (int i = 0; i < starts.size(); ++i) {
			MultiThread_RtSbr3DForRay3DFindPathMultiThread_SIMO multiThread_RtSbr3DForRay3DFindPathMultiThread_SIMO;
			multiThread_RtSbr3DForRay3DFindPathMultiThread_SIMO.start = starts[i];
			multiThread_RtSbr3DForRay3DFindPathMultiThread_SIMO.end = starts[i] + indexGap;

			threadPool.submit(RtSbr3DForRay3DFindPathMultiThread_SIMO_Core, multiThread_RtSbr3DForRay3DFindPathMultiThread_SIMO);

		}

		threadPool.join();

	}
}

namespace RtSbr3DForRay3DFindPathStd {

	PropagationTypeStd::PropagationType GetPropagationType(int type) {
		switch (type) {
		case 0:
			return PropagationTypeStd::PropagationType::TransmittingAntenna;
		case 1:
			return PropagationTypeStd::PropagationType::ReceiverAntenna;
		case 2:
			return PropagationTypeStd::PropagationType::Reflection;
		case 3:
			return PropagationTypeStd::PropagationType::Transmission;
		case 4:
			return PropagationTypeStd::PropagationType::Diffraction;
		case 5:
			return PropagationTypeStd::PropagationType::DiffuseScattering;
		default:
			return PropagationTypeStd::PropagationType::Null;
		}
	}

	void Solve1DPathByIm3D(
		double radiusCorner,
		const TransmitterAntenna& transmitterAntenna,
		const Scenario3D& scenario,
		const MaterialSet& materialSet) {
		std::vector<std::list<std::vector<ElectricFieldNode>>> result;
		//进行一次绕射的完备计算
		SolveOneTimeDiffractionPathByEquationStd::SolveOneTimeDiffractionPathByEquation(
			radiusCorner,
			transmitterAntenna,
			scenario,
			materialSet,
			result);

		for (int rx_loop_index = 0; rx_loop_index < result.size(); ++rx_loop_index) {

			for (auto& path : result[rx_loop_index]) {

				std::vector<RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode> path_useful(path.size());
				for (size_t j = 0; j < path_useful.size(); j++) {
					path_useful[j].type = GetPropagationType(path[j].type);
					path_useful[j].nodeElementId = path[j].attachmentNumber;
					path_useful[j].location.x = path[j].location_x;
					path_useful[j].location.y = path[j].location_y;
					path_useful[j].location.z = path[j].location_z;
				}

				SIMOGlobalStd::globalRayTracingGeometricPathNodeSIMO->AddPath(rx_loop_index, path_useful);
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
		const MaterialSet& materialSet) {

		std::vector<std::list<std::vector<ElectricFieldNode>>> result;
		Solve1SPathByIm3DStd::Solve1SPathByIm3D(diffuseScatteringMaxDiscreteSideLength,
			diffuseScatteringAr,
			diffuseScatteringCoefficient,
			diffuseScatteringRayleighRange,
			transmitterAntenna,
			scenario,
			materialSet,
			result);

		for (int rx_loop_index = 0; rx_loop_index < result.size(); ++rx_loop_index) {

			for (auto& path : result[rx_loop_index]) {

				std::vector<RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode> path_useful(path.size());
				for (size_t j = 0; j < path_useful.size(); j++) {
					path_useful[j].type = GetPropagationType(path[j].type);
					path_useful[j].nodeElementId = path[j].attachmentNumber;
					path_useful[j].location.x = path[j].location_x;
					path_useful[j].location.y = path[j].location_y;
					path_useful[j].location.z = path[j].location_z;
				}

				SIMOGlobalStd::globalRayTracingGeometricPathNodeSIMO->AddPath(rx_loop_index, path_useful);
			}

		}
	}

	/// <summary>
	/// 
	/// </summary>
	/// <param name="deduplicateRadius"></param>
	/// <param name="multithreadParameterConfig"></param>
	/// <param name="rayEjectionParameterConfig"></param>
	/// <param name="rayVecs"></param>
	/// <param name="scenarioDataInformation"></param>
	/// <param name="antennaSIMO"></param>
	/// <param name="antennaSIMOPath"></param>
	void RtSbr3DForRay3DFindPath_SIMO(
		double deduplicateRadius,
		const MultithreadParameterConfigStd::MultithreadParameterConfig& multithreadParameterConfig,
		const TransmitterAntenna& transmitterAntenna,
		const Scenario3D& scenario,
		const MaterialSet& materialSet,
		ScenarioDataInformationStd::ScenarioDataInformation& scenarioDataInformation,
		AntennaSIMOStd::AntennaSIMO& antennaSIMO,
		AntennaSIMOPathStd::AntennaSIMOPath& antennaSIMOPath) {


		size_t transmitterRayNumber = SIMOGlobalStd::globalRtSbr3DForRay3DPrivateParameterConfig.rayNumber;

		std::vector<Point3DStd::Point3D> rayVecs; {
			{
				std::ostringstream oss;
				oss << "【射线束初始化完毕】";
				ProjectDependenciesStd::DisplayPromptOrReason(oss.str(), false, __FILE__, __LINE__);
			}
			CalRunTimeStd::CalRunTime calRunTime(true);
			SbrRayGeneratedByTransmittingAntennaStd::InitSBRLaunchRayVec(transmitterRayNumber, rayVecs);
		}

		//所有单核或者多核的射线跟踪计算都在这里进行

		//准备数据

		RayTracingGeometricPathNodeSIMOStd::RayTracingGeometricPathNodeSIMO rayTracingGeometricPathNodeSIMO(antennaSIMO.transmittingAntenna, antennaSIMO.receiverAntennas);

		if (!SIMOGlobalStd::SetSIMOGlobal(rayVecs, scenarioDataInformation, antennaSIMO, rayTracingGeometricPathNodeSIMO)) {
			return;
		}

		bool calculate_1D_path = false;
		bool calculate_1S_path = false;
		if (SIMOGlobalStd::globalRayEjectionParameterConfig.ejectionsOfDiffractionMaxNumber > 0) {
			calculate_1D_path = true;
			SIMOGlobalStd::globalRayEjectionParameterConfig.ejectionsOfDiffractionMaxNumber = 0;
		}
		if (SIMOGlobalStd::globalRayEjectionParameterConfig.ejectionsOfDiffuseScatteringMaxNumber > 0) {
			calculate_1S_path = true;
			SIMOGlobalStd::globalRayEjectionParameterConfig.ejectionsOfDiffuseScatteringMaxNumber = 0;
		}

		if (multithreadParameterConfig.multithreadConfigSwitchOfMultithread) {
			RtSbr3DForRay3DFindPathMultiThreadStd::RtSbr3DForRay3DFindPathMultiThread_SIMO(multithreadParameterConfig);
		}
		else {
			RtSbr3DForRay3DFindPathSingleThreadStd::RtSbr3DForRay3DFindPathSingleThread_SIMO();
		}

		{
			//CalRunTimeStd::CalRunTime CalRunTime(true);
			//std::cout << "与接收机交互" << std::endl;

			//InteractionBetweenRayAndRxStd::interactionBetweenRayAndRx_threadPool.join();
		}
		//std::cout << rayTracingGeometricPathNodeSIMO.rayTracingGeometricPathNodeSISOs[0].paths.size() << std::endl;

		
		if (calculate_1D_path) {
			double radiusCorner = SIMOGlobalStd::globalRtSbr3DForRay3DPrivateParameterConfig.radiusCorner;
			Solve1DPathByIm3D(radiusCorner, transmitterAntenna, scenario, materialSet);
		}
		if (calculate_1S_path) {
			double diffuseScatteringMaxDiscreteSideLength = 0.5;
			double diffuseScatteringAr = SIMOGlobalStd::globalRtSbr3DForRay3DPrivateParameterConfig.diffuseScatteringParameter.diffuseScatteringAr;
			double diffuseScatteringCoefficient = SIMOGlobalStd::globalRtSbr3DForRay3DPrivateParameterConfig.diffuseScatteringParameter.diffuseScatteringCoefficient;
			double diffuseScatteringRayleighRange = SIMOGlobalStd::globalRtSbr3DForRay3DPrivateParameterConfig.diffuseScatteringParameter.diffuseScatteringRayleighRange;
			Solve1SPathByIm3D(diffuseScatteringMaxDiscreteSideLength, diffuseScatteringAr, diffuseScatteringCoefficient, diffuseScatteringRayleighRange, transmitterAntenna, scenario, materialSet);
		}
		

		{

			if (deduplicateRadius > 0.001) {
				CalRunTimeStd::CalRunTime CalRunTime(true);
				std::cout << "删除重复路径." << std::endl;
				//在这里开启筛选会导致临界花纹
				RayTracingGeometricPathNodeSIMOStd::DeleteSamePath_SIMO(
					multithreadParameterConfig.multithreadConfigSwitchOfMultithread, deduplicateRadius, rayTracingGeometricPathNodeSIMO);
			}
			CalRunTimeStd::CalRunTime CalRunTime(true);
			std::cout << "多径转化.计算节点信息." << std::endl;
			RayTracingGeometricPathNodeSIMOStd::RayTracingGeometricPathNodeToMultiPathNodeInfo_SIMO(
				multithreadParameterConfig.multithreadConfigSwitchOfMultithread, scenarioDataInformation, rayTracingGeometricPathNodeSIMO, antennaSIMOPath);

		}

		//std::cout << antennaSIMOPath.paths[0].paths.size() << std::endl;

		SIMOGlobalStd::FreeSIMOGlobal();

	}

	TransmitterAntenna GetTransmitterAntenna(short materialTypeNumber,const AntennaSIMOStd::AntennaSIMO& antennaSIMO) {

		TransmitterAntenna transmitterAntenna;
		transmitterAntenna.frequencyBandwidth.size = (int)antennaSIMO.transmittingAntenna.antennaProperty.frequencys.size();
		transmitterAntenna.frequencyBandwidth.frequencys = (long long*)malloc(transmitterAntenna.frequencyBandwidth.size * sizeof(long long));
		for (int i = 0; i < transmitterAntenna.frequencyBandwidth.size; i++) {
			transmitterAntenna.frequencyBandwidth.frequencys[i] = antennaSIMO.transmittingAntenna.antennaProperty.frequencys[i];
		}
		transmitterAntenna.location.x = antennaSIMO.transmittingAntenna.antennaProperty.location.x;
		transmitterAntenna.location.y = antennaSIMO.transmittingAntenna.antennaProperty.location.y;
		transmitterAntenna.location.z = antennaSIMO.transmittingAntenna.antennaProperty.location.z;
		transmitterAntenna.materialTypeNumber = materialTypeNumber;
		transmitterAntenna.transmitterAntennaId = antennaSIMO.transmittingAntenna.transmittingAntennaId;

		transmitterAntenna.receiversCount = (int)antennaSIMO.receiverAntennas.size();
		transmitterAntenna.receivers = (ReceiverAntenna*)malloc(transmitterAntenna.receiversCount * sizeof(ReceiverAntenna));
		for (int i = 0; i < transmitterAntenna.receiversCount; i++) {
			transmitterAntenna.receivers[i].receiverAntennaId = antennaSIMO.receiverAntennas[i].receiverAntennaId;
			transmitterAntenna.receivers[i].location.x = antennaSIMO.receiverAntennas[i].antennaProperty.location.x;
			transmitterAntenna.receivers[i].location.y = antennaSIMO.receiverAntennas[i].antennaProperty.location.y;
			transmitterAntenna.receivers[i].location.z = antennaSIMO.receiverAntennas[i].antennaProperty.location.z;
		}
		return transmitterAntenna;
	}

	void RtSbr3DForRay3DFindPath_MIMO(
		short materialTypeNumber,
		double deduplicateRadius,
		const MultithreadParameterConfigStd::MultithreadParameterConfig& multithreadParameterConfig,
		const Scenario3D& scenario,
		const MaterialSet& materialSet,
		ScenarioDataInformationStd::ScenarioDataInformation& scenarioDataInformation,
		AntennaMIMOStd::AntennaMIMO& antennaMIMO,
		AntennaMIMOPathStd::AntennaMIMOPath& antennaMIMOPath) {

		for (int loop_transmittingAntenna_index = 0; loop_transmittingAntenna_index < antennaMIMO.antennaSIMOs.size(); ++loop_transmittingAntenna_index) {
			AntennaSIMOStd::AntennaSIMO antennaSIMO = antennaMIMO.antennaSIMOs[loop_transmittingAntenna_index];

			TransmitterAntenna transmitterAntenna = GetTransmitterAntenna(materialTypeNumber,antennaSIMO);
			RtSbr3DForRay3DFindPath_SIMO(
				deduplicateRadius,
				multithreadParameterConfig,
				transmitterAntenna,
				scenario,
				materialSet,
				scenarioDataInformation,
				antennaSIMO,
				antennaMIMOPath.paths[loop_transmittingAntenna_index]);
		}
	}

	void RtSbr3DForRay3DFindPath(
		short materialTypeNumber,
		double deduplicateRadius,
		const MultithreadParameterConfigStd::MultithreadParameterConfig& multithreadParameterConfig,
		const RayEjectionParameterConfigStd::RayEjectionParameterConfig& rayEjectionParameterConfig,
		const RtSbr3DForRay3DPrivateParameterConfigStd::RtSbr3DForRay3DPrivateParameterConfig& rtSbr3DForRay3DPrivateParameterConfig,
		const Scenario3D& scenario,
		const MaterialSet& materialSet,
		ScenarioDataInformationStd::ScenarioDataInformation& scenarioDataInformation,
		AntennaMIMOStd::AntennaMIMO& antennaMIMO,
		AntennaMIMOPathStd::AntennaMIMOPath& antennaMIMOPath) {

		SIMOGlobalStd::SetRtSbr3DForRay3DPrivateParameterConfig(rtSbr3DForRay3DPrivateParameterConfig, rayEjectionParameterConfig);

		RtSbr3DForRay3DFindPath_MIMO(
			materialTypeNumber,
			deduplicateRadius,
			multithreadParameterConfig,
			scenario,
			materialSet,
			scenarioDataInformation,
			antennaMIMO,
			antennaMIMOPath);

	}
}


namespace RTSbr3DCircularPolarization3DInitParameterStd {

	bool InitAntennaPolarization3DModel(const AntennaPolarization3DModel& antennaPolarization3DModel) {
		
		if (antennaPolarization3DModel.polarization3DModelId == 0) {
			//全极化
		}
		else if (antennaPolarization3DModel.polarization3DModelId == 1) {
			//线极化0,0,1
			MultiLinearPolarization3DDatabaseStd::InitLinearPolarization3DZ();
		}
		else {

			//MultiLinearPolarization3DObjectStd::MultiLinearPolarization3DObject multiLinearPolarization3DObject;
			//multiLinearPolarization3DObject.polarization3DModelId = antennaPolarization3DModel.polarization3DModelId;
			//
			//for (int i = 0; i < antennaPolarization3DModel.size; i++) {
			//
			//	auto oneAntennaLinearPolarization3D = antennaPolarization3DModel.multiLinearPolarization3D[i];
			//
			//	auto weight = oneAntennaLinearPolarization3D.weight;
			//	auto phi0 = oneAntennaLinearPolarization3D.linearPolarization3DObject.phi0;
			//	auto vec = oneAntennaLinearPolarization3D.linearPolarization3DObject.vec;
			//
			//	LinearPolarization3DObjectStd::LinearPolarization3DObject linearPolarization3DObject1(phi0, Point3DStd::Point3D(vec.x, vec.y, vec.z));
			//	OneLinearPolarization3DStd::OneLinearPolarization3D oneLinearPolarization3D1(weight, linearPolarization3DObject1);
			//	multiLinearPolarization3DObject.multiLinearPolarization3D.emplace_back(oneLinearPolarization3D1);
			//}
			//
			//MultiLinearPolarization3DObjectDatabaseStd::MultiLinearPolarization3DObjectDatabase multiLinearPolarization3DObjectDatabase1;
			//multiLinearPolarization3DObjectDatabase1.Add(multiLinearPolarization3DObject);
			//MultiLinearPolarization3DDatabaseStd::InitDatabaseByMultiLinearPolarization3DObjectDatabase(multiLinearPolarization3DObjectDatabase1);
		}

		return true;
	}

	bool InitAntennaRadiationPattern3DModel(const AntennaRadiationPattern3DModel& antennaRadiationPattern3DModel) {

		AntennaPatternDatabaseStd::Clear();
		if (antennaRadiationPattern3DModel.radiationPatternId != -1) {

			if (antennaRadiationPattern3DModel.rows != 360) {
				return false;
			}
			if (antennaRadiationPattern3DModel.columns != 181) {
				return false;
			}

			AntennaPatternObjectStd::AntennaPatternObject antennaPatternObject;
			antennaPatternObject.UpdateData2(antennaRadiationPattern3DModel.radiationPatternId, antennaRadiationPattern3DModel.radiationPattern);
			AntennaPatternDatabaseStd::AddAntennaPatternObject(antennaPatternObject);
		}

		return true;
	}

	bool InitTransmitterAntenna(const TransmitterAntenna& transmitterAntenna, AntennaSIMOStd::AntennaSIMO& interface_antennaSIMO) {

		interface_antennaSIMO.transmittingAntenna.transmittingAntennaId = transmitterAntenna.transmitterAntennaId;
		interface_antennaSIMO.transmittingAntenna.emissionPower = transmitterAntenna.transmitPower;
		interface_antennaSIMO.transmittingAntenna.antennaProperty.location.x = transmitterAntenna.location.x;
		interface_antennaSIMO.transmittingAntenna.antennaProperty.location.y = transmitterAntenna.location.y;
		interface_antennaSIMO.transmittingAntenna.antennaProperty.location.z = transmitterAntenna.location.z;
		interface_antennaSIMO.transmittingAntenna.antennaProperty.polarization3DModelId = transmitterAntenna.polarization3DModelId;
		interface_antennaSIMO.transmittingAntenna.antennaProperty.radiationPatternId = transmitterAntenna.radiationPatternId;
		interface_antennaSIMO.transmittingAntenna.antennaProperty.frequencys.clear();

		for (int i = 0; i < transmitterAntenna.frequencyBandwidth.size; i++) {
			interface_antennaSIMO.transmittingAntenna.antennaProperty.frequencys.emplace_back(transmitterAntenna.frequencyBandwidth.frequencys[i]);
		}
		interface_antennaSIMO.receiverAntennas.resize(transmitterAntenna.receiversCount);
		for (int i = 0; i < transmitterAntenna.receiversCount; i++) {
			interface_antennaSIMO.receiverAntennas[i].receiverAntennaId = transmitterAntenna.receivers[i].receiverAntennaId;
			interface_antennaSIMO.receiverAntennas[i].antennaProperty.location.x = transmitterAntenna.receivers[i].location.x;
			interface_antennaSIMO.receiverAntennas[i].antennaProperty.location.y = transmitterAntenna.receivers[i].location.y;
			interface_antennaSIMO.receiverAntennas[i].antennaProperty.location.z = transmitterAntenna.receivers[i].location.z;
			interface_antennaSIMO.receiverAntennas[i].transmittingAntennaIds.clear();
		}

		if (interface_antennaSIMO.receiverAntennas.size() < 1) {
			return false;
		}

		return true;
	}

	bool InitMaterialSet(const MaterialSet& materialSet) {
		std::vector<MaterialObjectStd::MaterialObject> interface_materials;
		for (int i = 0; i < materialSet.size; i++) {
			MaterialObjectStd::MaterialObject materialObject;
			materialObject.typeNumber = materialSet.materials[i].materialTypeNumber;
			materialObject.frequency = materialSet.materials[i].frequency;
			materialObject.conductivity = materialSet.materials[i].conductivity;
			materialObject.relativePermittivity = materialSet.materials[i].relativePermittivity;
			interface_materials.emplace_back(materialObject);
		}

		MaterialObjectDatabaseStd::Init(interface_materials);
		return true;
	}

	bool InitScenario3D(const Scenario3D& scenario3d, ScenarioObjectStd::ScenarioObject& interface_scenarioObject) {

		interface_scenarioObject.scenarioPoint3D.resize(scenario3d.pointsCount);
		for (int i = 0; i < scenario3d.pointsCount; i++) {
			interface_scenarioObject.scenarioPoint3D[i].x = scenario3d.scenario_point3d_set[i].x;
			interface_scenarioObject.scenarioPoint3D[i].y = scenario3d.scenario_point3d_set[i].y;
			interface_scenarioObject.scenarioPoint3D[i].z = scenario3d.scenario_point3d_set[i].z;
		}

		for (int i = 0; i < scenario3d.trianglesCount; i++) {
			auto scenario_triangle3d = scenario3d.scenario_triangle3d_set[i];
			ScenarioTriangle3DIndexStd::ScenarioTriangle3DIndex scenarioTriangle3DIndex;
			scenarioTriangle3DIndex.DownTypeNumber = scenario_triangle3d.bottomMaterialTypeNumber;
			scenarioTriangle3DIndex.UpTypeNumber = scenario_triangle3d.topMaterialTypeNumber;
			scenarioTriangle3DIndex.n.x = scenario_triangle3d.n.x;
			scenarioTriangle3DIndex.n.y = scenario_triangle3d.n.y;
			scenarioTriangle3DIndex.n.z = scenario_triangle3d.n.z;
			scenarioTriangle3DIndex.Roughness = scenario_triangle3d.roughness;
			scenarioTriangle3DIndex.TriangleP1Index = scenario_triangle3d.triangleP1Index;
			scenarioTriangle3DIndex.TriangleP2Index = scenario_triangle3d.triangleP2Index;
			scenarioTriangle3DIndex.TriangleP3Index = scenario_triangle3d.triangleP3Index;

			interface_scenarioObject.scenarioTriangle3DIndex.emplace_back(scenarioTriangle3DIndex);
		}

		for (int i = 0; i < scenario3d.cornersCount; i++) {
			auto scenario_corner3d = scenario3d.scenario_corner3d_set[i];
			ScenarioCorner3DIndexStd::ScenarioCorner3DIndex scenarioCorner3DIndex;
			scenarioCorner3DIndex.Face0Index = scenario_corner3d.face1Index;
			scenarioCorner3DIndex.FaceNIndex = scenario_corner3d.face2Index;
			scenarioCorner3DIndex.P1Index = scenario_corner3d.p1Index;
			scenarioCorner3DIndex.P2Index = scenario_corner3d.p2Index;
			scenarioCorner3DIndex.P3Face0Index = scenario_corner3d.p3Face1Index;
			scenarioCorner3DIndex.P3FaceNIndex = scenario_corner3d.p3Face2Index;

			interface_scenarioObject.scenarioCorner3DIndex.emplace_back(scenarioCorner3DIndex);
		}

		interface_scenarioObject.scenarioTriangle3DIndex.shrink_to_fit();
		interface_scenarioObject.scenarioCorner3DIndex.shrink_to_fit();

		return true;
	}


	bool InitSbr3DFindPathConfig(const Sbr3DFindPathConfig& sbr3DFindPathConfig,
		CommonParameterConfigStd::CommonParameterConfig& interface_commonParameterConfig,
		GeometricSpaceAccelerateParameterConfigStd::GeometricSpaceAccelerateParameterConfig& interface_geometricSpaceAccelerateParameterConfig,
		MultithreadParameterConfigStd::MultithreadParameterConfig& interface_multithreadParameterConfig,
		RayEjectionParameterConfigStd::RayEjectionParameterConfig& interface_rayEjectionParameterConfig,
		RtSbr3DForRay3DPrivateParameterConfigStd::RtSbr3DForRay3DPrivateParameterConfig& rtSbr3DForRay3DPrivateParameterConfig) {

		interface_commonParameterConfig.deduplicateRadius = 2.4;
		interface_commonParameterConfig.powerThreshold = sbr3DFindPathConfig.powerThreshold;
		interface_commonParameterConfig.rebuildEdge = sbr3DFindPathConfig.rebuildEdge;
		interface_commonParameterConfig.electricFieldCalculationMode = 1;

		interface_geometricSpaceAccelerateParameterConfig.geometricSpaceAccelerateType = sbr3DFindPathConfig.geometricSpaceAccelerateType;

		interface_multithreadParameterConfig.multithreadConfigSwitchOfMultithread = sbr3DFindPathConfig.multithreadConfigSwitchOfMultithread;
		interface_multithreadParameterConfig.multithreadConfigThreadNum = 10;
		interface_multithreadParameterConfig.multithreadConfigThreadOneCpuCalNum = 1;

		interface_rayEjectionParameterConfig.ejectionsMaxTotalNumber = sbr3DFindPathConfig.ejectionsMaxTotalNumber;
		interface_rayEjectionParameterConfig.ejectionsOfDiffractionMaxNumber = sbr3DFindPathConfig.ejectionsOfDiffractionMaxNumber;
		interface_rayEjectionParameterConfig.ejectionsOfDiffuseScatteringMaxNumber = sbr3DFindPathConfig.ejectionsOfDiffuseScatteringMaxNumber;
		interface_rayEjectionParameterConfig.ejectionsOfReflectionMaxNumber = sbr3DFindPathConfig.ejectionsOfReflectionMaxNumber;
		interface_rayEjectionParameterConfig.ejectionsOfTransmissionMaxNumber = sbr3DFindPathConfig.ejectionsOfTransmissionMaxNumber;
		interface_rayEjectionParameterConfig.switchOfLos = sbr3DFindPathConfig.switchOfLos;

		rtSbr3DForRay3DPrivateParameterConfig.diffuseScatteringParameter.diffuseScatteringAr = sbr3DFindPathConfig.diffuseScatteringAr;
		rtSbr3DForRay3DPrivateParameterConfig.diffuseScatteringParameter.diffuseScatteringCoefficient = sbr3DFindPathConfig.diffuseScatteringCoefficient;
		rtSbr3DForRay3DPrivateParameterConfig.diffuseScatteringParameter.diffuseScatteringRayleighRange = sbr3DFindPathConfig.diffuseScatteringRayleighRange;
		rtSbr3DForRay3DPrivateParameterConfig.gapDiffractionRad = sbr3DFindPathConfig.gapDiffractionRad;
		rtSbr3DForRay3DPrivateParameterConfig.gapDiffuseScatteringAzimuth = sbr3DFindPathConfig.gapDiffuseScatteringAzimuth;
		rtSbr3DForRay3DPrivateParameterConfig.gapDiffuseScatteringPitchAngle = sbr3DFindPathConfig.gapDiffuseScatteringPitchAngle;
		rtSbr3DForRay3DPrivateParameterConfig.radiusCorner = sbr3DFindPathConfig.radiusCorner;
		rtSbr3DForRay3DPrivateParameterConfig.radiusRx = sbr3DFindPathConfig.radiusRx;
		rtSbr3DForRay3DPrivateParameterConfig.rayNumber = (size_t)sbr3DFindPathConfig.rayNumber;

		rtSbr3DForRay3DPrivateParameterConfig.cylindricalTube = true;
		rtSbr3DForRay3DPrivateParameterConfig.realWorldRefraction = true;


		return true;
	}

	bool RTSbr3DCircularPolarization3DInitParameter(
		const Sbr3DFindPathConfig& config,
		const MaterialSet& materialSet,
		const Scenario3D& scenario,
		const AntennaPolarization3DModel& antennaPolarization3DModel,
		const AntennaRadiationPattern3DModel& antennaRadiationPattern3DModel,
		const TransmitterAntenna& transmitterAntenna,
		RtSbr3DForRay3DParameterConfigStd::RtSbr3DForRay3DParameterConfig& rtSbr3DForRay3DParameterConfig,
		AntennaSIMOStd::AntennaSIMO& interface_antennaSIMO,
		ScenarioObjectStd::ScenarioObject& interface_scenarioObject,
		OutputNewsOfSimulationCalculation& outputNewsOfSimulationCalculation) {

		if (!InitMaterialSet(materialSet)) {
			outputNewsOfSimulationCalculation.type = OutputTypeOfSimulationCalculation::Null;
			outputNewsOfSimulationCalculation.news = "Failed to initialize the material information.";
			return false;
		}

		if (!InitScenario3D(scenario, interface_scenarioObject)) {
			outputNewsOfSimulationCalculation.type = OutputTypeOfSimulationCalculation::Null;
			outputNewsOfSimulationCalculation.news = "Failed to initialize the scenario information.";
			return false;
		}

		if (!InitAntennaPolarization3DModel(antennaPolarization3DModel)) {
			outputNewsOfSimulationCalculation.type = OutputTypeOfSimulationCalculation::Null;
			outputNewsOfSimulationCalculation.news = "Failed to initialize the antenna polarization information.";
			return false;
		}

		if (!InitAntennaRadiationPattern3DModel(antennaRadiationPattern3DModel)) {
			outputNewsOfSimulationCalculation.type = OutputTypeOfSimulationCalculation::Null;
			outputNewsOfSimulationCalculation.news = "Failed to initialize the antenna radiation pattern information.";
			return false;
		}

		if (!InitTransmitterAntenna(transmitterAntenna, interface_antennaSIMO)) {
			outputNewsOfSimulationCalculation.type = OutputTypeOfSimulationCalculation::Null;
			outputNewsOfSimulationCalculation.news = "Failed to initialize the transmitter antenna information.";
			return false;
		}

		if (!InitSbr3DFindPathConfig(config,
			rtSbr3DForRay3DParameterConfig.commonParameterConfig,
			rtSbr3DForRay3DParameterConfig.geometricSpaceAccelerateParameterConfig,
			rtSbr3DForRay3DParameterConfig.multithreadParameterConfig,
			rtSbr3DForRay3DParameterConfig.rayEjectionParameterConfig,
			rtSbr3DForRay3DParameterConfig.rtSbr3DForRay3DPrivateParameterConfig)) {
			outputNewsOfSimulationCalculation.type = OutputTypeOfSimulationCalculation::Null;
			outputNewsOfSimulationCalculation.news = "Failed to initialize the SBR3D find path configuration.";
			return false;
		}

		return true;

	}

	void ChangeAABBBox(ScenarioObjectStd::ScenarioObject& interface_scenarioObject,
		AntennaSIMOStd::AntennaSIMO& interface_antennaSIMO) {

		if (interface_scenarioObject.scenarioPoint3D.size() > 0) {

			interface_scenarioObject.scenarioMinPoint3D = interface_scenarioObject.scenarioPoint3D[0];
			interface_scenarioObject.scenarioMaxPoint3D = interface_scenarioObject.scenarioPoint3D[0];

			for (int i = 1; i < interface_scenarioObject.scenarioPoint3D.size(); ++i)
			{
				Geometry3DOperateStd::ChangeSenceMinPoint3D_point(interface_scenarioObject.scenarioPoint3D[i], interface_scenarioObject.scenarioMinPoint3D);
				Geometry3DOperateStd::ChangeSenceMaxPoint3D_point(interface_scenarioObject.scenarioPoint3D[i], interface_scenarioObject.scenarioMaxPoint3D);
			}

		}
		Geometry3DOperateStd::ChangeSenceMinPoint3D_point(interface_antennaSIMO.transmittingAntenna.antennaProperty.location, interface_scenarioObject.scenarioMinPoint3D);
		Geometry3DOperateStd::ChangeSenceMaxPoint3D_point(interface_antennaSIMO.transmittingAntenna.antennaProperty.location, interface_scenarioObject.scenarioMaxPoint3D);

		for (int i = 1; i < interface_antennaSIMO.receiverAntennas.size(); ++i) {

			Geometry3DOperateStd::ChangeSenceMinPoint3D_point(interface_antennaSIMO.receiverAntennas[i].antennaProperty.location, interface_scenarioObject.scenarioMinPoint3D);
			Geometry3DOperateStd::ChangeSenceMaxPoint3D_point(interface_antennaSIMO.receiverAntennas[i].antennaProperty.location, interface_scenarioObject.scenarioMaxPoint3D);
		}

	}


	bool CalScenarioDataInformation(
		double cornerRadius,
		const Scenario3D& scenario,
		const AntennaSIMOStd::AntennaSIMO& antennaSIMO,
		RtSbr3DForRay3DParameterConfigStd::RtSbr3DForRay3DParameterConfig& rtSbr3DForRay3DParameterConfig,
		AntennaMIMOStd::AntennaMIMO& antennaMIMO,
		ScenarioDataInformationStd::ScenarioDataInformation& scenarioDataInformation,
		AntennaMIMOPathStd::AntennaMIMOPath& antennaMIMOPath,
		ScenarioObjectStd::ScenarioObject& scenarioObject) {

		bool rebuildEdge = rtSbr3DForRay3DParameterConfig.commonParameterConfig.rebuildEdge;
		int ejectionsMaxTotalNumber = rtSbr3DForRay3DParameterConfig.rayEjectionParameterConfig.ejectionsMaxTotalNumber;
		int ejectionsOfDiffractionMaxNumber = rtSbr3DForRay3DParameterConfig.rayEjectionParameterConfig.ejectionsOfDiffractionMaxNumber;

		bool switchOfOfDiffraction = false;
		if (ejectionsMaxTotalNumber > 0 && ejectionsOfDiffractionMaxNumber > 0) {
			switchOfOfDiffraction = true;
		}
		//3.1 重构边
		if (rebuildEdge && switchOfOfDiffraction) {
			ScenarioObjectStd::RebuildEdgeInformation(scenarioObject);
		}

		if (!ScenarioDataInformationStd::InitScenarioDataInformationByScenarioObject(switchOfOfDiffraction, scenarioObject, scenarioDataInformation)) {
			return false;
		}


		if (!RtProgramReadsDataAndPreprocessesDataStd::GeometricSpacePartitionProcessingMethod(
			rtSbr3DForRay3DParameterConfig.geometricSpaceAccelerateParameterConfig.geometricSpaceAccelerateType,
			scenario,
			false,
			cornerRadius)) {
			return false;
		}


		CalRunTimeStd::CalRunTime CCC(true);
		//if (switchOfOfDiffraction) {
		//
		//	{
		//		size_t seg_seg_size = scenarioObject.scenarioCorner3DIndex.size();
		//		seg_seg_samepoint.resize(seg_seg_size, std::vector<bool>(seg_seg_size, false));
		//
		//		std::vector<std::vector<bool>> seg_seg_samepoint_visited(seg_seg_size, std::vector<bool>(seg_seg_size, false));
		//
		//		std::unordered_map<int, std::unordered_set<int>> point_seg_index_map;
		//
		//		for (int loop_1 = 0; loop_1 < seg_seg_size; ++loop_1) {
		//			auto seg_index_1 = scenarioObject.scenarioCorner3DIndex[loop_1];
		//			point_seg_index_map[seg_index_1.P1Index].insert(loop_1);
		//			point_seg_index_map[seg_index_1.P2Index].insert(loop_1);
		//
		//			seg_seg_samepoint[loop_1][loop_1] = true;
		//			seg_seg_samepoint_visited[loop_1][loop_1] = true;
		//		}
		//
		//		for (auto& ele_1 : point_seg_index_map) {
		//			std::unordered_set<int> seg_index = ele_1.second;
		//			if (seg_index.size() > 1) {
		//				std::vector<int> index_seg_vector;
		//				index_seg_vector.insert(index_seg_vector.end(), seg_index.begin(), seg_index.end());
		//
		//				for (int loop_1 = 0; loop_1 < (int)index_seg_vector.size() - 1; ++loop_1) {
		//					int seg_1_index = index_seg_vector[loop_1];
		//					for (int loop_2 = loop_1 + 1; loop_2 < index_seg_vector.size(); ++loop_2) {
		//						int seg_2_index = index_seg_vector[loop_2];
		//						if (seg_seg_samepoint_visited[loop_1][loop_2]) {
		//							continue;
		//						}
		//						bool state = false;
		//						{
		//							auto seg_index_1 = scenarioObject.scenarioCorner3DIndex[seg_1_index];
		//							auto seg_1_start = scenarioObject.scenarioPoint3D[seg_index_1.P1Index];
		//							auto seg_1_end = scenarioObject.scenarioPoint3D[seg_index_1.P2Index];
		//
		//							auto seg_index_2 = scenarioObject.scenarioCorner3DIndex[seg_2_index];
		//							auto seg_2_start = scenarioObject.scenarioPoint3D[seg_index_2.P1Index];
		//							auto seg_2_end = scenarioObject.scenarioPoint3D[seg_index_2.P2Index];
		//							if (Geometry3DIntersectStd::Intersect_LineSegment3D_LineSegment3D_plus(seg_2_start, seg_2_end, seg_1_start, seg_1_end)) {
		//								state = true;
		//							}
		//
		//						}
		//
		//						seg_seg_samepoint[loop_1][loop_2] = state;
		//						seg_seg_samepoint[loop_2][loop_1] = state;
		//						seg_seg_samepoint_visited[loop_1][loop_2] = true;
		//						seg_seg_samepoint_visited[loop_2][loop_1] = true;
		//					}
		//				}
		//
		//			}
		//		}
		//
		//	}
		//
		//}
		//
		//{
		//
		//	{
		//		size_t triangle_triangle_size = scenarioObject.scenarioTriangle3DIndex.size();
		//		triangle_triangle_sameside.resize(triangle_triangle_size, std::vector<bool>(triangle_triangle_size, false));
		//
		//		std::vector<std::vector<bool>> triangle_triangle_sameside_visited(triangle_triangle_size, std::vector<bool>(triangle_triangle_size, false));
		//
		//		std::unordered_map<int, std::unordered_set<int>> point_triangle_index_map;
		//		for (int loop_1 = 0; loop_1 < triangle_triangle_size; ++loop_1) {
		//			auto triangle_index_1 = scenarioObject.scenarioTriangle3DIndex[loop_1];
		//			auto triangle_index_1_p1 = triangle_index_1.TriangleP1Index;
		//			auto triangle_index_1_p2 = triangle_index_1.TriangleP2Index;
		//			auto triangle_index_1_p3 = triangle_index_1.TriangleP3Index;
		//			point_triangle_index_map[triangle_index_1_p1].insert(loop_1);
		//			point_triangle_index_map[triangle_index_1_p2].insert(loop_1);
		//			point_triangle_index_map[triangle_index_1_p3].insert(loop_1);
		//
		//			triangle_triangle_sameside[loop_1][loop_1] = true;
		//			triangle_triangle_sameside_visited[loop_1][loop_1] = true;
		//		}
		//
		//		for (auto& ele_1 : point_triangle_index_map) {
		//			std::unordered_set<int> triangle_index = ele_1.second;
		//			if (triangle_index.size() > 1) {
		//				std::vector<int> index_triangle_vector;
		//				index_triangle_vector.insert(index_triangle_vector.end(), triangle_index.begin(), triangle_index.end());
		//
		//				for (int loop_1 = 0; loop_1 < (int)index_triangle_vector.size() - 1; ++loop_1) {
		//					int triangle_1_index = index_triangle_vector[loop_1];
		//					for (int loop_2 = loop_1 + 1; loop_2 < index_triangle_vector.size(); ++loop_2) {
		//						int triangle_2_index = index_triangle_vector[loop_2];
		//						if (triangle_triangle_sameside_visited[loop_1][loop_2]) {
		//							continue;
		//						}
		//						bool state = false;
		//						{
		//							auto triangle_index_1 = scenarioObject.scenarioTriangle3DIndex[triangle_1_index];
		//							auto triangle_index_1_p1 = triangle_index_1.TriangleP1Index;
		//							auto triangle_index_1_p2 = triangle_index_1.TriangleP2Index;
		//							auto triangle_index_1_p3 = triangle_index_1.TriangleP3Index;
		//							auto triangle_index_2 = scenarioObject.scenarioTriangle3DIndex[triangle_2_index];
		//							auto triangle_index_2_p1 = triangle_index_2.TriangleP1Index;
		//							auto triangle_index_2_p2 = triangle_index_2.TriangleP2Index;
		//							auto triangle_index_2_p3 = triangle_index_2.TriangleP3Index;
		//							std::unordered_set<int> num_index_set;
		//							num_index_set.insert(triangle_index_1_p1);
		//							num_index_set.insert(triangle_index_1_p2);
		//							num_index_set.insert(triangle_index_1_p3);
		//
		//							num_index_set.insert(triangle_index_2_p1);
		//							num_index_set.insert(triangle_index_2_p2);
		//							num_index_set.insert(triangle_index_2_p3);
		//							if (num_index_set.size() < 5) {
		//								state = true;
		//								auto triangle_1_n = triangle_index_1.n;
		//								auto triangle_2_n = triangle_index_2.n;
		//								if (!Geometry3DOperateStd::Equals_Point3D_N(triangle_1_n, triangle_2_n)) {
		//									state = false;
		//								}
		//							}
		//
		//
		//						}
		//
		//						triangle_triangle_sameside[loop_1][loop_2] = state;
		//						triangle_triangle_sameside[loop_2][loop_1] = state;
		//						triangle_triangle_sameside_visited[loop_1][loop_2] = true;
		//						triangle_triangle_sameside_visited[loop_2][loop_1] = true;
		//					}
		//				}
		//
		//			}
		//		}
		//
		//	}
		//}
		antennaMIMO.antennaSIMOs.clear();
		antennaMIMO.antennaSIMOs.emplace_back(antennaSIMO);

		//内存初始化
		RtProgramReadsDataAndPreprocessesDataStd::InitAntennaMIMOAndAntennaMIMOPath(antennaMIMO, antennaMIMOPath);
		return true;
	}

}

/// <summary>
/// 计算电场
/// </summary>
namespace RTSbr3DCircularPolarization3DCalElectricFieldStd {

	Point3D ToIsacPoint3d(const Point3DStd::Point3D& point3D) {

		Point3D isacPoint3d;
		isacPoint3d.x = point3D.x;
		isacPoint3d.y = point3D.y;
		isacPoint3d.z = point3D.z;

		return isacPoint3d;
	}

	int IndexOfMaterialObjectType(
		long long frequency, int materialObjectType) {

		return MaterialObjectDatabaseStd::IndexOf(materialObjectType, frequency);
	}

	ElectricFieldNode ToElectricFieldNode_0(
		int transmitterAntennamMaterialTypeNumberIndex,
		MultiPathNodeInfoStd::MultiPathNodeInfo* node) {

		ElectricFieldNode electricFieldNode;
		electricFieldNode.type = 0;
		electricFieldNode.location_x = node->location.x;
		electricFieldNode.location_y = node->location.y;
		electricFieldNode.location_z = node->location.z;
		electricFieldNode.attachmentNumber = node->attachmentId;
		electricFieldNode.materialIndex1 = transmitterAntennamMaterialTypeNumberIndex;
		electricFieldNode.pre_distance = 0.0;

		return electricFieldNode;

	}

	ElectricFieldNode ToElectricFieldNode_1(
		MultiPathNodeInfoStd::MultiPathNodeInfo* node) {

		ElectricFieldNode electricFieldNode;
		electricFieldNode.type = 1;
		electricFieldNode.location_x = node->location.x;
		electricFieldNode.location_y = node->location.y;
		electricFieldNode.location_z = node->location.z;
		electricFieldNode.attachmentNumber = node->attachmentId;

		return electricFieldNode;
	}


	ElectricFieldNode ToElectricFieldNode_2(
		long long frequency,
		MultiPathNodeInfoStd::MultiPathNodeInfo* node) {

		ElectricFieldNode electricFieldNode;
		electricFieldNode.type = 2;
		electricFieldNode.location_x = node->location.x;
		electricFieldNode.location_y = node->location.y;
		electricFieldNode.location_z = node->location.z;
		electricFieldNode.attachmentNumber = node->attachmentId;

		MultiPathNodeInfoStd::MultiPathNodeInfoReflection* reflectionNode =
			MultiPathNodeInfoOperateStd::MultiPathNodeInfo_ptr_to_MultiPathNodeInfoReflection_ptr(node);

		electricFieldNode.n_x = reflectionNode->normalVector.x;
		electricFieldNode.n_y = reflectionNode->normalVector.y;
		electricFieldNode.n_z = reflectionNode->normalVector.z;
		electricFieldNode.r_t_s_d_thetai_beta0 = reflectionNode->thetai;

		electricFieldNode.materialIndex1 = IndexOfMaterialObjectType(frequency, reflectionNode->upObjectType);
		electricFieldNode.materialIndex2 = IndexOfMaterialObjectType(frequency, reflectionNode->downObjectType);
		return electricFieldNode;
	}


	ElectricFieldNode ToElectricFieldNode_3(
		long long frequency,
		MultiPathNodeInfoStd::MultiPathNodeInfo* node) {

		ElectricFieldNode electricFieldNode;
		electricFieldNode.type = 3;
		electricFieldNode.location_x = node->location.x;
		electricFieldNode.location_y = node->location.y;
		electricFieldNode.location_z = node->location.z;
		electricFieldNode.attachmentNumber = node->attachmentId;

		MultiPathNodeInfoStd::MultiPathNodeInfoTransmission* transmissionNode =
			MultiPathNodeInfoOperateStd::MultiPathNodeInfo_ptr_to_MultiPathNodeInfoTransmission_ptr(node);

		electricFieldNode.n_x = transmissionNode->normalVector.x;
		electricFieldNode.n_y = transmissionNode->normalVector.y;
		electricFieldNode.n_z = transmissionNode->normalVector.z;
		electricFieldNode.r_t_s_d_thetai_beta0 = transmissionNode->thetai;

		electricFieldNode.materialIndex1 = IndexOfMaterialObjectType(frequency, transmissionNode->upObjectType);
		electricFieldNode.materialIndex2 = IndexOfMaterialObjectType(frequency, transmissionNode->downObjectType);
		return electricFieldNode;
	}


	ElectricFieldNode ToElectricFieldNode_4(
		long long frequency,
		MultiPathNodeInfoStd::MultiPathNodeInfo* node) {

		ElectricFieldNode electricFieldNode;
		electricFieldNode.type = 4;
		electricFieldNode.location_x = node->location.x;
		electricFieldNode.location_y = node->location.y;
		electricFieldNode.location_z = node->location.z;
		electricFieldNode.attachmentNumber = node->attachmentId;

		MultiPathNodeInfoStd::MultiPathNodeInfoDiffraction* diffractionNode =
			MultiPathNodeInfoOperateStd::MultiPathNodeInfo_ptr_to_MultiPathNodeInfoDiffraction_ptr(node);

		electricFieldNode.n_x = diffractionNode->normalVector.x;
		electricFieldNode.n_y = diffractionNode->normalVector.y;
		electricFieldNode.n_z = diffractionNode->normalVector.z;
		electricFieldNode.r_t_s_d_thetai_beta0 = diffractionNode->beta;
		electricFieldNode.d_phi1_s_thetar = diffractionNode->phi1;
		electricFieldNode.d_phi2_s_ar = diffractionNode->phi2;
		electricFieldNode.d_phiE_s_s = diffractionNode->phiE;

		int materialIndex1 = IndexOfMaterialObjectType(frequency, diffractionNode->upObjectType);
		int materialIndex2 = IndexOfMaterialObjectType(frequency, diffractionNode->downObjectType);

		electricFieldNode.materialIndex1 = materialIndex1;
		electricFieldNode.materialIndex2 = materialIndex2;
		return electricFieldNode;
	}


	ElectricFieldNode ToElectricFieldNode_5(
		long long frequency,
		MultiPathNodeInfoStd::MultiPathNodeInfo* node) {

		ElectricFieldNode electricFieldNode;
		electricFieldNode.type = 5;
		electricFieldNode.location_x = node->location.x;
		electricFieldNode.location_y = node->location.y;
		electricFieldNode.location_z = node->location.z;
		electricFieldNode.attachmentNumber = node->attachmentId;

		MultiPathNodeInfoStd::MultiPathNodeInfoDiffuseScattering* diffuseScatteringNode =
			MultiPathNodeInfoOperateStd::MultiPathNodeInfo_ptr_to_MultiPathNodeInfoDiffuseScattering_ptr(node);

		electricFieldNode.n_x = diffuseScatteringNode->normalVector.x;
		electricFieldNode.n_y = diffuseScatteringNode->normalVector.y;
		electricFieldNode.n_z = diffuseScatteringNode->normalVector.z;
		electricFieldNode.r_t_s_d_thetai_beta0 = diffuseScatteringNode->thetai;
		electricFieldNode.d_phi1_s_thetar = diffuseScatteringNode->beta;
		electricFieldNode.d_phi2_s_ar = diffuseScatteringNode->widthCoefficientOfDiffuseLobe;
		electricFieldNode.d_phiE_s_s = diffuseScatteringNode->scatteringCoefficient;
		electricFieldNode.s_roughness = diffuseScatteringNode->roughness;

		int materialIndex1 = IndexOfMaterialObjectType(frequency, diffuseScatteringNode->upObjectType);
		int materialIndex2 = IndexOfMaterialObjectType(frequency, diffuseScatteringNode->downObjectType);

		electricFieldNode.materialIndex1 = materialIndex1;
		electricFieldNode.materialIndex2 = materialIndex2;
		return electricFieldNode;
	}

	ElectricFieldNode ToElectricFieldNode(
		int transmitterAntennamMaterialTypeNumberIndex,
		long long frequency,
		MultiPathNodeInfoStd::MultiPathNodeInfo* node) {

		switch (node->type) {
		case PropagationTypeStd::PropagationType::TransmittingAntenna:
			return ToElectricFieldNode_0(transmitterAntennamMaterialTypeNumberIndex, node);
		case PropagationTypeStd::PropagationType::ReceiverAntenna:
			return ToElectricFieldNode_1(node);
		case PropagationTypeStd::PropagationType::Reflection:
			return ToElectricFieldNode_2(frequency, node);
		case PropagationTypeStd::PropagationType::Transmission:
			return ToElectricFieldNode_3(frequency, node);
		case PropagationTypeStd::PropagationType::Diffraction:
			return ToElectricFieldNode_4(frequency, node);
		case PropagationTypeStd::PropagationType::DiffuseScattering:
			return ToElectricFieldNode_5(frequency, node);
		default:
			return ElectricFieldNode();
		}

	}

	ElectricFieldPath ToElectricFieldPath(
		int transmitterAntennamMaterialTypeNumberIndex,
		long long frequency,
		const AntennaSISOOnePathStd::AntennaSISOOnePath& antennaSISOOnePath) {

		ElectricFieldPath electricFieldPath;
		int path_size = (int)antennaSISOOnePath.path.size();
		electricFieldPath.node_size = path_size;
		if (path_size > 0) {
			electricFieldPath.nodes = (ElectricFieldNode*)malloc(path_size * sizeof(ElectricFieldNode));
			for (int i = 0; i < path_size; ++i) {
				electricFieldPath.nodes[i] = ToElectricFieldNode(
					transmitterAntennamMaterialTypeNumberIndex,
					frequency,
					antennaSISOOnePath.path[i]);
			}

		}

		return electricFieldPath;

	}

	ElectricFieldAllPath ToElectricFieldAllPath(
		int transmitterAntennamMaterialTypeNumberIndex,
		long long frequency,
		const AntennaSISOPathStd::AntennaSISOPath& antennaSISOPath) {

		ElectricFieldAllPath electricFieldAllPath;

		electricFieldAllPath.receiverAntennaId = antennaSISOPath.receiverAntennaId;
		electricFieldAllPath.rx_location = ToIsacPoint3d(antennaSISOPath.rx_location);


		int pathNum = (int)antennaSISOPath.paths.size();
		electricFieldAllPath.path_size = pathNum;
		if (pathNum > 0) {
			electricFieldAllPath.paths = (ElectricFieldPath*)malloc(pathNum * sizeof(ElectricFieldPath));
			for (int i = 0; i < pathNum; ++i) {
				electricFieldAllPath.paths[i] = ToElectricFieldPath(
					transmitterAntennamMaterialTypeNumberIndex,
					frequency,
					antennaSISOPath.paths[i]);
			}
		}


		return electricFieldAllPath;

	}

	PropagationFrequencyInformation ToPropagationFrequencyInformation(
		int materialTypeNumber,
		long long frequency,
		double s_coefficient,
		const AntennaSIMOPathStd::AntennaSIMOPath& antennaSIMOPath) {
		int transmitterAntennamMaterialTypeNumberIndex = IndexOfMaterialObjectType(frequency, materialTypeNumber);
		PropagationFrequencyInformation propagationFrequencyInformation;
		propagationFrequencyInformation.frequency = frequency;
		propagationFrequencyInformation.s_coefficient = s_coefficient;

		int receiverNum = (int)antennaSIMOPath.paths.size();
		propagationFrequencyInformation.receivers_information_size = receiverNum;
		if (receiverNum > 0) {
			propagationFrequencyInformation.receivers_information = (ElectricFieldAllPath*)malloc(receiverNum * sizeof(ElectricFieldAllPath));
			for (int i = 0; i < receiverNum; ++i) {
				propagationFrequencyInformation.receivers_information[i] = ToElectricFieldAllPath(
					transmitterAntennamMaterialTypeNumberIndex, frequency, antennaSIMOPath.paths[i]);
			}
		}

		return propagationFrequencyInformation;

	}


	void ToRtoiOutputInformation(
		int materialTypeNumber,
		double s_coefficient,
		const AntennaSIMOStd::AntennaSIMO& antennaSIMO,
		const AntennaSIMOPathStd::AntennaSIMOPath& antennaSIMOPath,
		RtoiOutputInformation& outputInfo) {

		outputInfo.tx_location = ToIsacPoint3d(antennaSIMO.transmittingAntenna.antennaProperty.location);
		outputInfo.transmitterAntennaId = antennaSIMO.transmittingAntenna.transmittingAntennaId;
		outputInfo.frequency_size = 0;
		outputInfo.frequencys_information = NULL;
		int size = (int)antennaSIMO.transmittingAntenna.antennaProperty.frequencys.size();
		outputInfo.frequency_size = size;
		if (antennaSIMO.transmittingAntenna.antennaProperty.frequencys.size() > 0) {
			outputInfo.frequencys_information = (PropagationFrequencyInformation*)malloc(size * sizeof(PropagationFrequencyInformation));
			if (outputInfo.frequencys_information == NULL) {
				std::cout << "malloc memory failed in RTSbr3DCircularPolarization3D" << std::endl;
				return;
			}
			for (int i = 0; i < antennaSIMO.transmittingAntenna.antennaProperty.frequencys.size(); ++i) {
				auto frequency = antennaSIMO.transmittingAntenna.antennaProperty.frequencys[i];
				outputInfo.frequencys_information[i] = ToPropagationFrequencyInformation(
					materialTypeNumber, frequency, s_coefficient, antennaSIMOPath);

			}
		}


	}

	void CalculateElectricField_ElectricFieldAllPath(
		int radiationPatternId, double transmitPowerW, long long frequency, double s_coefficient, const MaterialSet& materialList, ElectricFieldAllPath& electricFieldAllPath) {

		CalculateWaveImpactResponseDBmUnderCircularPolarization3Ds(
			radiationPatternId, transmitPowerW, frequency, s_coefficient,
			materialList,
			electricFieldAllPath.path_size,
			electricFieldAllPath.paths,
			electricFieldAllPath.pathLoss,
			electricFieldAllPath.power);

	}

	void CalculateElectricField_PropagationFrequencyInformation(
		int radiationPatternId, double transmitPowerW, const MaterialSet& materialList, PropagationFrequencyInformation& propagationFrequencyInformation) {

		for (int i = 0; i < propagationFrequencyInformation.receivers_information_size; ++i) {
			CalculateElectricField_ElectricFieldAllPath(
				radiationPatternId, transmitPowerW, propagationFrequencyInformation.frequency, propagationFrequencyInformation.s_coefficient,
				materialList,
				propagationFrequencyInformation.receivers_information[i]);
		}

	}

	void CalculateElectricField(int radiationPatternId, double transmitPowerW, const MaterialSet& materialList, RtoiOutputInformation& outputInfo) {

		for (int i = 0; i < outputInfo.frequency_size; ++i) {
			CalculateElectricField_PropagationFrequencyInformation(
				radiationPatternId, transmitPowerW, materialList, outputInfo.frequencys_information[i]);
		}

	}

}




extern "C" _declspec(dllexport) void RTSbr3DCircularPolarization3D(
	const Sbr3DFindPathConfig & config,
	const MaterialSet & materialSet,
	const Scenario3D & scenario,
	const AntennaPolarization3DModel & antennaPolarization3DModel,
	const AntennaRadiationPattern3DModel & antennaRadiationPattern3DModel,
	const TransmitterAntenna & transmitterAntenna,
	RtoiOutputInformation & outputInfo,
	OutputNewsOfSimulationCalculation & outputNewsOfSimulationCalculation) {


	CalRunTimeStd::CalRunTime calRunTime(false);

	//1.初始化参数
	RtSbr3DForRay3DParameterConfigStd::RtSbr3DForRay3DParameterConfig rtSbr3DForRay3DParameterConfig;

	AntennaSIMOStd::AntennaSIMO antennaSIMO;
	ScenarioObjectStd::ScenarioObject scenarioObject;
	if (!RTSbr3DCircularPolarization3DInitParameterStd::RTSbr3DCircularPolarization3DInitParameter(
		config,
		materialSet,
		scenario,
		antennaPolarization3DModel,
		antennaRadiationPattern3DModel,
		transmitterAntenna,
		rtSbr3DForRay3DParameterConfig,
		antennaSIMO,
		scenarioObject,
		outputNewsOfSimulationCalculation)) {
		return;
	}
	RTSbr3DCircularPolarization3DInitParameterStd::ChangeAABBBox(scenarioObject, antennaSIMO);

	double time_calPath_start, time_calPath_end;
	AntennaMIMOPathStd::AntennaMIMOPath antennaMIMOPath;

	{

		AntennaMIMOStd::AntennaMIMO antennaMIMO;
		ScenarioDataInformationStd::ScenarioDataInformation scenarioDataInformation;
		if (!RTSbr3DCircularPolarization3DInitParameterStd::CalScenarioDataInformation(
			config.radiusCorner,
			scenario,
			antennaSIMO,
			rtSbr3DForRay3DParameterConfig,
			antennaMIMO,
			scenarioDataInformation, antennaMIMOPath,
			scenarioObject)) {
			return;
		}

		//3.计算路径

		//设置计算精度

		time_calPath_start = calRunTime.GetTimeAll();
		//基于正向射线根据计算路径

		{
			GlobalConstantStd::SetAirSubstanceType(rtSbr3DForRay3DParameterConfig.commonParameterConfig.airSubstanceType);
			//计算路径
			//
			RtSbr3DForRay3DFindPathStd::RtSbr3DForRay3DFindPath(
				transmitterAntenna.materialTypeNumber,
				rtSbr3DForRay3DParameterConfig.commonParameterConfig.deduplicateRadius,
				rtSbr3DForRay3DParameterConfig.multithreadParameterConfig,
				rtSbr3DForRay3DParameterConfig.rayEjectionParameterConfig,
				rtSbr3DForRay3DParameterConfig.rtSbr3DForRay3DPrivateParameterConfig,
				scenario,
				materialSet,
				scenarioDataInformation,
				antennaMIMO,
				antennaMIMOPath);
		}

		time_calPath_end = calRunTime.GetTimeAll();


	}
	double time_calElectricField_start, time_calElectricField_end;

	//4.计算电场
	time_calElectricField_start = calRunTime.GetTimeAll();

	std::cout << "开始电场计算!" << std::endl;

	if (antennaMIMOPath.paths.size() > 0) {

		//第一步，转化为结构体
		RTSbr3DCircularPolarization3DCalElectricFieldStd::ToRtoiOutputInformation(
			transmitterAntenna.materialTypeNumber,
			rtSbr3DForRay3DParameterConfig.rtSbr3DForRay3DPrivateParameterConfig.diffuseScatteringParameter.diffuseScatteringRayleighRange,
			antennaSIMO, antennaMIMOPath.paths[0], outputInfo);
		//第二步，计算电场
		RTSbr3DCircularPolarization3DCalElectricFieldStd::CalculateElectricField(
			transmitterAntenna.radiationPatternId,
			transmitterAntenna.transmitPower,
			materialSet,
			outputInfo);

	}
	else {

		outputNewsOfSimulationCalculation.type = OutputTypeOfSimulationCalculation::Null;
		outputNewsOfSimulationCalculation.news = "No valid path was found by the program.";
		return;
	}

	std::cout << "电场计算完成!" << std::endl;

	time_calElectricField_end = calRunTime.GetTimeAll();


	double time_run_all = calRunTime.GetTimeAll();

	{
		std::ostringstream oss;
		oss << "几何寻径用时为 " << (time_calPath_end - time_calPath_start) << " ms." << std::endl;
		oss << "电场计算用时为 " << (time_calElectricField_end - time_calElectricField_start) << " ms." << std::endl;
		std::cout << oss.str() << std::endl;
	}



	//10.内存释放，程序结束

	{
		//所有的多径节点最终被统一释放内存
		MultiPathNodeInfoOperateStd::FreeMultiPathNodeInfo_vector_all();

	}


	outputNewsOfSimulationCalculation.type = OutputTypeOfSimulationCalculation::Successfully;
	outputNewsOfSimulationCalculation.news = "Successfully.";

}

RtoiOutputInformation* RtoiOutputInformationPtr = NULL;
OutputNewsOfSimulationCalculation OutputNewsOfSimulationCalculationPtr;
extern "C" _declspec(dllexport) void FreeMemoryRTSbr3DCircularPolarization3DPtr() {
	OutputNewsOfSimulationCalculationPtr.type = OutputTypeOfSimulationCalculation::Null;

}

extern "C" _declspec(dllexport) RtoiOutputInformation* RTSbr3DCircularPolarization3DPtr(
	const Sbr3DFindPathConfig & config,
	const MaterialSet & materialSet,
	const Scenario3D & scenario,
	const AntennaPolarization3DModel & antennaPolarization3DModel,
	const AntennaRadiationPattern3DModel & antennaRadiationPattern3DModel,
	const TransmitterAntenna & transmitterAntenna,
	bool& success) {

	FreeMemoryRTSbr3DCircularPolarization3DPtr();

	RtoiOutputInformationPtr = (RtoiOutputInformation*)malloc(sizeof(RtoiOutputInformation));
	if (RtoiOutputInformationPtr == NULL) {
		success = false;
		return NULL;
	}

	RTSbr3DCircularPolarization3D(config, materialSet, scenario,
		antennaPolarization3DModel, antennaRadiationPattern3DModel, transmitterAntenna,
		*RtoiOutputInformationPtr,
		OutputNewsOfSimulationCalculationPtr);

	if (OutputNewsOfSimulationCalculationPtr.type == OutputTypeOfSimulationCalculation::Successfully) {

		success = true;
		return RtoiOutputInformationPtr;
	}
	else {
		success = false;
		return NULL;
	}

}