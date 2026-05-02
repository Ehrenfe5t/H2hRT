#pragma once


#include"QzQGeometryRTMultiPathBaseNode.h"
#include"LxQLineSegment3D.h"

/// <summary>
/// 使用求解约束方程组的思路求解一次绕射计算
/// </summary>
namespace BuildGeometryPathDStd {

	class GeometryPathDInputParameter
	{
	public:
		int id_tx;
		int id_rx;
		int cornerIndex;
		Point3DStd::Point3D tx_location;
		Point3DStd::Point3D rx_location;
		LineSegment3DStd::LineSegment3D segment1;
		GeometryPathDInputParameter();
		GeometryPathDInputParameter(
			int id_tx,
			int id_rx,
			int cornerIndex,
			const Point3DStd::Point3D& tx_location,
			const Point3DStd::Point3D& rx_location,
			const LineSegment3DStd::LineSegment3D& segment1);
		~GeometryPathDInputParameter();

	private:

	};

	void BuildGeometryPathD(
		int numbersOfGuess,
		int maxLevel,
		const GeometryPathDInputParameter& geometryPathDInputParameter,
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths);

	void BuildGeometryPathD(
		const GeometryPathDInputParameter& geometryPathDInputParameter,
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths);

	void BuildGeometryPathDs(
		int numbersOfGuess,
		int maxLevel,
		const std::vector<GeometryPathDInputParameter>& geometryPathDInputParameters,
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths);

	void BuildGeometryPathDs(
		const std::vector<GeometryPathDInputParameter>& geometryPathDInputParameters,
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths);

	void BuildGeometryPathDByAnalyticalSolution(
		const GeometryPathDInputParameter& geometryPathDInputParameter,
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths);

	void BuildGeometryPathDsByAnalyticalSolution(
		const std::vector<GeometryPathDInputParameter>& geometryPathDInputParameters,
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths);

}