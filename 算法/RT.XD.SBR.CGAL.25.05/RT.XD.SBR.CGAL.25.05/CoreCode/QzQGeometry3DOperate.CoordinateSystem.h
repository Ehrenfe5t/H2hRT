#pragma once


#include"HdQBall3D.h"
#include"HdQBaseCoordinateSystem.h"
#include"HdQBoundingBox3D.h"
#include"LxQLine3D.h"
#include"LxQLineSegment3D.h"
#include"LxQPlane3D.h"
#include"LxQPoint2I.h"
#include"LxQPoint3DI.h"
#include"LxQPoint3F.h"
#include"LxQPoint3I.h"
#include"LxQPoint2D.h"
#include"LxQPoint3D.h"
#include"DxQRay3D.h"
#include"DxQTriangle3D.h"
#include"DxQTriangleI.h"

#include"HdQConfig.h"



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
    INTERFACE_API Point3DStd::Point3D GetRelativeXYZ(
        const Point3DStd::Point3D& o1,
        const Point3DStd::Point3D& ex,
        const Point3DStd::Point3D& ey,
        const Point3DStd::Point3D& ez,
        const Point3DStd::Point3D& p0);

    INTERFACE_API Point3DStd::Point3D GetRelativeXYZPlus(
        const Point3DStd::Point3D& o1,
        const Point3DStd::Point3D& ex,
        const Point3DStd::Point3D& ey,
        const Point3DStd::Point3D& ez,
        const Point3DStd::Point3D& p0);

    INTERFACE_API bool CoordinatedSystemTransformation(const BaseCoordinateSystemStd::BaseCoordinateSystem& sys1, const Point3DStd::Point3D& goal,
        const BaseCoordinateSystemStd::BaseCoordinateSystem& sys2, Point3DStd::Point3D& res);


    INTERFACE_API bool BuildDiffractionCoordinateSystem(
        const Point3DStd::Point3D& o,
        const Point3DStd::Point3D& vec_faker_z,
        const Point3DStd::Point3D& p3,
        const Point3DStd::Point3D& n,
        BaseCoordinateSystemStd::BaseCoordinateSystem& res);


    INTERFACE_API bool BuildDiffractionCoordinateSystemBySegment(
        const Point3DStd::Point3D& start,
        const Point3DStd::Point3D& end,
        const Point3DStd::Point3D& p3,
        const Point3DStd::Point3D& n,
        BaseCoordinateSystemStd::BaseCoordinateSystem& res);


    INTERFACE_API bool CanBuildDiffractionCoordinateSystemByCorner(
        const Point3DStd::Point3D& start,
        const Point3DStd::Point3D& end,
        const Point3DStd::Point3D& triangle_0_p3,
        const Point3DStd::Point3D& triangle_n_p3,
        const Point3DStd::Point3D& face0_n);



    INTERFACE_API bool BuildDiffractionCoordinateSystemByCorner(
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
        double& phiE);




}