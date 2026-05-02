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


    INTERFACE_API bool Location_Point3DonLineSegment3D_plus_plus(const Point3DStd::Point3D& b, const Point3DStd::Point3D& a, const Point3DStd::Point3D& c, double& distaceAB, double& distancBC);
    INTERFACE_API bool Location_Point3DonLineSegment3D_plus(const Point3DStd::Point3D& b, const Point3DStd::Point3D& a, const Point3DStd::Point3D& c);
    INTERFACE_API bool Location_Point3DonLineSegment3D_plus_2(const Point3DStd::Point3D& b, const Point3DStd::Point3D& a, const Point3DStd::Point3D& c, double eps);
    INTERFACE_API bool Location_Point3DonLineSegment3D(const Point3DStd::Point3D& p, const LineSegment3DStd::LineSegment3D& seg);
    INTERFACE_API bool Location_Point3DonLine3D_plus_safe(const Point3DStd::Point3D& p, const Point3DStd::Point3D& lineO, const Point3DStd::Point3D& lineVec);
    INTERFACE_API bool Location_Point3DonLine3D_plus_unsafe(const Point3DStd::Point3D& p, const Point3DStd::Point3D& lineO, const Point3DStd::Point3D& lineVec);
    INTERFACE_API bool Location_Point3DonLine3D_safe(const Point3DStd::Point3D& p, const Line3DStd::Line3D& line);
    INTERFACE_API bool Location_Point3DonLine3D_unsafe(const Point3DStd::Point3D& p, const Line3DStd::Line3D& line);
    INTERFACE_API bool Location_Point3DonPlane3D_plus_unsafe(const Point3DStd::Point3D& p, const Point3DStd::Point3D& planeO, const Point3DStd::Point3D& planeN);
    INTERFACE_API bool Location_Point3DonPlane3D_plus_safe(const Point3DStd::Point3D& p, const Point3DStd::Point3D& planeO, const Point3DStd::Point3D& planeN);
    INTERFACE_API bool Location_Point3DonPlane3D_unsafe(const Point3DStd::Point3D& p, const Plane3DStd::Plane3D& plane);
    INTERFACE_API bool Location_Point3DonPlane3D_safe(const Point3DStd::Point3D& p, const Plane3DStd::Plane3D& plane);
    INTERFACE_API bool Location_ParallelOrCoincident_Point3D_Point3D_plus_unsafe(const Point3DStd::Point3D& line1Vec,const Point3DStd::Point3D& line2Vec);
    INTERFACE_API bool Location_ParallelOrCoincident_Line3D_Line3D_plus_unsafe(const Point3DStd::Point3D& line1O,const Point3DStd::Point3D& line1Vec,const Point3DStd::Point3D& line2O,const Point3DStd::Point3D& line2Vec);
    INTERFACE_API bool Location_ParallelOrCoincident_Line3D_Line3D_unsafe(const Line3DStd::Line3D& line1, const Line3DStd::Line3D& line2);
    INTERFACE_API bool Location_Coincident_Line3D_Line3D_plus_unsafe(const Point3DStd::Point3D& line1O,const Point3DStd::Point3D& line1Vec,const Point3DStd::Point3D& line2O,const Point3DStd::Point3D& line2Vec);
    INTERFACE_API bool Location_Coincident_Line3D_Line3D_unsafe(const Line3DStd::Line3D& line1,const Line3DStd::Line3D& line2);
    INTERFACE_API bool Location_ParallelOrCoincident_LineSegment3D_LineSegment3D_plus_unsafe(const Point3DStd::Point3D& seg1Start,const Point3DStd::Point3D& seg1End,const Point3DStd::Point3D& seg2Start,const Point3DStd::Point3D& seg2End);
    INTERFACE_API bool Location_ParallelOrCoincident_LineSegment3D_LineSegment3D_unsafe(const LineSegment3DStd::LineSegment3D& seg1,const LineSegment3DStd::LineSegment3D& seg2);
    INTERFACE_API bool Location_Coincident_LineSegment3D_LineSegment3D_plus_unsafe(const Point3DStd::Point3D& seg1Start,const Point3DStd::Point3D& seg1End,const Point3DStd::Point3D& seg2Start,const Point3DStd::Point3D& seg2End);
    INTERFACE_API bool Location_Coincident_LineSegment3D_LineSegment3D_unsafe(const LineSegment3DStd::LineSegment3D& seg1,const LineSegment3DStd::LineSegment3D& seg2);
     
    INTERFACE_API bool Location_Point3DonRay3D_SameLine3D_plus_safe(const Point3DStd::Point3D& p, const Point3DStd::Point3D& rayO, const Point3DStd::Point3D& rayVec);
    INTERFACE_API bool Location_Point3DonRay3D_SameLine3D_safe(const Point3DStd::Point3D& p, const Ray3DStd::Ray3D& ray);
     
    INTERFACE_API char Location_Point3DOfShadowonLineSegment3D_plus(const Point3DStd::Point3D& b, const Point3DStd::Point3D& start, const Point3DStd::Point3D& end);
    INTERFACE_API char Location_Point3D_Plane3D_UpDown_plus(const Point3DStd::Point3D& p, const Point3DStd::Point3D& planeO, const Point3DStd::Point3D& planeN);
     
    INTERFACE_API bool Location_Point3DOfShadowonLineSegment3D_SameLine3D_plus(const Point3DStd::Point3D& b, const Point3DStd::Point3D& a, const Point3DStd::Point3D& c);
     
    INTERFACE_API int Location_Point3DonTriangle3D_SamePlane3D_unsafe(const Point3DStd::Point3D& p, const Point3DStd::Point3D& t1, const Point3DStd::Point3D& t2, const Point3DStd::Point3D& t3);
}