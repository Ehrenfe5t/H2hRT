

#include "0.SolveOneTimeDiffractionPathByEquationModule.Impl.Input.h"

#include "SotdpbeRay3DIntersectTriangle3DBallBvhTree.h"

#include<iostream>


namespace SolveOneTimeDiffractionPathByEquationStd {
	const double Eps = 1e-9;
	const double Pi = 3.141592653589793238462643383279;

	int OneNumberIsZeroByEps(double d)
	{
		/*if (eps < 0)
		{
			std::cout << "OneNumberIsZeroByEps(double d,double eps)传入了非法参数\n";
			return 0;
		}*/
		if (d > -Eps && d < Eps)
		{
			return 1;
		}
		return 0;
	}

	Point3D CreatePoint3D(double x, double y, double z) {
		Point3D res;
		res.x = x;
		res.y = y;
		res.z = z;
		return res;
	}

	double DotPoint3DPoint3D(const Point3D& p1, const Point3D& p2)
	{
		return p1.x * p2.x + p1.y * p2.y + p1.z * p2.z;
	}


	/// <summary>
	/// 计算两个点的和
	/// </summary>
	/// <param name="p1">点1</param>
	/// <param name="p2">点2</param>
	/// <returns>两个点的和</returns>
	Point3D AddPoint3DPoint3D(const Point3D& p1, const Point3D& p2)
	{
		return CreatePoint3D(p1.x + p2.x, p1.y + p2.y, p1.z + p2.z);
	}
	/// <summary>
	/// 计算向量长度
	/// </summary>
	/// <param name="p"></param>
	/// <returns></returns>
	double Length_Point3D(const Point3D& p)
	{
		return sqrt(DotPoint3DPoint3D(p, p));
	}

	Point3D MulDoublePoint3D(double d, const Point3D& p)
	{
		return CreatePoint3D(d * p.x, d * p.y, d * p.z);
	}

	Point3D SubPoint3DPoint3D(const Point3D& v1, const Point3D& v2) {
		return CreatePoint3D(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
	}

	/// <summary>
	/// 计算两个点的叉乘
	/// </summary>
	/// <param name="p1">点1</param>
	/// <param name="p2">点2</param>
	/// <returns>两个点的叉乘</returns>
	Point3D CrossPoint3DPoint3D(const Point3D& p1, const Point3D& p2)
	{
		double l = p1.x;
		double m = p1.y;
		double n = p1.z;
		double o = p2.x;
		double p = p2.y;
		double q = p2.z;
		Point3D res = CreatePoint3D(m * q - n * p, n * o - l * q, l * p - m * o);

		return res;
	}

	bool GetRelativeXYZPlus_DelX(
		double x1,
		double y1,
		double z1,
		double x2,
		double y2,
		double z2,
		double x3,
		double y3,
		double z3,
		double x4,
		double y4,
		double z4,
		double& x,
		double& y,
		double& z) {


		double d11 = x3 * y2 - y3 * x2;
		double d12 = x4 * y2 - y4 * x2;
		double d13 = x1 * y2 - y1 * x2;

		double d21 = x3 * z2 - x2 * z3;
		double d22 = x4 * z2 - x2 * z4;
		double d23 = x1 * z2 - x2 * z1;

		double t1 = d12 * d21 - d11 * d22;//计算z

		double t2 = d11 * d22 - d12 * d21;//计算y
		if (abs(t1) <= Eps) {
			if (abs(t2) <= Eps) {
				return false;
			}
			else {
				//t2不是0
				y = (d13 * d22 - d12 * d23) / t2;
				z = (d13 + d23 - (d11 + d21) * y) / (d12 + d22);
			}
		}
		else {
			//t1不是0
			z = (d13 * d21 - d11 * d23) / t1;
			y = (d13 + d23 - (d12 + d22) * z) / (d11 + d21);
		}

		x = ((x1 + y1 + z1) - y * (x3 + y3 + z3) - z * (x4 + y4 + z4)) / (x2 + y2 + z2);
		return true;
	}

	bool GetRelativeXYZPlus_DelY(
		double x1,
		double y1,
		double z1,
		double x2,
		double y2,
		double z2,
		double x3,
		double y3,
		double z3,
		double x4,
		double y4,
		double z4,
		double& x,
		double& y,
		double& z) {


		double d11 = x2 * y3 - x3 * y2;
		double d12 = x4 * y3 - x3 * y4;
		double d13 = x1 * y3 - x3 * y1;

		double d21 = x2 * z3 - x3 * z2;
		double d22 = x4 * z3 - x3 * z4;
		double d23 = x1 * z3 - x3 * z1;

		double t1 = d12 * d21 - d11 * d22;//计算z

		double t2 = d11 * d22 - d12 * d21;//计算x
		if (abs(t1) <= Eps) {
			if (abs(t2) <= Eps) {
				return false;
			}
			else {
				//t2不是0
				x = (d13 * d22 - d12 * d23) / t2;
				z = (d13 + d23 - (d11 + d21) * x) / (d12 + d22);
			}
		}
		else {
			//t1不是0
			z = (d13 * d21 - d11 * d23) / t1;
			x = (d13 + d23 - (d12 + d22) * z) / (d11 + d21);
		}
		y = ((x1 + y1 + z1) - x * (x2 + y2 + z2) - z * (x4 + y4 + z4)) / (x3 + y3 + z3);
		return true;
	}

	bool GetRelativeXYZPlus_DelZ(
		double x1,
		double y1,
		double z1,
		double x2,
		double y2,
		double z2,
		double x3,
		double y3,
		double z3,
		double x4,
		double y4,
		double z4,
		double& x,
		double& y,
		double& z) {


		double d11 = x2 * y4 - x4 * y2;
		double d12 = x3 * y4 - x4 * y3;
		double d13 = x1 * y4 - x4 * y1;

		double d21 = x2 * z4 - x4 * z2;
		double d22 = x3 * z4 - x4 * z3;
		double d23 = x1 * z4 - x4 * z1;

		double t1 = d12 * d21 - d11 * d22;//计算z

		double t2 = d11 * d22 - d12 * d21;//计算x
		if (abs(t1) <= Eps) {
			if (abs(t2) <= Eps) {
				return false;
			}
			else {
				//t2不是0
				x = (d13 * d22 - d12 * d23) / t2;
				y = (d13 + d23 - (d11 + d21) * x) / (d12 + d22);
			}
		}
		else {
			//t1不是0
			y = (d13 * d21 - d11 * d23) / t1;
			x = (d13 + d23 - (d12 + d22) * y) / (d11 + d21);
		}
		z = ((x1 + y1 + z1) - x * (x2 + y2 + z2) - y * (x3 + y3 + z3)) / (x4 + y4 + z4);
		return true;
	}


	Point3D GetRelativeXYZPlus(
		const Point3D& o1,
		const Point3D& ex,
		const Point3D& ey,
		const Point3D& ez,
		const Point3D& p0)
	{
		Point3D op = SubPoint3DPoint3D(p0, o1);
		double x1 = op.x;
		double y1 = op.y;
		double z1 = op.z;

		double x2 = ex.x;
		double y2 = ex.y;
		double z2 = ex.z;

		double x3 = ey.x;
		double y3 = ey.y;
		double z3 = ey.z;

		double x4 = ez.x;
		double y4 = ez.y;
		double z4 = ez.z;

		double x = 0.0;
		double y = 0.0;
		double z = 0.0;

		if (GetRelativeXYZPlus_DelX(x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, x, y, z)) {
			return CreatePoint3D(x, y, z);
		}

		if (GetRelativeXYZPlus_DelY(x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, x, y, z)) {
			return CreatePoint3D(x, y, z);
		}

		if (GetRelativeXYZPlus_DelZ(x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, x, y, z)) {
			return CreatePoint3D(x, y, z);
		}

		return CreatePoint3D(x, y, z);
	}


	double GetAngleByXY(double x, double y) {
		if (OneNumberIsZeroByEps(x) == 1) {
			if (y > 0.0) {
				return 0.5 * Pi;
			}
			else if (y < 0.0) {
				return 1.5 * Pi;
			}
			//std::cout << "file =" << __FILE__ << "\tline =" << __LINE__ << "\t";
			//std::cout << "GetAngleByXY warning 1!\n";
			return 0;
		}
		if (OneNumberIsZeroByEps(y) == 1) {
			if (x > 0.0) {
				return 0.0;
			}
			else if (x < 0.0) {
				return Pi;
			}
			std::cout << "GetAngleByXY warning 2!\n";
			return 0;
		}
		double ddd = atan2(y, x);
		if (y < 0) {
			return 2 * Pi + ddd;
		}
		return ddd;
	}

}

namespace SolveOneTimeDiffractionPathByEquationStd {


	struct SotdpbeTriangle3DMaterial
	{

		int materialIndex1=0;
		int materialIndex2=0;

		float roughness = 0.0f;
		Point3D p1;
		Point3D p2;
		Point3D p3;
		Point3D n;

	};

	struct SotdpbeCorner3DMaterial
	{
		double phiE = 0.0;
		int materialIndex11 = 0;
		int materialIndex12 = 0;
		int materialIndex21 = 0;
		int materialIndex22 = 0;
		Point3D cornerStart;
		Point3D cornerEnd;
		Point3D cornerN1;
		Point3D cornerN2;
		Point3D cornerT1p3;
		Point3D cornerT2p3;

		/// <summary>
		/// 局部坐标系
		/// </summary>
		Point3D o;
		Point3D x;
		Point3D y;
		Point3D z;
	};

	void CalculateLocalCoordinateSystem(SotdpbeCorner3DMaterial& cornerMaterial) {

		Point3D p1 = cornerMaterial.cornerStart;
		Point3D p2 = cornerMaterial.cornerEnd;
		Point3D p3 = cornerMaterial.cornerT1p3;
		Point3D p4 = cornerMaterial.cornerT2p3;

		Point3D z1 = SubPoint3DPoint3D(p2, p1);
		z1 = MulDoublePoint3D(1.0 / Length_Point3D(z1), z1);
		Point3D t1 = SubPoint3DPoint3D(p3, p1);
		double len1 = DotPoint3DPoint3D(z1, t1);
		Point3D o = AddPoint3DPoint3D(MulDoublePoint3D(len1, z1), p1);
		Point3D x = SubPoint3DPoint3D(p3, o);
		x = MulDoublePoint3D(1.0 / Length_Point3D(x), x);
		Point3D y1 = CrossPoint3DPoint3D(z1, x);

		Point3D p5 = AddPoint3DPoint3D(o, cornerMaterial.cornerN1);
		{
			Point3D relative_1_13 = GetRelativeXYZPlus(o, x, y1, z1, p3);
			Point3D relative_1_14 = GetRelativeXYZPlus(o, x, y1, z1, p4);
			Point3D relative_1_15 = GetRelativeXYZPlus(o, x, y1, z1, p5);


			double phi_1_13 = GetAngleByXY(relative_1_13.x, relative_1_13.y);
			double phi_1_14 = GetAngleByXY(relative_1_14.x, relative_1_14.y);
			double phi_1_15 = GetAngleByXY(relative_1_15.x, relative_1_15.y);

			if (OneNumberIsZeroByEps(phi_1_13) == 1) {
				if (OneNumberIsZeroByEps(phi_1_15 - 0.5 * Pi) == 1) {
					cornerMaterial.phiE = phi_1_14;
					cornerMaterial.o = o;
					cornerMaterial.x = x;
					cornerMaterial.y = y1;
					cornerMaterial.z = z1;
					return;
				}
			}
		}

		{
			Point3D y2;
			y2.x = -y1.x;
			y2.y = -y1.y;
			y2.z = -y1.z;
			Point3D z2 = CrossPoint3DPoint3D(x, y2);

			Point3D relative_2_13 = GetRelativeXYZPlus(o, x, y2, z2, p3);
			Point3D relative_2_14 = GetRelativeXYZPlus(o, x, y2, z2, p4);
			Point3D relative_2_15 = GetRelativeXYZPlus(o, x, y2, z2, p5);

			double phi_2_13 = GetAngleByXY(relative_2_13.x, relative_2_13.y);
			double phi_2_14 = GetAngleByXY(relative_2_14.x, relative_2_14.y);
			double phi_2_15 = GetAngleByXY(relative_2_15.x, relative_2_15.y);

			if (OneNumberIsZeroByEps(phi_2_13) == 1) {
				if (OneNumberIsZeroByEps(phi_2_15 - 0.5 * Pi) == 1) {
					cornerMaterial.phiE = phi_2_14;
					cornerMaterial.o = o;
					cornerMaterial.x = x;
					cornerMaterial.y = y2;
					cornerMaterial.z = z2;
					return;
				}
			}

		}

		//无法构建，请检查数据
		std::cout << "Can't build, please check the data.!\n";

	}


	std::vector<SotdpbeTriangle3DMaterial> globalTriangle3DMaterial;
	std::vector<SotdpbeCorner3DMaterial> globalCorner3DMaterial;
	MaterialSet globalMaterialSet;

	int GetMaterialIndex(short materialTypeNumber, long long frequency) {

		for (int i = 0; i < globalMaterialSet.size; i++) {
			if (globalMaterialSet.materials[i].materialTypeNumber == materialTypeNumber
				&& globalMaterialSet.materials[i].frequency == frequency) {
				return i;
			}
		}

		std::cout << "GetMaterialIndex: not found materialTypeNumber=" << materialTypeNumber << std::endl;
		std::cout << "GetMaterialIndex: not found frequency=" << frequency << std::endl;
		return -1;

	}


	void InitScenarioAndMaterial(
		long long frequency,
		const Scenario3D& scenario,
		const MaterialSet& materialSet) {
		globalMaterialSet = materialSet;

		globalTriangle3DMaterial.clear();
		globalCorner3DMaterial.clear();

		globalTriangle3DMaterial.resize(scenario.trianglesCount);
		for (int i = 0; i < scenario.trianglesCount; i++) {

			auto tt = scenario.scenario_triangle3d_set[i];
			globalTriangle3DMaterial[i].n = tt.n;
			globalTriangle3DMaterial[i].roughness = tt.roughness;

			auto p1 = scenario.scenario_point3d_set[tt.triangleP1Index];
			auto p2 = scenario.scenario_point3d_set[tt.triangleP2Index];
			auto p3 = scenario.scenario_point3d_set[tt.triangleP3Index];

			int materialIndex1 = GetMaterialIndex(tt.topMaterialTypeNumber, frequency);
			int materialIndex2 = GetMaterialIndex(tt.bottomMaterialTypeNumber, frequency);
			globalTriangle3DMaterial[i].p1 = p1;
			globalTriangle3DMaterial[i].p2 = p2;
			globalTriangle3DMaterial[i].p3 = p3;
			globalTriangle3DMaterial[i].materialIndex1 = materialIndex1;
			globalTriangle3DMaterial[i].materialIndex2 = materialIndex2;
		}

		globalCorner3DMaterial.resize(scenario.cornersCount);
		for (int i = 0; i < scenario.cornersCount; i++) {
			auto cc = scenario.scenario_corner3d_set[i];

			auto t1 = scenario.scenario_triangle3d_set[cc.face1Index];
			auto t2 = scenario.scenario_triangle3d_set[cc.face2Index];
			globalCorner3DMaterial[i].cornerStart = scenario.scenario_point3d_set[cc.p1Index];
			globalCorner3DMaterial[i].cornerEnd = scenario.scenario_point3d_set[cc.p2Index];
			globalCorner3DMaterial[i].cornerN1 = t1.n;
			globalCorner3DMaterial[i].cornerN2 = t2.n;
			globalCorner3DMaterial[i].cornerT1p3 = scenario.scenario_point3d_set[cc.p3Face1Index];
			globalCorner3DMaterial[i].cornerT2p3 = scenario.scenario_point3d_set[cc.p3Face2Index];

			globalCorner3DMaterial[i].materialIndex11 = globalTriangle3DMaterial[cc.face1Index].materialIndex1;
			globalCorner3DMaterial[i].materialIndex12 = globalTriangle3DMaterial[cc.face1Index].materialIndex2;
			globalCorner3DMaterial[i].materialIndex21 = globalTriangle3DMaterial[cc.face2Index].materialIndex1;
			globalCorner3DMaterial[i].materialIndex22 = globalTriangle3DMaterial[cc.face2Index].materialIndex2;

			//计算局部坐标系
			CalculateLocalCoordinateSystem(globalCorner3DMaterial[i]);
		}
	}
}

namespace SolveOneTimeDiffractionPathByEquationStd {


	double GetDistancePoint3DPoint3D(
		const Point3D& p1,
		const Point3D& p2) {
		Point3D temp111 = SubPoint3DPoint3D(p1, p2);
		return Length_Point3D(temp111);
	}

	bool Location_Point3DonLineSegment3D(
		const Point3D& b, 
		const Point3D& a, 
		const Point3D& c) {
		double distaceAB = GetDistancePoint3DPoint3D(a, b);
		double ac = GetDistancePoint3DPoint3D(a, c);
		double distancBC = GetDistancePoint3DPoint3D(b, c);
		if (abs(distaceAB + distancBC - ac) <= Eps) {
			return true;
		}
		return false;
	}

	bool CreateLine3DByTwoPoint3D_safe(
		const Point3D& start, 
		const Point3D& end, 
		Point3D& lineVec) {
		Point3D vec = SubPoint3DPoint3D(end, start);
		double len = Length_Point3D(vec);
		if (OneNumberIsZeroByEps(len) == 1)
		{
			return false;
		}
		lineVec = MulDoublePoint3D(1.0 / len, vec);
		return true;
	}


	bool Equals_Point3D(const Point3D& p1, const Point3D& p2)
	{
		Point3D temp111 = SubPoint3DPoint3D(p1, p2);
		double dis = Length_Point3D(temp111);
		if (dis <= Eps)
		{
			return true;
		}
		return false;
	}
	

	Point3D Normalization_Point3D(const Point3D& p)
	{
		double len = Length_Point3D(p);
		if (len < Eps * Eps)
		{
			std::cout << "Point3DNormalization传入了非法参数" << std::endl;
			return CreatePoint3D(0.0, 0.0, 0.0);
		}
		return MulDoublePoint3D(1 / len, p);
	}

	double GetAnglePoint3DPoint3D(const Point3D& a, const Point3D& b)
	{
		double f = Length_Point3D(a) * Length_Point3D(b);
		if (f <= Eps)
		{
			const Point3D a2 = Normalization_Point3D(a);
			const Point3D b2 = Normalization_Point3D(b);
			f = Length_Point3D(a2) * Length_Point3D(b2);
			if (f <= Eps)
			{
				std::cout << "GetAnglePoint3DPoint3D，向量长度为0无法计算夹角大小" << std::endl;
				return 0.0;
			}
		}
		double d1 = DotPoint3DPoint3D(a, b);
		return acos(abs(d1) / f);
	}

	Point3D CalculateShadowPoint3DOfPoint3DonLine3D_plus_unsafe_height(
		const Point3D& obj, 
		const Point3D& lineO, 
		const Point3D& lineVec,
		double& height) {

		if (Equals_Point3D(obj, lineO)) {
			return obj;
		}
		Point3D pa = SubPoint3DPoint3D(obj, lineO);
		double lenpa = Length_Point3D(pa);
		double theta = GetAnglePoint3DPoint3D(lineVec, pa);
		height = lenpa * sin(theta);
		Point3D res;
		if (DotPoint3DPoint3D(lineVec, pa) > 0) {
			Point3D temp_1 = MulDoublePoint3D(lenpa * cos(theta), lineVec);
			res = AddPoint3DPoint3D(temp_1, lineO);
		}
		else {
			Point3D temp_1 = MulDoublePoint3D(-lenpa * cos(theta), lineVec);
			res = AddPoint3DPoint3D(temp_1, lineO);
		}
		return res;
	}

	/// <summary>
	/// 解析法解析绕射点
	/// </summary>
	/// <param name="tx_location"></param>
	/// <param name="rx_location"></param>
	/// <param name="segmentStart"></param>
	/// <param name="segmentEnd"></param>
	/// <param name="node_location"></param>
	/// <returns></returns>
	bool BuildGeometryPathDByAnalyticalSolution_node_line(
		const Point3D& tx_location,
		const Point3D& rx_location,
		const Point3D& segmentStart,
		const Point3D& segmentEnd,
		Point3D& node_location) {

		auto p1 = tx_location;
		auto p2 = rx_location;
		auto p3 = segmentStart;
		auto p4 = segmentEnd;

		Point3D lineP, lineVec;
		if (CreateLine3DByTwoPoint3D_safe(p3, p4,  lineVec)) {
			lineP = p3;
			double h1, h2;
			auto E = CalculateShadowPoint3DOfPoint3DonLine3D_plus_unsafe_height(p1, lineP, lineVec, h1);
			auto C = CalculateShadowPoint3DOfPoint3DonLine3D_plus_unsafe_height(p2, lineP, lineVec, h2);

			double k = GetDistancePoint3DPoint3D(C, E);
			if (k < Eps) {
				node_location = C;
				return true;
			}
			double k2 = h2 / (h1 + h2);
			auto CE = SubPoint3DPoint3D(E, C);
			auto temp1 = MulDoublePoint3D(k2, CE);
			auto p10 = AddPoint3DPoint3D(C, temp1);

			node_location = p10;
			return true;
		}

		return false;
	}


	double GetThetaI(const Point3D& v1, const Point3D& n1) {
		if (DotPoint3DPoint3D(n1, v1) > 0) {
			return GetAnglePoint3DPoint3D(v1, n1);
		}
		else {
			Point3D v2(-v1.x, -v1.y, -v1.z);
			return GetAnglePoint3DPoint3D(v2, n1);
		}
	}

	bool Calculate_beta0_phi1_phi2(
		long long frequency,
		const SotdpbeCorner3DMaterial& cornerMaterial,
		const Point3D& a, const Point3D& b, const Point3D& c,
		double& beta0,
		double& phi1,
		double& phi2,
		int& materialIndex1,
		int& materialIndex2,
		Point3D& n) {


		Point3D ab = SubPoint3DPoint3D(b, a);
		{
			double dot1 = DotPoint3DPoint3D(ab, cornerMaterial.cornerN1);
			double dot2 = DotPoint3DPoint3D(ab, cornerMaterial.cornerN2);
			if (dot1 > 0 && dot2 > 0) {
				return false;
			}
		}
		{
			Point3D cb = SubPoint3DPoint3D(b, c);
			double dot1 = DotPoint3DPoint3D(cb, cornerMaterial.cornerN1);
			double dot2 = DotPoint3DPoint3D(cb, cornerMaterial.cornerN2);
			if (dot1 > 0 && dot2 > 0) {
				return false;
			}
		}
		

		beta0 = GetThetaI(ab, cornerMaterial.z);
		double phiE = cornerMaterial.phiE;
		Point3D relative_a = GetRelativeXYZPlus(cornerMaterial.o, cornerMaterial.x, cornerMaterial.y, cornerMaterial.z, a);
		Point3D relative_c = GetRelativeXYZPlus(cornerMaterial.o, cornerMaterial.x, cornerMaterial.y, cornerMaterial.z, c);

		double phi_a = GetAngleByXY(relative_a.x, relative_a.y);
		
		double phi_c = GetAngleByXY(relative_c.x, relative_c.y);
		

		if (phi_a < phiE - phi_a) {
			//当前面就是0面
			phi1 = phi_a;
			phi2 = phi_c;
			materialIndex1 = GetMaterialIndex(globalMaterialSet.materials[cornerMaterial.materialIndex11].materialTypeNumber, frequency);
			materialIndex2 = GetMaterialIndex(globalMaterialSet.materials[cornerMaterial.materialIndex12].materialTypeNumber, frequency);
			n = cornerMaterial.cornerN1;
		}
		else {
			phi1 = phiE - phi_a;
			phi2 = phiE - phi_c;
			materialIndex1 = GetMaterialIndex(globalMaterialSet.materials[cornerMaterial.materialIndex21].materialTypeNumber, frequency);
			materialIndex2 = GetMaterialIndex(globalMaterialSet.materials[cornerMaterial.materialIndex22].materialTypeNumber, frequency);
			n = cornerMaterial.cornerN2;
		}

		return true;

	}

	bool CalculateOneTimeDiffractionNode(
		long long frequency,
		const Point3D& tx_location,
		const Point3D& rx_location,
		const SotdpbeCorner3DMaterial& cornerMaterial,
		double& beta0,
		double& phi1,
		double& phi2,
		int& materialIndex1,
		int& materialIndex2,
		Point3D& n,
		Point3D& oneTimeDiffractionNodeLocation) {

		bool state = BuildGeometryPathDByAnalyticalSolution_node_line(
			tx_location,rx_location, cornerMaterial.cornerStart, cornerMaterial.cornerEnd, oneTimeDiffractionNodeLocation);

		if (state) {
			//判断点是否在线段上
			if (!Location_Point3DonLineSegment3D(oneTimeDiffractionNodeLocation, cornerMaterial.cornerStart, cornerMaterial.cornerEnd)) {
				return false;
			}

			//形成的绕射是否在绕射角内
			if (!Calculate_beta0_phi1_phi2(frequency,cornerMaterial, tx_location, oneTimeDiffractionNodeLocation, rx_location, beta0, phi1, phi2,
				materialIndex1, materialIndex2,n)) {
				return false;
			}

		}
		else {
			return false;
		}

		return true;
	}


	bool CalculateCanSeeNode(
		double cornerRadius, 
		const Point3D& segStart,
		const Point3D& segEnd) {

		Point3D lineVec = SubPoint3DPoint3D(segEnd, segStart);
		double len = Length_Point3D(lineVec);
		lineVec = MulDoublePoint3D(1.0 / len, lineVec);

		SotdpbeRay3DIntersectTriangle3DBallBvhTreeStd::SotdpbeRay3DIntersectGeometry3DElementResult result;
		SotdpbeRay3DIntersectTriangle3DBallBvhTreeStd::CalculateRay3DIntersectTriangle3DAabbBvhTreeFirst(segStart, lineVec, result);
		if (result.distance + cornerRadius < len) {
			return false;
		}
		return true;

	}

	void SolveOneTimeDiffractionPathByEquationSISO_OneCorner(
		int corner_index,
		double cornerRadius,
		const TransmitterAntenna& transmitterAntenna,
		const ReceiverAntenna& receiver,
		std::list<std::vector<ElectricFieldNode>>& result) {

		const SotdpbeCorner3DMaterial& cornerMaterial = globalCorner3DMaterial[corner_index];
		//先计算是否有结果，
		//计算是否有一个绕射点
		Point3D oneTimeDiffractionNodeLocation;

		double beta0;
		double phi1;
		double phi2;


		Point3D n;
		int materialIndex1 = -1;
		int materialIndex2 = -1;

		bool state1 = CalculateOneTimeDiffractionNode(
			transmitterAntenna.frequencyBandwidth.frequencys[0],
			transmitterAntenna.location,
			receiver.location,
			cornerMaterial, 
			beta0,
			phi1, 
			phi2,
			materialIndex1,
			materialIndex2, n,
			oneTimeDiffractionNodeLocation);
		
		if (!state1) {
			return;
		}


		//计算是否遮挡
		bool state2 = CalculateCanSeeNode(
			cornerRadius,
			transmitterAntenna.location,
			oneTimeDiffractionNodeLocation);
		if (!state2) {
			return;
		}

		bool state3 = CalculateCanSeeNode(
			cornerRadius,
			receiver.location,
			oneTimeDiffractionNodeLocation);
		if (!state3) {
			return;
		}

		double length1 = GetDistancePoint3DPoint3D(transmitterAntenna.location, oneTimeDiffractionNodeLocation);
		double length2 = GetDistancePoint3DPoint3D(oneTimeDiffractionNodeLocation, receiver.location);

		std::vector<ElectricFieldNode> path(3);
		path[0].attachmentNumber = transmitterAntenna.transmitterAntennaId;
		path[0].location_x = transmitterAntenna.location.x;
		path[0].location_y = transmitterAntenna.location.y;
		path[0].location_z = transmitterAntenna.location.z;
		path[0].type = 0;
		path[0].pre_distance = 0.0;
		path[0].next_distance = length1;
		path[0].materialIndex1 = GetMaterialIndex(transmitterAntenna.materialTypeNumber, transmitterAntenna.frequencyBandwidth.frequencys[0]);
		path[0].materialIndex2 = path[0].materialIndex1;

		path[1].attachmentNumber = corner_index;
		path[1].location_x = oneTimeDiffractionNodeLocation.x;
		path[1].location_y = oneTimeDiffractionNodeLocation.y;
		path[1].location_z = oneTimeDiffractionNodeLocation.z;
		path[1].r_t_s_d_thetai_beta0 = beta0;
		path[1].d_phi1_s_thetar = phi1;
		path[1].d_phi2_s_ar = phi2;
		path[1].d_phiE_s_s = cornerMaterial.phiE;
		path[1].materialIndex1 = materialIndex1;
		path[1].materialIndex2 = materialIndex2;
		path[1].n_x = n.x;
		path[1].n_y = n.y;
		path[1].n_z = n.z;
		path[1].type = 4;
		path[1].pre_distance = length1;
		path[1].next_distance = length2;

		path[2].attachmentNumber = receiver.receiverAntennaId;
		path[2].location_x = receiver.location.x;
		path[2].location_y = receiver.location.y;
		path[2].location_z = receiver.location.z;
		path[2].type = 1;
		path[2].next_distance = 0.0;
		path[2].pre_distance = length1 + length2;
		path[2].materialIndex1 = path[0].materialIndex1;
		path[2].materialIndex2 = path[0].materialIndex2;

		result.emplace_back(path);

	}

	void SolveOneTimeDiffractionPathByEquationSISO(
		double cornerRadius,
		const TransmitterAntenna& transmitterAntenna,
		const ReceiverAntenna& receiver,
		std::list<std::vector<ElectricFieldNode>>& result) {

		for (int i = 0; i < globalCorner3DMaterial.size(); i++) {

			SolveOneTimeDiffractionPathByEquationSISO_OneCorner(i, cornerRadius, transmitterAntenna, receiver,  result);
			
		}

	}

	/// <summary>
	/// 一次绕射，没有任何遮挡
	/// </summary>
	/// <param name="transmitterAntenna"></param>
	/// <param name="scenario"></param>
	/// <param name="result"></param>
	void SolveOneTimeDiffractionPathByEquation(
		double cornerRadius,
		const TransmitterAntenna& transmitterAntenna,
		const Scenario3D& scenario,
		const MaterialSet& materialSet,
		std::vector<std::list<std::vector<ElectricFieldNode>>>& result) {

		std::cout << "进行一次绕射计算.开始..." << std::endl;
		//这个模块可以完全独立运行

		//初始化棱边的数据
		InitScenarioAndMaterial(transmitterAntenna.frequencyBandwidth.frequencys[0], scenario, materialSet);
		//初始化场景的加速结构,这里只需要三角形的加速结构即可
		SotdpbeRay3DIntersectTriangle3DBallBvhTreeStd::InitializeAabbBvhTree(scenario);

		result.clear();
		result.resize(transmitterAntenna.receiversCount);

		int printf_jg = transmitterAntenna.receiversCount / 20;
		if (printf_jg < 2) {
			printf_jg = 2;
		}
		for (int i = 0; i < transmitterAntenna.receiversCount; i++) {

			SolveOneTimeDiffractionPathByEquationSISO(cornerRadius, transmitterAntenna, transmitterAntenna.receivers[i], result[i]);

			if (i % printf_jg == 0) {
				std::cout << "已完成" << ((100.00 * i) / transmitterAntenna.receiversCount) << "%计算." << std::endl;
			}
		}

		std::cout << "进行一次绕射计算.结束..." << std::endl;

	}

}