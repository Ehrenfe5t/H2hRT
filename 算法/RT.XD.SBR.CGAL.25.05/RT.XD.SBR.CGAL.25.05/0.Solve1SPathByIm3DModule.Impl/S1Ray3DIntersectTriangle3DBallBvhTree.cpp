

#include "S1Ray3DIntersectTriangle3DBallBvhTree.h"

#include<iostream>
#include<list>
#include<set>
#include<memory>
#include<algorithm>


namespace S1Ray3DIntersectTriangle3DBallBvhTreeStd {


	double GetAxis(Point3D p, int axis) {
		switch (axis)
		{
		case 0:return p.x;
		case 1:return p.y;
		case 2:return p.z;
		default: break;
		}
		std::cout << "Error: Invalid axis" << std::endl;
		return 0.0;
	}

	bool IsZero(double d) {
		if (d <= Eps)
		{
			return true;
		}
		return false;
	}

	bool IsZeroAbs(double d) {
		return IsZero(abs(d));
	}

	Point3D SubPoint3DPoint3D(const Point3D& p1, const Point3D& p2)
	{
		Point3D result;
		result.x = p1.x - p2.x;
		result.y = p1.y - p2.y;
		result.z = p1.z - p2.z;
		return result;
	}


	Point3D CrossPoint3DPoint3D(const Point3D& p1, const Point3D& p2)
	{
		double l = p1.x;
		double m = p1.y;
		double n = p1.z;
		double o = p2.x;
		double p = p2.y;
		double q = p2.z;
		Point3D res;
		res.x = m * q - n * p;
		res.y = n * o - l * q;
		res.z = l * p - m * o;
		return res;
	}

	double DotPoint3DPoint3D(const Point3D& p1, const Point3D& p2)
	{
		return p1.x * p2.x + p1.y * p2.y + p1.z * p2.z;
	}

	double Length_Point3D(const Point3D& p)
	{
		return sqrt(DotPoint3DPoint3D(p, p));
	}

	Point3D MulDoublePoint3D(double d, const Point3D& p)
	{
		Point3D result;
		result.x = d * p.x;
		result.y = d * p.y;
		result.z = d * p.z;
		return result;
	}

	Point3D Normalization_Point3D(const Point3D& p)
	{
		Point3D result;
		double len = Length_Point3D(p);
		if (len < Eps * Eps)
		{
			std::cout << "Point3DNormalization传入了非法参数" << std::endl;
			return result;
		}
		return MulDoublePoint3D(1 / len, p);
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

	Point3D AddPoint3DPoint3D(const Point3D& p1, const Point3D& p2)
	{
		Point3D result;
		result.x = p1.x + p2.x;
		result.y = p1.y + p2.y;
		result.z = p1.z + p2.z;
		return result;
	}

	double GetDistancePoint3DPoint3D(const Point3D& p1, const Point3D& p2)
	{
		Point3D temp111 = SubPoint3DPoint3D(p1, p2);
		return Length_Point3D(temp111);
	}

	double CalMartix33(double num[3][3]) {
		return num[0][0] * num[1][1] * num[2][2] +
			num[0][1] * num[1][2] * num[2][0] +
			num[0][2] * num[1][0] * num[2][1] -
			num[2][0] * num[1][1] * num[0][2] -
			num[2][1] * num[1][2] * num[0][0] -
			num[1][0] * num[0][1] * num[2][2];
	}

	double GetDistanceLine3DLine3D(
		const Point3D& o1, const Point3D& d1,
		const Point3D& o2, const Point3D& d2,
		double& distance1, double& distance2, Point3D& res1, Point3D& res2) {
		Point3D d3 = CrossPoint3DPoint3D(d1, d2);
		Point3D o = SubPoint3DPoint3D(o2, o1);
		double D[3][3];
		D[0][0] = d1.x;
		D[1][0] = d1.y;
		D[2][0] = d1.z;
		D[0][1] = -d2.x;
		D[1][1] = -d2.y;
		D[2][1] = -d2.z;
		D[0][2] = d3.x;
		D[1][2] = d3.y;
		D[2][2] = d3.z;

		double D1[3][3];
		D1[0][0] = o.x;
		D1[1][0] = o.y;
		D1[2][0] = o.z;
		D1[0][1] = -d2.x;
		D1[1][1] = -d2.y;
		D1[2][1] = -d2.z;
		D1[0][2] = d3.x;
		D1[1][2] = d3.y;
		D1[2][2] = d3.z;
		double D2[3][3];
		D2[0][0] = d1.x;
		D2[1][0] = d1.y;
		D2[2][0] = d1.z;
		D2[0][1] = o.x;
		D2[1][1] = o.y;
		D2[2][1] = o.z;
		D2[0][2] = d3.x;
		D2[1][2] = d3.y;
		D2[2][2] = d3.z;
		double D3[3][3];
		D3[0][0] = d1.x;
		D3[1][0] = d1.y;
		D3[2][0] = d1.z;
		D3[0][1] = -d2.x;
		D3[1][1] = -d2.y;
		D3[2][1] = -d2.z;
		D3[0][2] = o.x;
		D3[1][2] = o.y;
		D3[2][2] = o.z;

		double martixD = CalMartix33(D);
		if (IsZeroAbs(martixD)) {
			return RayMaxMovingDistance;
		}
		double martixD1 = CalMartix33(D1);
		double martixD2 = CalMartix33(D2);
		//double martixD3 = CalMartix33(D3);
		double t1 = martixD1 / martixD;
		double t2 = martixD2 / martixD;
		//double k = martixD3 / martixD;
		Point3D p1 = AddPoint3DPoint3D(o1, MulDoublePoint3D(t1, d1));
		Point3D p2 = AddPoint3DPoint3D(o2, MulDoublePoint3D(t2, d2));
		res1 = p1;
		res2 = p2;
		distance1 = t1;
		distance2 = t2;
		return GetDistancePoint3DPoint3D(p1, p2);
	}


	bool Intersect_Ray3D_Triangle3D_Plus(
		const Point3D& O, const Point3D& rayVec,
		const Point3D& V1,
		const Point3D& E1,
		const Point3D& E2,
		const Point3D& E1E2,
		const Point3D& E2E1,
		double& distance, Point3D& res) {
		distance = RayMaxMovingDistance;
		Point3D D;
		D.x = -rayVec.x;
		D.y = -rayVec.y;
		D.z = -rayVec.z;

		Point3D T = SubPoint3DPoint3D(V1, O);
		double f1 = DotPoint3DPoint3D(D, E2E1);
		if (IsZeroAbs(f1)) {
			//除数不能是0
			return false;
		}
		double f2 = DotPoint3DPoint3D(T, E1E2);
		double t = f2 / f1;
		//此时交点在射线负轴上
		if (t < Eps)
		{
			return false;
		}
		Point3D P = CrossPoint3DPoint3D(D, E2);
		double f3 = DotPoint3DPoint3D(P, T);
		double u = f3 / f1;
		if (u < 0.0)
		{
			//在三角形的平面内但不在三角形内
			return false;
		}
		Point3D Q = CrossPoint3DPoint3D(T, E1);
		double f4 = DotPoint3DPoint3D(Q, D);
		double v = f4 / f1;
		if (v < 0.0 || u + v > 1.0f)
		{
			//在三角形的平面内但不在三角形内
			return false;
		}
		distance = t;
		{
			res.x = O.x + t * rayVec.x;
			res.y = O.y + t * rayVec.y;
			res.z = O.z + t * rayVec.z;
		}
		return true;
	}


	bool Ray3DIntersectBall(
		const Point3D& ray_origin,
		const Point3D& ray_direction,
		const Point3D& center,
		double radius) {

		Point3D op;
		op.x = ray_origin.x - center.x;
		op.y = ray_origin.y - center.y;
		op.z = ray_origin.z - center.z;

		double length_op = sqrt(op.x * op.x + op.y * op.y + op.z * op.z);

		if (length_op < radius) {
			return true;
		}
		double dot = op.x * ray_direction.x + op.y * ray_direction.y + op.z * ray_direction.z;
		if (dot > 0) {
			return false;
		}
		double temp_1 = length_op * length_op;
		double h_radius = sqrt(temp_1 - dot * dot);
		if (h_radius < radius) {
			//没有达到接受条件
			return true;
		}
		else {
			return false;
		}
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

	double GetThetaI(const Point3D& v1, const Point3D& n1) {
		if (DotPoint3DPoint3D(n1, v1) > 0) {
			return GetAnglePoint3DPoint3D(v1, n1);
		}
		else {
			Point3D v2(-v1.x, -v1.y, -v1.z);
			return GetAnglePoint3DPoint3D(v2, n1);
		}
	}
}


/// <summary>
/// 当前全局数据
/// </summary>
namespace S1Ray3DIntersectTriangle3DBallBvhTreeStd {

	// 精度
	double kAABBAccuracy = 0.01;
	double globalCornerRadius = 0.03;
	long long globalFrequency = (long long)3e9;
	std::vector<Triangle3DMaterial> globalTriangle3DMaterials;
	std::vector<bool> globalTriangle3DMaterialsInvalid;
	std::vector<Corner3DMaterial> globalCorner3DMaterials;
	std::vector<bool> globalCorner3DMaterialsInvalid;
	std::vector<Material> globalMaterials;

	void InitAccelerateStructMaterial(const MaterialSet& materialSet) {
		globalMaterials.clear();
		globalMaterials.resize(materialSet.size);
		for (int i = 0; i < globalMaterials.size(); ++i) {
			globalMaterials[i] = materialSet.materials[i];
		}
	}

	int GetMaterialIndex(short materialTypeNumber) {

		for (int i = 0; i < globalMaterials.size(); i++) {
			if (globalMaterials[i].materialTypeNumber == materialTypeNumber
				&& globalMaterials[i].frequency == globalFrequency) {
				return i;
			}
		}

		std::cout << "GetMaterialIndex: not found materialTypeNumber=" << materialTypeNumber << std::endl;
		std::cout << "GetMaterialIndex: not found frequency=" << globalFrequency << std::endl;
		return -1;

	}

	void InitAccelerateStructTriangle3DMaterial(const Scenario3D& scenario) {

		globalTriangle3DMaterials.clear();
		globalTriangle3DMaterials.resize(scenario.trianglesCount);
		globalTriangle3DMaterialsInvalid.clear();
		globalTriangle3DMaterialsInvalid.resize(scenario.trianglesCount, false);

		for (int loop = 0; loop < scenario.trianglesCount; ++loop) {
			Point3D p1 = scenario.scenario_point3d_set[scenario.scenario_triangle3d_set[loop].triangleP1Index];
			Point3D p2 = scenario.scenario_point3d_set[scenario.scenario_triangle3d_set[loop].triangleP2Index];
			Point3D p3 = scenario.scenario_point3d_set[scenario.scenario_triangle3d_set[loop].triangleP3Index];
			Point3D n = CrossPoint3DPoint3D(SubPoint3DPoint3D(p2, p1), SubPoint3DPoint3D(p3, p1));
			double length_n = Length_Point3D(n);
			if (IsZeroAbs(length_n)) {
				std::cout << "Triangle3DMaterial的法向量长度为0" << std::endl;
				globalTriangle3DMaterialsInvalid[loop] = true;
				continue;
			}
			n = MulDoublePoint3D(1.0 / length_n, n);
			if (DotPoint3DPoint3D(n, scenario.scenario_triangle3d_set[loop].n) < 0.0) {
				auto temp = p2;
				p2 = p3;
				p3 = temp;
			}

			Point3D E1 = SubPoint3DPoint3D(p1, p2);
			Point3D E2 = SubPoint3DPoint3D(p1, p3);
			Point3D E1E2 = CrossPoint3DPoint3D(E1, E2);
			Point3D E2E1 = CrossPoint3DPoint3D(E2, E1);

			int materialIndex1 = GetMaterialIndex(scenario.scenario_triangle3d_set[loop].topMaterialTypeNumber);
			int materialIndex2 = GetMaterialIndex(scenario.scenario_triangle3d_set[loop].bottomMaterialTypeNumber);
			globalTriangle3DMaterials[loop].scenarioTriangleP1 = p1;
			globalTriangle3DMaterials[loop].scenarioTriangleP2 = p2;
			globalTriangle3DMaterials[loop].scenarioTriangleP3 = p3;
			globalTriangle3DMaterials[loop].scenarioTriangleN = scenario.scenario_triangle3d_set[loop].n;
			globalTriangle3DMaterials[loop].scenarioE1 = E1;
			globalTriangle3DMaterials[loop].scenarioE2 = E2;
			globalTriangle3DMaterials[loop].scenarioE1E2 = E1E2;
			globalTriangle3DMaterials[loop].scenarioE2E1 = E2E1;
			globalTriangle3DMaterials[loop].roughness = (double)scenario.scenario_triangle3d_set[loop].roughness;
			globalTriangle3DMaterials[loop].materialIndex1 = materialIndex1;
			globalTriangle3DMaterials[loop].materialIndex2 = materialIndex2;
		}
	}

	bool GetRelativeXYZPlus_DelX(
		double x1, double y1, double z1,
		double x2, double y2, double z2,
		double x3, double y3, double z3,
		double x4, double y4, double z4,
		double& x, double& y, double& z) {

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
		double x1, double y1, double z1,
		double x2, double y2, double z2,
		double x3, double y3, double z3,
		double x4, double y4, double z4,
		double& x, double& y, double& z) {

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
		double x1, double y1, double z1,
		double x2, double y2, double z2,
		double x3, double y3, double z3,
		double x4, double y4, double z4,
		double& x, double& y, double& z) {

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


	Point3D GetRelativeXYZPlus(const Point3D& o1, const Point3D& ex, const Point3D& ey, const Point3D& ez, const Point3D& p0)
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

		Point3D result;
		if (GetRelativeXYZPlus_DelX(x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, result.x, result.y, result.z)) {
			return result;
		}

		if (GetRelativeXYZPlus_DelY(x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, result.x, result.y, result.z)) {
			return result;
		}

		if (GetRelativeXYZPlus_DelZ(x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, result.x, result.y, result.z)) {
			return result;
		}
		result.x = 0.0;
		result.y = 0.0;
		result.z = 0.0;
		return result;
	}


	double GetAngleByXY(double x, double y) {
		if (IsZeroAbs(x) == 1) {
			if (y > 0.0) {
				return 0.5 * Pi;
			}
			else if (y < 0.0) {
				return 1.5 * Pi;
			}
			return 0;
		}
		if (IsZeroAbs(y) == 1) {
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

	bool CalculateLocalCoordinateSystem(Corner3DMaterial& cornerMaterial) {

		Point3D p1 = cornerMaterial.cornerStart;
		Point3D p2 = cornerMaterial.cornerEnd;
		Point3D p3 = cornerMaterial.cornerT1p3;
		Point3D p4 = cornerMaterial.cornerT2p3;

		Point3D z1 = SubPoint3DPoint3D(p2, p1);
		double len_vec = Length_Point3D(z1);
		if (IsZeroAbs(len_vec)) {
			std::cout << "CalculateLocalCoordinateSystem: z1长度为0" << std::endl;
			return false;
		}
		cornerMaterial.length = len_vec;
		z1 = MulDoublePoint3D(1.0 / len_vec, z1);
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

			if (IsZeroAbs(phi_1_13) == 1) {
				if (IsZeroAbs(phi_1_15 - 0.5 * Pi) == 1) {
					cornerMaterial.phiE = phi_1_14;
					cornerMaterial.o = o;
					cornerMaterial.x = x;
					cornerMaterial.y = y1;
					cornerMaterial.z = z1;
					return true;
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

			if (IsZeroAbs(phi_2_13) == 1) {
				if (IsZeroAbs(phi_2_15 - 0.5 * Pi) == 1) {
					cornerMaterial.phiE = phi_2_14;
					cornerMaterial.o = o;
					cornerMaterial.x = x;
					cornerMaterial.y = y2;
					cornerMaterial.z = z2;
					return true;
				}
			}
		}

		//无法构建，请检查数据
		std::cout << "Can't build, please check the data.!\n";
		return false;
	}

	void InitAccelerateStructCorner3DMaterial(const Scenario3D& scenario) {

		globalCorner3DMaterials.clear();
		globalCorner3DMaterials.resize(scenario.cornersCount);
		globalCorner3DMaterialsInvalid.clear();
		globalCorner3DMaterialsInvalid.resize(scenario.cornersCount, false);

		for (int i = 0; i < scenario.cornersCount; ++i) {

			const Corner3D& cc = scenario.scenario_corner3d_set[i];
			const Triangle3D& t1 = scenario.scenario_triangle3d_set[cc.face1Index];
			const Triangle3D& t2 = scenario.scenario_triangle3d_set[cc.face2Index];

			globalCorner3DMaterials[i].cornerStart = scenario.scenario_point3d_set[cc.p1Index];
			globalCorner3DMaterials[i].cornerEnd = scenario.scenario_point3d_set[cc.p2Index];
			globalCorner3DMaterials[i].cornerN1 = t1.n;
			globalCorner3DMaterials[i].cornerN2 = t2.n;
			globalCorner3DMaterials[i].cornerT1p3 = scenario.scenario_point3d_set[cc.p3Face1Index];
			globalCorner3DMaterials[i].cornerT2p3 = scenario.scenario_point3d_set[cc.p3Face2Index];

			globalCorner3DMaterials[i].materialIndex11 = globalTriangle3DMaterials[cc.face1Index].materialIndex1;
			globalCorner3DMaterials[i].materialIndex12 = globalTriangle3DMaterials[cc.face1Index].materialIndex2;
			globalCorner3DMaterials[i].materialIndex21 = globalTriangle3DMaterials[cc.face2Index].materialIndex1;
			globalCorner3DMaterials[i].materialIndex22 = globalTriangle3DMaterials[cc.face2Index].materialIndex2;

			//计算局部坐标系
			if (!CalculateLocalCoordinateSystem(globalCorner3DMaterials[i])) {
				globalCorner3DMaterialsInvalid[i] = true;
			}
		}
	}

}

/// <summary>
/// Bvh加速结果
/// </summary>
namespace S1Ray3DIntersectTriangle3DBallBvhTreeStd {

	// AABB盒
	struct AABB {
		// AABB盒表面积
		double Surface = 0.0;
		Point3D Min;
		Point3D Max;
		// AABB盒几何元素元索引
		std::set<int> GeometryElementIndex;
	};

	// BvhNode
	struct AabbBvhTreeNode {

		// 该节点是否为叶子节点
		bool IsLeaf = true;

		double Radius = 0.0;
		Point3D Center;
		// 左叶子节点
		std::unique_ptr<AabbBvhTreeNode> Left;
		// 右叶子节点
		std::unique_ptr<AabbBvhTreeNode> Right;
		// 该节点包含的AABB盒
		AABB Bound;

	};

	// Sah优化参数
	struct SahSplit {

		// 代价
		double BestCost = 0.0;
		std::vector<int> LeftBoxesIndex;
		std::vector<int> RightBoxesIndex;
		AABB LeftBox;
		AABB RightBox;

	};

	double CalculateAABBSurfaceArea(const AABB& aabb) {

		double dx = aabb.Max.x - aabb.Min.x;
		double dy = aabb.Max.y - aabb.Min.y;
		double dz = aabb.Max.z - aabb.Min.z;

		// 若面元平行于某坐标平面，加厚AABB盒
		dx = fmax(kAABBAccuracy, dx);
		dy = fmax(kAABBAccuracy, dy);
		dz = fmax(kAABBAccuracy, dz);

		double surface = 0.02 * (dx * dy + dy * dz + dz * dx);
		return surface;
	}

	bool GetAABBByPoints(
		const std::list<Point3D>& points,
		AABB& aabb) {
		if (points.size() < 1) {
			return false;
		}

		aabb.Min = points.front();
		aabb.Max = points.front();

		for (auto& p : points) {

			aabb.Min.x = std::min(aabb.Min.x, p.x);
			aabb.Min.y = std::min(aabb.Min.y, p.y);
			aabb.Min.z = std::min(aabb.Min.z, p.z);

			aabb.Max.x = std::max(aabb.Max.x, p.x);
			aabb.Max.y = std::max(aabb.Max.y, p.y);
			aabb.Max.z = std::max(aabb.Max.z, p.z);

		}

		aabb.Min.x = aabb.Min.x - kAABBAccuracy;
		aabb.Min.y = aabb.Min.y - kAABBAccuracy;
		aabb.Min.z = aabb.Min.z - kAABBAccuracy;

		aabb.Max.x = aabb.Max.x + kAABBAccuracy;
		aabb.Max.y = aabb.Max.y + kAABBAccuracy;
		aabb.Max.z = aabb.Max.z + kAABBAccuracy;

		return true;
	}

	bool GetAABBByTriangle(
		const Scenario3D& scenario,
		int triangleIndex,
		AABB& aabb) {

		std::list<Point3D> points;
		{
			Triangle3D triangle = scenario.scenario_triangle3d_set[triangleIndex];
			Point3D p1 = scenario.scenario_point3d_set[triangle.triangleP1Index];
			Point3D p2 = scenario.scenario_point3d_set[triangle.triangleP2Index];
			Point3D p3 = scenario.scenario_point3d_set[triangle.triangleP3Index];
			points.push_back(p1);
			points.push_back(p2);
			points.push_back(p3);
		}
		if (!GetAABBByPoints(points, aabb)) {
			return false;
		}
		aabb.GeometryElementIndex.insert(triangleIndex);
		aabb.Surface = CalculateAABBSurfaceArea(aabb);

		return true;

	}

	bool GetAABBByTriangles(
		const Scenario3D& scenario,
		const std::vector<int>& indices,
		AABB& aabb) {
		std::list<Point3D> points;
		for (int i = 0; i < indices.size(); ++i) {
			int triangleIndex = indices[i];
			Triangle3D triangle = scenario.scenario_triangle3d_set[triangleIndex];
			Point3D p1 = scenario.scenario_point3d_set[triangle.triangleP1Index];
			Point3D p2 = scenario.scenario_point3d_set[triangle.triangleP2Index];
			Point3D p3 = scenario.scenario_point3d_set[triangle.triangleP3Index];
			points.push_back(p1);
			points.push_back(p2);
			points.push_back(p3);
		}
		if (!GetAABBByPoints(points, aabb)) {
			return false;
		}

		for (int triangleIndex : indices) {
			aabb.GeometryElementIndex.insert(triangleIndex);
		}
		aabb.Surface = indices.size() * CalculateAABBSurfaceArea(aabb);
		return true;

	}


	void ChangeBoxMinByPoint(const Point3D& p, Point3D& min) {

		min.x = std::min(min.x, p.x);
		min.y = std::min(min.y, p.y);
		min.z = std::min(min.z, p.z);

	}

	void ChangeBoxMaxByPoint(const Point3D& p, Point3D& max) {
		max.x = std::max(max.x, p.x);
		max.y = std::max(max.y, p.y);
		max.z = std::max(max.z, p.z);
	}


	void ChangeTriangleBoxMinMax(
		int triangleIndex,
		const Scenario3D& scenario,
		Point3D& min,
		Point3D& max) {

		const Triangle3D& triangle = scenario.scenario_triangle3d_set[triangleIndex];
		const Point3D& p1 = scenario.scenario_point3d_set[triangle.triangleP1Index];
		const Point3D& p2 = scenario.scenario_point3d_set[triangle.triangleP2Index];
		const Point3D& p3 = scenario.scenario_point3d_set[triangle.triangleP3Index];

		ChangeBoxMinByPoint(p1, min);
		ChangeBoxMaxByPoint(p1, max);

		ChangeBoxMinByPoint(p2, min);
		ChangeBoxMaxByPoint(p2, max);

		ChangeBoxMinByPoint(p3, min);
		ChangeBoxMaxByPoint(p3, max);

	}

	void ChangeCornerBoxMinMax(
		int cornerIndex,
		const Scenario3D& scenario,
		Point3D& min,
		Point3D& max) {

		const Corner3D& corner = scenario.scenario_corner3d_set[cornerIndex];
		const Point3D& p1 = scenario.scenario_point3d_set[corner.p1Index];
		const Point3D& p2 = scenario.scenario_point3d_set[corner.p2Index];

		ChangeBoxMinByPoint(p1, min);
		ChangeBoxMaxByPoint(p1, max);

		ChangeBoxMinByPoint(p2, min);
		ChangeBoxMaxByPoint(p2, max);

	}

	void FindBestSplitTriangle3DCoreHelp(
		const Scenario3D& scenario,
		std::vector<int>& sortedIndices,
		SahSplit& bestSplit) {

		int left_size = (int)sortedIndices.size() - 1;

		std::vector<Point3D> left_split_min(left_size);
		std::vector<Point3D> left_split_max(left_size);

		std::vector<Point3D> right_split_min(left_size);
		std::vector<Point3D> right_split_max(left_size);

		{
			int index = 0;
			left_split_min[index].x = std::numeric_limits<double>::max();
			left_split_min[index].y = std::numeric_limits<double>::max();
			left_split_min[index].z = std::numeric_limits<double>::max();

			left_split_max[index].x = -std::numeric_limits<double>::max();
			left_split_max[index].y = -std::numeric_limits<double>::max();
			left_split_max[index].z = -std::numeric_limits<double>::max();

			ChangeTriangleBoxMinMax(sortedIndices[index], scenario, left_split_min[index], left_split_max[index]);

			right_split_min[left_size - 1 - index].x = std::numeric_limits<double>::max();
			right_split_min[left_size - 1 - index].y = std::numeric_limits<double>::max();
			right_split_min[left_size - 1 - index].z = std::numeric_limits<double>::max();

			right_split_max[left_size - 1 - index].x = -std::numeric_limits<double>::max();
			right_split_max[left_size - 1 - index].y = -std::numeric_limits<double>::max();
			right_split_max[left_size - 1 - index].z = -std::numeric_limits<double>::max();

			ChangeTriangleBoxMinMax(sortedIndices[left_size - index], scenario, right_split_min[left_size - 1 - index], right_split_max[left_size - 1 - index]);
			index = 1;
			for (; index < left_size; ++index) {

				left_split_min[index].x = left_split_min[index - 1].x;
				left_split_min[index].y = left_split_min[index - 1].y;
				left_split_min[index].z = left_split_min[index - 1].z;

				left_split_max[index].x = left_split_max[index - 1].x;
				left_split_max[index].y = left_split_max[index - 1].y;
				left_split_max[index].z = left_split_max[index - 1].z;

				ChangeTriangleBoxMinMax(sortedIndices[index], scenario, left_split_min[index], left_split_max[index]);


				right_split_min[left_size - 1 - index].x = right_split_min[left_size - index].x;
				right_split_min[left_size - 1 - index].y = right_split_min[left_size - index].y;
				right_split_min[left_size - 1 - index].z = right_split_min[left_size - index].z;

				right_split_max[left_size - 1 - index].x = right_split_max[left_size - index].x;
				right_split_max[left_size - 1 - index].y = right_split_max[left_size - index].y;
				right_split_max[left_size - 1 - index].z = right_split_max[left_size - index].z;
				ChangeTriangleBoxMinMax(sortedIndices[left_size - index], scenario, right_split_min[left_size - 1 - index], right_split_max[left_size - 1 - index]);
			}
		}

		for (int i = 0; i < left_size; ++i) {
			left_split_min[i].x = left_split_min[i].x - kAABBAccuracy;
			left_split_min[i].y = left_split_min[i].y - kAABBAccuracy;
			left_split_min[i].z = left_split_min[i].z - kAABBAccuracy;

			left_split_max[i].x = left_split_max[i].x + kAABBAccuracy;
			left_split_max[i].y = left_split_max[i].y + kAABBAccuracy;
			left_split_max[i].z = left_split_max[i].z + kAABBAccuracy;

			right_split_min[i].x = right_split_min[i].x - kAABBAccuracy;
			right_split_min[i].y = right_split_min[i].y - kAABBAccuracy;
			right_split_min[i].z = right_split_min[i].z - kAABBAccuracy;

			right_split_max[i].x = right_split_max[i].x + kAABBAccuracy;
			right_split_max[i].y = right_split_max[i].y + kAABBAccuracy;
			right_split_max[i].z = right_split_max[i].z + kAABBAccuracy;
		}


		for (int i = 0; i < left_size; ++i) {
			int left_num = i + 1;
			int right_num = left_size - i;
			SahSplit split;
			split.LeftBox.Min = left_split_min[i];
			split.LeftBox.Max = left_split_max[i];
			split.LeftBox.Surface = left_num * CalculateAABBSurfaceArea(split.LeftBox);
			split.RightBox.Min = right_split_min[i];
			split.RightBox.Max = right_split_max[i];
			split.RightBox.Surface = right_num * CalculateAABBSurfaceArea(split.RightBox);
			split.BestCost = split.LeftBox.Surface + split.RightBox.Surface;


			if (split.BestCost < bestSplit.BestCost) {
				bestSplit.BestCost = split.BestCost;
				bestSplit.LeftBox.Surface = split.LeftBox.Surface;
				bestSplit.LeftBox.Min = split.LeftBox.Min;
				bestSplit.LeftBox.Max = split.LeftBox.Max;

				bestSplit.RightBox.Surface = split.RightBox.Surface;
				bestSplit.RightBox.Min = split.RightBox.Min;
				bestSplit.RightBox.Max = split.RightBox.Max;

				bestSplit.LeftBoxesIndex.clear();
				bestSplit.LeftBoxesIndex.resize(left_num);
				for (int j = 0; j < left_num; ++j) {
					bestSplit.LeftBoxesIndex[j] = sortedIndices[j];
				}

				bestSplit.RightBoxesIndex.clear();
				bestSplit.RightBoxesIndex.resize(right_num);
				for (int j = 0; j < right_num; ++j) {
					bestSplit.RightBoxesIndex[right_num - 1 - j] = sortedIndices[left_size - j];
				}


			}

		}

	}

	void FindBestSplitTriangle3DCore(
		const Scenario3D& scenario,
		const std::vector<Point3D>& center_points,
		std::vector<int>& sortedIndices,
		SahSplit& bestSplit) {
		for (int axis = 0; axis < 3; ++axis) {
			// 按当前轴对索引排序
			std::sort(sortedIndices.begin(), sortedIndices.end(),
				[&](int a, int b) { return GetAxis(center_points[a], axis) < GetAxis(center_points[b], axis); });
			FindBestSplitTriangle3DCoreHelp(scenario, sortedIndices, bestSplit);
		}
	}

	SahSplit FindBestSplitTriangle3D(
		const Scenario3D& scenario,
		const std::vector<Point3D>& center_points,
		std::vector<int>& sortedIndices)
	{
		SahSplit bestSplit;
		bestSplit.BestCost = std::numeric_limits<double>::max();
		if (center_points.size() < 1 || sortedIndices.size() < 1) {
			return bestSplit;
		}
		else if (sortedIndices.size() == 1) {

			AABB leftBox;
			if (!GetAABBByTriangle(scenario, sortedIndices[0], leftBox)) {
				return bestSplit;
			}
			bestSplit.BestCost = leftBox.Surface;
			{
				bestSplit.LeftBoxesIndex.push_back(sortedIndices[0]);
				bestSplit.LeftBox.Min = leftBox.Min;
				bestSplit.LeftBox.Max = leftBox.Max;
				bestSplit.LeftBox.Surface = leftBox.Surface;
				bestSplit.LeftBox.GeometryElementIndex.clear();
				for (auto index : leftBox.GeometryElementIndex) {
					bestSplit.LeftBox.GeometryElementIndex.insert(index);
				}

			}
			return bestSplit;
		}
		else if (sortedIndices.size() == 2) {
			AABB leftBox;
			AABB rightBox;
			if (!GetAABBByTriangle(scenario, sortedIndices[0], leftBox)) {
				return bestSplit;
			}
			if (!GetAABBByTriangle(scenario, sortedIndices[1], rightBox)) {
				return bestSplit;
			}
			bestSplit.BestCost = leftBox.Surface + rightBox.Surface;
			{
				bestSplit.LeftBoxesIndex.push_back(sortedIndices[0]);
				bestSplit.LeftBox.Min = leftBox.Min;
				bestSplit.LeftBox.Max = leftBox.Max;
				bestSplit.LeftBox.Surface = leftBox.Surface;
				bestSplit.LeftBox.GeometryElementIndex.clear();
				for (auto index : leftBox.GeometryElementIndex) {
					bestSplit.LeftBox.GeometryElementIndex.insert(index);
				}
			}
			{
				bestSplit.RightBoxesIndex.push_back(sortedIndices[1]);
				bestSplit.RightBox.Min = rightBox.Min;
				bestSplit.RightBox.Max = rightBox.Max;
				bestSplit.RightBox.Surface = rightBox.Surface;
				bestSplit.RightBox.GeometryElementIndex.clear();
				for (auto index : rightBox.GeometryElementIndex) {
					bestSplit.RightBox.GeometryElementIndex.insert(index);
				}

			}
			return bestSplit;
		}
		FindBestSplitTriangle3DCore(scenario, center_points, sortedIndices, bestSplit);
		return bestSplit;
	}

	std::unique_ptr<AabbBvhTreeNode> BuildAabbBvhTreeRecursiveTriangle3D(
		int depth,
		int maxLeafSize,
		int maxDepth,
		const Scenario3D& scenario,
		const std::vector<Point3D>& center_points,
		std::vector<int>& indices) {
		auto root_node = std::make_unique<AabbBvhTreeNode>();
		root_node->IsLeaf = false;
		if (!GetAABBByTriangles(scenario, indices, root_node->Bound)) {
			return std::unique_ptr<AabbBvhTreeNode>();
		}
		root_node->Center.x = 0.5 * (root_node->Bound.Min.x + root_node->Bound.Max.x);
		root_node->Center.y = 0.5 * (root_node->Bound.Min.y + root_node->Bound.Max.y);
		root_node->Center.z = 0.5 * (root_node->Bound.Min.z + root_node->Bound.Max.z);
		{
			double dx = root_node->Bound.Max.x - root_node->Bound.Min.x;
			double dy = root_node->Bound.Max.y - root_node->Bound.Min.y;
			double dz = root_node->Bound.Max.z - root_node->Bound.Min.z;
			root_node->Radius = 0.5 * sqrt(dx * dx + dy * dy + dz * dz);
		}
		// 终止条件：叶子节点
		if (indices.size() <= maxLeafSize || depth >= maxDepth) {
			root_node->IsLeaf = true;
			return root_node;
		}
		// 寻找最优分割
		SahSplit split = FindBestSplitTriangle3D(scenario, center_points, indices);
		// 递归构建子树
		if (!split.LeftBoxesIndex.empty()) {
			root_node->Left = BuildAabbBvhTreeRecursiveTriangle3D(depth + 1, maxLeafSize, maxDepth, scenario, center_points, split.LeftBoxesIndex);
		}
		if (!split.RightBoxesIndex.empty()) {
			root_node->Right = BuildAabbBvhTreeRecursiveTriangle3D(depth + 1, maxLeafSize, maxDepth, scenario, center_points, split.RightBoxesIndex);
		}
		return root_node;
	}

	std::unique_ptr<AabbBvhTreeNode> BuildTriangle3DAabbBvhTreeNode(
		int maxLeafSize,
		int maxDepth,
		const Scenario3D& scenario) {
		// 准备数据
		std::vector<Point3D> center_points(scenario.trianglesCount);
		for (int i = 0; i < scenario.trianglesCount; ++i) {
			Triangle3D& triangle = scenario.scenario_triangle3d_set[i];
			Point3D p1 = scenario.scenario_point3d_set[triangle.triangleP1Index];
			Point3D p2 = scenario.scenario_point3d_set[triangle.triangleP2Index];
			Point3D p3 = scenario.scenario_point3d_set[triangle.triangleP3Index];
			center_points[i].x = (p1.x + p2.x + p3.x) / 3.0;
			center_points[i].y = (p1.y + p2.y + p3.y) / 3.0;
			center_points[i].z = (p1.z + p2.z + p3.z) / 3.0;
		}
		// 创建初始索引列表
		std::vector<int> indices(center_points.size());
		for (int i = 0; i < center_points.size(); ++i) {
			indices[i] = i;
		}
		// 递归构建Tri-BVH-Tree
		return BuildAabbBvhTreeRecursiveTriangle3D(0, maxLeafSize, maxDepth, scenario, center_points, indices);
	}


	bool GetAABBByCorner(
		const Scenario3D& scenario,
		int cornerIndex,
		AABB& aabb) {

		std::list<Point3D> points;
		{
			const Corner3D& corner = scenario.scenario_corner3d_set[cornerIndex];
			Point3D p1 = scenario.scenario_point3d_set[corner.p1Index];
			Point3D p2 = scenario.scenario_point3d_set[corner.p2Index];
			points.push_back(p1);
			points.push_back(p2);
		}

		if (!GetAABBByPoints(points, aabb)) {
			return false;
		}

		aabb.GeometryElementIndex.insert(cornerIndex);


		aabb.Surface = CalculateAABBSurfaceArea(aabb);

		return true;

	}

	bool GetAABBByCorners(
		const Scenario3D& scenario,
		const std::vector<int>& indices,
		AABB& aabb) {

		std::list<Point3D> points;
		for (int i = 0; i < indices.size(); ++i) {
			int cornerIndex = indices[i];
			const Corner3D& corner = scenario.scenario_corner3d_set[cornerIndex];
			Point3D p1 = scenario.scenario_point3d_set[corner.p1Index];
			Point3D p2 = scenario.scenario_point3d_set[corner.p2Index];
			points.push_back(p1);
			points.push_back(p2);
		}

		if (!GetAABBByPoints(points, aabb)) {
			return false;
		}

		for (int cornerIndex : indices) {
			aabb.GeometryElementIndex.insert(cornerIndex);
		}

		aabb.Surface = indices.size() * CalculateAABBSurfaceArea(aabb);

		return true;

	}


	void FindBestSplitCorner3DCoreHelp(
		const Scenario3D& scenario,
		std::vector<int>& sortedIndices,
		SahSplit& bestSplit) {

		int left_size = (int)sortedIndices.size() - 1;

		std::vector<Point3D> left_split_min(left_size);
		std::vector<Point3D> left_split_max(left_size);

		std::vector<Point3D> right_split_min(left_size);
		std::vector<Point3D> right_split_max(left_size);

		{
			int index = 0;
			left_split_min[index].x = std::numeric_limits<double>::max();
			left_split_min[index].y = std::numeric_limits<double>::max();
			left_split_min[index].z = std::numeric_limits<double>::max();

			left_split_max[index].x = -std::numeric_limits<double>::max();
			left_split_max[index].y = -std::numeric_limits<double>::max();
			left_split_max[index].z = -std::numeric_limits<double>::max();

			ChangeCornerBoxMinMax(sortedIndices[index], scenario, left_split_min[index], left_split_max[index]);

			right_split_min[left_size - 1 - index].x = std::numeric_limits<double>::max();
			right_split_min[left_size - 1 - index].y = std::numeric_limits<double>::max();
			right_split_min[left_size - 1 - index].z = std::numeric_limits<double>::max();

			right_split_max[left_size - 1 - index].x = -std::numeric_limits<double>::max();
			right_split_max[left_size - 1 - index].y = -std::numeric_limits<double>::max();
			right_split_max[left_size - 1 - index].z = -std::numeric_limits<double>::max();


			ChangeCornerBoxMinMax(sortedIndices[left_size - index], scenario, right_split_min[left_size - 1 - index], right_split_max[left_size - 1 - index]);

			index = 1;
			for (; index < left_size; ++index) {

				left_split_min[index].x = left_split_min[index - 1].x;
				left_split_min[index].y = left_split_min[index - 1].y;
				left_split_min[index].z = left_split_min[index - 1].z;

				left_split_max[index].x = left_split_max[index - 1].x;
				left_split_max[index].y = left_split_max[index - 1].y;
				left_split_max[index].z = left_split_max[index - 1].z;

				ChangeCornerBoxMinMax(sortedIndices[index], scenario, left_split_min[index], left_split_max[index]);


				right_split_min[left_size - 1 - index].x = right_split_min[left_size - index].x;
				right_split_min[left_size - 1 - index].y = right_split_min[left_size - index].y;
				right_split_min[left_size - 1 - index].z = right_split_min[left_size - index].z;

				right_split_max[left_size - 1 - index].x = right_split_max[left_size - index].x;
				right_split_max[left_size - 1 - index].y = right_split_max[left_size - index].y;
				right_split_max[left_size - 1 - index].z = right_split_max[left_size - index].z;
				ChangeCornerBoxMinMax(sortedIndices[left_size - index], scenario, right_split_min[left_size - 1 - index], right_split_max[left_size - 1 - index]);
			}
		}

		for (int i = 0; i < left_size; ++i) {
			left_split_min[i].x = left_split_min[i].x - kAABBAccuracy;
			left_split_min[i].y = left_split_min[i].y - kAABBAccuracy;
			left_split_min[i].z = left_split_min[i].z - kAABBAccuracy;

			left_split_max[i].x = left_split_max[i].x + kAABBAccuracy;
			left_split_max[i].y = left_split_max[i].y + kAABBAccuracy;
			left_split_max[i].z = left_split_max[i].z + kAABBAccuracy;

			right_split_min[i].x = right_split_min[i].x - kAABBAccuracy;
			right_split_min[i].y = right_split_min[i].y - kAABBAccuracy;
			right_split_min[i].z = right_split_min[i].z - kAABBAccuracy;

			right_split_max[i].x = right_split_max[i].x + kAABBAccuracy;
			right_split_max[i].y = right_split_max[i].y + kAABBAccuracy;
			right_split_max[i].z = right_split_max[i].z + kAABBAccuracy;
		}


		for (int i = 0; i < left_size; ++i) {
			int left_num = i + 1;
			int right_num = left_size - i;
			SahSplit split;
			split.LeftBox.Min = left_split_min[i];
			split.LeftBox.Max = left_split_max[i];
			split.LeftBox.Surface = left_num * CalculateAABBSurfaceArea(split.LeftBox);
			split.RightBox.Min = right_split_min[i];
			split.RightBox.Max = right_split_max[i];
			split.RightBox.Surface = right_num * CalculateAABBSurfaceArea(split.RightBox);
			split.BestCost = split.LeftBox.Surface + split.RightBox.Surface;


			if (split.BestCost < bestSplit.BestCost) {
				bestSplit.BestCost = split.BestCost;
				bestSplit.LeftBox.Surface = split.LeftBox.Surface;
				bestSplit.LeftBox.Min = split.LeftBox.Min;
				bestSplit.LeftBox.Max = split.LeftBox.Max;

				bestSplit.RightBox.Surface = split.RightBox.Surface;
				bestSplit.RightBox.Min = split.RightBox.Min;
				bestSplit.RightBox.Max = split.RightBox.Max;

				bestSplit.LeftBoxesIndex.clear();
				bestSplit.LeftBoxesIndex.resize(left_num);
				for (int j = 0; j < left_num; ++j) {
					bestSplit.LeftBoxesIndex[j] = sortedIndices[j];
				}

				bestSplit.RightBoxesIndex.clear();
				bestSplit.RightBoxesIndex.resize(right_num);
				for (int j = 0; j < right_num; ++j) {
					bestSplit.RightBoxesIndex[right_num - 1 - j] = sortedIndices[left_size - j];
				}


			}

		}

	}

	void FindBestSplitCorner3DCore(
		const Scenario3D& scenario,
		const std::vector<Point3D>& center_points,
		std::vector<int>& sortedIndices,
		SahSplit& bestSplit) {

		for (int axis = 0; axis < 3; ++axis) {
			// 按当前轴对索引排序
			std::sort(sortedIndices.begin(), sortedIndices.end(),
				[&](int a, int b) { return GetAxis(center_points[a], axis) < GetAxis(center_points[b], axis); });
			FindBestSplitCorner3DCoreHelp(scenario, sortedIndices, bestSplit);
		}

	}

	SahSplit FindBestSplitCorner3D(
		const Scenario3D& scenario,
		const std::vector<Point3D>& center_points,
		std::vector<int>& sortedIndices)
	{

		const int SAH_STEP = 1;

		SahSplit bestSplit;
		bestSplit.BestCost = std::numeric_limits<double>::max();

		if (center_points.size() < 1 || sortedIndices.size() < 1) {
			return bestSplit;
		}
		else if (sortedIndices.size() == 1) {

			AABB leftBox;
			if (!GetAABBByCorner(scenario, sortedIndices[0], leftBox)) {
				return bestSplit;
			}

			bestSplit.BestCost = leftBox.Surface;
			{
				bestSplit.LeftBoxesIndex.push_back(sortedIndices[0]);
				bestSplit.LeftBox.Min = leftBox.Min;
				bestSplit.LeftBox.Max = leftBox.Max;
				bestSplit.LeftBox.Surface = leftBox.Surface;
				bestSplit.LeftBox.GeometryElementIndex.clear();
				for (auto index : leftBox.GeometryElementIndex) {
					bestSplit.LeftBox.GeometryElementIndex.insert(index);
				}

			}
			return bestSplit;
		}
		else if (sortedIndices.size() == 2) {
			AABB leftBox;
			AABB rightBox;

			if (!GetAABBByCorner(scenario, sortedIndices[0], leftBox)) {
				return bestSplit;
			}

			if (!GetAABBByCorner(scenario, sortedIndices[1], rightBox)) {
				return bestSplit;
			}


			bestSplit.BestCost = leftBox.Surface + rightBox.Surface;
			{
				bestSplit.LeftBoxesIndex.push_back(sortedIndices[0]);
				bestSplit.LeftBox.Min = leftBox.Min;
				bestSplit.LeftBox.Max = leftBox.Max;
				bestSplit.LeftBox.Surface = leftBox.Surface;
				bestSplit.LeftBox.GeometryElementIndex.clear();
				for (auto index : leftBox.GeometryElementIndex) {
					bestSplit.LeftBox.GeometryElementIndex.insert(index);
				}

			}
			{
				bestSplit.RightBoxesIndex.push_back(sortedIndices[1]);
				bestSplit.RightBox.Min = rightBox.Min;
				bestSplit.RightBox.Max = rightBox.Max;
				bestSplit.RightBox.Surface = rightBox.Surface;
				bestSplit.RightBox.GeometryElementIndex.clear();
				for (auto index : rightBox.GeometryElementIndex) {
					bestSplit.RightBox.GeometryElementIndex.insert(index);
				}

			}
			return bestSplit;
		}

		FindBestSplitCorner3DCore(scenario, center_points, sortedIndices, bestSplit);
		return bestSplit;
	}

	std::unique_ptr<AabbBvhTreeNode> BuildAabbBvhTreeRecursiveCorner3D(
		int depth,
		int maxLeafSize,
		int maxDepth,
		const Scenario3D& scenario,
		const std::vector<Point3D>& center_points,
		std::vector<int>& indices) {

		auto root_node = std::make_unique<AabbBvhTreeNode>();
		root_node->IsLeaf = false;
		if (!GetAABBByCorners(scenario, indices, root_node->Bound)) {
			return std::unique_ptr<AabbBvhTreeNode>();
		}
		root_node->Center.x = 0.5 * (root_node->Bound.Min.x + root_node->Bound.Max.x);
		root_node->Center.y = 0.5 * (root_node->Bound.Min.y + root_node->Bound.Max.y);
		root_node->Center.z = 0.5 * (root_node->Bound.Min.z + root_node->Bound.Max.z);
		{
			double dx = root_node->Bound.Max.x - root_node->Bound.Min.x;
			double dy = root_node->Bound.Max.y - root_node->Bound.Min.y;
			double dz = root_node->Bound.Max.z - root_node->Bound.Min.z;
			root_node->Radius = 0.5 * sqrt(dx * dx + dy * dy + dz * dz);
		}

		// 终止条件：叶子节点
		if (indices.size() <= maxLeafSize || depth >= maxDepth) {
			root_node->IsLeaf = true;
			return root_node;
		}

		// 寻找最优分割
		SahSplit split = FindBestSplitCorner3D(scenario, center_points, indices);


		// 递归构建子树
		if (!split.LeftBoxesIndex.empty()) {
			root_node->Left = BuildAabbBvhTreeRecursiveCorner3D(depth + 1, maxLeafSize, maxDepth, scenario, center_points, split.LeftBoxesIndex);
		}

		if (!split.RightBoxesIndex.empty()) {
			root_node->Right = BuildAabbBvhTreeRecursiveCorner3D(depth + 1, maxLeafSize, maxDepth, scenario, center_points, split.RightBoxesIndex);
		}

		return root_node;
	}

	std::unique_ptr<AabbBvhTreeNode> BuildCorner3DAabbBvhTreeNode(
		int maxLeafSize,
		int maxDepth,
		const Scenario3D& scenario) {
		// 准备数据
		std::vector<Point3D> center_points(scenario.cornersCount);
		for (int i = 0; i < scenario.cornersCount; ++i) {

			Corner3D& corner = scenario.scenario_corner3d_set[i];
			Point3D p1 = scenario.scenario_point3d_set[corner.p1Index];
			Point3D p2 = scenario.scenario_point3d_set[corner.p2Index];

			center_points[i].x = (p1.x + p2.x) * 0.5;
			center_points[i].y = (p1.y + p2.y) * 0.5;
			center_points[i].z = (p1.z + p2.z) * 0.5;

		}
		// 创建初始索引列表
		std::vector<int> indices(center_points.size());
		for (int i = 0; i < center_points.size(); ++i) {
			indices[i] = i;
		}
		// 递归构建Tri-BVH-Tree
		return BuildAabbBvhTreeRecursiveCorner3D(0, maxLeafSize, maxDepth, scenario, center_points, indices);
	}



	void Ray3DIntersectAabbBvhTreeTriangle3D(
		const Point3D& ray_origin,
		const Point3D& ray_direction,
		const std::unique_ptr<AabbBvhTreeNode>& root,
		std::list<int>& triangleIndices) {

		if (!root) {
			return;
		}

		bool isIntersect = Ray3DIntersectBall(ray_origin, ray_direction, root->Center, root->Radius);

		if (!isIntersect) {
			return;
		}

		if (root->IsLeaf) {

			for (auto triangleIndex : root->Bound.GeometryElementIndex) {
				triangleIndices.emplace_back(triangleIndex);
			}
		}
		Ray3DIntersectAabbBvhTreeTriangle3D(ray_origin, ray_direction, root->Left, triangleIndices);
		Ray3DIntersectAabbBvhTreeTriangle3D(ray_origin, ray_direction, root->Right, triangleIndices);
	}

	void Ray3DIntersectAabbBvhTreeCorner3D(
		const Point3D& ray_origin,
		const Point3D& ray_direction,
		const std::unique_ptr<AabbBvhTreeNode>& root,
		std::list<int>& cornerIndices) {

		if (!root) {
			return;
		}

		bool isIntersect = Ray3DIntersectBall(ray_origin, ray_direction, root->Center, root->Radius);

		if (!isIntersect) {
			return;
		}

		if (root->IsLeaf) {
			for (auto cornerIndex : root->Bound.GeometryElementIndex) {
				cornerIndices.emplace_back(cornerIndex);
			}
		}
		Ray3DIntersectAabbBvhTreeCorner3D(ray_origin, ray_direction, root->Left, cornerIndices);
		Ray3DIntersectAabbBvhTreeCorner3D(ray_origin, ray_direction, root->Right, cornerIndices);
	}

}

/// <summary>
/// 实现外部接口
/// </summary>
namespace S1Ray3DIntersectTriangle3DBallBvhTreeStd {

	std::unique_ptr<AabbBvhTreeNode> Triangle3DAabbBvhTree;
	std::unique_ptr<AabbBvhTreeNode> Corner3DAabbBvhTree;
	/// <summary>
	/// 初始化数据，包括加速结构
	/// 一个硬件模块(即一个集成电路板上)只需要一个这样的函数即可，因为只需要一次计算（初始化的数据必须存储在DDR中）
	/// </summary>
	/// <param name="scenario"></param>
	/// <param name="switchCorner"></param>
	/// <param name="cornerRadius"></param>
	void InitializeScenario3D(bool switchCorner, double cornerRadius, long long frequency, const Scenario3D& scenario, const MaterialSet& materialSet) {
		globalFrequency = frequency;
		InitAccelerateStructMaterial(materialSet);
		InitAccelerateStructTriangle3DMaterial(scenario);
		if (switchCorner) {
			kAABBAccuracy = 2.0 * cornerRadius;
			globalCornerRadius = cornerRadius;
			InitAccelerateStructCorner3DMaterial(scenario);
			Corner3DAabbBvhTree = std::move(BuildCorner3DAabbBvhTreeNode(1, 17, scenario));
		}
		Triangle3DAabbBvhTree = std::move(BuildTriangle3DAabbBvhTreeNode(1, 17, scenario));
	}


	/// <summary>
	/// 一根射线和一个三角形的碰撞结果计算
	/// </summary>
	/// <param name="triangle_index"></param>
	/// <param name="ray_origin"></param>
	/// <param name="ray_direction"></param>
	/// <param name="result"></param>
	/// <returns></returns>
	bool Ray3DIntersectTriangle3D(int triangle_index, const Point3D& ray_origin, const Point3D& ray_direction, Ray3DIntersectGeometry3DResult& result) {
		if (globalTriangle3DMaterialsInvalid[triangle_index]) {
			return false;
		}
		const Triangle3DMaterial& triangle3DMaterial = globalTriangle3DMaterials[triangle_index];
		bool state = Intersect_Ray3D_Triangle3D_Plus(ray_origin, ray_direction,
			triangle3DMaterial.scenarioTriangleP1,
			triangle3DMaterial.scenarioE1,
			triangle3DMaterial.scenarioE2,
			triangle3DMaterial.scenarioE1E2,
			triangle3DMaterial.scenarioE2E1,
			result.distance,
			result.location);
		if (state) {
			result.elementIndex = triangle_index;
			result.type = 0;
			return true;
		}
		return false;
	}

	/// <summary>
	/// 一根射线和所有三角形的碰撞结果计算
	/// </summary>
	/// <param name="ray_origin"></param>
	/// <param name="ray_direction"></param>
	/// <param name="result"></param>
	void Ray3DIntersectTriangle3Ds(const Point3D& ray_origin, const Point3D& ray_direction, std::vector<Ray3DIntersectGeometry3DResult>& result) {

		std::list<int> triangleIndices;
		Ray3DIntersectAabbBvhTreeTriangle3D(ray_origin, ray_direction, Triangle3DAabbBvhTree, triangleIndices);

		std::list<Ray3DIntersectGeometry3DResult> ray_intersect_triangles_result_list;
		for (int triangle_index : triangleIndices) {
			Ray3DIntersectGeometry3DResult ray_intersect_triangles_result;
			if (Ray3DIntersectTriangle3D(triangle_index, ray_origin, ray_direction, ray_intersect_triangles_result)) {
				ray_intersect_triangles_result_list.emplace_back(ray_intersect_triangles_result);
			}
		}

		result.clear();
		result.resize(ray_intersect_triangles_result_list.size());
		int jsq = 0;
		for (auto& ray_intersect_triangles_result : ray_intersect_triangles_result_list) {
			result[jsq] = ray_intersect_triangles_result;
			++jsq;
		}
	}

	int CalculateInsertIndex(const Ray3DIntersectGeometry3DResult& ray3DIntersectGeometry3DResult, std::vector<Ray3DIntersectGeometry3DResult>& result) {
		int pre_number = (int)result.size();
		if (ray3DIntersectGeometry3DResult.elementIndex == -1) {
			return pre_number;
		}
		for (int i = 0; i < pre_number; ++i) {
			if (IsZeroAbs(ray3DIntersectGeometry3DResult.distance - result[i].distance)) {
				if (ray3DIntersectGeometry3DResult.elementIndex < result[i].elementIndex) {
					result[i].elementIndex = ray3DIntersectGeometry3DResult.elementIndex;
				}
			}
			else if (result[i].distance < ray3DIntersectGeometry3DResult.distance) {
				return i;
			}
		}
		return pre_number;
	}

	/// <summary>
	/// 一根射线和所有三角形的碰撞结果计算，并按照碰撞距离排序，没有碰撞结果不做记录，只记录前几个结果
	/// </summary>
	/// <param name="pre_number"></param>
	/// <param name="ray_origin"></param>
	/// <param name="ray_direction"></param>
	/// <param name="result"></param>
	void Ray3DIntersectTriangle3Ds_sorted(int pre_number, const Point3D& ray_origin, const Point3D& ray_direction, std::vector<Ray3DIntersectGeometry3DResult>& result) {

		std::vector<Ray3DIntersectGeometry3DResult> ray_intersect_triangles_result_vector;
		Ray3DIntersectTriangle3Ds(ray_origin, ray_direction, ray_intersect_triangles_result_vector);
		result.clear();
		result.resize(pre_number);
		for (int i = 0; i < ray_intersect_triangles_result_vector.size(); ++i) {
			//计算需要插入的位置
			int index = CalculateInsertIndex(ray_intersect_triangles_result_vector[i], result);
			if (index < pre_number) {
				for (int j = pre_number - 1; j > index; --j) {
					result[j] = result[j - 1];
				}
				result[index] = ray_intersect_triangles_result_vector[i];
			}
		}

		for (int j = (int)result.size() - 1; j >= 0; --j) {
			if (result[j].elementIndex == -1) {
				result.erase(result.begin() + j);
			}
		}
	}


	/// <summary>
	/// 多根射线和所有三角形的碰撞结果计算，并按照碰撞距离排序，没有碰撞结果不做记录，只记录前几个结果
	/// </summary>
	/// <param name="pre_number"></param>
	/// <param name="ray_origin"></param>
	/// <param name="ray_direction"></param>
	/// <param name="result"></param>
	void Ray3DsIntersectTriangle3Ds_sorted(int pre_number, const std::vector<Point3D>& ray_origin, const std::vector<Point3D>& ray_direction, std::vector<std::vector<Ray3DIntersectGeometry3DResult>>& result) {
		result.clear();
		result.resize(ray_origin.size());
		for (int i = 0; i < result.size(); ++i) {
			Ray3DIntersectTriangle3Ds_sorted(pre_number, ray_origin[i], ray_direction[i], result[i]);
		}
	}

	/// <summary>
	/// 一根射线和所有三角形的第一次碰撞结果计算
	/// </summary>
	/// <param name="ray_origin"></param>
	/// <param name="ray_direction"></param>
	/// <param name=""></param>
	void Ray3DIntersectTriangle3DsFirst(const Point3D& ray_origin, const Point3D& ray_direction, Ray3DIntersectGeometry3DResult& result) {

		std::list<int> triangleIndices;
		Ray3DIntersectAabbBvhTreeTriangle3D(ray_origin, ray_direction, Triangle3DAabbBvhTree, triangleIndices);
		for (int triangle_index : triangleIndices) {
			Ray3DIntersectGeometry3DResult ray_intersect_triangles_result;
			if (Ray3DIntersectTriangle3D(triangle_index, ray_origin, ray_direction, ray_intersect_triangles_result)) {
				if (IsZeroAbs(result.distance - ray_intersect_triangles_result.distance)) {
					if (ray_intersect_triangles_result.elementIndex < result.elementIndex) {
						result.elementIndex = ray_intersect_triangles_result.elementIndex;
					}
				}
				else if (result.distance > ray_intersect_triangles_result.distance) {
					result.distance = ray_intersect_triangles_result.distance;
					result.elementIndex = ray_intersect_triangles_result.elementIndex;
					result.type = ray_intersect_triangles_result.type;
					result.location = ray_intersect_triangles_result.location;
				}
			}
		}
	}

	/// <summary>
	/// 多根射线和所有三角形的第一次碰撞结果计算
	/// </summary>
	/// <param name="ray_origin"></param>
	/// <param name="ray_direction"></param>
	/// <param name="result"></param>
	void Ray3DsIntersectTriangle3DsFirst(const std::vector<Point3D>& ray_origin, const std::vector<Point3D>& ray_direction, std::vector<Ray3DIntersectGeometry3DResult>& result) {
		result.clear();
		result.resize(ray_origin.size());
		for (int i = 0; i < result.size(); ++i) {
			Ray3DIntersectTriangle3DsFirst(ray_origin[i], ray_direction[i], result[i]);
		}
	}


	/// <summary>
	/// 一根射线和一个边的碰撞结果计算 
	/// </summary>
	/// <param name="corner_index"></param>
	/// <param name="ray_origin"></param>
	/// <param name="ray_direction"></param>
	/// <param name="result"></param>
	/// <returns></returns>
	bool Ray3DIntersectCorner3D(int corner_index, const Point3D& ray_origin, const Point3D& ray_direction, Ray3DIntersectGeometry3DResult& result) {
		if (globalCorner3DMaterialsInvalid[corner_index]) {
			return false;
		}
		const Corner3DMaterial& corner3DMaterial = globalCorner3DMaterials[corner_index];
		Point3D ray_intersect_point;
		Point3D corner_intersect_point;
		double ray_intersect_distance;
		double corner_intersect_distance;
		double line_line_distance = GetDistanceLine3DLine3D(ray_origin, ray_direction, corner3DMaterial.cornerStart, corner3DMaterial.z, ray_intersect_distance, corner_intersect_distance, ray_intersect_point, corner_intersect_point);
		if (line_line_distance > globalCornerRadius) {
			return false;
		}
		if (ray_intersect_distance < -Eps) {
			return false;
		}
		if (corner_intersect_distance < -Eps) {
			return false;
		}
		if (corner_intersect_distance > corner3DMaterial.length + Eps) {
			return false;
		}
		//返回的距离是射线到线段碰撞点的距离
		result.distance = GetDistancePoint3DPoint3D(ray_origin, corner_intersect_point);
		result.elementIndex = corner_index;
		result.type = 1;
		result.location = corner_intersect_point;
		return true;
	}

	/// <summary>
	///  一根射线和所有边的碰撞结果计算，如果没有被三角形遮挡，所有结果都需要返回
	/// </summary>
	/// <param name="ray_origin"></param>
	/// <param name="ray_direction"></param>
	/// <param name="result"></param>
	void Ray3DIntersectCorner3Ds(const Point3D& ray_origin, const Point3D& ray_direction, std::vector<Ray3DIntersectGeometry3DResult>& result) {
		Ray3DIntersectGeometry3DResult ray_intersect_triangles_first_result;
		Ray3DIntersectTriangle3DsFirst(ray_origin, ray_direction, ray_intersect_triangles_first_result);
		std::list<Ray3DIntersectGeometry3DResult> ray_intersect_corners_result_list;

		std::list<int> cornerIndices;
		Ray3DIntersectAabbBvhTreeCorner3D(ray_origin, ray_direction, Corner3DAabbBvhTree, cornerIndices);
		for (int corner_index : cornerIndices) {
			Ray3DIntersectGeometry3DResult ray_intersect_corners_result;
			if (Ray3DIntersectCorner3D(corner_index, ray_origin, ray_direction, ray_intersect_corners_result)) {
				if (ray_intersect_corners_result.distance < ray_intersect_triangles_first_result.distance + globalCornerRadius) {
					ray_intersect_corners_result_list.emplace_back(ray_intersect_corners_result);
				}
			}
		}
		result.clear();
		result.resize(ray_intersect_corners_result_list.size());
		int jsq = 0;
		for (auto& ray_intersect_corners_result : ray_intersect_corners_result_list) {
			result[jsq] = ray_intersect_corners_result;
			++jsq;
		}
	}

	/// <summary>
	/// 多根射线和所有边的碰撞结果计算，如果没有被三角形遮挡，所有结果都需要返回
	/// </summary>
	/// <param name="ray_origin"></param>
	/// <param name="ray_direction"></param>
	/// <param name="result"></param>
	void Ray3DsIntersectCorner3Ds(const std::vector<Point3D>& ray_origin, const std::vector<Point3D>& ray_direction, std::vector<std::vector<Ray3DIntersectGeometry3DResult>>& result) {
		result.clear();
		result.resize(ray_origin.size());
		for (int i = 0; i < result.size(); ++i) {
			Ray3DIntersectCorner3Ds(ray_origin[i], ray_direction[i], result[i]);
		}
	}


	bool CalculateCanSeeNode_Plus(
		double cornerRadius,
		const Point3D& segStart,
		const Point3D& segEnd,
		Point3D& lineVec) {

		lineVec = SubPoint3DPoint3D(segEnd, segStart);
		double len = Length_Point3D(lineVec);
		lineVec = MulDoublePoint3D(1.0 / len, lineVec);

		Ray3DIntersectGeometry3DResult result;
		Ray3DIntersectTriangle3DsFirst(segStart, lineVec, result);
		if (result.distance + cornerRadius < len) {
			return false;
		}
		return true;

	}


	bool CalculateCanSeeNode(
		double cornerRadius,
		const Point3D& segStart,
		const Point3D& segEnd) {

		Point3D lineVec;
		return CalculateCanSeeNode_Plus(cornerRadius, segStart, segEnd, lineVec);

	}
}