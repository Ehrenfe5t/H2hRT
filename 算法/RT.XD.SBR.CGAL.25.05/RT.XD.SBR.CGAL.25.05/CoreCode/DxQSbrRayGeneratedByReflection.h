#pragma once

#include"HdQConfig.h"
#include"LxQPoint3D.h"
#include"DxQRay3D.h"
#include<vector>

namespace SbrRayGeneratedByReflectionStd {
    /// <summary>
    /// 生成反射射线
    /// </summary>
    /// <param name="a"></param>
    /// <param name="b"></param>
    /// <param name="face"></param>
    /// <param name="eps"></param>
    /// <param name="newRay"></param>
    /// <param name="curNode"></param>
    /// <returns></returns>
    INTERFACE_API int BuildReflectionRay(
        const Point3DStd::Point3D& a,
        const Point3DStd::Point3D& b,
        const Point3DStd::Point3D& n,
        Ray3DStd::Ray3D& newRay,
        double& thetai);
}