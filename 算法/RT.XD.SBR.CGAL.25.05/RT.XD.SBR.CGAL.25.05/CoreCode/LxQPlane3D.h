#pragma once



#include"LxQPoint3D.h"

namespace Plane3DStd {
    /// <summary>
    /// 平面
    /// </summary>
    class Plane3D {
    public:
        /// <summary>
        /// 平面上一点
        /// </summary>
        Point3DStd::Point3D p;
        /// <summary>
        /// 平面法向量
        /// </summary>
        Point3DStd::Point3D n;
        ~Plane3D();

        Plane3D(const Point3DStd::Point3D& p, const Point3DStd::Point3D& n);
        Plane3D();
        Plane3D(const Plane3D& obj);
        bool operator<(const Plane3D& obj) const;
    };

}

