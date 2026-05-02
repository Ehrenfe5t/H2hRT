#pragma once

#include"QzQGeometryRTMultiPathBaseNode.h"
#include"LxQLineSegment3D.h"
#include"QzQEquation2VariableObject.h"

namespace BuildGeometryPathDDStd {


	class GeometryPathDDInputParameter
	{
	public:
		int id_tx;
		int id_rx;
		int cornerIndex1;
		int cornerIndex2;
		Point3DStd::Point3D tx_location;
		Point3DStd::Point3D rx_location;
		LineSegment3DStd::LineSegment3D segment1;
		LineSegment3DStd::LineSegment3D segment2;
		GeometryPathDDInputParameter();
		GeometryPathDDInputParameter(
			int id_tx,
			int id_rx,
			int cornerIndex1,
			int cornerIndex2, 
			const Point3DStd::Point3D& tx_location,
			const Point3DStd::Point3D& rx_location,
			const LineSegment3DStd::LineSegment3D& segment1,
			const LineSegment3DStd::LineSegment3D& segment2);
		~GeometryPathDDInputParameter();

	private:

	};

	void Test(const Equation2VariableObjectStd::Equation2VariableObject& equation2VariableObject,
		const GeometryPathDDInputParameter& geometryPathDRInputParameter);

	void BuildGeometryPathDDCoreNoTxAndRx(
		int maxLevel,
		const std::vector<Equation2VariableObjectStd::Equation2VariableObject>& variableObjectsOfGuess,
		const GeometryPathDDInputParameter& geometryPathDRInputParameter,
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths);

	void BuildGeometryPathDD(
		int numbersOfGuess,
		int maxLevel,
		const GeometryPathDDInputParameter& geometryPathDInputParameter,
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths);

	void BuildGeometryPathDD(
		const GeometryPathDDInputParameter& geometryPathDInputParameter,
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths);

	void BuildGeometryPathDDsAndParameter(
		int numbersOfGuess,
		int maxLevel,
		const std::vector<GeometryPathDDInputParameter>& geometryPathDInputParameters,
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths);

	void BuildGeometryPathDDs(
		const std::vector<GeometryPathDDInputParameter>& geometryPathDInputParameters,
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths);


}