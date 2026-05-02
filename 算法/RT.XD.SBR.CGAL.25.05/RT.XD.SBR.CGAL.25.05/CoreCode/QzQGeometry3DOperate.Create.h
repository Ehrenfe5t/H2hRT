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


    INTERFACE_API Point3DStd::Point3D CreatePoint3D(double x, double y, double z);

    INTERFACE_API BoundingBox3DStd::BoundingBox3D CreateBoundingBox3DBy3Point3D(const Point3DStd::Point3D& a, const Point3DStd::Point3D& b, const Point3DStd::Point3D& c);

    INTERFACE_API Ball3DStd::Ball3D CreateBall3DByListPoint3D(const std::vector<Point3DStd::Point3D>& list);

    INTERFACE_API Ball3DStd::Ball3D CreateBall3DBy3Point3D(const Point3DStd::Point3D& a, const Point3DStd::Point3D& b, const Point3DStd::Point3D& c);


    INTERFACE_API bool CreateLine3DByTwoPoint3D_safe(const Point3DStd::Point3D& start, const Point3DStd::Point3D& end, Line3DStd::Line3D& res);

    INTERFACE_API Line3DStd::Line3D CreateLine3DByTwoPoint3D_unsafe(const Point3DStd::Point3D& start, const Point3DStd::Point3D& end);

    INTERFACE_API Line3DStd::Line3D CreateLine3D_unsafe(const Point3DStd::Point3D& o, const Point3DStd::Point3D& vec);
}