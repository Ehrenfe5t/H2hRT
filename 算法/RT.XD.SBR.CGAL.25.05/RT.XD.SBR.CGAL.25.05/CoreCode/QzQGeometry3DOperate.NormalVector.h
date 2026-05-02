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

    INTERFACE_API Point3DStd::Point3D NormalVector_Triangle_plus_unsafe(const Point3DStd::Point3D& p1, const Point3DStd::Point3D& p2, const Point3DStd::Point3D& p3);
    INTERFACE_API Point3DStd::Point3D NormalVector_Triangle_unsafe(const Triangle3DStd::Triangle3D& triangle);
    INTERFACE_API bool NormalVector_Triangle_plus_safe(const Point3DStd::Point3D& p1, const Point3DStd::Point3D& p2, const Point3DStd::Point3D& p3, Point3DStd::Point3D& res);
    INTERFACE_API bool NormalVector_Triangle_safe(const Triangle3DStd::Triangle3D& triangle, Point3DStd::Point3D& res);
    INTERFACE_API Point3DStd::Point3D NormalVector_Point3D(Point3DStd::Point3D& p);
}