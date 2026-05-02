
#include"HdQBuildGeometryPathDD.h"

#include"QzQFunctionOperator.h"
#include"QzQGeometryRTMultiPathDNode.h"
#include"QzQGeometryRTMultiPathTxNode.h"
#include"QzQGeometryRTMultiPathRxNode.h"
#include"QzQGeometry3DOperate.Create.h"
#include"QzQGeometry3DOperate.Shadow.h"
#include"QzQGeometry3DOperate.Point3D.h"
#include"QzQGeometry3DOperate.Equals.h"
#include"QzQGlobalConstant.h"

#include"LxQMatrixOperator.h"

namespace BuildGeometryPathDDStd {

	/// <summary>
	/// 方程式个数
	/// </summary>
	const int NumberOfEquations = 2;

	/// <summary>
	/// 变量个数
	/// </summary>
	const int NumberOfVariable = 2;


	GeometryPathDDInputParameter::GeometryPathDDInputParameter()
	{
		this->cornerIndex1 = -1;
		this->cornerIndex2 = -1;
		this->id_tx = -1;
		this->id_rx = -1;
	}

	GeometryPathDDInputParameter::GeometryPathDDInputParameter(
		int id_tx,
		int id_rx,
		int cornerIndex1,
		int cornerIndex2,
		const Point3DStd::Point3D& tx_location,
		const Point3DStd::Point3D& rx_location,
		const LineSegment3DStd::LineSegment3D& segment1,
		const LineSegment3DStd::LineSegment3D& segment2)
	{
		this->id_tx = id_tx;
		this->id_rx = id_rx;
		this->cornerIndex1 = cornerIndex1;
		this->cornerIndex2 = cornerIndex2;
		this->tx_location = tx_location;
		this->rx_location = rx_location;
		this->segment1 = segment1;
		this->segment2 = segment2;
	}

	GeometryPathDDInputParameter::~GeometryPathDDInputParameter()
	{
	}

	class InputConstantParameter
	{
	public:
		//数组
		double J1;
		double J2;
		double J3;
		double J4;
		double J5;
		double J6;
		double J7;
		double J8;
		double J9;
		double J10;
		double J11;
		double J12;
		double J13;
		double J14;

		double K1;
		double K2;
		double K3;
		double K4;
		double K5;
		double K6;
		double K7;
		double K8;
		double K9;
		double K10;
		double K11;
		double K12;
		double K13;
		double K14;
		Point3DStd::Point3D p3;
		Point3DStd::Point3D p5;
		Point3DStd::Point3D p7;
		Point3DStd::Point3D p8;
		InputConstantParameter() {
			this->K1 = 0.0;
			this->K2 = 0.0;
			this->K3 = 0.0;
			this->K4 = 0.0;
			this->K5 = 0.0;
			this->K6 = 0.0;
			this->K7 = 0.0;
			this->K8 = 0.0;
			this->K9 = 0.0;
			this->K10 = 0.0;
			this->K11 = 0.0;
			this->K12 = 0.0;
			this->K13 = 0.0;
			this->K14 = 0.0;
			this->J1 = 0.0;
			this->J2 = 0.0;
			this->J3 = 0.0;
			this->J4 = 0.0;
			this->J5 = 0.0;
			this->J6 = 0.0;
			this->J7 = 0.0;
			this->J8 = 0.0;
			this->J9 = 0.0;
			this->J10 = 0.0;
			this->J11 = 0.0;
			this->J12 = 0.0;
			this->J13 = 0.0;
			this->J14 = 0.0;
		}
		~InputConstantParameter() {

		}
	};

	double CalInputConstantParameter_11(double x1, double y1, double z1, double x2, double y2, double z2) {
		return x1 * x2 + y1 * y2 + z1 * z2;
	}

	double CalInputConstantParameter_12_1(double x1, double y1, double z1) {
		double abs_max = abs(x1);
		double d2 = abs(y1);
		double d3 = abs(y1);
		if (abs_max < d2) {
			abs_max = d2;
		}
		if (abs_max < d3) {
			abs_max = d3;
		}
		return abs_max;
	}
	double CalInputConstantParameter_12_2(double x1, double d2, double d3) {
		double abs_max = x1;
		if (abs_max < d2) {
			abs_max = d2;
		}
		if (abs_max < d3) {
			abs_max = d3;
		}
		return abs_max;
	}

	bool CalInputConstantParameter_12(InputConstantParameter& res) {

		{
			double abs_max_J_1 = CalInputConstantParameter_12_1(res.J1, res.J2, res.J3);
			double abs_max_J_2 = CalInputConstantParameter_12_1(res.J4, res.J5, res.J6);
			double abs_max_J_3 = CalInputConstantParameter_12_1(res.J7, res.J8, res.J9);
			double abs_max_J = CalInputConstantParameter_12_2(abs_max_J_1, abs_max_J_2, abs_max_J_3) / 2.0;
			if (abs_max_J < GlobalConstantStd::MoreEps) {
				return false;
			}
			res.J1 = res.J1 / abs_max_J;
			res.J2 = res.J2 / abs_max_J;
			res.J3 = res.J3 / abs_max_J;
			res.J4 = res.J4 / abs_max_J;
			res.J5 = res.J5 / abs_max_J;
			res.J6 = res.J6 / abs_max_J;
			res.J7 = res.J7 / abs_max_J;
			res.J8 = res.J8 / abs_max_J;
			res.J9 = res.J9 / abs_max_J;
		}
		
		{
			double abs_max_K_1 = CalInputConstantParameter_12_1(res.K1, res.K2, res.K3);
			double abs_max_K_2 = CalInputConstantParameter_12_1(res.K4, res.K5, res.K6);
			double abs_max_K_3 = CalInputConstantParameter_12_1(res.K7, res.K8, res.K9);
			double abs_max_K = CalInputConstantParameter_12_2(abs_max_K_1, abs_max_K_2, abs_max_K_3) / 2.0;
			if (abs_max_K < GlobalConstantStd::MoreEps) {
				return false;
			}
			res.K1 = res.K1 / abs_max_K;
			res.K2 = res.K2 / abs_max_K;
			res.K3 = res.K3 / abs_max_K;
			res.K4 = res.K4 / abs_max_K;
			res.K5 = res.K5 / abs_max_K;
			res.K6 = res.K6 / abs_max_K;
			res.K7 = res.K7 / abs_max_K;
			res.K8 = res.K8 / abs_max_K;
			res.K9 = res.K9 / abs_max_K;
		}

		return true;
	}

	bool CalInputConstantParameter(
		const GeometryPathDDInputParameter& geometryPathDInputParameter,
		InputConstantParameter& res) {
		auto p3 = geometryPathDInputParameter.segment1.start;
		auto p4 = geometryPathDInputParameter.segment1.end;
		Line3DStd::Line3D line1;
		if (Geometry3DOperateStd::CreateLine3DByTwoPoint3D_safe(p3, p4, line1)) {

			Line3DStd::Line3D line2;
			auto p5 = geometryPathDInputParameter.segment2.start;
			auto p6 = geometryPathDInputParameter.segment2.end;
			if (Geometry3DOperateStd::CreateLine3DByTwoPoint3D_safe(p5, p6, line2)) {
				auto p1 = geometryPathDInputParameter.tx_location;
				auto p2 = geometryPathDInputParameter.rx_location;
				auto p7 = Geometry3DOperateStd::SubPoint3DPoint3D(p4, p3);
				auto p8 = Geometry3DOperateStd::SubPoint3DPoint3D(p6, p5);
				auto b1 = Geometry3DOperateStd::CalculateShadowPoint3DOfPoint3DonLine3D_plus_unsafe(p1, line1.p, line1.vec);
				auto b4 = Geometry3DOperateStd::CalculateShadowPoint3DOfPoint3DonLine3D_plus_unsafe(p2, line2.p, line2.vec);
				double L1 = p1.x;
				double L2 = p1.y;
				double L3 = p1.z;
				double L4 = p2.x;
				double L5 = p2.y;
				double L6 = p2.z;
				double L7 = p3.x;
				double L8 = p3.y;
				double L9 = p3.z;
				double L10 = p4.x;
				double L11 = p4.y;
				double L12 = p4.z;
				double L13 = p5.x;
				double L14 = p5.y;
				double L15 = p5.z;
				double L16 = p6.x;
				double L17 = p6.y;
				double L18 = p6.z;
				double L19 = p7.x;
				double L20 = p7.y;
				double L21 = p7.z;
				double L22 = p8.x;
				double L23 = p8.y;
				double L24 = p8.z;
				double L25 = b1.x;
				double L26 = b1.y;
				double L27 = b1.z;
				double L28 = b4.x;
				double L29 = b4.y;
				double L30 = b4.z;
				double h1 = (L1 - L25) * (L1 - L25) + (L2 - L26) * (L2 - L26) + (L3 - L27) * (L3 - L27);
				double h4 = (L4 - L28) * (L4 - L28) + (L5 - L29) * (L5 - L29) + (L6 - L30) * (L6 - L30);
				double L31 = L13 - L7;
				double L32 = L14 - L8;
				double L33 = L15 - L9;
				double L34 = L19 * L19 + L20 * L20 + L21 * L21;
				double L35 = L19 * L22 + L20 * L23 + L21 * L24;
				double L36 = L19 * L31 + L20 * L32 + L21 * L33;
				double L37 = L22 * L22 + L23 * L23 + L24 * L24;
				double L38 = L22 * L31 + L23 * L32 + L24 * L33;
				double L39 = L35 / L34;
				double L40 = L36 / L34;
				double L41 = L19 * L39;
				double L42 = L20 * L39;
				double L43 = L21 * L39;
				double L44 = L7 + L19 * L40;
				double L45 = L8 + L20 * L40;
				double L46 = L9 + L21 * L40;
				double L47 = L35 / L37;
				double L48 = -L38 / L37;
				double L49 = L22 * L47;
				double L50 = L23 * L47;
				double L51 = L24 * L47;
				double L52 = L13 + L22 * L48;
				double L53 = L14 + L23 * L48;
				double L54 = L15 + L24 * L48;


				double L55 = CalInputConstantParameter_11(L7, L8, L9, L7, L8, L9);
				double L56 = CalInputConstantParameter_11(L7, L8, L9, L19, L20, L21);
				double L57 = CalInputConstantParameter_11(L7, L8, L9, L25, L26, L27);
				double L58 = CalInputConstantParameter_11(L7, L8, L9, L41, L42, L43);
				double L59 = CalInputConstantParameter_11(L7, L8, L9, L44, L45, L46);
				double L60 = CalInputConstantParameter_11(L7, L8, L9, L49, L50, L51);
				double L61 = CalInputConstantParameter_11(L7, L8, L9, L52, L53, L54);
				double L62 = CalInputConstantParameter_11(L13, L14, L15, L13, L14, L15);
				double L63 = CalInputConstantParameter_11(L13, L14, L15, L22, L23, L24);
				double L64 = CalInputConstantParameter_11(L13, L14, L15, L28, L29, L30);
				double L65 = CalInputConstantParameter_11(L13, L14, L15, L41, L42, L43);
				double L66 = CalInputConstantParameter_11(L13, L14, L15, L44, L45, L46);
				double L67 = CalInputConstantParameter_11(L13, L14, L15, L49, L50, L51);
				double L68 = CalInputConstantParameter_11(L13, L14, L15, L52, L53, L54);
				double L69 = CalInputConstantParameter_11(L19, L20, L21, L19, L20, L21);
				double L70 = CalInputConstantParameter_11(L19, L20, L21, L25, L26, L27);
				double L71 = CalInputConstantParameter_11(L19, L20, L21, L41, L42, L43);
				double L72 = CalInputConstantParameter_11(L19, L20, L21, L44, L45, L46);
				double L73 = CalInputConstantParameter_11(L19, L20, L21, L49, L50, L51);
				double L74 = CalInputConstantParameter_11(L19, L20, L21, L52, L53, L54);
				double L75 = CalInputConstantParameter_11(L22, L23, L24, L22, L23, L24);
				double L76 = CalInputConstantParameter_11(L22, L23, L24, L28, L29, L30);
				double L77 = CalInputConstantParameter_11(L22, L23, L24, L41, L42, L43);
				double L78 = CalInputConstantParameter_11(L22, L23, L24, L44, L45, L46);
				double L79 = CalInputConstantParameter_11(L22, L23, L24, L49, L50, L51);
				double L80 = CalInputConstantParameter_11(L22, L23, L24, L52, L53, L54);
				double L81 = CalInputConstantParameter_11(L25, L26, L27, L25, L26, L27);
				double L82 = CalInputConstantParameter_11(L28, L29, L30, L28, L29, L30);
				double L83 = CalInputConstantParameter_11(L41, L42, L43, L41, L42, L43);
				double L84 = CalInputConstantParameter_11(L41, L42, L43, L44, L45, L46);
				double L85 = CalInputConstantParameter_11(L44, L45, L46, L44, L45, L46);
				double L86 = CalInputConstantParameter_11(L49, L50, L51, L49, L50, L51);
				double L87 = CalInputConstantParameter_11(L49, L50, L51, L52, L53, L54);
				double L88 = CalInputConstantParameter_11(L52, L53, L54, L52, L53, L54);
				
				
				double L89 = - 2*L72 + 2*L56;
				double L90 = 2 * L84 - 2 * L58;
				double L91 = L85 - 2 * L59 + L55;
				double L92 = 2 * L56 - 2 * L70;
				double L93 = L81 - 2 * L57 + L55;
				double L94 = L75 - 2 * L77 + L83;
				double L95 = 2 * L63 - 2 * L65 - 2 * L78 + 2 * L84;
				double L96 = L62 - 2 * L66 + L85;
				double L97 = 2 * L87 - 2 * L67;
				double L98 = 2 * L63 - 2 * L80;
				double L99 = L88 - 2 * L68 + L62;
				double L100 = L69 - 2 * L73 + L86;
				double L101 = -2 * L74 + 2 * L56 + 2 * L87 - 2 * L60;
				double L102 = L88 - 2 * L61 + L55;
				double L103 = 2 * L63 - 2 * L76;
				double L104 = L62 - 2 * L64 + L82;

				res.J1 = -L69 * L94;
				res.J2 = -L69 * L95;
				res.J3 = -L92 * L94;
				res.J4 = L69 * h1 - L69 * L96;
				res.J5 = -2 * L71 * h1 - L92 * L95;
				res.J6 = L83 * h1 - L93 * L94;
				res.J7 = L89 * h1 - L92 * L96;
				res.J8 = L90 * h1 - L93 * L95;
				res.J9 = L91 * h1 - L93 * L96;



				res.K1 = -L100 * L75;
				res.K2 = -L100 * L103;
				res.K3 = -L101 * L75;
				res.K4 = L86 * h4 - L100 * L104;
				res.K5 = -L101 * L103 - 2 * L79 * h4;
				res.K6 = L75 * h4 - L102 * L75;
				res.K7 = L97 * h4 - L101 * L104;
				res.K8 = L98 * h4 - L102 * L103;
				res.K9 = L99 * h4 - L102 * L104;

				//应该防止大数吃小数

				CalInputConstantParameter_12(res);

				res.J10 = 2.0 * res.J1;
				res.J11 = 2.0 * res.J2;
				res.J12 = 2.0 * res.J3;
				res.J13 = 2.0 * res.J4;
				res.J14 = 2.0 * res.J6;

				res.K10 = 2.0 * res.K1;
				res.K11 = 2.0 * res.K2;
				res.K12 = 2.0 * res.K3;
				res.K13 = 2.0 * res.K4;
				res.K14 = 2.0 * res.K6;

				res.p3 = p3;
				res.p7 = p7;
				res.p5 = p5;
				res.p8 = p8;

				return true;
			}

		}
		return false;
	}


	/// <summary>
	/// 原方程组第1个方程式
	/// </summary>
	/// <param name="input"></param>
	/// <returns></returns>
	double CalFun_0(const InputConstantParameter& inputConstantParameter,
		const Equation2VariableObjectStd::Equation2VariableObject& input) {

		double x1 = input.x1;
		double x2 = input.x2;

		double temp1 = x1 * x1;
		double temp2 = x2 * x2;
		double temp3 = x1 * x2;

		double J1 = inputConstantParameter.J1;
		double J2 = inputConstantParameter.J2;
		double J3 = inputConstantParameter.J3;
		double J4 = inputConstantParameter.J4;
		double J5 = inputConstantParameter.J5;
		double J6 = inputConstantParameter.J6;
		double J7 = inputConstantParameter.J7;
		double J8 = inputConstantParameter.J8;
		double J9 = inputConstantParameter.J9;

		double a1 = J1 * temp1 * temp2;
		double a2 = J2 * temp1 * x2;
		double a3 = J3 * x1 * temp2;
		double a4 = J4 * temp1 + J5 * temp3 + J6 * temp2;
		double a5 = J7 * x1 + J8 * x2 + J9;

		return (a1 + a2 + a3 + a4 + a5) ;
	}
	/// <summary>
	/// 原方程组第2个方程式
	/// </summary>
	/// <param name="input"></param>
	/// <returns></returns>
	double CalFun_1(const InputConstantParameter& inputConstantParameter,
		const Equation2VariableObjectStd::Equation2VariableObject& input) {

		double x1 = input.x1;
		double x2 = input.x2;

		double temp1 = x1 * x1;
		double temp2 = x2 * x2;
		double temp3 = x1 * x2;

		double K1 = inputConstantParameter.K1;
		double K2 = inputConstantParameter.K2;
		double K3 = inputConstantParameter.K3;
		double K4 = inputConstantParameter.K4;
		double K5 = inputConstantParameter.K5;
		double K6 = inputConstantParameter.K6;
		double K7 = inputConstantParameter.K7;
		double K8 = inputConstantParameter.K8;
		double K9 = inputConstantParameter.K9;

		double a1 = K1 * temp1 * temp2;
		double a2 = K2 * temp1 * x2;
		double a3 = K3 * x1 * temp2;
		double a4 = K4 * temp1 + K5 * temp3 + K6 * temp2;
		double a5 = K7 * x1 + K8 * x2 + K9;

		return (a1 + a2 + a3 + a4 + a5);
	}


	/// <summary>
	/// 原方程组计算结果
	/// </summary>
	/// <param name="input"></param>
	/// <returns></returns>
	std::vector<double> CalFun(
		const InputConstantParameter& inputConstantParameter,
		const Equation2VariableObjectStd::Equation2VariableObject& input) {
		//几个方程式就有几个结果
		std::vector<double> res(NumberOfEquations);
		res[0] = CalFun_0(inputConstantParameter, input);
		res[1] = CalFun_1(inputConstantParameter, input);
		return res;
	}

	bool CheckFun(
		const InputConstantParameter& inputConstantParameter,
		const Equation2VariableObjectStd::Equation2VariableObject& input) {
		std::vector<double> res = CalFun(inputConstantParameter, input);
		return FunctionOperatorStd::CheckFunction(res);
	}

	double CalJacobianDeterminant_0_0(
		const InputConstantParameter& inputConstantParameter,
		const Equation2VariableObjectStd::Equation2VariableObject& input) {

		double x1 = input.x1;
		double x2 = input.x2;

		//double temp1 = x1 * x1;
		double temp2 = x2 * x2;
		double temp3 = x1 * x2;

		double J3 = inputConstantParameter.J3;
		double J5 = inputConstantParameter.J5;
		double J7 = inputConstantParameter.J7;
		double J10 = inputConstantParameter.J10;
		double J11 = inputConstantParameter.J11;
		double J13 = inputConstantParameter.J13;

		double res = J10 * x1 * temp2 + J11 * temp3 + J3 * temp2 + J13 * x1 + J5 * x2 + J7;

		return res;
	}

	double CalJacobianDeterminant_0_1(
		const InputConstantParameter& inputConstantParameter,
		const Equation2VariableObjectStd::Equation2VariableObject& input) {

		double x1 = input.x1;
		double x2 = input.x2;

		double temp1 = x1 * x1;
		//double temp2 = x2 * x2;
		double temp3 = x1 * x2;

		double J2 = inputConstantParameter.J2;
		double J5 = inputConstantParameter.J5;
		double J8 = inputConstantParameter.J8;
		double J10 = inputConstantParameter.J10;
		double J12 = inputConstantParameter.J12;
		double J14 = inputConstantParameter.J14;

		double res = J10 * temp1 * x2 + J2 * temp1 + J12 * temp3 + J5 * x1 + J14 * x2 + J8;

		return res;
	}

	double CalJacobianDeterminant_1_0(
		const InputConstantParameter& inputConstantParameter,
		const Equation2VariableObjectStd::Equation2VariableObject& input) {

		double x1 = input.x1;
		double x2 = input.x2;

		//double temp1 = x1 * x1;
		double temp2 = x2 * x2;
		double temp3 = x1 * x2;

		double K3 = inputConstantParameter.K3;
		double K5 = inputConstantParameter.K5;
		double K7 = inputConstantParameter.K7;
		double K10 = inputConstantParameter.K10;
		double K11 = inputConstantParameter.K11;
		double K13 = inputConstantParameter.K13;

		double res = K10 * x1 * temp2 + K11 * temp3 + K3 * temp2 + K13 * x1 + K5 * x2 + K7;

		return res;
	}
	double CalJacobianDeterminant_1_1(
		const InputConstantParameter& inputConstantParameter,
		const Equation2VariableObjectStd::Equation2VariableObject& input) {

		double x1 = input.x1;
		double x2 = input.x2;

		double temp1 = x1 * x1;
		//double temp2 = x2 * x2;
		double temp3 = x1 * x2;

		double K2 = inputConstantParameter.K2;
		double K5 = inputConstantParameter.K5;
		double K8 = inputConstantParameter.K8;
		double K10 = inputConstantParameter.K10;
		double K12 = inputConstantParameter.K12;
		double K14 = inputConstantParameter.K14;

		double res = K10 * temp1 * x2 + K2 * temp1 + K12 * temp3 + K5 * x1 + K14 * x2 + K8;

		return res;
	}


	MatrixObjectStd::MatrixObject CalJacobianDeterminant(
		const InputConstantParameter& inputConstantParameter,
		const Equation2VariableObjectStd::Equation2VariableObject& input) {

		auto jacobianDeterminant = MatrixObjectStd::Init(NumberOfEquations, NumberOfVariable);
		jacobianDeterminant.value[0][0] = CalJacobianDeterminant_0_0(inputConstantParameter, input);
		jacobianDeterminant.value[0][1] = CalJacobianDeterminant_0_1(inputConstantParameter, input);
		jacobianDeterminant.value[1][0] = CalJacobianDeterminant_1_0(inputConstantParameter, input);
		jacobianDeterminant.value[1][1] = CalJacobianDeterminant_1_1(inputConstantParameter, input);
		return jacobianDeterminant;
	}

	bool CalNext(
		const InputConstantParameter& inputConstantParameter,
		const Equation2VariableObjectStd::Equation2VariableObject& input,
		Equation2VariableObjectStd::Equation2VariableObject& result) {

		auto jacobianDeterminant = CalJacobianDeterminant(inputConstantParameter, input);
		MatrixObjectStd::MatrixObject jacobianDeterminant_inv;
		if (!MatrixOperatorStd::pinv_safe(jacobianDeterminant, jacobianDeterminant_inv)) {
			return false;
		}

		/*if (jacobianDeterminant_inv.m == 0) {
			if (!MatrixOperatorStd::pinv_safe(jacobianDeterminant, jacobianDeterminant_inv)) {
				return false;
			}
		}*/

		std::vector<double> fun_res = CalFun(inputConstantParameter, input);
		auto fun_matrix = MatrixObjectStd::Init(fun_res, NumberOfEquations, 1);
		auto temp1 = MatrixOperatorStd::Multiply(jacobianDeterminant_inv, fun_matrix);
		result.x1 = input.x1 - temp1.value[0][0];
		result.x2 = input.x2 - temp1.value[1][0];
		return true;
	}




	bool GuessTrueOneResult(
		int curlevel,
		int maxLevel,
		const InputConstantParameter& inputConstantParameter,
		const Equation2VariableObjectStd::Equation2VariableObject& input,
		Equation2VariableObjectStd::Equation2VariableObject& result) {
		if (curlevel > maxLevel) {
			return false;
		}
		//第1步，检查是否是满足方程组的，如果是则返回true，如果不是则开始迭代
		if (CheckFun(inputConstantParameter, input)) {
			//受到约束
			if (input.x1 >= 0.0 && input.x1 <= 1.0) {
				if (input.x2 >= 0.0 && input.x2 <= 1.0) {
					result = input;
					return true;
				}
			}
			return false;
		}
		Equation2VariableObjectStd::Equation2VariableObject input_next;
		if (!CalNext(inputConstantParameter, input, input_next)) {
			return false;
		}
		return GuessTrueOneResult(curlevel + 1, maxLevel, inputConstantParameter, input_next, result);
	}

	void Test(const Equation2VariableObjectStd::Equation2VariableObject& equation2VariableObject,
		const GeometryPathDDInputParameter& geometryPathDRInputParameter) {
		InputConstantParameter inputConstantParameter;
		if (CalInputConstantParameter(geometryPathDRInputParameter, inputConstantParameter)) {
			std::vector<Equation2VariableObjectStd::Equation2VariableObject> results;
			Equation2VariableObjectStd::Equation2VariableObject result;
			if (GuessTrueOneResult(0, 3, inputConstantParameter, equation2VariableObject, result)) {
				Equation2VariableObjectStd::Add(result, results);
			}
		}
	}

	void BuildGeometryPathDDCoreNoTxAndRx(
		int maxLevel,
		const std::vector<Equation2VariableObjectStd::Equation2VariableObject>& variableObjectsOfGuess,
		const GeometryPathDDInputParameter& geometryPathDRInputParameter,
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths) {
		InputConstantParameter inputConstantParameter;
		if (CalInputConstantParameter(geometryPathDRInputParameter, inputConstantParameter)) {
			std::vector<Equation2VariableObjectStd::Equation2VariableObject> results;
			for (int i = 0; i < variableObjectsOfGuess.size(); ++i) {
				Equation2VariableObjectStd::Equation2VariableObject result;
				if (GuessTrueOneResult(0, maxLevel, inputConstantParameter, variableObjectsOfGuess[i], result)) {
					Equation2VariableObjectStd::Add(result, results);
				}
			}
			if (results.size() > 0) {
				for (int i = 0; i < results.size(); ++i) {
					auto A1 = Geometry3DOperateStd::AddPoint3DPoint3D(inputConstantParameter.p3,
						Geometry3DOperateStd::MulDoublePoint3D(results[i].x1, inputConstantParameter.p7));
					auto A2 = Geometry3DOperateStd::AddPoint3DPoint3D(inputConstantParameter.p5,
						Geometry3DOperateStd::MulDoublePoint3D(results[i].x1, inputConstantParameter.p8));

					std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*> path;
					GeometryRTMultiPathDNodeStd::GeometryRTMultiPathDNode* dNode1 = new GeometryRTMultiPathDNodeStd::GeometryRTMultiPathDNode(geometryPathDRInputParameter.cornerIndex1, A1);
					GeometryRTMultiPathDNodeStd::GeometryRTMultiPathDNode* dNode2 = new GeometryRTMultiPathDNodeStd::GeometryRTMultiPathDNode(geometryPathDRInputParameter.cornerIndex2, A2);

					path.emplace_back(dNode1);
					path.emplace_back(dNode2);

					paths.emplace_back(path);
				}
			}
		}
	}

	void BuildGeometryPathDDCore(
		int maxLevel,
		const std::vector<Equation2VariableObjectStd::Equation2VariableObject>& variableObjectsOfGuess,
		const GeometryPathDDInputParameter& geometryPathDRInputParameter,
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths) {
		InputConstantParameter inputConstantParameter;
		if (CalInputConstantParameter(geometryPathDRInputParameter, inputConstantParameter)) {
			std::vector<Equation2VariableObjectStd::Equation2VariableObject> results;
			for (int i = 0; i < variableObjectsOfGuess.size(); ++i) {
				Equation2VariableObjectStd::Equation2VariableObject result;
				if (GuessTrueOneResult(0, maxLevel, inputConstantParameter, variableObjectsOfGuess[i], result)) {
					Equation2VariableObjectStd::Add(result, results);
				}
			}
			if (results.size() > 0) {
				GeometryRTMultiPathTxNodeStd::GeometryRTMultiPathTxNode* txNode = new GeometryRTMultiPathTxNodeStd::GeometryRTMultiPathTxNode(geometryPathDRInputParameter.id_tx,geometryPathDRInputParameter.tx_location);
				GeometryRTMultiPathRxNodeStd::GeometryRTMultiPathRxNode* rxNode = new GeometryRTMultiPathRxNodeStd::GeometryRTMultiPathRxNode(geometryPathDRInputParameter.id_rx,geometryPathDRInputParameter.rx_location);
				for (int i = 0; i < results.size(); ++i) {
					auto A1 = Geometry3DOperateStd::AddPoint3DPoint3D(inputConstantParameter.p3,
						Geometry3DOperateStd::MulDoublePoint3D(results[i].x1, inputConstantParameter.p7));
					auto A2 = Geometry3DOperateStd::AddPoint3DPoint3D(inputConstantParameter.p5,
						Geometry3DOperateStd::MulDoublePoint3D(results[i].x1, inputConstantParameter.p8));

					//不能是重合点
					if (Geometry3DOperateStd::Equals_Point3D(A1, A2)) {
						continue;
					}
					
					std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*> path;
					GeometryRTMultiPathDNodeStd::GeometryRTMultiPathDNode* dNode1 = new GeometryRTMultiPathDNodeStd::GeometryRTMultiPathDNode(geometryPathDRInputParameter.cornerIndex1, A1);
					GeometryRTMultiPathDNodeStd::GeometryRTMultiPathDNode* dNode2 = new GeometryRTMultiPathDNodeStd::GeometryRTMultiPathDNode(geometryPathDRInputParameter.cornerIndex2, A2);
					path.emplace_back(txNode);
					path.emplace_back(dNode1);
					path.emplace_back(dNode2);
					path.emplace_back(rxNode);
					paths.emplace_back(path);
				}
			}
		}
	}


	void BuildGeometryPathDD(
		int numbersOfGuess,
		int maxLevel,
		const GeometryPathDDInputParameter& geometryPathDInputParameter,
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths) {
		//默认0-1
		RangeDoubleStd::Range2Double range2Double;
		//初始化猜测得值
		auto variableObjectsOfGuess = Equation2VariableObjectStd::GenerateSelfVariableObjectsByGridMethod(
			numbersOfGuess, range2Double);
		BuildGeometryPathDDCore(maxLevel, variableObjectsOfGuess, geometryPathDInputParameter, paths);
	}

	void BuildGeometryPathDD(
		const GeometryPathDDInputParameter& geometryPathDInputParameter,
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths) {
		BuildGeometryPathDD(3, 10, geometryPathDInputParameter, paths);
	}

	void BuildGeometryPathDDsAndParameter(
		int numbersOfGuess,
		int maxLevel,
		const std::vector<GeometryPathDDInputParameter>& geometryPathDInputParameters,
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths) {
		//默认0-1
		RangeDoubleStd::Range2Double range2Double;
		//初始化猜测得值
		auto variableObjectsOfGuess = Equation2VariableObjectStd::GenerateSelfVariableObjectsByGridMethod(
			numbersOfGuess, range2Double);
		for (int loop = 0; loop < geometryPathDInputParameters.size(); ++loop) {
			BuildGeometryPathDDCore(maxLevel, variableObjectsOfGuess, geometryPathDInputParameters[loop], paths);
		}

	}
	void BuildGeometryPathDDs(
		const std::vector<GeometryPathDDInputParameter>& geometryPathDInputParameters,
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths) {
		BuildGeometryPathDDsAndParameter(3, 10, geometryPathDInputParameters, paths);
	}

}