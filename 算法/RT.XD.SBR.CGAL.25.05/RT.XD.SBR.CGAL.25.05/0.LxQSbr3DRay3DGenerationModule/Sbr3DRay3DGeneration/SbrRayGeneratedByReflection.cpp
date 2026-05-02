

#include"../Input.h"


namespace SbrRayGeneratedByReflectionStd {
    /// <summary>
    /// 生成反射射线
    /// </summary>
    /// <param name="a"></param>
    /// <param name="b"></param>
    /// <param name="face"></param>
    /// <param name="eps"></param>
    /// <param name="newRay"></param>
    /// <param name="curNode"></param>
    /// <returns></returns>
    int BuildReflectionRay(
        const Point3DStd::Point3D& a,
        const Point3DStd::Point3D& b,
        const Point3DStd::Point3D& n,
        Ray3DStd::Ray3D& newRay,
        double& thetai) {
        Point3DStd::Point3D ab = Geometry3DOperateStd::SubPoint3DPoint3D(b, a);
        Point3DStd::Point3D n2(n);
        double d1 = Geometry3DOperateStd::DotPoint3DPoint3D(ab, n2);
        double d2 = Geometry3DOperateStd::Length_Point3D(ab);
        if (d2 < GlobalConstantStd::Eps) {
            return 0;
        }
        double d3 = d1 / d2;


        if (d1 < 0) {
            d3 = -d3;
        }
        else {
            n2.x = -n2.x;
            n2.y = -n2.y;
            n2.z = -n2.z;
        }
        thetai = acos(d3);
        ab = Geometry3DOperateStd::MulDoublePoint3D(1.0 / d2, ab);
        Point3DStd::Point3D bc = Geometry3DOperateStd::MulDoublePoint3D(d3, n2);
        Point3DStd::Point3D ac = Geometry3DOperateStd::AddPoint3DPoint3D(ab, bc);
        Point3DStd::Point3D vec = Geometry3DOperateStd::AddPoint3DPoint3D(ac, bc);
        Geometry3DOperateStd::AssignmentPoint3DPoint3D(newRay.o, b);
        Geometry3DOperateStd::AssignmentPoint3DPoint3D(newRay.vec, vec);
        return 1;
    }
}