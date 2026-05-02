


#include"../Input.h"


namespace Geometry3DOperateStd {


    double AreaCalculateOfTriangle3D_plus(const Point3DStd::Point3D& p1, const Point3DStd::Point3D& p2, const Point3DStd::Point3D& p3)
    {
        double ab = GetDistancePoint3DPoint3D(p1, p2);
        double ac = GetDistancePoint3DPoint3D(p3, p1);
        double bc = GetDistancePoint3DPoint3D(p2, p3);
        double p = 0.5 * (ab + ac + bc);
        double tt = p * (p - ab) * (p - ac) * (p - bc);
        tt = sqrt(tt);
        if (tt <= GlobalConstantStd::Eps)
        {
            return 0.0;
        }
        return tt;
    }

    double AreaCalculateOfTriangle3D(const Triangle3DStd::Triangle3D& triangle) {
        return AreaCalculateOfTriangle3D_plus(triangle.p1, triangle.p2, triangle.p3);
    }


}