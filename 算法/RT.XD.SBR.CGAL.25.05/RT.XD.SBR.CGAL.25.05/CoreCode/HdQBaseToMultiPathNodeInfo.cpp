


#include"HdQBaseToMultiPathNodeInfo.h"
#include"QzQGeometryRTMultiPathDNode.h"
#include"QzQGeometryRTMultiPathRNode.h"
#include"QzQGeometryRTMultiPathTxNode.h"
#include"QzQGeometryRTMultiPathRxNode.h"
#include"QzQGeometry3DIntersect.h"
#include"QzQGeometry3DOperate.Point3D.h"
#include"QzQGeometry3DOperate.Length.h"
#include"QzQGlobalConstant.h"
#include"LxQProjectDependencies.h"
#include"DxQRayScenarioIntersect.h"



namespace BaseToMultiPathNodeInfoStd {

	bool CheckByRayScenarioIntersect(
		const Point3DStd::Point3D& pre_location,
		const Point3DStd::Point3D& location,
		TriangleAccelerateStructDatabaseStd::TriangleAccelerateStructDatabase* triangleAccelerateStructDatabase) {

		Point3DStd::Point3D vec = Geometry3DOperateStd::SubPoint3DPoint3D(location, pre_location);

		double len = Geometry3DOperateStd::Length_Point3D(vec);
		if (len < GlobalConstantStd::Eps) {
			return false;
		}

		vec = Geometry3DOperateStd::MulDoublePoint3D(1.0 / len, vec);
		Ray3DStd::Ray3D ray(pre_location, vec);

		Ray3DTriangle3DIntersectResultStd::Ray3DTriangle3DIntersectResult ray3DTriangle3DIntersectResult = 
			RayScenarioIntersectStd::RayScenarioTriangle3DIntersect(ray, triangleAccelerateStructDatabase);

		if (ray3DTriangle3DIntersectResult.distance > len - GlobalConstantStd::Eps) {
			return true;
		}

		return false;
	}

	bool CheckByRaysScenarioIntersect(
		TriangleAccelerateStructDatabaseStd::TriangleAccelerateStructDatabase* triangleAccelerateStructDatabase,
		const std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>& path) {

		for (int i = 1; i < path.size(); ++i) {
			Point3DStd::Point3D pre_location = path[i - 1]->location;
			Point3DStd::Point3D location = path[i]->location;
			if (!CheckByRayScenarioIntersect(pre_location, location, triangleAccelerateStructDatabase)) {
				return false;
			}
		}

		return true;

	}




	RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode ToOneMultiPathNodeInfo(
		GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode* geometryRTMultiPathBaseNode) {
		auto type = geometryRTMultiPathBaseNode->GetPropagationType();
		switch (type)
		{
		case PropagationTypeStd::PropagationType::TransmittingAntenna:
		{
			GeometryRTMultiPathTxNodeStd::GeometryRTMultiPathTxNode* ptr = (GeometryRTMultiPathTxNodeStd::GeometryRTMultiPathTxNode*)geometryRTMultiPathBaseNode;
			RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode rayTracingGeometricPathNode(PropagationTypeStd::PropagationType::TransmittingAntenna, ptr->id, ptr->location);
			return rayTracingGeometricPathNode;
		}
			
		case PropagationTypeStd::PropagationType::ReceiverAntenna:
		{
			GeometryRTMultiPathRxNodeStd::GeometryRTMultiPathRxNode* ptr = (GeometryRTMultiPathRxNodeStd::GeometryRTMultiPathRxNode*)geometryRTMultiPathBaseNode;
			RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode rayTracingGeometricPathNode(PropagationTypeStd::PropagationType::ReceiverAntenna, ptr->id, ptr->location);
			return rayTracingGeometricPathNode;
		}

		case PropagationTypeStd::PropagationType::Reflection:
		{
			GeometryRTMultiPathRNodeStd::GeometryRTMultiPathRNode* ptr = (GeometryRTMultiPathRNodeStd::GeometryRTMultiPathRNode*)geometryRTMultiPathBaseNode;
			RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode rayTracingGeometricPathNode(PropagationTypeStd::PropagationType::Reflection, ptr->triangleIndex, ptr->location);
			return rayTracingGeometricPathNode;
		}
		case PropagationTypeStd::PropagationType::Diffraction:
		{

			GeometryRTMultiPathDNodeStd::GeometryRTMultiPathDNode* ptr = (GeometryRTMultiPathDNodeStd::GeometryRTMultiPathDNode*)geometryRTMultiPathBaseNode;
			RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode rayTracingGeometricPathNode(PropagationTypeStd::PropagationType::Diffraction, ptr->cornerIndex, ptr->location);
			return rayTracingGeometricPathNode;
		}

		default:
			break;
		}

		ProjectDependenciesStd::DisplayPromptOrReason("不支持该类型的转化.", true, __FILE__, __LINE__);

		return RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode();
	}

	//对是否是平行入射进行判断
	bool ParallelIncident(const Point3DStd::Point3D& a, const Point3DStd::Point3D& b, const Point3DStd::Point3D& n) {

		auto ab = Geometry3DOperateStd::SubPoint3DPoint3D(b, a);
		double dot = Geometry3DOperateStd::DotPoint3DPoint3D(n, ab);

		if (abs(dot) < GlobalConstantStd::Eps) {
			return true;
		}
		return false;
	}

	void ToMultiPathNodeInfo(
		const ScenarioDataInformationStd::ScenarioDataInformation& scenarioDataInformation,
		const std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& objs,
		std::vector<std::vector<RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode>*>& res) {

		for (int loop = 0; loop< objs.size();++loop) {
			//碰撞检测
			//if (!CheckByRaysScenarioIntersect(scenarioDataInformation.ray3DIntersectScenarioAccelerateStruct, objs[loop])) {
			//	continue;
			//}

			std::vector<RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode>* path =
				new std::vector<RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode>();

			bool check = true;
			{
				RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode node = ToOneMultiPathNodeInfo(objs[loop][0]);
				path->emplace_back(node);
				if (node.type == PropagationTypeStd::PropagationType::Null) {
					check = false;
				}
			}

			for (int i = 1; i < objs[loop].size() - 1; ++i) {
				RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode node = ToOneMultiPathNodeInfo(objs[loop][i]);
				path->emplace_back(node);
				if (node.type == PropagationTypeStd::PropagationType::Null) {
					check = false;
					break;
				}
			}

			{
				RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode node = ToOneMultiPathNodeInfo(objs[loop][(int)objs[loop].size() - 1]);
				path->emplace_back(node);
				if (node.type == PropagationTypeStd::PropagationType::Null) {
					check = false;
				}
			}

			if (check) {
				res.emplace_back(path);
			}
			else {
				path->clear();
				delete path;
			}
		}

	}



}

