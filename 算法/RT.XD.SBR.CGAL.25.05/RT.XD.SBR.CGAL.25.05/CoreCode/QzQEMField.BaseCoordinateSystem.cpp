#include "QzQEMField.BaseCoordinateSystem.h"

#include "QzQGeometry3DOperate.Point3D.h"
#include "QzQGeometry3DOperate.Normalization.h"
namespace EMFieldBaseCoordinateSystemStd {

    BaseCoordinateSystemStd::BaseCoordinateSystem EMField_BaseCoordinateSystem_thetai0(
        double dot_r_n,
        const Point3DStd::Point3D& n,
        const Point3DStd::Point3D& y,
        const Point3DStd::Point3D& cur_point) {
        Point3DStd::Point3D z(n.x, n.y, n.z);
        if (dot_r_n < 0.0) {
            //发生在表面
            z.x = -z.x;
            z.y = -z.y;
            z.z = -z.z;
        }
        Point3DStd::Point3D x = Geometry3DOperateStd::CrossPoint3DPoint3D(y, z);

        x = Geometry3DOperateStd::Normalization_Point3D(x);
        //建立TE(垂直极化波),TM(平行极化波)坐标系，其中以n的方向为x轴，以n和入射线构成的平面xoz
        BaseCoordinateSystemStd::BaseCoordinateSystem baseCoordinateSystem(cur_point, x, y, z);
        return baseCoordinateSystem;
    }


    BaseCoordinateSystemStd::BaseCoordinateSystem EMField_BaseCoordinateSystem(
        double dot_r_n,
        const Point3DStd::Point3D& n,
        const Point3DStd::Point3D& r,
        const Point3DStd::Point3D& cur_point) {
        //建立坐标系，
            // 1.以碰撞点为坐标原点，以n所在的直线为z轴，z正方向与入射射线的方向应该是同向的
            // 2.以入射线和n所在的平面为xoz平面，x正方向与入射射线的方向应该是同向的
            // 3.最后确定y轴
            // 4. 显然，xoz面的场是tm极化(水平极化)，y对应te极化(垂直极化)

        Point3DStd::Point3D z(n.x, n.y, n.z);
        if (dot_r_n < 0.0) {
            //发生在表面
            z.x = -z.x;
            z.y = -z.y;
            z.z = -z.z;
        }
        Point3DStd::Point3D y = Geometry3DOperateStd::CrossPoint3DPoint3D(z, r);
        Point3DStd::Point3D x = Geometry3DOperateStd::CrossPoint3DPoint3D(y, z);
        if (Geometry3DOperateStd::DotPoint3DPoint3D(x, r) < 0.0) {
            x.x = -x.x;
            x.y = -x.y;
            x.z = -x.z;
            y = Geometry3DOperateStd::CrossPoint3DPoint3D(z, x);
        }
        x = Geometry3DOperateStd::Normalization_Point3D(x);
        y = Geometry3DOperateStd::Normalization_Point3D(y);
        //建立TE(垂直极化波),TM(平行极化波)坐标系，其中以n的方向为x轴，以n和入射线构成的平面xoz
        BaseCoordinateSystemStd::BaseCoordinateSystem baseCoordinateSystem(cur_point, x, y, z);
        return baseCoordinateSystem;
    }
}