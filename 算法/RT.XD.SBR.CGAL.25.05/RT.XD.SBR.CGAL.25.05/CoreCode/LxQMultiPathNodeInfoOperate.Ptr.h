#pragma once

#include"LxQMultiPathNodeInfo.h"
#include"LxQMultiPathNodeInfoDiffraction.h"
#include"LxQMultiPathNodeInfoDiffuseScattering.h"
#include"LxQMultiPathNodeInfoReceiverAntenna.h"
#include"LxQMultiPathNodeInfoReflection.h"
#include"LxQMultiPathNodeInfoTransmission.h"
#include"LxQMultiPathNodeInfoTransmittingAntenna.h"


namespace MultiPathNodeInfoOperateStd {

    MultiPathNodeInfoStd::MultiPathNodeInfo* CreateMultiPathNodeInfoPtr();

     MultiPathNodeInfoStd::MultiPathNodeInfoTransmittingAntenna* CreateMultiPathNodeInfoTransmittingAntenna(
        int baseStationAntennaID, const Point3DStd::Point3D& location);

     MultiPathNodeInfoStd::MultiPathNodeInfo* MultiPathNodeInfoTransmittingAntenna_to_MultiPathNodeInfo_ptr(
        const MultiPathNodeInfoStd::MultiPathNodeInfoTransmittingAntenna& child);

     MultiPathNodeInfoStd::MultiPathNodeInfo* MultiPathNodeInfoTransmittingAntenna_ptr_to_MultiPathNodeInfo_ptr(
        MultiPathNodeInfoStd::MultiPathNodeInfoTransmittingAntenna* child);

     MultiPathNodeInfoStd::MultiPathNodeInfoTransmittingAntenna* MultiPathNodeInfo_ptr_to_MultiPathNodeInfoTransmittingAntenna_ptr(
        MultiPathNodeInfoStd::MultiPathNodeInfo* father);


     MultiPathNodeInfoStd::MultiPathNodeInfoReceiverAntenna* CreateMultiPathNodeInfoReceiverAntenna(
        int antennaID,
        const Point3DStd::Point3D& location);
     MultiPathNodeInfoStd::MultiPathNodeInfo* MultiPathNodeInfoReceiverAntenna_to_MultiPathNodeInfo_ptr(
        const MultiPathNodeInfoStd::MultiPathNodeInfoReceiverAntenna& child);
     MultiPathNodeInfoStd::MultiPathNodeInfo* MultiPathNodeInfoReceiverAntenna_ptr_to_MultiPathNodeInfo_ptr(
        MultiPathNodeInfoStd::MultiPathNodeInfoReceiverAntenna* child);
     MultiPathNodeInfoStd::MultiPathNodeInfoReceiverAntenna* MultiPathNodeInfo_ptr_to_MultiPathNodeInfoReceiverAntenna_ptr(
        MultiPathNodeInfoStd::MultiPathNodeInfo* father);

     MultiPathNodeInfoStd::MultiPathNodeInfoReflection* CreateMultiPathNodeInfoReflection(
        int upObjectType, int downObjectType, double thetai,
        const Point3DStd::Point3D& location,
        const Point3DStd::Point3D& normalVector);

     MultiPathNodeInfoStd::MultiPathNodeInfo* MultiPathNodeInfoReflection_to_MultiPathNodeInfo_ptr(
        const MultiPathNodeInfoStd::MultiPathNodeInfoReflection& child);

     MultiPathNodeInfoStd::MultiPathNodeInfo* MultiPathNodeInfoReflection_ptr_to_MultiPathNodeInfo_ptr(
        MultiPathNodeInfoStd::MultiPathNodeInfoReflection* child);

     MultiPathNodeInfoStd::MultiPathNodeInfoReflection* MultiPathNodeInfo_ptr_to_MultiPathNodeInfoReflection_ptr(
        MultiPathNodeInfoStd::MultiPathNodeInfo* father);



     MultiPathNodeInfoStd::MultiPathNodeInfoTransmission* CreateMultiPathNodeInfoTransmission(
        int upObjectType, int downObjectType, double thetai,
        const Point3DStd::Point3D& location,
        const Point3DStd::Point3D& normalVector);

     MultiPathNodeInfoStd::MultiPathNodeInfo* MultiPathNodeInfoTransmission_to_MultiPathNodeInfo_ptr(
        const MultiPathNodeInfoStd::MultiPathNodeInfoTransmission& child);

     MultiPathNodeInfoStd::MultiPathNodeInfo* MultiPathNodeInfoTransmission_ptr_to_MultiPathNodeInfo_ptr(
        MultiPathNodeInfoStd::MultiPathNodeInfoTransmission* child);

     MultiPathNodeInfoStd::MultiPathNodeInfoTransmission* MultiPathNodeInfo_ptr_to_MultiPathNodeInfoTransmission_ptr(
        MultiPathNodeInfoStd::MultiPathNodeInfo* father);



     MultiPathNodeInfoStd::MultiPathNodeInfoDiffraction* CreateMultiPathNodeInfoDiffraction(
        int upObjectType, int downObjectType,
        double beta, double phiE, double phi1, double phi2,
        const Point3DStd::Point3D& location,
        const Point3DStd::Point3D& normalVector);

     MultiPathNodeInfoStd::MultiPathNodeInfo* MultiPathNodeInfoDiffraction_to_MultiPathNodeInfo_ptr(
        const MultiPathNodeInfoStd::MultiPathNodeInfoDiffraction& child);

     MultiPathNodeInfoStd::MultiPathNodeInfo* MultiPathNodeInfoDiffraction_ptr_to_MultiPathNodeInfo_ptr(
        MultiPathNodeInfoStd::MultiPathNodeInfoDiffraction* child);

     MultiPathNodeInfoStd::MultiPathNodeInfoDiffraction* MultiPathNodeInfo_ptr_to_MultiPathNodeInfoDiffraction_ptr(
        MultiPathNodeInfoStd::MultiPathNodeInfo* father);

     MultiPathNodeInfoStd::MultiPathNodeInfoDiffuseScattering* CreateMultiPathNodeInfoDiffuseScattering(
        int upObjectType, int downObjectType,
        int widthCoefficientOfDiffuseLobe, float roughness, double thetai, double beta, double area, double scatteringCoefficient,
        const Point3DStd::Point3D& location,
        const Point3DStd::Point3D& normalVector);

     MultiPathNodeInfoStd::MultiPathNodeInfo* MultiPathNodeInfoDiffuseScattering_to_MultiPathNodeInfo_ptr(
        const MultiPathNodeInfoStd::MultiPathNodeInfoDiffuseScattering& child);

     MultiPathNodeInfoStd::MultiPathNodeInfo* MultiPathNodeInfoDiffuseScattering_ptr_to_MultiPathNodeInfo_ptr(
        MultiPathNodeInfoStd::MultiPathNodeInfoDiffuseScattering* child);

     MultiPathNodeInfoStd::MultiPathNodeInfoDiffuseScattering* MultiPathNodeInfo_ptr_to_MultiPathNodeInfoDiffuseScattering_ptr(
        MultiPathNodeInfoStd::MultiPathNodeInfo* father);

     void FreeMultiPathNodeInfo(MultiPathNodeInfoStd::MultiPathNodeInfo* father);

     void FreeMultiPathNodeInfo_vector(std::vector<MultiPathNodeInfoStd::MultiPathNodeInfo*>& path);

     void FreeMultiPathNodeInfo_vector_all();
}