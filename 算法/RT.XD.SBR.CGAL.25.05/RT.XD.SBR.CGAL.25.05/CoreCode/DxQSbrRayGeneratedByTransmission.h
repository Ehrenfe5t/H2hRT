#pragma once

#include"HdQConfig.h"
#include"LxQMaterialObject.h"
#include"DxQRay3D.h"
#include"DxQScenarioTriangle3DIndex.h"
#include<vector>

namespace SbrRayGeneratedByTransmissionStd {



    /// <summary>
    /// …˙≥…’€…‰…‰œﬂ
    /// </summary>
    /// <param name="a"></param>
    /// <param name="b"></param>
    /// <param name="face"></param>
    /// <param name="materialObjects"></param>
    /// <param name="f"></param>
    /// <param name="eps"></param>
    /// <param name="newRay"></param>
    /// <param name="curNode"></param>
    /// <returns></returns>
    INTERFACE_API int BuildTransmissionRay(
        const Point3DStd::Point3D& a,
        const Point3DStd::Point3D& b,
        const ScenarioTriangle3DIndexStd::ScenarioTriangle3DIndex& face,
        const Point3DStd::Point3D& faceN,
        const std::vector<MaterialObjectStd::MaterialObject>& materialObjects,
        long long f,
        Ray3DStd::Ray3D& newRay,
        double& thetai);

    INTERFACE_API int BuildTransmissionRay_RayTracingGeometricPathNode(
        int faceUpTypeNumber,
        int faceDownTypeNumber,
        long long f,
        const Point3DStd::Point3D& a,
        const Point3DStd::Point3D& b,
        const Point3DStd::Point3D& faceN,
        Ray3DStd::Ray3D& newRay,
        double& thetai);
}