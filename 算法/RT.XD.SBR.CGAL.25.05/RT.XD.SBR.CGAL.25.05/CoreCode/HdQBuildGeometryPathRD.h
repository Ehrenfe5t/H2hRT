#pragma once

#include"HdQBuildGeometryPathDR.h"

namespace BuildGeometryPathRDStd {


	/// <summary>
	/// 这里的发射机和接收机是传入的应该是相反的
	/// </summary>
	/// <param name="numbersOfGuess"></param>
	/// <param name="maxLevel"></param>
	/// <param name="geometryPathDInputParameter"></param>
	/// <param name="paths"></param>
	void BuildGeometryPathRD(
		int numbersOfGuess,
		int maxLevel,
		const BuildGeometryPathDRStd::GeometryPathDRInputParameter& geometryPathDInputParameter,
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths);

	/// <summary>
	/// 这里的发射机和接收机是传入的应该是相反的
	/// </summary>
	/// <param name="geometryPathDInputParameter"></param>
	/// <param name="paths"></param>
	void BuildGeometryPathRD(
		const BuildGeometryPathDRStd::GeometryPathDRInputParameter& geometryPathDInputParameter,
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths);

	/// <summary>
	/// 这里的发射机和接收机是传入的应该是相反的
	/// </summary>
	/// <param name="numbersOfGuess"></param>
	/// <param name="maxLevel"></param>
	/// <param name="geometryPathDInputParameters"></param>
	/// <param name="paths"></param>
	void BuildGeometryPathRDs(
		int numbersOfGuess,
		int maxLevel,
		const std::vector<BuildGeometryPathDRStd::GeometryPathDRInputParameter>& geometryPathDInputParameters,
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths);

	/// <summary>
	/// 这里的发射机和接收机是传入的应该是相反的
	/// </summary>
	/// <param name="geometryPathDInputParameters"></param>
	/// <param name="paths"></param>
	void BuildGeometryPathRDs(
		const std::vector<BuildGeometryPathDRStd::GeometryPathDRInputParameter>& geometryPathDInputParameters,
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths);

}