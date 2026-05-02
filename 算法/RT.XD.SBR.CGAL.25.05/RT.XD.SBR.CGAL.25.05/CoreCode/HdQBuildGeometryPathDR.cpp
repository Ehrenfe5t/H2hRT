

#include"HdQBuildGeometryPathDR.h"
#include"QzQGeometryRTMultiPathDNode.h"
#include"QzQGeometryRTMultiPathRNode.h"
#include"QzQGeometryRTMultiPathTxNode.h"
#include"QzQGeometryRTMultiPathRxNode.h"


#include"QzQGeometry3DOperate.Create.h"
#include"QzQGeometry3DOperate.Point3D.h"
#include"QzQGeometry3DOperate.Shadow.h"
#include"QzQGeometry3DOperate.Equals.h"

#include"LxQMatrixOperator.h"
#include"QzQFunctionOperator.h"

namespace BuildGeometryPathDRStd {

	/// <summary>
	/// 方程式个数
	/// </summary>
	const int NumberOfEquations = 3;

	/// <summary>
	/// 变量个数
	/// </summary>
	const int NumberOfVariable = 3;

	GeometryPathDRInputParameter::GeometryPathDRInputParameter()
	{
		this->cornerIndex = -1;
		this->triangleIndex = -1;
		this->id_tx = -1;
		this->id_rx = -1;
	}

	GeometryPathDRInputParameter::~GeometryPathDRInputParameter()
	{
	}

	GeometryPathDRInputParameter::GeometryPathDRInputParameter(
		int id_tx,
		int id_rx,
		int cornerIndex, 
		int triangleIndex, 
		const Point3DStd::Point3D& tx_location, 
		const Point3DStd::Point3D& rx_location, 
		const Point3DStd::Point3D& triangle1_n,
		const LineSegment3DStd::LineSegment3D& segment1, 
		const Triangle3DStd::Triangle3D& triangle1)
	{
		this->id_tx = id_tx;
		this->id_rx = id_rx;
		this->cornerIndex = cornerIndex;
		this->triangleIndex = triangleIndex;
		this->tx_location = tx_location;
		this->rx_location = rx_location;
		this->triangle1_n = triangle1_n;
		this->segment1 = segment1;
		this->triangle1 = triangle1;
	}


	class InputConstantParameter
	{
	public:
		//数组
		double m1 ;
		double m2 ;
		double m3 ;
		double m4 ;
		double m5 ;
		double m6 ;
		double m7 ;
		double m8 ;
		double m9 ;
		double m10;
		double m11;
		double m12;
		double m13;
		double m14;
		double m15;
		double m16;
		double m17;
		double m18;
		double m19;
		double m20;
		double m21;
		double m22;
		double m23;
		double m24;
		double m25;
		double m26;
		double m27;
		double m28;
		double m29;
		double m30;
		double m31;
		double m32;
		double m33;
		double m34;
		double m35;
		double m36;
		double m37;
		double m38;
		double m39;
		double m40;
		Point3DStd::Point3D p3;
		Point3DStd::Point3D p6;
		Point3DStd::Point3D p7;
		Point3DStd::Point3D p10;
		Point3DStd::Point3D p11;
		InputConstantParameter() {
			this->m1  = 0.0;
			this->m2  = 0.0;
			this->m3  = 0.0;
			this->m4  = 0.0;
			this->m5  = 0.0;
			this->m6  = 0.0;
			this->m7  = 0.0;
			this->m8  = 0.0;
			this->m9  = 0.0;
			this->m10 = 0.0;
			this->m11 = 0.0;
			this->m12 = 0.0;
			this->m13 = 0.0;
			this->m14 = 0.0;
			this->m15 = 0.0;
			this->m16 = 0.0;
			this->m17 = 0.0;
			this->m18 = 0.0;
			this->m19 = 0.0;
			this->m20 = 0.0;
			this->m21 = 0.0;
			this->m22 = 0.0;
			this->m23 = 0.0;
			this->m24 = 0.0;
			this->m25 = 0.0;
			this->m26 = 0.0;
			this->m27 = 0.0;
			this->m28 = 0.0;
			this->m29 = 0.0;
			this->m30 = 0.0;
			this->m31 = 0.0;
			this->m32 = 0.0;
			this->m33 = 0.0;
			this->m34 = 0.0;
			this->m35 = 0.0;
			this->m36 = 0.0;
			this->m37 = 0.0;
			this->m38 = 0.0;
			this->m39 = 0.0;
			this->m40 = 0.0;
		}
		~InputConstantParameter() {

		}
	};

	bool CalInputConstantParameter(const GeometryPathDRInputParameter& geometryPathDInputParameter, InputConstantParameter& res) {
		auto p1 = geometryPathDInputParameter.tx_location;
		auto p2 = geometryPathDInputParameter.rx_location;
		auto p3 = geometryPathDInputParameter.segment1.start;
		auto p4 = geometryPathDInputParameter.segment1.end;
		Line3DStd::Line3D line;
		if (Geometry3DOperateStd::CreateLine3DByTwoPoint3D_safe(p3, p4, line)) {
			auto p5 = Geometry3DOperateStd::CalculateShadowPoint3DOfPoint3DonLine3D_plus_unsafe(p1, line.p, line.vec);
			auto p6 = Geometry3DOperateStd::SubPoint3DPoint3D(p4, p3);
			auto p7 = geometryPathDInputParameter.triangle1.p1;
			auto p8 = geometryPathDInputParameter.triangle1.p2;
			auto p9 = geometryPathDInputParameter.triangle1.p3;
			auto p10 = Geometry3DOperateStd::SubPoint3DPoint3D(p8, p7);
			auto p11 = Geometry3DOperateStd::SubPoint3DPoint3D(p9, p7);
			Point3DStd::Point3D p12;
			if (Geometry3DOperateStd::GetMirrorPoint3D(p2, 
				geometryPathDInputParameter.triangle1.p1, geometryPathDInputParameter.triangle1_n, p12)) {
				//没有镜像点，坐标到面元的距离为0
				return false;
			}

			double temp_1 = p1.x - p5.x;
			double temp_2 = p1.y - p5.y;
			double temp_3 = p1.z - p5.z;
			double h1 = temp_1 * temp_1 + temp_2 * temp_2 + temp_3 * temp_3;


			double l1 = Geometry3DOperateStd::DotPoint3DPoint3D(p6, p6);
			double l2 = Geometry3DOperateStd::DotPoint3DPoint3D(p3, p6);
			double l3 = p6.x / l1;
			double l4 = p6.y / l1;
			double l5 = p6.z / l1;
			double l6 = p3.x - l2 * l3;
			double l7 = p3.y - l2 * l4;
			double l8 = p3.z - l2 * l5;
			double l9  = Geometry3DOperateStd::DotPoint3DPoint3D(p7, p6);
			double l10 = Geometry3DOperateStd::DotPoint3DPoint3D(p10, p6);
			double l11 = Geometry3DOperateStd::DotPoint3DPoint3D(p11, p6);
			double l12 = l3 * l10;
			double l13 = l3 * l11;
			double l14 = l3 * l9 + l6;
			double l15 = l4 * l10;
			double l16 = l4 * l11;
			double l17 = l4 * l9 + l7;
			double l18 = l5 * l10;
			double l19 = l5 * l11;
			double l20 = l5 * l9 + l8;

			double l21 = p3.x - l14;
			double l22 = p3.y - l17;
			double l23 = p3.z - l20;
			double l24 = p3.x - p5.x;
			double l25 = p3.y - p5.y;
			double l26 = p3.z - p5.z;
			double l27 = p7.x - l14;
			double l28 = p7.y - l17;
			double l29 = p7.z - l20;
			double l30 = p10.x - l12;
			double l31 = p10.y - l15;
			double l32 = p10.z - l18;
			double l33 = p11.x - l13;
			double l34 = p11.y - l16;
			double l35 = p11.z - l19;

			double l36 = l21 * l21 + l22 * l22 + l23 * l23;
			double l37 = l24 * l24 + l25 * l25 + l26 * l26;
			double l38 = l27 * l27 + l28 * l28 + l29 * l29;

			double l39 = l12 * l12 + l15 * l15 + l18 * l18;
			double l40 = l13 * l13 + l16 * l16 + l19 * l19;
			double l41 = 2.0 * (p6.x * l12 + p6.y * l15 + p6.z * l18);
			double l42 = 2.0 * (p6.x * l13 + p6.y * l16 + p6.z * l19);
			double l43 = -2.0 * (l12 * l13 + l15 * l16 + l18 * l19);
			double l44 = 2.0 * (p6.x * l21 + p6.y * l22 + p6.z * l23);
			double l45 = -2.0 * (l12 * l21 + l15 * l22 + l18 * l23);
			double l46 = -2.0 * (l21 * l13 + l22 * l16 + l23 * l19);

			double l47 = 2.0 * (p6.x * l24 + p6.y * l25 + p6.z * l26);
			double l48 = l30 * l30 + l31 * l31 + l32 * l32;
			double l49 = 2.0 * (l30 * l33 + l31 * l34 + l32 * l35);
			double l50 = l33 * l33 + l34 * l34 + l35 * l35;
			double l51 = 2.0 * (l27 * l30 + l28 * l31 + l29 * l32);
			double l52 = 2.0 * (l27 * l33 + l28 * l34 + l29 * l35);

			double l53 = p3.x - p7.x;
			double l54 = p3.y - p7.y;
			double l55 = p3.z - p7.z;
			double l56 = p3.x - p12.x;
			double l57 = p3.y - p12.y;
			double l58 = p3.z - p12.z;
			res.m1 = l1 * l48;
			res.m2 = l1 * l50;
			res.m3 = l1 * l49;
			res.m4 = l1 * l51;
			res.m5 = l1 * l52;
			res.m6 = l47 * l48;
			res.m7 = l47 * l50;
			res.m8 = l47 * l49;
			res.m9 = l1 * (l38 - h1);
			res.m10 = l37 * l48 - l39 * h1;
			res.m11 = l37 * l50 - l40 * h1;
			res.m12 = l47 * l51 - l41 * h1;
			res.m13 = l47 * l52 - l42 * h1;
			res.m14 = l37 * l49 - l43 * h1;
			res.m15 = l38 * l47 - l44 * h1;
			res.m16 = l37 * l51 - l45 * h1;
			res.m17 = l37 * l52 - l46 * h1;
			res.m18 = l37 * l38 - l36 * h1;
			res.m19 = p6.x * p10.y - p6.y * p10.x;
			res.m20 = p6.x * p11.y - p6.y * p11.x;
			res.m21 = p6.y * l53 + p6.x * l57 - p6.x * l54 - p6.y * l56;
			res.m22 = p10.y * l56 - p10.x * l57;
			res.m23 = p11.y * l56 - p11.x * l57;
			res.m24 = l53 * l57 - l54 * l56;
			res.m25 = p6.x * p10.z - p6.z * p10.x;
			res.m26 = p6.x * p11.z - p6.z * p11.x;
			res.m27 = p6.z * l53 + p6.x * l58 - p6.x * l55 - p6.z * l56;
			res.m28 = p10.z * l56 - p10.x * l58;
			res.m29 = p11.z * l56 - p11.x * l58;
			res.m30 = l53 * l58 - l55 * l56;
			res.m31 = 2.0 * res.m1;
			res.m32 = 2.0 * res.m2;
			res.m33 = 2.0 * res.m3;
			res.m34 = 2.0 * res.m4;
			res.m35 = 2.0 * res.m5;
			res.m36 = 2.0 * res.m6;
			res.m37 = 2.0 * res.m7;
			res.m38 = 2.0 * res.m9;
			res.m39 = 2.0 * res.m10;
			res.m40 = 2.0 * res.m11;
			res.p3 = p3;
			res.p6 = p6;
			res.p7 = p7;
			res.p10 = p10;
			res.p11 = p11;
			return true;
		}
		return false;
	}


	/// <summary>
	/// 原方程组第1个方程式
	/// </summary>
	/// <param name="input"></param>
	/// <returns></returns>
	double CalFun_0(const InputConstantParameter& inputConstantParameter,
		const Equation3VariableObjectStd::Equation3VariableObject& input) {

		double x1 = input.x1;
		double x2 = input.x2;
		double x3 = input.x3;

		double temp1 = x2 * x2;
		double temp2 = x3 * x3;
		double temp3 = x2 * x3;

		double m1 =  inputConstantParameter.m1 ;
		double m2 =  inputConstantParameter.m2 ;
		double m3 =  inputConstantParameter.m3 ;
		double m4 =  inputConstantParameter.m4 ;
		double m5 =  inputConstantParameter.m5 ;
		double m6 =  inputConstantParameter.m6 ;
		double m7 =  inputConstantParameter.m7 ;
		double m8 =  inputConstantParameter.m8 ;
		double m9 =  inputConstantParameter.m9 ;
		double m10 = inputConstantParameter.m10;
		double m11 = inputConstantParameter.m11;
		double m12 = inputConstantParameter.m12;
		double m13 = inputConstantParameter.m13;
		double m14 = inputConstantParameter.m14;
		double m15 = inputConstantParameter.m15;
		double m16 = inputConstantParameter.m16;
		double m17 = inputConstantParameter.m17;
		double m18 = inputConstantParameter.m18;
		double a = m1 * temp1 + m2 * temp2 + m3 * temp3 + m4 * x2 + m5 * x3 + m9;
		double b = m6 * temp1 + m7 * temp2 + m8 * temp3 + m12 * x2 + m13 * x3 + m15;
		double c = m10 * temp1 + m11 * temp2 + m14 * temp3 + m16 * x2 + m17 * x3 + m18;
		return a*x1*x1+b*x1+c;
	}
	/// <summary>
	/// 原方程组第2个方程式
	/// </summary>
	/// <param name="input"></param>
	/// <returns></returns>
	double CalFun_1(const InputConstantParameter& inputConstantParameter, 
		const Equation3VariableObjectStd::Equation3VariableObject& input) {

		double m19 = inputConstantParameter.m19;
		double m20 = inputConstantParameter.m20;
		double m21 = inputConstantParameter.m21;
		double m22 = inputConstantParameter.m22;
		double m23 = inputConstantParameter.m23;
		double m24 = inputConstantParameter.m24;
		double x1 = input.x1;
		double x2 = input.x2;
		double x3 = input.x3;
		return m19 * x1 * x2 + m20 * x1 * x3 + m21 * x1 + m22 * x2 + m23 * x3 + m24;
	}
	/// <summary>
	/// 原方程组第3个方程式
	/// </summary>
	/// <param name="input"></param>
	/// <returns></returns>
	double CalFun_2(const InputConstantParameter& inputConstantParameter, 
		const Equation3VariableObjectStd::Equation3VariableObject& input) {

		double m25 = inputConstantParameter.m25;
		double m26 = inputConstantParameter.m26;
		double m27 = inputConstantParameter.m27;
		double m28 = inputConstantParameter.m28;
		double m29 = inputConstantParameter.m29;
		double m30 = inputConstantParameter.m30;
		double x1 = input.x1;
		double x2 = input.x2;
		double x3 = input.x3;
		return m25 * x1 * x2 + m26 * x1 * x3 + m27 * x1 + m28 * x2 + m29 * x3 + m30;
	}

	/// <summary>
	/// 原方程组计算结果
	/// </summary>
	/// <param name="input"></param>
	/// <returns></returns>
	std::vector<double> CalFun(
		const InputConstantParameter& inputConstantParameter, 
		const Equation3VariableObjectStd::Equation3VariableObject& input) {
		//几个方程式就有几个结果
		std::vector<double> res(NumberOfEquations);
		res[0] = CalFun_0(inputConstantParameter, input);
		res[1] = CalFun_1(inputConstantParameter, input);
		res[2] = CalFun_2(inputConstantParameter, input);
		return res;
	}

	bool CheckFun(
		const InputConstantParameter& inputConstantParameter, 
		const Equation3VariableObjectStd::Equation3VariableObject& input) {
		std::vector<double> res = CalFun(inputConstantParameter, input);
		return FunctionOperatorStd::CheckFunction(res);
	}

	double CalJacobianDeterminant_0_0(
		const InputConstantParameter& inputConstantParameter, const Equation3VariableObjectStd::Equation3VariableObject& input) {

		double m6  = inputConstantParameter.m6 ;
		double m7  = inputConstantParameter.m7 ;
		double m8  = inputConstantParameter.m8 ;
		double m12 = inputConstantParameter.m12;
		double m13 = inputConstantParameter.m13;
		double m15 = inputConstantParameter.m15;
		double m31 = inputConstantParameter.m31;
		double m32 = inputConstantParameter.m32;
		double m33 = inputConstantParameter.m33;
		double m34 = inputConstantParameter.m34;
		double m35 = inputConstantParameter.m35;
		double m38 = inputConstantParameter.m38;
		double x1 = input.x1;
		double x2 = input.x2;
		double x3 = input.x3;

		double temp1 = x2 * x2;
		double temp2 = x3 * x3;
		double temp3 = x2 * x3;

		double a = m31 * temp1 + m32 * temp2 + m33 * temp3 + m34 * x2 + m35 * x3 + m38;
		double b = m6 * temp1 + m7 * temp2 + m8 * temp3 + m12 * x2 + m13 * x3 + m15;

		return a * x1 + b;
	}
	double CalJacobianDeterminant_0_1(
		const InputConstantParameter& inputConstantParameter, const Equation3VariableObjectStd::Equation3VariableObject& input) {

		double x1  = input.x1;
		double x2  = input.x2;
		double x3  = input.x3;
		double m3  = inputConstantParameter.m3; 
		double m4  = inputConstantParameter.m4; 
		double m8  = inputConstantParameter.m8; 
		double m12 = inputConstantParameter.m12;
		double m14 = inputConstantParameter.m14;
		double m16 = inputConstantParameter.m16;
		double m31 = inputConstantParameter.m31;
		double m36 = inputConstantParameter.m36;
		double m39 = inputConstantParameter.m39;

		double a = m31 * x2 + m3 * x3 + m4;
		double b = m36 * x2 + m8 * x3 + m12;
		double c = m39 * x2 + m14 * x3 + m16;
		return a * x1 * x1 + b * x1 + c;
	}
	double CalJacobianDeterminant_0_2(
		const InputConstantParameter& inputConstantParameter, const Equation3VariableObjectStd::Equation3VariableObject& input) {

		double x1 = input.x1;
		double x2 = input.x2;
		double x3 = input.x3;
		double m3 = inputConstantParameter.m3;
		double m5 = inputConstantParameter.m5;
		double m8 = inputConstantParameter.m8;
		double m13 = inputConstantParameter.m13;
		double m14 = inputConstantParameter.m14;
		double m17 = inputConstantParameter.m17;
		double m32 = inputConstantParameter.m32;
		double m37 = inputConstantParameter.m37;
		double m40 = inputConstantParameter.m40;

		double a = m3 * x2 + m32 * x3 + m5;
		double b = m8 * x2 + m37 * x3 + m13;
		double c = m14 * x2 + m40 * x3 + m17;
		return a * x1 * x1 + b * x1 + c;
	}
	double CalJacobianDeterminant_1_0(
		const InputConstantParameter& inputConstantParameter, const Equation3VariableObjectStd::Equation3VariableObject& input) {

		return inputConstantParameter.m19 * input.x2 + inputConstantParameter.m20 * input.x3 + inputConstantParameter.m21;
	}
	double CalJacobianDeterminant_1_1(
		const InputConstantParameter& inputConstantParameter, const Equation3VariableObjectStd::Equation3VariableObject& input) {

		return inputConstantParameter.m19 * input.x1 + inputConstantParameter.m22;
	}
	double CalJacobianDeterminant_1_2(
		const InputConstantParameter& inputConstantParameter, const Equation3VariableObjectStd::Equation3VariableObject& input) {

		return inputConstantParameter.m20 * input.x1 + inputConstantParameter.m23;
	}
	double CalJacobianDeterminant_2_0(
		const InputConstantParameter& inputConstantParameter, const Equation3VariableObjectStd::Equation3VariableObject& input) {

		return inputConstantParameter.m25 * input.x2 + inputConstantParameter.m26 * input.x3 + inputConstantParameter.m27;
	}
	double CalJacobianDeterminant_2_1(
		const InputConstantParameter& inputConstantParameter, const Equation3VariableObjectStd::Equation3VariableObject& input) {

		return inputConstantParameter.m25 * input.x1 + inputConstantParameter.m28;
	}
	double CalJacobianDeterminant_2_2(
		const InputConstantParameter& inputConstantParameter, const Equation3VariableObjectStd::Equation3VariableObject& input) {

		return inputConstantParameter.m26 * input.x1 + inputConstantParameter.m29;
	}

	MatrixObjectStd::MatrixObject CalJacobianDeterminant(
		const InputConstantParameter& inputConstantParameter,
		const Equation3VariableObjectStd::Equation3VariableObject& input) {

		auto jacobianDeterminant = MatrixObjectStd::Init(NumberOfEquations, NumberOfVariable);
		jacobianDeterminant.value[0][0] = CalJacobianDeterminant_0_0(inputConstantParameter,input);
		jacobianDeterminant.value[0][1] = CalJacobianDeterminant_0_1(inputConstantParameter,input);
		jacobianDeterminant.value[0][2] = CalJacobianDeterminant_0_2(inputConstantParameter,input);
		jacobianDeterminant.value[1][0] = CalJacobianDeterminant_1_0(inputConstantParameter,input);
		jacobianDeterminant.value[1][1] = CalJacobianDeterminant_1_1(inputConstantParameter,input);
		jacobianDeterminant.value[1][2] = CalJacobianDeterminant_1_2(inputConstantParameter,input);
		jacobianDeterminant.value[2][0] = CalJacobianDeterminant_2_0(inputConstantParameter,input);
		jacobianDeterminant.value[2][1] = CalJacobianDeterminant_2_1(inputConstantParameter,input);
		jacobianDeterminant.value[2][2] = CalJacobianDeterminant_2_2(inputConstantParameter,input);
		return jacobianDeterminant;
	}

	bool CalNext(
		const InputConstantParameter& inputConstantParameter, 
		const Equation3VariableObjectStd::Equation3VariableObject& input,
		Equation3VariableObjectStd::Equation3VariableObject& result) {

		auto jacobianDeterminant = CalJacobianDeterminant(inputConstantParameter, input);
		MatrixObjectStd::MatrixObject jacobianDeterminant_inv;
		if (!MatrixOperatorStd::pinv_safe(jacobianDeterminant, jacobianDeterminant_inv)) {
			return false;
		}
		std::vector<double> fun_res = CalFun(inputConstantParameter, input);
		auto fun_matrix = MatrixObjectStd::Init(fun_res, NumberOfEquations, 1);
		auto temp1 = MatrixOperatorStd::Multiply(jacobianDeterminant_inv, fun_matrix);
		result.x1 = input.x1 - temp1.value[0][0];
		result.x2 = input.x2 - temp1.value[1][0];
		result.x3 = input.x3 - temp1.value[2][0];
		return true;
	}

	bool GuessTrueOneResult(
		int curlevel,
		int maxLevel,
		const InputConstantParameter& inputConstantParameter,
		const Equation3VariableObjectStd::Equation3VariableObject& input,
		Equation3VariableObjectStd::Equation3VariableObject& result) {
		if (curlevel > maxLevel) {
			return false;
		}
		//第1步，检查是否是满足方程组的，如果是则返回true，如果不是则开始迭代
		if (CheckFun(inputConstantParameter, input)) {
			//受到约束
			if (input.x1 >= 0.0 && input.x1 <= 1.0) {
				if (input.x2 >= 0.0 && input.x2 <= 1.0) {
					if (input.x3 >= 0.0 && input.x3 <= 1.0) {
						if (input.x2 + input.x3 >= 0.0 && input.x2 + input.x3 <= 1.0) {
							result = input;
							return true;
						}
					}
				}
			}
			return false;
		}
		Equation3VariableObjectStd::Equation3VariableObject input_next;
		if (!CalNext(inputConstantParameter, input, input_next)) {
			return false;
		}
		return GuessTrueOneResult(curlevel + 1, maxLevel, inputConstantParameter, input_next, result);
	}

	void BuildGeometryPathDRCoreNoTxAndRx(
		int maxLevel,
		const std::vector<Equation3VariableObjectStd::Equation3VariableObject>& variableObjectsOfGuess,
		const GeometryPathDRInputParameter& geometryPathDRInputParameter,
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths) {
		InputConstantParameter inputConstantParameter;
		if (CalInputConstantParameter(geometryPathDRInputParameter, inputConstantParameter)) {
			std::vector<Equation3VariableObjectStd::Equation3VariableObject> results;
			for (int i = 0; i < variableObjectsOfGuess.size(); ++i) {
				Equation3VariableObjectStd::Equation3VariableObject result;
				if (GuessTrueOneResult(0, maxLevel, inputConstantParameter, variableObjectsOfGuess[i], result)) {
					Equation3VariableObjectStd::Add(result, results);
				}
			}
			if (results.size() > 0) {
				for (int i = 0; i < results.size(); ++i) {
					auto A1 = Geometry3DOperateStd::AddPoint3DPoint3D(inputConstantParameter.p3,
						Geometry3DOperateStd::MulDoublePoint3D(results[i].x1, inputConstantParameter.p6));
					auto A2 = Geometry3DOperateStd::AddPoint3DPoint3DPoint3D(inputConstantParameter.p7,
						Geometry3DOperateStd::MulDoublePoint3D(results[i].x2, inputConstantParameter.p10),
						Geometry3DOperateStd::MulDoublePoint3D(results[i].x3, inputConstantParameter.p11));
					
					//不能是重合点
					if (Geometry3DOperateStd::Equals_Point3D(A1, A2)) {
						continue;
					}

					std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*> path;
					GeometryRTMultiPathDNodeStd::GeometryRTMultiPathDNode* dNode = new GeometryRTMultiPathDNodeStd::GeometryRTMultiPathDNode(geometryPathDRInputParameter.cornerIndex, A1);
					GeometryRTMultiPathRNodeStd::GeometryRTMultiPathRNode* rNode = new GeometryRTMultiPathRNodeStd::GeometryRTMultiPathRNode(geometryPathDRInputParameter.triangleIndex, A2);
					path.emplace_back(dNode);
					path.emplace_back(rNode);
					paths.emplace_back(path);
				}
			}
		}
	}

	void BuildGeometryPathDRCore(
		int maxLevel,
		const std::vector<Equation3VariableObjectStd::Equation3VariableObject>& variableObjectsOfGuess,
		const GeometryPathDRInputParameter& geometryPathDRInputParameter,
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths) {
		InputConstantParameter inputConstantParameter;
		if (CalInputConstantParameter(geometryPathDRInputParameter, inputConstantParameter)) {
			std::vector<Equation3VariableObjectStd::Equation3VariableObject> results;
			for (int i = 0; i < variableObjectsOfGuess.size(); ++i) {
				Equation3VariableObjectStd::Equation3VariableObject result;
				if (GuessTrueOneResult(0, maxLevel, inputConstantParameter, variableObjectsOfGuess[i], result)) {
					Equation3VariableObjectStd::Add(result, results);
				}
			}
			if (results.size() > 0) {
				GeometryRTMultiPathTxNodeStd::GeometryRTMultiPathTxNode* txNode = new GeometryRTMultiPathTxNodeStd::GeometryRTMultiPathTxNode(geometryPathDRInputParameter.id_tx,geometryPathDRInputParameter.tx_location);
				GeometryRTMultiPathRxNodeStd::GeometryRTMultiPathRxNode* rxNode = new GeometryRTMultiPathRxNodeStd::GeometryRTMultiPathRxNode(geometryPathDRInputParameter.id_rx,geometryPathDRInputParameter.rx_location);
				for (int i = 0; i < results.size(); ++i) {
					auto A1 = Geometry3DOperateStd::AddPoint3DPoint3D(inputConstantParameter.p3,
						Geometry3DOperateStd::MulDoublePoint3D(results[i].x1, inputConstantParameter.p6));
					auto A2 = Geometry3DOperateStd::AddPoint3DPoint3DPoint3D(inputConstantParameter.p7,
						Geometry3DOperateStd::MulDoublePoint3D(results[i].x2, inputConstantParameter.p10),
						Geometry3DOperateStd::MulDoublePoint3D(results[i].x3, inputConstantParameter.p11));

					//不能是重合点
					if (Geometry3DOperateStd::Equals_Point3D(A1, A2)) {
						continue;
					}

					std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*> path;
					GeometryRTMultiPathDNodeStd::GeometryRTMultiPathDNode* dNode = new GeometryRTMultiPathDNodeStd::GeometryRTMultiPathDNode(geometryPathDRInputParameter.cornerIndex, A1);
					GeometryRTMultiPathRNodeStd::GeometryRTMultiPathRNode* rNode = new GeometryRTMultiPathRNodeStd::GeometryRTMultiPathRNode(geometryPathDRInputParameter.triangleIndex, A2);
					path.emplace_back(txNode);
					path.emplace_back(dNode);
					path.emplace_back(rNode);
					path.emplace_back(rxNode);
					paths.emplace_back(path);
				}
			}
		}
	}


	void BuildGeometryPathDR(
		int numbersOfGuess,
		int maxLevel,
		const GeometryPathDRInputParameter& geometryPathDInputParameter,
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths) {
		//默认0-1
		RangeDoubleStd::Range3Double range3Double;
		//初始化猜测得值
		auto variableObjectsOfGuess = Equation3VariableObjectStd::GenerateSelfVariableObjectsByGridMethod(
			numbersOfGuess, range3Double);
		BuildGeometryPathDRCore(maxLevel, variableObjectsOfGuess, geometryPathDInputParameter, paths);
	}

	void BuildGeometryPathDR(
		const GeometryPathDRInputParameter& geometryPathDInputParameter,
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths) {
		BuildGeometryPathDR(3, 10, geometryPathDInputParameter, paths);
	}

	void BuildGeometryPathDRs(
		int numbersOfGuess,
		int maxLevel,
		const std::vector<GeometryPathDRInputParameter>& geometryPathDInputParameters,
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths) {
		//默认0-1
		RangeDoubleStd::Range3Double range3Double;
		//初始化猜测得值
		auto variableObjectsOfGuess = Equation3VariableObjectStd::GenerateSelfVariableObjectsByGridMethod(
			numbersOfGuess, range3Double);
		for (int loop = 0; loop < geometryPathDInputParameters.size(); ++loop) {
			BuildGeometryPathDRCore(maxLevel, variableObjectsOfGuess, geometryPathDInputParameters[loop], paths);
		}

	}
	void BuildGeometryPathDRs(
		const std::vector<GeometryPathDRInputParameter>& geometryPathDInputParameters,
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths) {
		BuildGeometryPathDRs(3, 10, geometryPathDInputParameters, paths);
	}

}