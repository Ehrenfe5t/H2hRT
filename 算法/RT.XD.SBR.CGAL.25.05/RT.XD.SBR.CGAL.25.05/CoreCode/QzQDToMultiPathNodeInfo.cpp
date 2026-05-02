


#include"QzQDToMultiPathNodeInfo.h"

#include"HdQBuildGeometryPathD.h"
#include"HdQBaseToMultiPathNodeInfo.h"
#include"QzQGeometryRTMultiPathTxNode.h"
#include"QzQGeometryRTMultiPathRxNode.h"
#include"QzQGeometryRTMultiPathDNode.h"
#include"QzQGeometry3DOperate.Location.h"

namespace DToMultiPathNodeInfoStd {

	void FreeVectorGeometryRTMultiPathBaseNode(std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>& paths) {

		while (!paths.empty()) {
			GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode* element = paths.back();
			paths.pop_back();
			switch (element->GetPropagationType())
			{
			case PropagationTypeStd::PropagationType::TransmittingAntenna:
			{
				GeometryRTMultiPathTxNodeStd::GeometryRTMultiPathTxNode* ptr = (GeometryRTMultiPathTxNodeStd::GeometryRTMultiPathTxNode*)element;
				delete ptr;
				break;
			}
			case PropagationTypeStd::PropagationType::ReceiverAntenna:
			{
				GeometryRTMultiPathRxNodeStd::GeometryRTMultiPathRxNode* ptr = (GeometryRTMultiPathRxNodeStd::GeometryRTMultiPathRxNode*)element;
				delete ptr;
				break;
			}
			case PropagationTypeStd::PropagationType::Diffraction:
			{
				GeometryRTMultiPathDNodeStd::GeometryRTMultiPathDNode* ptr = (GeometryRTMultiPathDNodeStd::GeometryRTMultiPathDNode*)element;
				delete ptr;
				break;
			}
			default:
			{
				delete element;
				break;
			}
			}
		}
		paths.clear();
	}

	void FreeVectorVectorGeometryRTMultiPathBaseNode(std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths) {

		while (!paths.empty()) {
			std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*> element = paths.back();
			paths.pop_back();
			FreeVectorGeometryRTMultiPathBaseNode(element);
		}
		paths.clear();
	}

	void FindDToMultiPathNodeInfo(
		int id_tx,
		int id_rx,
		const Point3DStd::Point3D& tx_location,
		const Point3DStd::Point3D& rx_location,
		const ScenarioDataInformationStd::ScenarioDataInformation& scenarioDataInformation,
		std::vector<std::vector<RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode>*>& res) {

		std::vector<BuildGeometryPathDStd::GeometryPathDInputParameter> geometryPathDInputParameters;
		for (int i = 0; i < scenarioDataInformation.scenarioObject.scenarioCorner3DIndex.size(); ++i) {
			BuildGeometryPathDStd::GeometryPathDInputParameter geometryPathDInputParameter;
			geometryPathDInputParameter.id_tx = id_tx;
			geometryPathDInputParameter.id_rx = id_rx;
			geometryPathDInputParameter.cornerIndex = i;
			geometryPathDInputParameter.tx_location = tx_location;
			geometryPathDInputParameter.rx_location = rx_location;
			auto scenarioCorner3DIndex = scenarioDataInformation.scenarioObject.scenarioCorner3DIndex[i];

			geometryPathDInputParameter.segment1.start =
				scenarioDataInformation.scenarioObject.scenarioPoint3D[scenarioCorner3DIndex.P1Index];

			geometryPathDInputParameter.segment1.end =
				scenarioDataInformation.scenarioObject.scenarioPoint3D[scenarioCorner3DIndex.P2Index];

			geometryPathDInputParameters.emplace_back(geometryPathDInputParameter);

		}
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>> paths;

		{
			//std::ostringstream oss;
			//oss << "组合数量：" << geometryPathDInputParameters.size();
			//ProjectDependenciesStd::DisplayPromptOrReason(oss.str(), false, __FILE__, __LINE__);
		}

		BuildGeometryPathDStd::BuildGeometryPathDsByAnalyticalSolution(geometryPathDInputParameters, paths);

		GeometryRTMultiPathBaseNodeStd::DeleteSamePath(paths);

		//
		BaseToMultiPathNodeInfoStd::ToMultiPathNodeInfo(scenarioDataInformation, paths, res);

		FreeVectorVectorGeometryRTMultiPathBaseNode(paths);
	}



	void FindDToMultiPathNodeInfo_Accelerated(
		int id_tx,
		int id_rx,
		const Point3DStd::Point3D& tx_location,
		const Point3DStd::Point3D& rx_location,
		const ScenarioDataInformationStd::ScenarioDataInformation& scenarioDataInformation,
		std::vector<std::vector<RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode>*>& res) {

		std::vector<BuildGeometryPathDStd::GeometryPathDInputParameter> geometryPathDInputParameters;
		for (int i = 0; i < scenarioDataInformation.scenarioObject.scenarioCorner3DIndex.size(); ++i) {



			auto scenarioCorner3DIndex = scenarioDataInformation.scenarioObject.scenarioCorner3DIndex[i];
			auto segment_start = scenarioDataInformation.scenarioObject.scenarioPoint3D[scenarioCorner3DIndex.P1Index];
			auto segment_end = scenarioDataInformation.scenarioObject.scenarioPoint3D[scenarioCorner3DIndex.P2Index];

			//if (segment_start.y < 250) {
			//	continue;
			//}
			//
			//if (segment_end.y < 250) {
			//	continue;
			//}
			//
			//if (segment_start.y > 300) {
			//	continue;
			//}
			//
			//if (segment_end.y > 300) {
			//	continue;
			//}
			//
			//if (segment_start.x < 650) {
			//	continue;
			//}
			//
			//if (segment_end.x < 650) {
			//	continue;
			//}
			//
			//if (segment_start.x > 700) {
			//	continue;
			//}
			//
			//if (segment_end.x > 700) {
			//	continue;
			//}

			//if (segment_start.x > 13 && segment_start.x < 15) {
			//	if (segment_start.y > 4 && segment_start.y < 4.2) {
			//		if (segment_end.x > 13 && segment_end.x < 15) {
			//			if (segment_end.y > 4 && segment_end.y < 4.2) {
			//				double zz = segment_end.z;
			//			}
			//			else {
			//				continue;
			//			}
			//		}
			//		else {
			//			continue;
			//		}
			//	}
			//	else {
			//		continue;
			//	}
			//}
			//else {
			//	continue;
			//}

			int seg_triangle1_index = scenarioCorner3DIndex.Face0Index;
			auto seg_triangle1 = scenarioDataInformation.scenarioObject.scenarioTriangle3DIndex[seg_triangle1_index];
			auto seg_triangle1_p1 = scenarioDataInformation.scenarioObject.scenarioPoint3D[seg_triangle1.TriangleP1Index];
			int seg_triangle2_index = scenarioCorner3DIndex.FaceNIndex;
			auto seg_triangle2 = scenarioDataInformation.scenarioObject.scenarioTriangle3DIndex[seg_triangle2_index];
			auto seg_triangle2_p1 = scenarioDataInformation.scenarioObject.scenarioPoint3D[seg_triangle2.TriangleP1Index];

			{
				char tx_seg_triangle1_plane_location = Geometry3DOperateStd::Location_Point3D_Plane3D_UpDown_plus(tx_location, seg_triangle1_p1, seg_triangle1.n);
				if (tx_seg_triangle1_plane_location != 1) {
					char tx_seg_triangle2_plane_location = Geometry3DOperateStd::Location_Point3D_Plane3D_UpDown_plus(tx_location, seg_triangle2_p1, seg_triangle2.n);
					if (tx_seg_triangle2_plane_location != 1) {
						continue;
					}
				}
			}

			{
				char rx_seg_triangle1_plane_location = Geometry3DOperateStd::Location_Point3D_Plane3D_UpDown_plus(rx_location, seg_triangle1_p1, seg_triangle1.n);
				if (rx_seg_triangle1_plane_location != 1) {
					char rx_seg_triangle2_plane_location = Geometry3DOperateStd::Location_Point3D_Plane3D_UpDown_plus(rx_location, seg_triangle2_p1, seg_triangle2.n);
					if (rx_seg_triangle2_plane_location != 1) {
						continue;
					}
				}
			}


			char tx_on_seg_location = Geometry3DOperateStd::Location_Point3DOfShadowonLineSegment3D_plus(tx_location, segment_start, segment_end);
			char rx_on_seg_location = Geometry3DOperateStd::Location_Point3DOfShadowonLineSegment3D_plus(rx_location, segment_start, segment_end);


			if (tx_on_seg_location == 1) {
				if (rx_on_seg_location == 2) {
					continue;
				}
			}
			if (tx_on_seg_location == 2) {
				if (rx_on_seg_location == 1 || rx_on_seg_location == 2) {
					continue;
				}
			}

			if (tx_on_seg_location == -1) {
				if (rx_on_seg_location == -2) {
					continue;
				}
			}
			if (tx_on_seg_location == -2) {
				if (rx_on_seg_location == -1 || rx_on_seg_location == -2) {
					continue;
				}
			}

			BuildGeometryPathDStd::GeometryPathDInputParameter geometryPathDInputParameter;
			geometryPathDInputParameter.id_tx = id_tx;
			geometryPathDInputParameter.id_rx = id_rx;
			geometryPathDInputParameter.cornerIndex = i;
			geometryPathDInputParameter.tx_location = tx_location;
			geometryPathDInputParameter.rx_location = rx_location;

			geometryPathDInputParameter.segment1.start =
				scenarioDataInformation.scenarioObject.scenarioPoint3D[scenarioCorner3DIndex.P1Index];

			geometryPathDInputParameter.segment1.end =
				scenarioDataInformation.scenarioObject.scenarioPoint3D[scenarioCorner3DIndex.P2Index];

			geometryPathDInputParameters.emplace_back(geometryPathDInputParameter);

		}
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>> paths;

		{
			//std::ostringstream oss;
			//oss << "组合数量：" << geometryPathDInputParameters.size();
			//ProjectDependenciesStd::DisplayPromptOrReason(oss.str(), false, __FILE__, __LINE__);
		}

		BuildGeometryPathDStd::BuildGeometryPathDsByAnalyticalSolution(geometryPathDInputParameters, paths);

		GeometryRTMultiPathBaseNodeStd::DeleteSamePath(paths);

		//
		BaseToMultiPathNodeInfoStd::ToMultiPathNodeInfo(scenarioDataInformation, paths, res);

		FreeVectorVectorGeometryRTMultiPathBaseNode(paths);

	}
	

}

