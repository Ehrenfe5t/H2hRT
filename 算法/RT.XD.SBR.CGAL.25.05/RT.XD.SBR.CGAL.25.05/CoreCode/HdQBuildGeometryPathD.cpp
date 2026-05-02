
#include"HdQBuildGeometryPathD.h"
#include"QzQGeometryRTMultiPathDNode.h"
#include"QzQGeometryRTMultiPathTxNode.h"
#include"QzQGeometryRTMultiPathRxNode.h"
#include"QzQGlobalConstant.h"
#include"QzQGeometry3DOperate.Point3D.h"
#include"QzQGeometry3DOperate.Shadow.h"
#include"QzQGeometry3DOperate.Distance.h"
#include"QzQGeometry3DOperate.Create.h"
#include"QzQEquation1VariableObject.h"
#include"QzQFunctionOperator.h"

namespace BuildGeometryPathDStd {

	GeometryPathDInputParameter::GeometryPathDInputParameter()
	{
		this->cornerIndex = -1;
		this->id_tx = -1;
		this->id_rx = -1;
	}

	GeometryPathDInputParameter::GeometryPathDInputParameter(
		int id_tx,
		int id_rx,
		int cornerIndex,
		const Point3DStd::Point3D& tx_location,
		const Point3DStd::Point3D& rx_location,
		const LineSegment3DStd::LineSegment3D& segment1)
	{
		this->id_tx = id_tx;
		this->id_rx = id_rx;
		this->cornerIndex = cornerIndex;
		this->tx_location = tx_location;
		this->rx_location = rx_location;
		this->segment1 = segment1;
	}

	GeometryPathDInputParameter::~GeometryPathDInputParameter()
	{
	}

	class InputConstantParameter
	{
	public:
		double m1;
		double m2;
		double m3;
		double m4;
		Point3DStd::Point3D p3;
		Point3DStd::Point3D p7;
		InputConstantParameter() {
			this->m1 = 0.0;
			this->m2 = 0.0;
			this->m3 = 0.0;
			this->m4 = 0.0;
		}
		~InputConstantParameter() {

		}
	};
	bool CalInputConstantParameter(const GeometryPathDInputParameter& geometryPathDInputParameter, InputConstantParameter&res, Point3DStd::Point3D& shadow) {
		auto p1 = geometryPathDInputParameter.tx_location;
		auto p2 = geometryPathDInputParameter.rx_location;
		auto p3 = geometryPathDInputParameter.segment1.start;
		auto p4 = geometryPathDInputParameter.segment1.end;
		Line3DStd::Line3D line;
		if (Geometry3DOperateStd::CreateLine3DByTwoPoint3D_safe(p3,p4, line)) {
			auto p5 = Geometry3DOperateStd::CalculateShadowPoint3DOfPoint3DonLine3D_plus_unsafe(p1, line.p, line.vec);
			auto p6 = Geometry3DOperateStd::CalculateShadowPoint3DOfPoint3DonLine3D_plus_unsafe(p2, line.p, line.vec);
			double h1 = Geometry3DOperateStd::GetDistancePoint3DPoint3D(p1, p5);
			double h2 = Geometry3DOperateStd::GetDistancePoint3DPoint3D(p2, p6);
			auto p7 = Geometry3DOperateStd::SubPoint3DPoint3D(p4, p3);
			auto p8 = Geometry3DOperateStd::SubPoint3DPoint3D(p3, p5);
			auto p9 = Geometry3DOperateStd::SubPoint3DPoint3D(p3, p6);

			shadow = p5;

			double l1 = Geometry3DOperateStd::DotPoint3DPoint3D(p7, p7);
			double l2 = Geometry3DOperateStd::DotPoint3DPoint3D(p8, p8);
			double l3 = Geometry3DOperateStd::DotPoint3DPoint3D(p9, p9);
			double l4 = 2.0 * Geometry3DOperateStd::DotPoint3DPoint3D(p7, p8);
			double l5 = 2.0 * Geometry3DOperateStd::DotPoint3DPoint3D(p7, p9);
			res.m1 = (h1 - h2) * l1;
			res.m2 = h1 * l5 - h2 * l4;
			res.m3 = h1 * l3 - h2 * l2;
			res.m4 = 2.0 * res.m1;
			res.p3 = p3;
			res.p7 = p7;
			return true;
		}
		return false;
	}

	/// <summary>
	/// 原方程组第1个方程式
	/// </summary>
	/// <param name="input"></param>
	/// <returns></returns>
	double CalFun_0(const InputConstantParameter& inputConstantParameter, const Equation1VariableObjectStd::Equation1VariableObject& input) {

		return inputConstantParameter.m1 * input.x1 * input.x1 + inputConstantParameter.m2 * input.x1 + inputConstantParameter.m3;
	}
	/// <summary>
	/// 原方程组计算结果
	/// </summary>
	/// <param name="input"></param>
	/// <returns></returns>
	std::vector<double> CalFun(const InputConstantParameter& inputConstantParameter, const Equation1VariableObjectStd::Equation1VariableObject& input) {
		//几个方程式就有几个结果
		std::vector<double> res(1);
		res[0] = CalFun_0(inputConstantParameter, input);
		return res;
	}

	bool CheckFun(const InputConstantParameter& inputConstantParameter, const Equation1VariableObjectStd::Equation1VariableObject& input) {
		std::vector<double> res = CalFun(inputConstantParameter, input);
		return FunctionOperatorStd::CheckFunction(res);
	}

	


	double CalJacobianDeterminant(const InputConstantParameter& inputConstantParameter,
		const Equation1VariableObjectStd::Equation1VariableObject& input) {
		return inputConstantParameter.m4 * input.x1 + inputConstantParameter.m2;
	}

	bool CalNext(const InputConstantParameter& inputConstantParameter, 
		const Equation1VariableObjectStd::Equation1VariableObject& input,
		Equation1VariableObjectStd::Equation1VariableObject& result) {

		double jacobianDeterminant = CalJacobianDeterminant(inputConstantParameter,input);
		if (abs(jacobianDeterminant)<GlobalConstantStd::Eps) {
			return false;
		}

		std::vector<double> fun_res = CalFun(inputConstantParameter, input);
		auto temp1 = fun_res[0]/ jacobianDeterminant;
		result.x1 = input.x1 - temp1;
		return true;
	}

	/// <summary>
	/// 由于方程本身就是一个一般的1元2次方程式，这种迭代的方式没有必要
	/// </summary>
	/// <param name="curlevel"></param>
	/// <param name="maxLevel"></param>
	/// <param name="inputConstantParameter"></param>
	/// <param name="input"></param>
	/// <param name="result"></param>
	/// <returns></returns>
	bool GuessTrueOneResult(
		int curlevel,
		int maxLevel,
		const InputConstantParameter& inputConstantParameter,
		const Equation1VariableObjectStd::Equation1VariableObject& input,
		Equation1VariableObjectStd::Equation1VariableObject& result) {
		if (curlevel > maxLevel) {
			return false;
		}
		//第1步，检查是否是满足方程组的，如果是则返回true，如果不是则开始迭代
		if (CheckFun(inputConstantParameter,input)) {
			//受到约束
			if (input.x1 >= 0.0 && input.x1 <= 1.0) {
				result = input;
				return true;
			}
			return false;
		}
		Equation1VariableObjectStd::Equation1VariableObject input_next;
		if (!CalNext(inputConstantParameter, input, input_next)) {
			return false;
		}
		return GuessTrueOneResult(curlevel + 1, maxLevel, inputConstantParameter, input_next, result);
	}




	void BuildGeometryPathDCore(
		int maxLevel,
		const std::vector<Equation1VariableObjectStd::Equation1VariableObject>& variableObjectsOfGuess,
		const GeometryPathDInputParameter& geometryPathDInputParameter,
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths) {
		InputConstantParameter inputConstantParameter;
		Point3DStd::Point3D one_shadow;
		if (CalInputConstantParameter(geometryPathDInputParameter, inputConstantParameter, one_shadow)) {
			std::vector<Equation1VariableObjectStd::Equation1VariableObject> results;
			for (int i = 0; i < variableObjectsOfGuess.size(); ++i) {
				Equation1VariableObjectStd::Equation1VariableObject result;
				if (GuessTrueOneResult(0, maxLevel, inputConstantParameter, variableObjectsOfGuess[i], result)) {
					Equation1VariableObjectStd::Add(result, results);
				}
			}
			if (results.size() > 0) {
				GeometryRTMultiPathTxNodeStd::GeometryRTMultiPathTxNode* txNode = new GeometryRTMultiPathTxNodeStd::GeometryRTMultiPathTxNode(geometryPathDInputParameter.id_tx,geometryPathDInputParameter.tx_location);
				GeometryRTMultiPathRxNodeStd::GeometryRTMultiPathRxNode* rxNode = new GeometryRTMultiPathRxNodeStd::GeometryRTMultiPathRxNode(geometryPathDInputParameter.id_rx,geometryPathDInputParameter.rx_location);
				for (int i = 0; i < results.size(); ++i) {
					auto p10 = Geometry3DOperateStd::AddPoint3DPoint3D(inputConstantParameter.p3,
						Geometry3DOperateStd::MulDoublePoint3D(results[i].x1, inputConstantParameter.p7));
					std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*> path;
					GeometryRTMultiPathDNodeStd::GeometryRTMultiPathDNode* dNode = new GeometryRTMultiPathDNodeStd::GeometryRTMultiPathDNode(geometryPathDInputParameter.cornerIndex, p10);
					path.emplace_back(txNode);
					path.emplace_back(dNode);
					path.emplace_back(rxNode);
					paths.emplace_back(path);
				}
			}
		}
	}

	/// <summary>
	/// 基于迭代法，牛顿下山法
	/// </summary>
	/// <param name="numbersOfGuess"></param>
	/// <param name="maxLevel"></param>
	/// <param name="geometryPathDInputParameter"></param>
	/// <param name="paths"></param>
	void BuildGeometryPathD(
		int numbersOfGuess,
		int maxLevel,
		const GeometryPathDInputParameter& geometryPathDInputParameter,
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths) {
		//默认0-1
		RangeDoubleStd::Range1Double range1Double;
		//初始化猜测得值
		auto variableObjectsOfGuess = Equation1VariableObjectStd::GenerateSelfVariableObjectsByGridMethod(
			numbersOfGuess, range1Double);
		BuildGeometryPathDCore(maxLevel, variableObjectsOfGuess, geometryPathDInputParameter, paths);
	}

	/// <summary>
	/// 基于迭代法，牛顿下山法
	/// </summary>
	/// <param name="geometryPathDInputParameter"></param>
	/// <param name="paths"></param>
	void BuildGeometryPathD(
		const GeometryPathDInputParameter& geometryPathDInputParameter,
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths) {
		BuildGeometryPathD(3, 10, geometryPathDInputParameter, paths);
	}

	/// <summary>
	/// 基于迭代法，牛顿下山法
	/// </summary>
	/// <param name="numbersOfGuess"></param>
	/// <param name="maxLevel"></param>
	/// <param name="geometryPathDInputParameters"></param>
	/// <param name="paths"></param>
	void BuildGeometryPathDs(
		int numbersOfGuess,
		int maxLevel,
		const std::vector<GeometryPathDInputParameter>& geometryPathDInputParameters,
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths) {
		//默认0-1
		RangeDoubleStd::Range1Double range1Double;
		//初始化猜测得值
		auto variableObjectsOfGuess = Equation1VariableObjectStd::GenerateSelfVariableObjectsByGridMethod(
			numbersOfGuess, range1Double);
		for (int loop = 0; loop < geometryPathDInputParameters.size(); ++loop) {
			BuildGeometryPathDCore(maxLevel, variableObjectsOfGuess, geometryPathDInputParameters[loop], paths);
		}

	}
	/// <summary>
	/// 基于迭代法，牛顿下山法
	/// </summary>
	/// <param name="geometryPathDInputParameters"></param>
	/// <param name="paths"></param>
	void BuildGeometryPathDs(
		const std::vector<GeometryPathDInputParameter>& geometryPathDInputParameters,
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths) {
		BuildGeometryPathDs(3, 10, geometryPathDInputParameters, paths);
	}

	/// <summary>
	/// 基于解析解
	/// </summary>
	/// <param name="geometryPathDInputParameter"></param>
	/// <param name="paths"></param>
	void BuildGeometryPathDByAnalyticalSolution(
		const GeometryPathDInputParameter& geometryPathDInputParameter,
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths) {
		InputConstantParameter inputConstantParameter;
		Point3DStd::Point3D one_shadow;
		if (CalInputConstantParameter(geometryPathDInputParameter, inputConstantParameter, one_shadow)) {
			if (abs(inputConstantParameter.m1) <= GlobalConstantStd::MaxEps) {
				if (abs(inputConstantParameter.m2) <= GlobalConstantStd::MaxEps) {
					GeometryRTMultiPathTxNodeStd::GeometryRTMultiPathTxNode* txNode =
						new GeometryRTMultiPathTxNodeStd::GeometryRTMultiPathTxNode(geometryPathDInputParameter.id_tx, geometryPathDInputParameter.tx_location);
					GeometryRTMultiPathRxNodeStd::GeometryRTMultiPathRxNode* rxNode =
						new GeometryRTMultiPathRxNodeStd::GeometryRTMultiPathRxNode(geometryPathDInputParameter.id_rx, geometryPathDInputParameter.rx_location);
					
					std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*> path;
					GeometryRTMultiPathDNodeStd::GeometryRTMultiPathDNode* dNode = new GeometryRTMultiPathDNodeStd::GeometryRTMultiPathDNode(geometryPathDInputParameter.cornerIndex, one_shadow);
					path.emplace_back(txNode);
					path.emplace_back(dNode);
					path.emplace_back(rxNode);
					paths.emplace_back(path);

					return;
				}
				{
					{
						double x1 = (-inputConstantParameter.m3) / inputConstantParameter.m2;
						if (x1 >= 0.0 && x1 <= 1.0) {
							GeometryRTMultiPathTxNodeStd::GeometryRTMultiPathTxNode* txNode = 
								new GeometryRTMultiPathTxNodeStd::GeometryRTMultiPathTxNode(geometryPathDInputParameter.id_tx,geometryPathDInputParameter.tx_location);
							GeometryRTMultiPathRxNodeStd::GeometryRTMultiPathRxNode* rxNode = 
								new GeometryRTMultiPathRxNodeStd::GeometryRTMultiPathRxNode(geometryPathDInputParameter.id_rx,geometryPathDInputParameter.rx_location);
							auto p10 = Geometry3DOperateStd::AddPoint3DPoint3D(inputConstantParameter.p3,
								Geometry3DOperateStd::MulDoublePoint3D(x1, inputConstantParameter.p7));

							std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*> path;
							GeometryRTMultiPathDNodeStd::GeometryRTMultiPathDNode* dNode = new GeometryRTMultiPathDNodeStd::GeometryRTMultiPathDNode(geometryPathDInputParameter.cornerIndex, p10);
							path.emplace_back(txNode);
							path.emplace_back(dNode);
							path.emplace_back(rxNode);
							paths.emplace_back(path);
						}
					}
				}
				return;
			}
			//double dt2 = -4.0 * inputConstantParameter.m1 * inputConstantParameter.m3 
			//	+ inputConstantParameter.m2 * inputConstantParameter.m2;
			//
			//double dt_test_1 = 100 * (-4.0 * (inputConstantParameter.m1 / 10.0) * (inputConstantParameter.m3 / 10.0)
			//	+ (inputConstantParameter.m2 / 10.0) * (inputConstantParameter.m2 / 10.0));
			//
			//double dt_test_2 = 10000 * (-4.0 * (inputConstantParameter.m1 / 100.0) * (inputConstantParameter.m3 / 100.0)
			//	+ (inputConstantParameter.m2 / 100.0) * (inputConstantParameter.m2 / 100.0));
			//
			//double dt_test_3 = 1000000 * (-4.0 * (inputConstantParameter.m1 / 1000.0) * (inputConstantParameter.m3 / 1000.0)
			//	+ (inputConstantParameter.m2 / 1000.0) * (inputConstantParameter.m2 / 1000.0));
			//
			//
			//double dt_test_4 = 0.01 * (-4.0 * (inputConstantParameter.m1 * 10) * (inputConstantParameter.m3 * 10)
			//	+ (inputConstantParameter.m2 * 10) * (inputConstantParameter.m2 * 10));
			//
			//double dt_test_5 = 0.0001 * (-4.0 * (inputConstantParameter.m1 * 100) * (inputConstantParameter.m3 * 100)
			//	+ (inputConstantParameter.m2 * 100) * (inputConstantParameter.m2 * 100));

			//double dt2 = 0.0001 * (-4.0 * (inputConstantParameter.m1 * 100) * (inputConstantParameter.m3 * 100)
			//	+ (inputConstantParameter.m2 * 100) * (inputConstantParameter.m2 * 100));


			double dt2 = -4.0 * inputConstantParameter.m1 * inputConstantParameter.m3 
				+ inputConstantParameter.m2 * inputConstantParameter.m2;

			if (abs(dt2)<=GlobalConstantStd::Eps*100) {
				{
					{
						double x1 = 0.5 * (-inputConstantParameter.m2) / inputConstantParameter.m1;
						if (x1 >= 0.0 && x1 <= 1.0) {
							GeometryRTMultiPathTxNodeStd::GeometryRTMultiPathTxNode* txNode = new GeometryRTMultiPathTxNodeStd::GeometryRTMultiPathTxNode(geometryPathDInputParameter.id_tx,geometryPathDInputParameter.tx_location);
							GeometryRTMultiPathRxNodeStd::GeometryRTMultiPathRxNode* rxNode = new GeometryRTMultiPathRxNodeStd::GeometryRTMultiPathRxNode(geometryPathDInputParameter.id_rx,geometryPathDInputParameter.rx_location);
							auto p10 = Geometry3DOperateStd::AddPoint3DPoint3D(inputConstantParameter.p3,
								Geometry3DOperateStd::MulDoublePoint3D(x1, inputConstantParameter.p7));

							std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*> path;
							GeometryRTMultiPathDNodeStd::GeometryRTMultiPathDNode* dNode = new GeometryRTMultiPathDNodeStd::GeometryRTMultiPathDNode(geometryPathDInputParameter.cornerIndex, p10);
							path.emplace_back(txNode);
							path.emplace_back(dNode);
							path.emplace_back(rxNode);
							paths.emplace_back(path);
						}
					}
				}
			}
			else if (dt2 < 0) {
				return;
			}
			else {
				{
					{
						double x1 = 0.5 * (-inputConstantParameter.m2 + sqrt(dt2)) / inputConstantParameter.m1;
						if (x1 >= 0.0 && x1 <= 1.0) {
							GeometryRTMultiPathTxNodeStd::GeometryRTMultiPathTxNode* txNode = new GeometryRTMultiPathTxNodeStd::GeometryRTMultiPathTxNode(geometryPathDInputParameter.id_tx,geometryPathDInputParameter.tx_location);
							GeometryRTMultiPathRxNodeStd::GeometryRTMultiPathRxNode* rxNode = new GeometryRTMultiPathRxNodeStd::GeometryRTMultiPathRxNode(geometryPathDInputParameter.id_rx,geometryPathDInputParameter.rx_location);
							auto p10 = Geometry3DOperateStd::AddPoint3DPoint3D(inputConstantParameter.p3,
								Geometry3DOperateStd::MulDoublePoint3D(x1, inputConstantParameter.p7));

							std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*> path;
							GeometryRTMultiPathDNodeStd::GeometryRTMultiPathDNode* dNode = new GeometryRTMultiPathDNodeStd::GeometryRTMultiPathDNode(geometryPathDInputParameter.cornerIndex, p10);
							path.emplace_back(txNode);
							path.emplace_back(dNode);
							path.emplace_back(rxNode);
							paths.emplace_back(path);
						}
					}
				}
				{
					{
						double x1 = 0.5 * (-inputConstantParameter.m2 - sqrt(dt2)) / inputConstantParameter.m1;
						if (x1 >= 0.0 && x1 <= 1.0) {
							GeometryRTMultiPathTxNodeStd::GeometryRTMultiPathTxNode* txNode = new GeometryRTMultiPathTxNodeStd::GeometryRTMultiPathTxNode(geometryPathDInputParameter.id_tx,geometryPathDInputParameter.tx_location);
							GeometryRTMultiPathRxNodeStd::GeometryRTMultiPathRxNode* rxNode = new GeometryRTMultiPathRxNodeStd::GeometryRTMultiPathRxNode(geometryPathDInputParameter.id_rx,geometryPathDInputParameter.rx_location);
							auto p10 = Geometry3DOperateStd::AddPoint3DPoint3D(inputConstantParameter.p3,
								Geometry3DOperateStd::MulDoublePoint3D(x1, inputConstantParameter.p7));

							std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*> path;
							GeometryRTMultiPathDNodeStd::GeometryRTMultiPathDNode* dNode = new GeometryRTMultiPathDNodeStd::GeometryRTMultiPathDNode(geometryPathDInputParameter.cornerIndex, p10);
							path.emplace_back(txNode);
							path.emplace_back(dNode);
							path.emplace_back(rxNode);
							paths.emplace_back(path);
						}
					}
				}
			}
		}
	}

	/// <summary>
	/// 基于解析解
	/// </summary>
	/// <param name="geometryPathDInputParameters"></param>
	/// <param name="paths"></param>
	void BuildGeometryPathDsByAnalyticalSolution(
		const std::vector<GeometryPathDInputParameter>& geometryPathDInputParameters,
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths) {
		for (int loop = 0; loop < geometryPathDInputParameters.size(); ++loop) {
			BuildGeometryPathDByAnalyticalSolution(geometryPathDInputParameters[loop], paths);
		}
	}

}