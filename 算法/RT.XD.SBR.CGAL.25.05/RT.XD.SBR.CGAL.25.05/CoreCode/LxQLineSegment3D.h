#pragma once



#include"LxQPoint3D.h"

namespace LineSegment3DStd {


    /// <summary>
    /// 三维线段
    /// </summary>
    class LineSegment3D {
    public:
        /// <summary>
        /// 线段开始点
        /// </summary>
        Point3DStd::Point3D start;
        /// <summary>
        /// 线段结束点
        /// </summary>
        Point3DStd::Point3D end;

        LineSegment3D();
        LineSegment3D(const Point3DStd::Point3D& start, const Point3DStd::Point3D& end);
        LineSegment3D(const LineSegment3D& obj);
        ~LineSegment3D();
    };


}