#pragma once

#include"../Input.h"


#include<iostream>

namespace SbrRayGeneratedByTransmissionStd {



    /// <summary>
    /// 生成折射射线
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
    int BuildTransmissionRay(
        const Point3DStd::Point3D& a,
        const Point3DStd::Point3D& b,
        const ScenarioTriangle3DIndexStd::ScenarioTriangle3DIndex& face,
        const Point3DStd::Point3D& faceN,
        const std::vector<MaterialObjectStd::MaterialObject>& materialObjects,
        long long f,
        Ray3DStd::Ray3D& newRay, 
        double& thetai) {
        Point3DStd::Point3D ab = Geometry3DOperateStd::SubPoint3DPoint3D(b, a);
        double d1 = Geometry3DOperateStd::DotPoint3DPoint3D(ab, faceN);
        double d2 = Geometry3DOperateStd::Length_Point3D(ab);
        if (d2 < GlobalConstantStd::Eps) {
            return 0;
        }
        double d3 = d1 / d2;
        Point3DStd::Point3D n(faceN);
        if (d1 < 0) {
            d3 = -d3;
        }
        else {
            n.x = -n.x;
            n.y = -n.y;
            n.z = -n.z;
        }
        if (d3 > 1.0)d3 = 1.0;
        if (d3 < -1.0)d3 = -1.0;
        thetai = acos(d3);
        ab = Geometry3DOperateStd::MulDoublePoint3D(1.0 / d2, ab);
        Point3DStd::Point3D bc = Geometry3DOperateStd::MulDoublePoint3D(d3, n);
        Point3DStd::Point3D ac = Geometry3DOperateStd::AddPoint3DPoint3D(ab, bc);
        double thetat = 0.0;

        int index_1_1 = MaterialObjectStd::FindIndexOfListMaterialParamByObjectTypeAndF(face.UpTypeNumber, f, materialObjects);
        if (index_1_1 == -1) {
            ProjectDependenciesStd::DisplayPromptOrReason("缺少材质数据.", true, __FILE__, __LINE__);
            return 0;
        }
        double er1 = materialObjects[index_1_1].relativePermittivity;
        double sigma1 = materialObjects[index_1_1].conductivity;
        double mu1 = 1;
        int index_2_1 = MaterialObjectStd::FindIndexOfListMaterialParamByObjectTypeAndF(face.DownTypeNumber, f, materialObjects);
        if (index_2_1 == -1) {
            ProjectDependenciesStd::DisplayPromptOrReason("缺少材质数据.", true, __FILE__, __LINE__);
            return 0;
        }
        double er2 = materialObjects[index_2_1].relativePermittivity;
        double sigma2 = materialObjects[index_2_1].conductivity;
        double mu2 = 1;
        if (d1 < 0) {
            //上表面除以下表面
            if (0 == MathOperateStd::GetThetaT(er1, mu1, er2, mu2, thetai, thetat)) {
                return 0;
            }
        }
        else {
            //下表面除以上表面
            if (0 == MathOperateStd::GetThetaT(er2, mu2, er1, mu1, thetai, thetat)) {
                return 0;
            }
        }

        ac = Geometry3DOperateStd::Normalization_Point3D_unsafe(ac);
        Point3DStd::Point3D de = Geometry3DOperateStd::MulDoublePoint3D(sin(thetat), ac);
        Point3DStd::Point3D bd = Geometry3DOperateStd::MulDoublePoint3D(cos(thetat), n);
        Point3DStd::Point3D vec = Geometry3DOperateStd::SubPoint3DPoint3D(de, bd);
        Geometry3DOperateStd::AssignmentPoint3DPoint3D(newRay.o, b);
        Geometry3DOperateStd::AssignmentPoint3DPoint3D(newRay.vec, vec);

        return 1;
    }


    int BuildTransmissionRay_RayTracingGeometricPathNode(
        int faceUpTypeNumber,
        int faceDownTypeNumber,
        long long f,
        const Point3DStd::Point3D& a,
        const Point3DStd::Point3D& b,
        const Point3DStd::Point3D& faceN,
        Ray3DStd::Ray3D& newRay,
        double& thetai) {
        Point3DStd::Point3D ab = Geometry3DOperateStd::SubPoint3DPoint3D(b, a);
        double d1 = Geometry3DOperateStd::DotPoint3DPoint3D(ab, faceN);
        double d2 = Geometry3DOperateStd::Length_Point3D(ab);
        if (d2 < GlobalConstantStd::Eps) {
            return 0;
        }
        double d3 = d1 / d2;
        Point3DStd::Point3D n(faceN);
        if (d1 < 0) {
            d3 = -d3;
        }
        else {
            n.x = -n.x;
            n.y = -n.y;
            n.z = -n.z;
        }
        if (d3 > 1.0)d3 = 1.0;
        if (d3 < -1.0)d3 = -1.0;
        thetai = acos(d3);
        ab = Geometry3DOperateStd::MulDoublePoint3D(1.0 / d2, ab);
        Point3DStd::Point3D bc = Geometry3DOperateStd::MulDoublePoint3D(d3, n);
        Point3DStd::Point3D ac = Geometry3DOperateStd::AddPoint3DPoint3D(ab, bc);
        double thetat = 0.0;

        MaterialObjectStd::MaterialObject materialObjectUp;
        if (!MaterialObjectDatabaseStd::Find(faceUpTypeNumber, f, materialObjectUp)) {
            return 0;
        }
        MaterialObjectStd::MaterialObject materialObjectDown;
        if (!MaterialObjectDatabaseStd::Find(faceDownTypeNumber, f, materialObjectDown)) {
            return 0;
        }

        if (d1 < 0) {
            //上表面除以下表面
            if (0 == MathOperateStd::GetThetaT(
                materialObjectUp.relativePermittivity, materialObjectUp.relativePermeability,
                materialObjectDown.relativePermittivity, materialObjectDown.relativePermeability,
                thetai, thetat)) {
                return 0;
            }
        }
        else {
            //下表面除以上表面
            if (0 == MathOperateStd::GetThetaT(
                materialObjectDown.relativePermittivity, materialObjectDown.relativePermeability,
                materialObjectUp.relativePermittivity, materialObjectUp.relativePermeability,
                thetai, thetat)) {
                return 0;
            }
        }

        ac = Geometry3DOperateStd::Normalization_Point3D_unsafe(ac);
        Point3DStd::Point3D de = Geometry3DOperateStd::MulDoublePoint3D(sin(thetat), ac);
        Point3DStd::Point3D bd = Geometry3DOperateStd::MulDoublePoint3D(cos(thetat), n);
        Point3DStd::Point3D vec = Geometry3DOperateStd::SubPoint3DPoint3D(de, bd);
        Geometry3DOperateStd::AssignmentPoint3DPoint3D(newRay.o, b);
        Geometry3DOperateStd::AssignmentPoint3DPoint3D(newRay.vec, vec);

        return 1;
    }

}