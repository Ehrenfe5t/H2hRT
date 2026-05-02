#pragma once


#include"QzQGeometryRTMultiPathBaseNode.h"
#include"LxQLineSegment3D.h"
#include"DxQTriangle3D.h"
#include"QzQEquation3VariableObject.h"

namespace BuildGeometryPathDRStd {

	class GeometryPathDRInputParameter
	{
	public:
		int id_tx;
		int id_rx;
		int cornerIndex;
		int triangleIndex;
		Point3DStd::Point3D tx_location;
		Point3DStd::Point3D rx_location;
		Point3DStd::Point3D triangle1_n;
		LineSegment3DStd::LineSegment3D segment1;
		Triangle3DStd::Triangle3D triangle1;
		GeometryPathDRInputParameter();
		GeometryPathDRInputParameter(
			int id_tx,
			int id_rx, 
			int cornerIndex, 
			int triangleIndex,
			const Point3DStd::Point3D& tx_location,
			const Point3DStd::Point3D& rx_location,
			const Point3DStd::Point3D& triangle1_n,
			const LineSegment3DStd::LineSegment3D& segment1,
			const Triangle3DStd::Triangle3D& triangle1);
		~GeometryPathDRInputParameter();

	private:

	};

	void BuildGeometryPathDRCoreNoTxAndRx(
		int maxLevel,
		const std::vector<Equation3VariableObjectStd::Equation3VariableObject>& variableObjectsOfGuess,
		const GeometryPathDRInputParameter& geometryPathDRInputParameter,
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths);

	void BuildGeometryPathDR(
		int numbersOfGuess,
		int maxLevel,
		const GeometryPathDRInputParameter& geometryPathDInputParameter,
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths);

	void BuildGeometryPathDR(
		const GeometryPathDRInputParameter& geometryPathDInputParameter,
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths);

	void BuildGeometryPathDRs(
		int numbersOfGuess,
		int maxLevel,
		const std::vector<GeometryPathDRInputParameter>& geometryPathDInputParameters,
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths);

	void BuildGeometryPathDRs(
		const std::vector<GeometryPathDRInputParameter>& geometryPathDInputParameters,
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths);

}