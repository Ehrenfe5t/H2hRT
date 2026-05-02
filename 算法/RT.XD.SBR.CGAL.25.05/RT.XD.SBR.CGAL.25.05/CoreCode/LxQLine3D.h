#pragma once



#include"LxQPoint3D.h"

namespace Line3DStd {

    /// <summary>
    /// 直线
    /// </summary>
    class Line3D {
    public:
        /// <summary>
        /// 直线上一点
        /// </summary>
        Point3DStd::Point3D p;
        /// <summary>
        /// 直线方向向量
        /// </summary>
        Point3DStd::Point3D vec;

        Line3D();
        Line3D(const Point3DStd::Point3D& p, const Point3DStd::Point3D& vec);
        Line3D(const Line3D& obj);
        ~Line3D();
    };

    
}

