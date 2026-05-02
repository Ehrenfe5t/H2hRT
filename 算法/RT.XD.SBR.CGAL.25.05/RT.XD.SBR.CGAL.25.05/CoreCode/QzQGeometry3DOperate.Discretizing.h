#pragma once



#include"HdQBall3D.h"
#include"HdQBaseCoordinateSystem.h"
#include"HdQBoundingBox3D.h"
#include"HdQConfig.h"
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



namespace Geometry3DOperateStd {


    INTERFACE_API void DiscretizingTriangle3DPlus(
        const Point3DStd::Point3D& p1,
        const Point3DStd::Point3D& p2,
        const Point3DStd::Point3D& p3,
        double maxSideLength,
        std::vector<Point3DStd::Point3D>& res);

    INTERFACE_API std::vector<Point3DStd::Point3D> DiscretizingTriangle3D(
        const Point3DStd::Point3D& p1,
        const Point3DStd::Point3D& p2,
        const Point3DStd::Point3D& p3,
        double maxSideLength);


}