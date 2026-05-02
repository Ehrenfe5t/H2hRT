#pragma once



#include"LxQPoint3D.h"

namespace Ray3DStd {
    /// <summary>
    /// 射线
    /// </summary>
    class Ray3D {
    public:
        /// <summary>
        /// 射线端点
        /// </summary>
        Point3DStd::Point3D o;
        /// <summary>
        /// 射线方向向量
        /// </summary>
        Point3DStd::Point3D vec;

        Ray3D(const Point3DStd::Point3D& o, const Point3DStd::Point3D& vec);

        Ray3D();

        Ray3D(const Ray3D& obj);
        ~Ray3D();
    };
}




