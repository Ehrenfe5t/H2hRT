

#include"DxQRayTracingGeometricPathNodeToMultiPathNodeInfo.h"

#include"QzQGlobalConstant.h"

#include"QzQGeometry3DOperate.Equals.h"
#include"QzQGeometry3DOperate.Point3D.h"
#include"QzQGeometry3DOperate.Length.h"
#include"QzQGeometry3DOperate.Location.h"
#include"QzQGeometry3DOperate.Shadow.h"
#include"QzQGeometry3DOperate.Create.h"
#include"QzQGeometry3DOperate.Normalization.h"
#include"QzQGeometry3DOperate.CoordinateSystem.h"


#include"DxQRayScenarioIntersect.h"

#include"LxQMathOperate.h"

#include"LxQMultiPathNodeInfoOperate.Ptr.h"

namespace RayTracingGeometricPathNodeToMultiPathNodeInfoStd {

	bool RayTracingGeometricPathNode_Equals(const RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode& node1,
		const RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode& node2) {
		return Geometry3DOperateStd::Equals_Point3D(node1.location, node2.location);
	}

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
			//表明这两点之间没有遮挡
			return true;
		}

		return false;
	}

	bool CheckByRaysScenarioIntersect(
		TriangleAccelerateStructDatabaseStd::TriangleAccelerateStructDatabase* triangleAccelerateStructDatabase,
		const std::vector<RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode>& path) {
		for (int i = 1; i < path.size(); ++i) {
			Point3DStd::Point3D pre_location = path[i - 1].location;
			Point3DStd::Point3D location = path[i].location;
			if (!CheckByRayScenarioIntersect(pre_location, location, triangleAccelerateStructDatabase)) {
				return false;
			}
		}
		return true;
	}

	Point3DStd::Point3D GetUnitLineSegmentByIndex(
		const ScenarioCorner3DIndexStd::ScenarioCorner3DIndex& face,
		const std::vector<Point3DStd::Point3D>& scenarioPoint3D) {
		Point3DStd::Point3D temp111 = Geometry3DOperateStd::SubPoint3DPoint3D(scenarioPoint3D[face.P2Index], scenarioPoint3D[face.P1Index]);
		return Geometry3DOperateStd::Normalization_Point3D(temp111);
	}



	bool BuildingBaseCoordinateSystemByD(
		const Point3DStd::Point3D& location,
		const Point3DStd::Point3D& r,
		const Point3DStd::Point3D& p3,
		const Point3DStd::Point3D& n,
		const Point3DStd::Point3D& seg_vec1,
		BaseCoordinateSystemStd::BaseCoordinateSystem& res) {
		auto z = n;
		//在控制y轴
		auto oy = Geometry3DOperateStd::CalculateShadowPoint3DOfPoint3DonPlane3D_plus_unsafe(p3, location, seg_vec1);
		auto y = Geometry3DOperateStd::SubPoint3DPoint3D(oy, location);
		res.y = Geometry3DOperateStd::Normalization_Point3D(y);
		res.o = location;
		res.z = z;
		res.x = Geometry3DOperateStd::CrossPoint3DPoint3D(res.y, res.z);

		//if (Point3DStd::DotPoint3DPoint3D(z, r) < 0.0) {
		//	z.x = -z.x;
		//	z.y = -z.y;
		//	z.z = -z.z;
		//}
		//auto baseCoordinateSystem = BaseCoordinateSystemStd::BuildBaseCoordinateSystemByRTDS(location, z, r);
		////这个坐标系中，triangleFace必定在xoy平面内，他的法向量就是z轴(可能是+z，也可能是-z)，
		////将p3在投影到新的坐标系中
		//Point3DStd::Point3D p3_new;
		//if (!BaseCoordinateSystemStd::CoordinatedSystemTransformation(
		//	BaseCoordinateSystemStd::BaseCoordinateSystem(Point3DStd::Point3D(0,0,0), Point3DStd::Point3D(1,0,0), Point3DStd::Point3D(0,1,0), Point3DStd::Point3D(0,0,1)),
		//	p3, baseCoordinateSystem, p3_new)) {
		//	return false;
		//}
		//if (p3_new.y < 0) {
		//	baseCoordinateSystem.y.x = -baseCoordinateSystem.y.x;
		//	baseCoordinateSystem.y.y = -baseCoordinateSystem.y.y;
		//	baseCoordinateSystem.y.z = -baseCoordinateSystem.y.z;
		//	baseCoordinateSystem.x = Point3DStd::CrossPoint3DPoint3D(baseCoordinateSystem.y, baseCoordinateSystem.z);
		//}
		//
		//res = baseCoordinateSystem;
		return true;
	}

	bool BuildByFaceN(
		const Point3DStd::Point3D& pre_location,
		const Point3DStd::Point3D& location,
		const Point3DStd::Point3D& next_location,
		const Point3DStd::Point3D& r,
		const Point3DStd::Point3D& seg_vec1,
		const ScenarioCorner3DIndexStd::ScenarioCorner3DIndex& face,
		const ScenarioDataInformationStd::ScenarioDataInformation& scenarioDataInformation,
		ScenarioTriangle3DIndexStd::ScenarioTriangle3DIndex& face0_real,
		double& phi1,
		double& phi2,
		double& phiE) {
		face0_real = scenarioDataInformation.scenarioObject.scenarioTriangle3DIndex[face.FaceNIndex];
		auto triangleFace0 = face0_real;
		auto triangleFace0_p3 = scenarioDataInformation.scenarioObject.scenarioPoint3D[face.P3FaceNIndex];
		BaseCoordinateSystemStd::BaseCoordinateSystem baseCoordinateSystem;
		if (BuildingBaseCoordinateSystemByD(location, r, triangleFace0_p3, triangleFace0.n, seg_vec1, baseCoordinateSystem)) {
			{
				auto triangleFaceN_p3 = scenarioDataInformation.scenarioObject.scenarioPoint3D[face.P3Face0Index];
				Point3DStd::Point3D triangleFaceN_p3_new;
				if (!Geometry3DOperateStd::CoordinatedSystemTransformation(
					BaseCoordinateSystemStd::BaseCoordinateSystem(Point3DStd::Point3D(0, 0, 0), Point3DStd::Point3D(1, 0, 0), Point3DStd::Point3D(0, 1, 0), Point3DStd::Point3D(0, 0, 1)),
					triangleFaceN_p3, baseCoordinateSystem, triangleFaceN_p3_new)) {
					return false;
				}
				phiE = MathOperateStd::GetAngleByXY(triangleFaceN_p3_new.y, triangleFaceN_p3_new.z);

				Point3DStd::Point3D triangleFace_p1_new;
				if (!Geometry3DOperateStd::CoordinatedSystemTransformation(
					BaseCoordinateSystemStd::BaseCoordinateSystem(Point3DStd::Point3D(0, 0, 0), Point3DStd::Point3D(1, 0, 0), Point3DStd::Point3D(0, 1, 0), Point3DStd::Point3D(0, 0, 1)),
					pre_location, baseCoordinateSystem, triangleFace_p1_new)) {
					return false;
				}
				phi1 = MathOperateStd::GetAngleByXY(triangleFace_p1_new.y, triangleFace_p1_new.z);
				if (phi1 >= phiE) {
					return false;
				}

				Point3DStd::Point3D triangleFace_p2_new;
				if (!Geometry3DOperateStd::CoordinatedSystemTransformation(
					BaseCoordinateSystemStd::BaseCoordinateSystem(Point3DStd::Point3D(0, 0, 0), Point3DStd::Point3D(1, 0, 0), Point3DStd::Point3D(0, 1, 0), Point3DStd::Point3D(0, 0, 1)),
					next_location, baseCoordinateSystem, triangleFace_p2_new)) {
					return false;
				}

				phi2 = MathOperateStd::GetAngleByXY(triangleFace_p2_new.y, triangleFace_p2_new.z);
				if (phi2 >= phiE) {
					return false;
				}
			}
			return true;
		}
		return false;
	}

	bool BuildByFace0(
		const Point3DStd::Point3D& pre_location,
		const Point3DStd::Point3D& location,
		const Point3DStd::Point3D& next_location,
		const Point3DStd::Point3D& r,
		const Point3DStd::Point3D& seg_vec1,
		const ScenarioCorner3DIndexStd::ScenarioCorner3DIndex& face,
		const ScenarioDataInformationStd::ScenarioDataInformation& scenarioDataInformation,
		ScenarioTriangle3DIndexStd::ScenarioTriangle3DIndex& face0_real,
		double& phi1,
		double& phi2,
		double& phiE) {
		face0_real = scenarioDataInformation.scenarioObject.scenarioTriangle3DIndex[face.Face0Index];
		auto triangleFace0 = face0_real;
		auto triangleFace0_p3 = scenarioDataInformation.scenarioObject.scenarioPoint3D[face.P3Face0Index];
		BaseCoordinateSystemStd::BaseCoordinateSystem baseCoordinateSystem;
		if (!BuildingBaseCoordinateSystemByD(location, r, triangleFace0_p3, triangleFace0.n, seg_vec1, baseCoordinateSystem)) {
			return false;
		}
		auto triangleFaceN_p3 = scenarioDataInformation.scenarioObject.scenarioPoint3D[face.P3FaceNIndex];
		Point3DStd::Point3D triangleFaceN_p3_new;
		if (!Geometry3DOperateStd::CoordinatedSystemTransformation(
			BaseCoordinateSystemStd::BaseCoordinateSystem(Point3DStd::Point3D(0, 0, 0), Point3DStd::Point3D(1, 0, 0), Point3DStd::Point3D(0, 1, 0), Point3DStd::Point3D(0, 0, 1)),
			triangleFaceN_p3, baseCoordinateSystem, triangleFaceN_p3_new)) {
			return false;
		}
		phiE = MathOperateStd::GetAngleByXY(triangleFaceN_p3_new.y, triangleFaceN_p3_new.z);

		Point3DStd::Point3D triangleFace_p1_new;
		if (!Geometry3DOperateStd::CoordinatedSystemTransformation(
			BaseCoordinateSystemStd::BaseCoordinateSystem(Point3DStd::Point3D(0, 0, 0), Point3DStd::Point3D(1, 0, 0), Point3DStd::Point3D(0, 1, 0), Point3DStd::Point3D(0, 0, 1)),
			pre_location, baseCoordinateSystem, triangleFace_p1_new)) {
			return false;
		}
		phi1 = MathOperateStd::GetAngleByXY(triangleFace_p1_new.y, triangleFace_p1_new.z);
		if (phi1 >= phiE) {
			return BuildByFaceN(pre_location, location, next_location, r, seg_vec1, face, scenarioDataInformation, face0_real, phi1, phi2, phiE);
		}

		Point3DStd::Point3D triangleFace_p2_new;
		if (!Geometry3DOperateStd::CoordinatedSystemTransformation(
			BaseCoordinateSystemStd::BaseCoordinateSystem(Point3DStd::Point3D(0, 0, 0), Point3DStd::Point3D(1, 0, 0), Point3DStd::Point3D(0, 1, 0), Point3DStd::Point3D(0, 0, 1)),
			next_location, baseCoordinateSystem, triangleFace_p2_new)) {
			return false;
		}

		phi2 = MathOperateStd::GetAngleByXY(triangleFace_p2_new.y, triangleFace_p2_new.z);
		if (phi2 >= phiE) {
			return BuildByFaceN(pre_location, location, next_location, r, seg_vec1, face, scenarioDataInformation, face0_real, phi1, phi2, phiE);
		}
		double t2 = phiE - phi1;
		if (t2 < phi1) {
			return BuildByFaceN(pre_location, location, next_location, r, seg_vec1, face, scenarioDataInformation, face0_real, phi1, phi2, phiE);
		}
		return true;
	}

	MultiPathNodeInfoStd::MultiPathNodeInfo* RayTracingGeometricPathNodeToMultiPathNodeInfo_Node_Diffraction(
		const RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode& rayTracingGeometricPathNode,
		const Point3DStd::Point3D& pre_location,
		const Point3DStd::Point3D& next_location,
		ScenarioDataInformationStd::ScenarioDataInformation* scenarioDataInformation) {

		auto location = rayTracingGeometricPathNode.location;
		auto face = scenarioDataInformation->scenarioObject.scenarioCorner3DIndex[rayTracingGeometricPathNode.nodeElementId];
		Point3DStd::Point3D r = Geometry3DOperateStd::SubPoint3DPoint3D(location, pre_location);

		double d1 = Geometry3DOperateStd::DotPoint3DPoint3D(r, scenarioDataInformation->scenarioObject.scenarioTriangle3DIndex[face.Face0Index].n);
		double d2 = Geometry3DOperateStd::DotPoint3DPoint3D(r, scenarioDataInformation->scenarioObject.scenarioTriangle3DIndex[face.FaceNIndex].n);
		//排除掉内部的绕射
		if (d1 >= GlobalConstantStd::Eps && d2 >= GlobalConstantStd::Eps) {
			return new MultiPathNodeInfoStd::MultiPathNodeInfo();
		}


		// 入射线与棱线的夹角 
		Point3DStd::Point3D seg_vec1 = GetUnitLineSegmentByIndex(face,
			scenarioDataInformation->scenarioObject.scenarioPoint3D);
		if (Geometry3DOperateStd::DotPoint3DPoint3D(seg_vec1, r) < 0) {
			seg_vec1.x = -seg_vec1.x;
			seg_vec1.y = -seg_vec1.y;
			seg_vec1.z = -seg_vec1.z;
		}

		{
			auto z1 = Geometry3DOperateStd::CalculateShadowPoint3DOfPoint3DonLine3D_plus_unsafe(pre_location, location, seg_vec1);
			auto z2 = Geometry3DOperateStd::CalculateShadowPoint3DOfPoint3DonLine3D_plus_unsafe(next_location, location, seg_vec1);
			if (!Geometry3DOperateStd::Location_Point3DonLineSegment3D_plus_2(location, z1, z2,0.02)) {
				return new MultiPathNodeInfoStd::MultiPathNodeInfo();
			}
		}


		ScenarioTriangle3DIndexStd::ScenarioTriangle3DIndex face0_real;
		double phi1, phi2, phiE;
		if (!BuildByFace0(pre_location, location, next_location, r, seg_vec1, face, *scenarioDataInformation, face0_real, phi1, phi2, phiE)) {
			return MultiPathNodeInfoOperateStd::CreateMultiPathNodeInfoPtr();
		}

		double beta = Geometry3DOperateStd::GetAnglePoint3DPoint3D(r, seg_vec1);

		int upObjectType = face0_real.UpTypeNumber;
		int downObjectType = face0_real.DownTypeNumber;
		Point3DStd::Point3D normalVector = face0_real.n;

		MultiPathNodeInfoStd::MultiPathNodeInfoDiffraction* ptr =
			MultiPathNodeInfoOperateStd::CreateMultiPathNodeInfoDiffraction(
				upObjectType, downObjectType, beta,  phiE,  phi1,  phi2,
				location, normalVector);
		ptr->attachmentId = rayTracingGeometricPathNode.nodeElementId;
		return MultiPathNodeInfoOperateStd::MultiPathNodeInfoDiffraction_ptr_to_MultiPathNodeInfo_ptr(ptr);
	}


	MultiPathNodeInfoStd::MultiPathNodeInfo* RayTracingGeometricPathNodeToMultiPathNodeInfo_Node_DiffuseScattering(
		int widthCoefficientOfDiffuseLobe, double area, double scatteringCoefficient,
		const RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode& rayTracingGeometricPathNode,
		const Point3DStd::Point3D& pre_location,
		const Point3DStd::Point3D& next_location,
		ScenarioDataInformationStd::ScenarioDataInformation* scenarioDataInformation) {

		auto location = rayTracingGeometricPathNode.location;
		auto face = scenarioDataInformation->scenarioObject.scenarioTriangle3DIndex[rayTracingGeometricPathNode.nodeElementId];

		int upObjectType = face.UpTypeNumber;
		int downObjectType = face.DownTypeNumber;
		Point3DStd::Point3D normalVector = face.n;

		Point3DStd::Point3D vec = Geometry3DOperateStd::SubPoint3DPoint3D(location, pre_location);
		double thetai = Geometry3DOperateStd::GetThetaI(vec, normalVector);

		if (Geometry3DOperateStd::IsZeroAbs(thetai)) {
			thetai = 0.0;
		}
		else if (thetai > 89.5 * GlobalConstantStd::Pi / 180.0) {
			return new MultiPathNodeInfoStd::MultiPathNodeInfo();
		}

		float roughness = (float)face.Roughness;

		Point3DStd::Point3D vec2 = Geometry3DOperateStd::SubPoint3DPoint3D(next_location, location);
		double thetai2 = Geometry3DOperateStd::GetThetaI(vec2, normalVector);


		double beta = abs(thetai- thetai2);


		MultiPathNodeInfoStd::MultiPathNodeInfoDiffuseScattering* ptr =
			MultiPathNodeInfoOperateStd::CreateMultiPathNodeInfoDiffuseScattering(
				upObjectType, downObjectType,
				widthCoefficientOfDiffuseLobe,  roughness,  thetai,  beta,  area,  scatteringCoefficient,
				location, normalVector);
		ptr->attachmentId = rayTracingGeometricPathNode.nodeElementId;
		return MultiPathNodeInfoOperateStd::MultiPathNodeInfoDiffuseScattering_ptr_to_MultiPathNodeInfo_ptr(ptr);
	}


	MultiPathNodeInfoStd::MultiPathNodeInfo* RayTracingGeometricPathNodeToMultiPathNodeInfo_Node(
		const RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode& rayTracingGeometricPathNode,
		const Point3DStd::Point3D& pre_location,
		const Point3DStd::Point3D& next_location,
		ScenarioDataInformationStd::ScenarioDataInformation* scenarioDataInformation) {

		auto type = rayTracingGeometricPathNode.type;
		switch (type)
		{
		case PropagationTypeStd::PropagationType::TransmittingAntenna:
		{
			MultiPathNodeInfoStd::MultiPathNodeInfoTransmittingAntenna* ptr = 
				MultiPathNodeInfoOperateStd::CreateMultiPathNodeInfoTransmittingAntenna(rayTracingGeometricPathNode.nodeElementId, rayTracingGeometricPathNode.location);
			ptr->attachmentId = rayTracingGeometricPathNode.nodeElementId;
			return MultiPathNodeInfoOperateStd::MultiPathNodeInfoTransmittingAntenna_ptr_to_MultiPathNodeInfo_ptr(ptr);
		}
		case PropagationTypeStd::PropagationType::ReceiverAntenna:
		{
			MultiPathNodeInfoStd::MultiPathNodeInfoReceiverAntenna* ptr =
				MultiPathNodeInfoOperateStd::CreateMultiPathNodeInfoReceiverAntenna(rayTracingGeometricPathNode.nodeElementId, rayTracingGeometricPathNode.location);
			ptr->attachmentId = rayTracingGeometricPathNode.nodeElementId;
			return MultiPathNodeInfoOperateStd::MultiPathNodeInfoReceiverAntenna_ptr_to_MultiPathNodeInfo_ptr(ptr);
		}
		case PropagationTypeStd::PropagationType::Reflection:
		{
			Point3DStd::Point3D location = rayTracingGeometricPathNode.location;
			int triangleIndex = rayTracingGeometricPathNode.nodeElementId;
			int upObjectType = scenarioDataInformation->scenarioObject.scenarioTriangle3DIndex[triangleIndex].UpTypeNumber;
			int downObjectType = scenarioDataInformation->scenarioObject.scenarioTriangle3DIndex[triangleIndex].DownTypeNumber;
			Point3DStd::Point3D normalVector = scenarioDataInformation->scenarioObject.scenarioTriangle3DIndex[triangleIndex].n;

			Point3DStd::Point3D vec = Geometry3DOperateStd::SubPoint3DPoint3D(location, pre_location);
			double thetai = Geometry3DOperateStd::GetThetaI(vec, normalVector);

			if (Geometry3DOperateStd::IsZeroAbs(thetai)) {
				thetai = 0.0;
			}
			else if (thetai > 89.5 * GlobalConstantStd::Pi / 180.0) {
				return new MultiPathNodeInfoStd::MultiPathNodeInfo();
			}
			MultiPathNodeInfoStd::MultiPathNodeInfoReflection* ptr =
				MultiPathNodeInfoOperateStd::CreateMultiPathNodeInfoReflection(upObjectType, downObjectType, thetai, location, normalVector);
			ptr->attachmentId = rayTracingGeometricPathNode.nodeElementId;
			return MultiPathNodeInfoOperateStd::MultiPathNodeInfoReflection_ptr_to_MultiPathNodeInfo_ptr(ptr);
		}
		case PropagationTypeStd::PropagationType::Transmission:
		{
			Point3DStd::Point3D location = rayTracingGeometricPathNode.location;
			int triangleIndex = rayTracingGeometricPathNode.nodeElementId;
			int upObjectType = scenarioDataInformation->scenarioObject.scenarioTriangle3DIndex[triangleIndex].UpTypeNumber;
			int downObjectType = scenarioDataInformation->scenarioObject.scenarioTriangle3DIndex[triangleIndex].DownTypeNumber;
			Point3DStd::Point3D normalVector = scenarioDataInformation->scenarioObject.scenarioTriangle3DIndex[triangleIndex].n;

			Point3DStd::Point3D vec = Geometry3DOperateStd::SubPoint3DPoint3D(location, pre_location);
			double thetai = Geometry3DOperateStd::GetThetaI(vec, normalVector);

			if (Geometry3DOperateStd::IsZeroAbs(thetai)) {
				thetai = 0.0;
			}
			else if (thetai > 89.5 * GlobalConstantStd::Pi / 180.0) {
				return new MultiPathNodeInfoStd::MultiPathNodeInfo();
			}
			MultiPathNodeInfoStd::MultiPathNodeInfoTransmission* ptr =
				MultiPathNodeInfoOperateStd::CreateMultiPathNodeInfoTransmission(upObjectType, downObjectType, thetai, location, normalVector);
			ptr->attachmentId = rayTracingGeometricPathNode.nodeElementId;
			return MultiPathNodeInfoOperateStd::MultiPathNodeInfoTransmission_ptr_to_MultiPathNodeInfo_ptr(ptr);
		}
		case PropagationTypeStd::PropagationType::Diffraction:
			return RayTracingGeometricPathNodeToMultiPathNodeInfo_Node_Diffraction(
				rayTracingGeometricPathNode, pre_location, next_location, scenarioDataInformation);
		case PropagationTypeStd::PropagationType::DiffuseScattering:
			return RayTracingGeometricPathNodeToMultiPathNodeInfo_Node_DiffuseScattering(
				2,  1.0,  0.5,
				rayTracingGeometricPathNode, pre_location, next_location, scenarioDataInformation);
		case PropagationTypeStd::PropagationType::Null:
			break;
		default:
			break;
		}
		return new MultiPathNodeInfoStd::MultiPathNodeInfo();
	}



	bool RayTracingGeometricPathNodeToMultiPathNodeInfo_Path(
		const std::vector<RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode>& input,
		ScenarioDataInformationStd::ScenarioDataInformation* scenarioDataInformation, 
		std::vector<MultiPathNodeInfoStd::MultiPathNodeInfo*>& res)
	{
		if (input.size() < 2) {
			return false;
		}
		std::vector<RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode> nodes;
		nodes.emplace_back(input[0]);
		for (int i = 1; i < input.size(); ++i) {
			//相邻空间位置点一样，则删除后来的点
			if (!RayTracingGeometricPathNode_Equals(nodes[(int)nodes.size() - 1], input[i])) {
				nodes.emplace_back(input[i]);
			}
		}

		bool isTransmission = false;
		if (nodes.size() < 2) {
			return false;
		}
		for (int i = 0; i < nodes.size(); ++i) {
			if (nodes[i].type == PropagationTypeStd::PropagationType::Transmission) {
				isTransmission = !isTransmission;
			}
			else {
				//不允许折射不成对出现
				if (isTransmission) {
					return false;
				}
			}
		}

		for (int i = 1; i < (int)nodes.size()-1; ++i) {
			if (nodes[i].type != PropagationTypeStd::PropagationType::Diffraction) {
				continue;
			}
			Point3DStd::Point3D pre_location= nodes[i - 1].location;
			Point3DStd::Point3D next_location= nodes[i + 1].location;
			int cornerIndex = nodes[i].nodeElementId;

			auto scenarioCorner = scenarioDataInformation->scenarioObject.scenarioCorner3DIndex[cornerIndex];
			Point3DStd::Point3D start_seg = scenarioDataInformation->scenarioObject.scenarioPoint3D[scenarioCorner.P1Index];
			Point3DStd::Point3D end_seg = scenarioDataInformation->scenarioObject.scenarioPoint3D[scenarioCorner.P2Index];
			Line3DStd::Line3D line;
			if (!Geometry3DOperateStd::CreateLine3DByTwoPoint3D_safe(start_seg, end_seg, line)) {
				return false;
			}
			Point3DStd::Point3D p1 = Geometry3DOperateStd::CalculateShadowPoint3DOfPoint3DonLine3D_unsafe(pre_location, line);
			Point3DStd::Point3D p2 = Geometry3DOperateStd::CalculateShadowPoint3DOfPoint3DonLine3D_unsafe(next_location, line);
			//必须保证
			//if (!Geometry3DOperateStd::Location_Point3DonLineSegment3D_plus(nodes[i].location, p1, p2)) {
			//	return false;
			//}

			//由于计算误差必须让这个点准许有误差
			if (!Geometry3DOperateStd::Location_Point3DonLineSegment3D_plus_2(nodes[i].location, p1, p2, 0.02)) {
				return false;
			}

			auto n1 = scenarioDataInformation->scenarioObject.scenarioTriangle3DIndex[scenarioCorner.Face0Index].n;
			auto n2 = scenarioDataInformation->scenarioObject.scenarioTriangle3DIndex[scenarioCorner.FaceNIndex].n;

			auto vec1 = Geometry3DOperateStd::SubPoint3DPoint3D(pre_location, nodes[i].location);
			auto vec2 = Geometry3DOperateStd::SubPoint3DPoint3D(next_location, nodes[i].location);

			double d11 = Geometry3DOperateStd::DotPoint3DPoint3D(vec1, n1);
			double d12 = Geometry3DOperateStd::DotPoint3DPoint3D(vec1, n2);
			if (d11 < 0.0 && d12 < 0.0) {
				return false;
			}

			double d21 = Geometry3DOperateStd::DotPoint3DPoint3D(vec2, n1);
			double d22 = Geometry3DOperateStd::DotPoint3DPoint3D(vec2, n2);
			if (d21 < 0.0 && d22 < 0.0) {
				return false;
			}
		}

		//if (!CheckByRaysScenarioIntersect(scenarioDataInformation->ray3DIntersectScenarioAccelerateStruct, nodes)) {
		//	return false;
		//}

		res.clear();
		for (int i = 0; i < nodes.size(); ++i) {
			Point3DStd::Point3D pre_location;
			if (i > 0) {
				pre_location = nodes[i - 1].location;
			}
			Point3DStd::Point3D next_location;
			if (i < (int)nodes.size() - 1) {
				next_location = nodes[i + 1].location;
			}
			MultiPathNodeInfoStd::MultiPathNodeInfo* multiPathNodeInfo = RayTracingGeometricPathNodeToMultiPathNodeInfo_Node(
				nodes[i], pre_location, next_location, scenarioDataInformation);
			res.emplace_back(multiPathNodeInfo);
			if (multiPathNodeInfo->type == PropagationTypeStd::PropagationType::Null) {
				return false;
			}
		}

		return true;
	}
}
