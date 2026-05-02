#pragma once

#include"HdQConfig.h"
#include"DxQRay3D.h"
#include<vector>

namespace SbrRayGeneratedByTransmittingAntennaStd{

    /// <summary>
    /// 基于Marsaglia方法生成均匀的发射球
    /// </summary>
    /// <param name="txPoints"></param>
    INTERFACE_API void GetUniformUnitVectorByMarsagliaMethod(int num, std::vector<Point3DStd::Point3D>& vecPoints);

    INTERFACE_API void GetUniformUnitVectorByMarsagliaMethod2(size_t num, std::vector<Ray3DStd::Ray3D>& txRays);

    /// <summary>
    /// 基于Marsaglia方法生成均匀的发射射线
    /// </summary>
    /// <param name="o"></param>
    /// <param name="num"></param>
    /// <param name="txRays"></param>
    INTERFACE_API void InitSBRLaunchRayMarsaglia(const Point3DStd::Point3D& o, int num, std::vector<Ray3DStd::Ray3D>& txRays);


    INTERFACE_API void InitSBRLaunchRayMarsaglia2(const Point3DStd::Point3D& o, size_t num, std::vector<Ray3DStd::Ray3D>& txRays);

    INTERFACE_API void InitSBRLaunchRayMarsaglia3(const Point3DStd::Point3D& o, size_t num, std::vector<Ray3DStd::Ray3D>& txRays);

    INTERFACE_API void InitSBRLaunchRayMarsaglia4(const Point3DStd::Point3D& o, size_t num, Ray3DStd::Ray3D* txRays);

    INTERFACE_API void InitSBRLaunchRayMarsagliaVec(size_t num, std::vector<Point3DStd::Point3D>& txRayVec);


    INTERFACE_API void InitSBRLaunchRayVec(size_t num, std::vector<Point3DStd::Point3D>& txRayVec);


}