

#include"../Input.h"

namespace Geometry3DOperateStd {




    /// <summary>
    /// 从全局坐标系转到相对坐标
    /// </summary>
    /// <param name="o1">相对坐标轴源点</param>
    /// <param name="ex"></param>
    /// <param name="ey"></param>
    /// <param name="ez"></param>
    /// <param name="p0"></param>
    /// <param name="eps"></param>
    /// <returns></returns>
    Point3DStd::Point3D GetRelativeXYZ(
        const Point3DStd::Point3D& o1,
        const Point3DStd::Point3D& ex,
        const Point3DStd::Point3D& ey,
        const Point3DStd::Point3D& ez,
        const Point3DStd::Point3D& p0)
    {
        Point3DStd::Point3D op = SubPoint3DPoint3D(p0, o1);
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
        double d11 = x1 * y2 - y1 * x2;
        double d12 = x3 * y2 - y3 * x2;
        double d13 = x4 * y2 - y4 * x2;

        double d21 = x2 * z1 - x1 * z2;
        double d22 = x2 * z3 - x3 * z2;
        double d23 = x2 * z4 - x4 * z2;
        double t1 = d12 * d23 - d22 * d13;
        double t2 = d13 * d22 - d23 * d12;
        if (abs(t1) <= GlobalConstantStd::Eps)
        {
            if (abs(t2) <= GlobalConstantStd::Eps)
            {
                z = z1;
            }
            else
            {
                z = (d11 * d22 - d21 * d12) / t2;
            }

            if (abs(d12) <= GlobalConstantStd::Eps)
            {
                if (abs(d22) <= GlobalConstantStd::Eps) {
                    double ttt = y2 * z3 - y3 * z2;
                    x = (y1 * z3 - z * y4 * z3 - y3 * z1 + z * y3 * z4) / ttt;
                    y = ((x1 + y1 + z1) - x * (x2 + y2 + z2) - z * (x4 + y4 + z4)) / (x3 + y3 + z3);
                    return CreatePoint3D(x, y, z);
                }
                else {
                    y = (d21 - d23 * z) / d22;
                }
            }
            else
            {
                y = (d11 - d13 * z) / d12;
            }
        }
        else
        {
            y = (d11 * d23 - d21 * d13) / t1;
            if (abs(d13) <= GlobalConstantStd::Eps)
            {
                if (abs(d23) <= GlobalConstantStd::Eps) {
                    ProjectDependenciesStd::DisplayPromptOrReason("GetRelativeXYZ 非法点", true, __FILE__, __LINE__);
                    return CreatePoint3D(x, y, z);
                }
                else {
                    z = (d21 - d22 * y) / d23;
                }
            }
            else
            {
                z = (d11 - d12 * y) / d13;
            }
        }
        x = ((x1 + y1 + z1) - y * (x3 + y3 + z3) - z * (x4 + y4 + z4)) / (x2 + y2 + z2);
        return CreatePoint3D(x, y, z);
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
        if (abs(t1) <= GlobalConstantStd::Eps) {
            if (abs(t2) <= GlobalConstantStd::Eps) {
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
        if (abs(t1) <= GlobalConstantStd::Eps) {
            if (abs(t2) <= GlobalConstantStd::Eps) {
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
        if (abs(t1) <= GlobalConstantStd::Eps) {
            if (abs(t2) <= GlobalConstantStd::Eps) {
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

    Point3DStd::Point3D GetRelativeXYZPlus(
        const Point3DStd::Point3D& o1,
        const Point3DStd::Point3D& ex,
        const Point3DStd::Point3D& ey,
        const Point3DStd::Point3D& ez,
        const Point3DStd::Point3D& p0)
    {
        Point3DStd::Point3D op = SubPoint3DPoint3D(p0, o1);
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

    Point3DStd::Point3D MulDeterminant33Determinant31Plus(
        double a00,
        double a01,
        double a02,
        double a10,
        double a11,
        double a12,
        double a20,
        double a21,
        double a22,
        double b00,
        double b10,
        double b20) {
        Point3DStd::Point3D res;
        res.x = a00 * b00 + a01 * b10 + a02 * b20;
        res.y = a10 * b00 + a11 * b10 + a12 * b20;
        res.z = a20 * b00 + a21 * b10 + a22 * b20;
        return res;
    }

    bool CoordinatedSystemTransformation_1(const DeterminantStd::Determinant& inverseMatrix,
        const BaseCoordinateSystemStd::BaseCoordinateSystem& sys1, const Point3DStd::Point3D& goal,
        const BaseCoordinateSystemStd::BaseCoordinateSystem& sys2, Point3DStd::Point3D& res) {

        auto a2 = sys2.o.x;
        auto b2 = sys2.o.y;
        auto c2 = sys2.o.z;

        auto a1 = sys1.o.x;
        auto b1 = sys1.o.y;
        auto c1 = sys1.o.z;
        auto d1 = sys1.x.x;
        auto e1 = sys1.x.y;
        auto f1 = sys1.x.z;
        auto g1 = sys1.y.x;
        auto h1 = sys1.y.y;
        auto i1 = sys1.y.z;
        auto j1 = sys1.z.x;
        auto k1 = sys1.z.y;
        auto m1 = sys1.z.z;

        Point3DStd::Point3D temp = MulDeterminant33Determinant31Plus(d1, g1, j1, e1, h1, k1, f1, i1, m1, goal.x, goal.y, goal.z);
        temp.x = temp.x + a1 - a2;
        temp.y = temp.y + b1 - b2;
        temp.z = temp.z + c1 - c2;

        res = MulDeterminant33Determinant31Plus(
            inverseMatrix.determinant[0][0],
            inverseMatrix.determinant[0][1],
            inverseMatrix.determinant[0][2],
            inverseMatrix.determinant[1][0],
            inverseMatrix.determinant[1][1],
            inverseMatrix.determinant[1][2],
            inverseMatrix.determinant[2][0],
            inverseMatrix.determinant[2][1],
            inverseMatrix.determinant[2][2], temp.x, temp.y, temp.z);
        return true;

    }

    bool CoordinatedSystemTransformation(const BaseCoordinateSystemStd::BaseCoordinateSystem& sys1, const Point3DStd::Point3D& goal,
        const BaseCoordinateSystemStd::BaseCoordinateSystem& sys2, Point3DStd::Point3D& res) {
        DeterminantStd::Determinant determinant(3, 3);

        auto d2 = sys2.x.x;
        auto e2 = sys2.x.y;
        auto f2 = sys2.x.z;

        auto g2 = sys2.y.x;
        auto h2 = sys2.y.y;
        auto i2 = sys2.y.z;

        auto j2 = sys2.z.x;
        auto k2 = sys2.z.y;
        auto m2 = sys2.z.z;

        determinant.determinant[0][0] = d2;
        determinant.determinant[0][1] = g2;
        determinant.determinant[0][2] = j2;
        determinant.determinant[1][0] = e2;
        determinant.determinant[1][1] = h2;
        determinant.determinant[1][2] = k2;
        determinant.determinant[2][0] = f2;
        determinant.determinant[2][1] = i2;
        determinant.determinant[2][2] = m2;
        DeterminantStd::Determinant inverseMatrix;
        auto state = DeterminantStd::InverseMatrix(determinant, inverseMatrix);
        if (state) {
            return CoordinatedSystemTransformation_1(inverseMatrix, sys1, goal, sys2, res);
        }
        else {
            return false;
        }


    }



    bool BuildDiffractionCoordinateSystem(
        const Point3DStd::Point3D& o,
        const Point3DStd::Point3D& vec_faker_z,
        const Point3DStd::Point3D& p3,
        const Point3DStd::Point3D& n,
        BaseCoordinateSystemStd::BaseCoordinateSystem& res) {

        Point3DStd::Point3D y = n;
        Point3DStd::Point3D z = vec_faker_z;

        Point3DStd::Point3D x;
        if (!CrossPoint3DPoint3D_safe(y, z, x)) {
            return false;
        }

        Point3DStd::Point3D op3 = SubPoint3DPoint3D(p3,o);
        double dot = DotPoint3DPoint3D(op3, x);
        if (IsZeroAbs(dot)) {
            return false;
        }
        else if (dot < 0) {
            x.x = -x.x;
            x.y = -x.y;
            x.z = -x.z;
            if (!CrossPoint3DPoint3D_safe(x, y, z)) {
                return false;
            }
        }
        res.o = o;
        res.x = Normalization_Point3D_unsafe(x);
        res.y = Normalization_Point3D_unsafe(y);
        res.z = Normalization_Point3D_unsafe(z);
        return true;
    }

    bool BuildDiffractionCoordinateSystemBySegment(
        const Point3DStd::Point3D& start,
        const Point3DStd::Point3D& end,
        const Point3DStd::Point3D& p3,
        const Point3DStd::Point3D& n,
        BaseCoordinateSystemStd::BaseCoordinateSystem& res) {

        Point3DStd::Point3D vec_faker_z = SubPoint3DPoint3D(end, start);

        return BuildDiffractionCoordinateSystem(start, vec_faker_z, p3, n, res);
    }

    bool CanBuildDiffractionCoordinateSystemByCorner(
        const Point3DStd::Point3D& start,
        const Point3DStd::Point3D& end,
        const Point3DStd::Point3D& triangle_0_p3,
        const Point3DStd::Point3D& triangle_n_p3,
        const Point3DStd::Point3D& face0_n) {
        Point3DStd::Point3D vec_faker_z = SubPoint3DPoint3D(end, start);

        BaseCoordinateSystemStd::BaseCoordinateSystem baseCoordinateSystem;
        if (!BuildDiffractionCoordinateSystem(start, vec_faker_z, triangle_0_p3, face0_n, baseCoordinateSystem)) {
            return false;
        }
        BaseCoordinateSystemStd::BaseCoordinateSystem glo(Point3DStd::Point3D(0.0, 0.0, 0.0), Point3DStd::Point3D(1.0, 0.0, 0.0), Point3DStd::Point3D(0.0, 1.0, 0.0), Point3DStd::Point3D(0.0, 0.0, 1.0));
        Point3DStd::Point3D phiE_faker;
        if (!CoordinatedSystemTransformation(glo, triangle_n_p3, baseCoordinateSystem, phiE_faker)) {
            return false;
        }
        double phiE = MathOperateStd::GetAngleByXY(phiE_faker.x, phiE_faker.y);
        if (phiE <= (180.5 * GlobalConstantStd::Pi / 180)) {
            return false;
        }
        return true;
    }

    bool BuildDiffractionCoordinateSystemByCorner(
        const Point3DStd::Point3D& pre_location,
        const Point3DStd::Point3D& location,
        const Point3DStd::Point3D& next_location,
        const Point3DStd::Point3D& start,
        const Point3DStd::Point3D& end,
        const Point3DStd::Point3D& triangle_0_p3,
        const Point3DStd::Point3D& triangle_n_p3,
        const Point3DStd::Point3D& face0_n,
        const Point3DStd::Point3D& facen_n,
        double& phi1,
        double& phi2,
        double& phiE) {

        Point3DStd::Point3D vec_faker_z = SubPoint3DPoint3D(end, start);

        BaseCoordinateSystemStd::BaseCoordinateSystem baseCoordinateSystem;
        if (!BuildDiffractionCoordinateSystem(location, vec_faker_z, triangle_0_p3, face0_n, baseCoordinateSystem)) {
            return false;
        }

        BaseCoordinateSystemStd::BaseCoordinateSystem glo(Point3DStd::Point3D(0.0, 0.0, 0.0), Point3DStd::Point3D(1.0, 0.0, 0.0), Point3DStd::Point3D(0.0, 1.0, 0.0), Point3DStd::Point3D(0.0, 0.0, 1.0));
        Point3DStd::Point3D phiE_faker;
        if (!CoordinatedSystemTransformation(glo,triangle_n_p3, baseCoordinateSystem,phiE_faker)) {
            return false;
        }
        phiE = MathOperateStd::GetAngleByXY(phiE_faker.x, phiE_faker.y);
        if (phiE <= (180.5 * GlobalConstantStd::Pi / 180)) {
            return false;
        }

        Point3DStd::Point3D phi1_faker;
        if (!CoordinatedSystemTransformation(glo, pre_location, baseCoordinateSystem, phi1_faker)) {
            return false;
        }
        phi1 = MathOperateStd::GetAngleByXY(phi1_faker.x, phi1_faker.y);
        if (phi1 < 0.0) {
            return false;
        } 
        
        if (phi1 >= phiE) {
            return false;
        }

        if (phi1 > 0.5 * phiE) {
            if (!BuildDiffractionCoordinateSystem(location, vec_faker_z, triangle_n_p3, facen_n, baseCoordinateSystem)) {
                return false;
            }
            phi1 = phiE - phi1;
        }
        Point3DStd::Point3D phi2_faker;
        if (!CoordinatedSystemTransformation(glo, next_location, baseCoordinateSystem, phi2_faker)) {
            return false;
        }
        phi2 = MathOperateStd::GetAngleByXY(phi2_faker.x, phi2_faker.y);

        if (phi2 < 0.0) {
            return false;
        }

        if (phi2 >= phiE) {
            return false;
        }

        return true;
    }


}