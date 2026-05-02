
#include"QzQDDToMultiPathNodeInfo.h"

#include"HdQBuildGeometryPathDD.h"
#include"HdQBaseToMultiPathNodeInfo.h"
#include"LxQProjectDependencies.h"
#include"QzQGeometry3DOperate.Location.h"

namespace DDToMultiPathNodeInfoStd {


	void FindDDToMultiPathNodeInfo(
		int id_tx,
		int id_rx,
		int numbersOfGuess,
		int maxLevel,
		const Point3DStd::Point3D& tx_location,
		const Point3DStd::Point3D& rx_location,
		const ScenarioDataInformationStd::ScenarioDataInformation& scenarioDataInformation,
		std::vector<std::vector<RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode>*>& res) {


		std::vector<BuildGeometryPathDDStd::GeometryPathDDInputParameter> geometryPathDDInputParameters;
		for (int i = 0; i < scenarioDataInformation.scenarioObject.scenarioCorner3DIndex.size(); ++i) {

			auto scenarioCorner3DIndex1 = scenarioDataInformation.scenarioObject.scenarioCorner3DIndex[i];
			for (int j = 0; j < scenarioDataInformation.scenarioObject.scenarioCorner3DIndex.size(); ++j) {
				if (i == j) {
					continue;
				}
				auto scenarioCorner3DIndex2 = scenarioDataInformation.scenarioObject.scenarioCorner3DIndex[j];

				BuildGeometryPathDDStd::GeometryPathDDInputParameter geometryPathDDInputParameter;
				geometryPathDDInputParameter.cornerIndex1 = i;
				geometryPathDDInputParameter.cornerIndex2 = j;

				geometryPathDDInputParameter.tx_location = tx_location;
				geometryPathDDInputParameter.rx_location = rx_location;
				geometryPathDDInputParameter.id_tx = id_tx;
				geometryPathDDInputParameter.id_rx = id_rx;

				geometryPathDDInputParameter.segment1.start =
					scenarioDataInformation.scenarioObject.scenarioPoint3D[scenarioCorner3DIndex1.P1Index];

				geometryPathDDInputParameter.segment1.end =
					scenarioDataInformation.scenarioObject.scenarioPoint3D[scenarioCorner3DIndex1.P2Index];


				geometryPathDDInputParameter.segment2.start =
					scenarioDataInformation.scenarioObject.scenarioPoint3D[scenarioCorner3DIndex2.P1Index];

				geometryPathDDInputParameter.segment2.end =
					scenarioDataInformation.scenarioObject.scenarioPoint3D[scenarioCorner3DIndex2.P2Index];

				geometryPathDDInputParameters.emplace_back(geometryPathDDInputParameter);
			}
		}


		{
			std::ostringstream oss;
			oss << "组合数量：" << geometryPathDDInputParameters.size();
			ProjectDependenciesStd::DisplayPromptOrReason(oss.str(), false, __FILE__, __LINE__);
		}

		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>> paths;
		BuildGeometryPathDDStd::BuildGeometryPathDDsAndParameter(numbersOfGuess, maxLevel, geometryPathDDInputParameters, paths);

		//
		BaseToMultiPathNodeInfoStd::ToMultiPathNodeInfo(scenarioDataInformation, paths, res);

	}

	/// <summary>
	/// 进行加速
	/// </summary>
	/// <param name="id_tx"></param>
	/// <param name="id_rx"></param>
	/// <param name="numbersOfGuess"></param>
	/// <param name="maxLevel"></param>
	/// <param name="tx_location"></param>
	/// <param name="rx_location"></param>
	/// <param name="scenarioDataInformation"></param>
	/// <param name="res"></param>
	void FindDDToMultiPathNodeInfo_Accelerated(
		int id_tx,
		int id_rx,
		int numbersOfGuess,
		int maxLevel,
		const Point3DStd::Point3D& tx_location,
		const Point3DStd::Point3D& rx_location,
		const ScenarioDataInformationStd::ScenarioDataInformation& scenarioDataInformation,
		std::vector<std::vector<RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode>*>& res) {

		//初始化点和线段的关系计算

		//初始化线段和线段的关系计算

		std::vector<BuildGeometryPathDDStd::GeometryPathDDInputParameter> geometryPathDDInputParameters;
		for (int i = 0; i < scenarioDataInformation.scenarioObject.scenarioCorner3DIndex.size(); ++i) {
			auto scenarioCorner3DIndex1 = scenarioDataInformation.scenarioObject.scenarioCorner3DIndex[i];

			{
				int seg1_triangle1_index = scenarioCorner3DIndex1.Face0Index;
				auto seg1_triangle1 = scenarioDataInformation.scenarioObject.scenarioTriangle3DIndex[seg1_triangle1_index];
				auto seg1_triangle1_p1 = scenarioDataInformation.scenarioObject.scenarioPoint3D[seg1_triangle1.TriangleP1Index];

				char tx_seg1_triangle1_plane_location = Geometry3DOperateStd::Location_Point3D_Plane3D_UpDown_plus(tx_location, seg1_triangle1_p1, seg1_triangle1.n);
				if (tx_seg1_triangle1_plane_location != 1) {
					int seg1_triangle2_index = scenarioCorner3DIndex1.FaceNIndex;
					auto seg1_triangle2 = scenarioDataInformation.scenarioObject.scenarioTriangle3DIndex[seg1_triangle2_index];
					auto seg1_triangle2_p1 = scenarioDataInformation.scenarioObject.scenarioPoint3D[seg1_triangle2.TriangleP1Index];
					char tx_seg2_triangle2_plane_location = Geometry3DOperateStd::Location_Point3D_Plane3D_UpDown_plus(tx_location, seg1_triangle2_p1, seg1_triangle2.n);
					if (tx_seg2_triangle2_plane_location != 1) {
						continue;
					}
				}
			}
			

			auto segment1_start = scenarioDataInformation.scenarioObject.scenarioPoint3D[scenarioCorner3DIndex1.P1Index];
			auto segment1_end = scenarioDataInformation.scenarioObject.scenarioPoint3D[scenarioCorner3DIndex1.P2Index];
			char tx_on_seg1_location = Geometry3DOperateStd::Location_Point3DOfShadowonLineSegment3D_plus(tx_location, segment1_start, segment1_end);

			for (int j = 0; j < scenarioDataInformation.scenarioObject.scenarioCorner3DIndex.size(); ++j) {
				if (i == j) {
					continue;
				}
				auto scenarioCorner3DIndex2 = scenarioDataInformation.scenarioObject.scenarioCorner3DIndex[j];

				{
					int seg2_triangle1_index = scenarioCorner3DIndex2.Face0Index;
					auto seg2_triangle1 = scenarioDataInformation.scenarioObject.scenarioTriangle3DIndex[seg2_triangle1_index];
					auto seg2_triangle1_p1 = scenarioDataInformation.scenarioObject.scenarioPoint3D[seg2_triangle1.TriangleP1Index];

					char rx_seg1_triangle1_plane_location = Geometry3DOperateStd::Location_Point3D_Plane3D_UpDown_plus(rx_location, seg2_triangle1_p1, seg2_triangle1.n);
					if (rx_seg1_triangle1_plane_location != 1) {
						int seg2_triangle2_index = scenarioCorner3DIndex2.FaceNIndex;
						auto seg2_triangle2 = scenarioDataInformation.scenarioObject.scenarioTriangle3DIndex[seg2_triangle2_index];
						auto seg2_triangle2_p1 = scenarioDataInformation.scenarioObject.scenarioPoint3D[seg2_triangle2.TriangleP1Index];
						char rx_seg2_triangle2_plane_location = Geometry3DOperateStd::Location_Point3D_Plane3D_UpDown_plus(rx_location, seg2_triangle2_p1, seg2_triangle2.n);
						if (rx_seg2_triangle2_plane_location != 1) {
							continue;
						}
					}
				}

				auto segment2_start = scenarioDataInformation.scenarioObject.scenarioPoint3D[scenarioCorner3DIndex2.P1Index];
				auto segment2_end = scenarioDataInformation.scenarioObject.scenarioPoint3D[scenarioCorner3DIndex2.P2Index];
				char segment2_start_on_seg1_location = Geometry3DOperateStd::Location_Point3DOfShadowonLineSegment3D_plus(segment2_start, segment1_start, segment1_end);
				char segment2_end_on_seg1_location = Geometry3DOperateStd::Location_Point3DOfShadowonLineSegment3D_plus(segment2_end, segment1_start, segment1_end);
				if (tx_on_seg1_location == 1 || tx_on_seg1_location == 2) {
					if (segment2_start_on_seg1_location == 1 || segment2_start_on_seg1_location == 2) {
						if (segment2_end_on_seg1_location == 1 || segment2_end_on_seg1_location == 2) {
							continue;
						}
					}
				}
				if (tx_on_seg1_location == -1 || tx_on_seg1_location == -2) {
					if (segment2_start_on_seg1_location == -1 || segment2_start_on_seg1_location == -2) {
						if (segment2_end_on_seg1_location == -1 || segment2_end_on_seg1_location == -2) {
							continue;
						}
					}
				}

				char rx_on_seg2_location = Geometry3DOperateStd::Location_Point3DOfShadowonLineSegment3D_plus(rx_location, segment2_start, segment2_end);
				char segment1_start_on_seg2_location = Geometry3DOperateStd::Location_Point3DOfShadowonLineSegment3D_plus(segment1_start, segment2_start, segment2_end);
				char segment1_end_on_seg2_location = Geometry3DOperateStd::Location_Point3DOfShadowonLineSegment3D_plus(segment1_end, segment2_start, segment2_end);

				if (rx_on_seg2_location == 1 || rx_on_seg2_location == 2) {
					if (segment1_start_on_seg2_location == 1 || segment1_start_on_seg2_location == 2) {
						if (segment1_end_on_seg2_location == 1 || segment1_end_on_seg2_location == 2) {
							continue;
						}
					}
				}
				if (rx_on_seg2_location == -1 || rx_on_seg2_location == -2) {
					if (segment1_start_on_seg2_location == -1 || segment1_start_on_seg2_location == -2) {
						if (segment1_end_on_seg2_location == -1 || segment1_end_on_seg2_location == -2) {
							continue;
						}
					}
				}

				BuildGeometryPathDDStd::GeometryPathDDInputParameter geometryPathDDInputParameter;
				geometryPathDDInputParameter.cornerIndex1 = i;
				geometryPathDDInputParameter.cornerIndex2 = j;

				geometryPathDDInputParameter.tx_location = tx_location;
				geometryPathDDInputParameter.rx_location = rx_location;
				geometryPathDDInputParameter.id_tx = id_tx;
				geometryPathDDInputParameter.id_rx = id_rx;

				geometryPathDDInputParameter.segment1.start = segment1_start;

				geometryPathDDInputParameter.segment1.end = segment1_end;


				geometryPathDDInputParameter.segment2.start = segment2_start;

				geometryPathDDInputParameter.segment2.end = segment2_end;

				geometryPathDDInputParameters.emplace_back(geometryPathDDInputParameter);
			}
		}


		{
			std::ostringstream oss;
			oss << "组合数量：" << geometryPathDDInputParameters.size();
			ProjectDependenciesStd::DisplayPromptOrReason(oss.str(), false, __FILE__, __LINE__);
		}

		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>> paths;
		BuildGeometryPathDDStd::BuildGeometryPathDDsAndParameter(numbersOfGuess, maxLevel, geometryPathDDInputParameters, paths);

		//
		BaseToMultiPathNodeInfoStd::ToMultiPathNodeInfo(scenarioDataInformation, paths, res);

	}


}