


#include"../Input.h"

namespace Geometry3DOperateStd {

    /// <summary>
    ///  计算2个点的中心点
    /// </summary>
    /// <param name="p1"></param>
    /// <param name="p2"></param>
    /// <returns></returns>
    Point3DStd::Point3D Center_Point3D_Point3D(const Point3DStd::Point3D& p1, const Point3DStd::Point3D& p2)
    {
        Point3DStd::Point3D temp111 = AddPoint3DPoint3D(p1, p2);
        return MulDoublePoint3D(1.0 / 2.0, temp111);
    }

    /// <summary>
    /// 计算3个点的中心点
    /// </summary>
    /// <param name="p1">点1</param>
    /// <param name="p2">点2</param>
    /// <param name="p3">点3</param>
    /// <returns></returns>
    Point3DStd::Point3D Center_Point3D_Point3D_Point3D(const Point3DStd::Point3D& p1,const Point3DStd::Point3D& p2,const Point3DStd::Point3D& p3)
    {
        Point3DStd::Point3D temp111 = AddPoint3DPoint3D(p1, p2);
        Point3DStd::Point3D temp222 = AddPoint3DPoint3D(temp111, p3);
        return MulDoublePoint3D(1.0 / 3.0, temp222);
    }




}