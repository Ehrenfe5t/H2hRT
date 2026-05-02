
#include"HdQBuildGeometryPathRD.h"
#include"QzQGeometryRTMultiPathDNode.h"
#include"QzQGeometryRTMultiPathRNode.h"
#include"QzQGeometryRTMultiPathTxNode.h"
#include"QzQGeometryRTMultiPathRxNode.h"

namespace BuildGeometryPathRDStd {


	void BuildGeometryPathRDCore(
		int maxLevel,
		const std::vector<Equation3VariableObjectStd::Equation3VariableObject>& variableObjectsOfGuess,
		const BuildGeometryPathDRStd::GeometryPathDRInputParameter& geometryPathDRInputParameter,
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths) {
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>> results;
		BuildGeometryPathDRStd::BuildGeometryPathDRCoreNoTxAndRx(maxLevel, variableObjectsOfGuess, geometryPathDRInputParameter, results);

		if (results.size()>0) {
			GeometryRTMultiPathTxNodeStd::GeometryRTMultiPathTxNode* txNode = new GeometryRTMultiPathTxNodeStd::GeometryRTMultiPathTxNode(geometryPathDRInputParameter.id_rx,geometryPathDRInputParameter.rx_location);
			GeometryRTMultiPathRxNodeStd::GeometryRTMultiPathRxNode* rxNode = new GeometryRTMultiPathRxNodeStd::GeometryRTMultiPathRxNode(geometryPathDRInputParameter.id_tx,geometryPathDRInputParameter.tx_location);

			for (int i = 0; i < results.size(); ++i) {

				std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*> path;
				path.emplace_back(txNode);
				path.emplace_back(results[i][1]);
				path.emplace_back(results[i][0]);
				path.emplace_back(rxNode);
				paths.emplace_back(path);
			}
		}
	}

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
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths) {
		//默认0-1
		RangeDoubleStd::Range3Double range3Double;
		//初始化猜测得值
		auto variableObjectsOfGuess = Equation3VariableObjectStd::GenerateSelfVariableObjectsByGridMethod(
			numbersOfGuess, range3Double);
		BuildGeometryPathRDCore(maxLevel, variableObjectsOfGuess, geometryPathDInputParameter, paths);
	}

	/// <summary>
	/// 这里的发射机和接收机是传入的应该是相反的
	/// </summary>
	/// <param name="geometryPathDInputParameter"></param>
	/// <param name="paths"></param>
	void BuildGeometryPathRD(
		const BuildGeometryPathDRStd::GeometryPathDRInputParameter& geometryPathDInputParameter,
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths) {
		BuildGeometryPathRD(3, 10, geometryPathDInputParameter, paths);
	}

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
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths) {
		//默认0-1
		RangeDoubleStd::Range3Double range3Double;
		//初始化猜测得值
		auto variableObjectsOfGuess = Equation3VariableObjectStd::GenerateSelfVariableObjectsByGridMethod(
			numbersOfGuess, range3Double);
		for (int loop = 0; loop < geometryPathDInputParameters.size(); ++loop) {
			BuildGeometryPathRDCore(maxLevel, variableObjectsOfGuess, geometryPathDInputParameters[loop], paths);
		}
	}

	/// <summary>
	/// 这里的发射机和接收机是传入的应该是相反的
	/// </summary>
	/// <param name="geometryPathDInputParameters"></param>
	/// <param name="paths"></param>
	void BuildGeometryPathRDs(
		const std::vector<BuildGeometryPathDRStd::GeometryPathDRInputParameter>& geometryPathDInputParameters,
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths) {
		BuildGeometryPathRDs(3, 10, geometryPathDInputParameters, paths);
	}

}