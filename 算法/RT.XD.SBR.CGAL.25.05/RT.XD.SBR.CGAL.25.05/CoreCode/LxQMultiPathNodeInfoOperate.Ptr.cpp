
#include"LxQMultiPathNodeInfoOperate.Ptr.h"

#include <mutex>
namespace MultiPathNodeInfoOperateStd {

    std::vector<MultiPathNodeInfoStd::MultiPathNodeInfo*> node_all;

    std::mutex mtx_node_all;
    void AddTonode_all(MultiPathNodeInfoStd::MultiPathNodeInfo* ptr) {
        if (ptr != NULL) {
            std::lock_guard<std::mutex> lock(mtx_node_all);
            node_all.emplace_back(ptr);
        }
    }


    void FreeMultiPathNodeInfo_vector_all() {
        FreeMultiPathNodeInfo_vector(node_all);
    }

    MultiPathNodeInfoStd::MultiPathNodeInfo* CreateMultiPathNodeInfoPtr() {
        MultiPathNodeInfoStd::MultiPathNodeInfo* ptr = new MultiPathNodeInfoStd::MultiPathNodeInfo();
        AddTonode_all(ptr);
        return ptr;
    }

    MultiPathNodeInfoStd::MultiPathNodeInfoTransmittingAntenna* CreateMultiPathNodeInfoTransmittingAntenna(
        int baseStationAntennaID, const Point3DStd::Point3D& location)
    {
        MultiPathNodeInfoStd::MultiPathNodeInfoTransmittingAntenna* ptr = new MultiPathNodeInfoStd::MultiPathNodeInfoTransmittingAntenna(
            baseStationAntennaID, location);
        AddTonode_all(ptr);
        return ptr;
    }

    MultiPathNodeInfoStd::MultiPathNodeInfo* MultiPathNodeInfoTransmittingAntenna_to_MultiPathNodeInfo_ptr(
        const MultiPathNodeInfoStd::MultiPathNodeInfoTransmittingAntenna& child)
    {
        return MultiPathNodeInfoTransmittingAntenna_ptr_to_MultiPathNodeInfo_ptr(CreateMultiPathNodeInfoTransmittingAntenna(
            child.baseStationAntennaID, child.location));
    }

    MultiPathNodeInfoStd::MultiPathNodeInfo* MultiPathNodeInfoTransmittingAntenna_ptr_to_MultiPathNodeInfo_ptr(
        MultiPathNodeInfoStd::MultiPathNodeInfoTransmittingAntenna* child)
    {
        return (MultiPathNodeInfoStd::MultiPathNodeInfo*)child;
    }

    MultiPathNodeInfoStd::MultiPathNodeInfoTransmittingAntenna* MultiPathNodeInfo_ptr_to_MultiPathNodeInfoTransmittingAntenna_ptr(
        MultiPathNodeInfoStd::MultiPathNodeInfo* father)
    {
        return (MultiPathNodeInfoStd::MultiPathNodeInfoTransmittingAntenna*)father;
    }


    MultiPathNodeInfoStd::MultiPathNodeInfoReceiverAntenna* CreateMultiPathNodeInfoReceiverAntenna(
        int antennaID,
        const Point3DStd::Point3D& location)
    {
        MultiPathNodeInfoStd::MultiPathNodeInfoReceiverAntenna* ptr = new MultiPathNodeInfoStd::MultiPathNodeInfoReceiverAntenna(antennaID, location);
        AddTonode_all(ptr);
        return ptr;
    }
    MultiPathNodeInfoStd::MultiPathNodeInfo* MultiPathNodeInfoReceiverAntenna_to_MultiPathNodeInfo_ptr(
        const MultiPathNodeInfoStd::MultiPathNodeInfoReceiverAntenna& child)
    {
        return MultiPathNodeInfoReceiverAntenna_ptr_to_MultiPathNodeInfo_ptr(CreateMultiPathNodeInfoReceiverAntenna(
            child.antennaID, child.location));
    }
    MultiPathNodeInfoStd::MultiPathNodeInfo* MultiPathNodeInfoReceiverAntenna_ptr_to_MultiPathNodeInfo_ptr(
        MultiPathNodeInfoStd::MultiPathNodeInfoReceiverAntenna* child)
    {
        return (MultiPathNodeInfoStd::MultiPathNodeInfo*)child;
    }
    MultiPathNodeInfoStd::MultiPathNodeInfoReceiverAntenna* MultiPathNodeInfo_ptr_to_MultiPathNodeInfoReceiverAntenna_ptr(
        MultiPathNodeInfoStd::MultiPathNodeInfo* father)
    {
        return (MultiPathNodeInfoStd::MultiPathNodeInfoReceiverAntenna*)father;
    }



    MultiPathNodeInfoStd::MultiPathNodeInfoReflection* CreateMultiPathNodeInfoReflection(
        int upObjectType, int downObjectType, double thetai,
        const Point3DStd::Point3D& location,
        const Point3DStd::Point3D& normalVector)
    {
        MultiPathNodeInfoStd::MultiPathNodeInfoReflection* ptr = new MultiPathNodeInfoStd::MultiPathNodeInfoReflection(
            upObjectType, downObjectType, thetai,
            location, normalVector);
        AddTonode_all(ptr);
        return ptr;
    }

    MultiPathNodeInfoStd::MultiPathNodeInfo* MultiPathNodeInfoReflection_to_MultiPathNodeInfo_ptr(
        const MultiPathNodeInfoStd::MultiPathNodeInfoReflection& child)
    {
        return MultiPathNodeInfoReflection_ptr_to_MultiPathNodeInfo_ptr(CreateMultiPathNodeInfoReflection(
            child.upObjectType, child.downObjectType, child.thetai,
            child.location, child.normalVector));
    }

    MultiPathNodeInfoStd::MultiPathNodeInfo* MultiPathNodeInfoReflection_ptr_to_MultiPathNodeInfo_ptr(
        MultiPathNodeInfoStd::MultiPathNodeInfoReflection* child)
    {
        return (MultiPathNodeInfoStd::MultiPathNodeInfo*)child;
    }

    MultiPathNodeInfoStd::MultiPathNodeInfoReflection* MultiPathNodeInfo_ptr_to_MultiPathNodeInfoReflection_ptr(
        MultiPathNodeInfoStd::MultiPathNodeInfo* father)
    {
        return (MultiPathNodeInfoStd::MultiPathNodeInfoReflection*)father;
    }



    MultiPathNodeInfoStd::MultiPathNodeInfoTransmission* CreateMultiPathNodeInfoTransmission(
        int upObjectType, int downObjectType, double thetai,
        const Point3DStd::Point3D& location,
        const Point3DStd::Point3D& normalVector)
    {
        MultiPathNodeInfoStd::MultiPathNodeInfoTransmission* ptr = new MultiPathNodeInfoStd::MultiPathNodeInfoTransmission(
            upObjectType, downObjectType, thetai,
            location, normalVector);
        AddTonode_all(ptr);
        return ptr;
    }

    MultiPathNodeInfoStd::MultiPathNodeInfo* MultiPathNodeInfoTransmission_to_MultiPathNodeInfo_ptr(
        const MultiPathNodeInfoStd::MultiPathNodeInfoTransmission& child)
    {
        return MultiPathNodeInfoTransmission_ptr_to_MultiPathNodeInfo_ptr(CreateMultiPathNodeInfoTransmission(
            child.upObjectType, child.downObjectType, child.thetai,
            child.location, child.normalVector));
    }

    MultiPathNodeInfoStd::MultiPathNodeInfo* MultiPathNodeInfoTransmission_ptr_to_MultiPathNodeInfo_ptr(
        MultiPathNodeInfoStd::MultiPathNodeInfoTransmission* child)
    {
        return (MultiPathNodeInfoStd::MultiPathNodeInfo*)child;
    }

    MultiPathNodeInfoStd::MultiPathNodeInfoTransmission* MultiPathNodeInfo_ptr_to_MultiPathNodeInfoTransmission_ptr(
        MultiPathNodeInfoStd::MultiPathNodeInfo* father)
    {
        return (MultiPathNodeInfoStd::MultiPathNodeInfoTransmission*)father;
    }



    MultiPathNodeInfoStd::MultiPathNodeInfoDiffraction* CreateMultiPathNodeInfoDiffraction(
        int upObjectType, int downObjectType,
        double beta, double phiE, double phi1, double phi2,
        const Point3DStd::Point3D& location,
        const Point3DStd::Point3D& normalVector)
    {
        MultiPathNodeInfoStd::MultiPathNodeInfoDiffraction* ptr = new MultiPathNodeInfoStd::MultiPathNodeInfoDiffraction(
            upObjectType, downObjectType,
            beta, phiE, phi1, phi2,
            location, normalVector);
        AddTonode_all(ptr);
        return ptr;
    }

    MultiPathNodeInfoStd::MultiPathNodeInfo* MultiPathNodeInfoDiffraction_to_MultiPathNodeInfo_ptr(
        const MultiPathNodeInfoStd::MultiPathNodeInfoDiffraction& child)
    {
        return MultiPathNodeInfoDiffraction_ptr_to_MultiPathNodeInfo_ptr(CreateMultiPathNodeInfoDiffraction(
            child.upObjectType, child.downObjectType,
            child.beta, child.phiE, child.phi1, child.phi2,
            child.location, child.normalVector));
    }

    MultiPathNodeInfoStd::MultiPathNodeInfo* MultiPathNodeInfoDiffraction_ptr_to_MultiPathNodeInfo_ptr(
        MultiPathNodeInfoStd::MultiPathNodeInfoDiffraction* child)
    {
        return (MultiPathNodeInfoStd::MultiPathNodeInfo*)child;
    }

    MultiPathNodeInfoStd::MultiPathNodeInfoDiffraction* MultiPathNodeInfo_ptr_to_MultiPathNodeInfoDiffraction_ptr(
        MultiPathNodeInfoStd::MultiPathNodeInfo* father)
    {
        return (MultiPathNodeInfoStd::MultiPathNodeInfoDiffraction*)father;
    }



    MultiPathNodeInfoStd::MultiPathNodeInfoDiffuseScattering* CreateMultiPathNodeInfoDiffuseScattering(
        int upObjectType, int downObjectType,
        int widthCoefficientOfDiffuseLobe, float roughness, double thetai, double beta, double area, double scatteringCoefficient,
        const Point3DStd::Point3D& location,
        const Point3DStd::Point3D& normalVector)
    {
        MultiPathNodeInfoStd::MultiPathNodeInfoDiffuseScattering* ptr = new MultiPathNodeInfoStd::MultiPathNodeInfoDiffuseScattering(
            upObjectType, downObjectType,
            widthCoefficientOfDiffuseLobe, roughness, thetai, beta, area, scatteringCoefficient,
            location, normalVector);
        AddTonode_all(ptr);
        return ptr;
    }

    MultiPathNodeInfoStd::MultiPathNodeInfo* MultiPathNodeInfoDiffuseScattering_to_MultiPathNodeInfo_ptr(
        const MultiPathNodeInfoStd::MultiPathNodeInfoDiffuseScattering& child)
    {
        return MultiPathNodeInfoDiffuseScattering_ptr_to_MultiPathNodeInfo_ptr(CreateMultiPathNodeInfoDiffuseScattering(
            child.upObjectType, child.downObjectType,
            child.widthCoefficientOfDiffuseLobe, child.roughness, child.thetai, child.beta, child.area, child.scatteringCoefficient,
            child.location, child.normalVector));
    }

    MultiPathNodeInfoStd::MultiPathNodeInfo* MultiPathNodeInfoDiffuseScattering_ptr_to_MultiPathNodeInfo_ptr(
        MultiPathNodeInfoStd::MultiPathNodeInfoDiffuseScattering* child)
    {
        return (MultiPathNodeInfoStd::MultiPathNodeInfo*)child;
    }

    MultiPathNodeInfoStd::MultiPathNodeInfoDiffuseScattering* MultiPathNodeInfo_ptr_to_MultiPathNodeInfoDiffuseScattering_ptr(
        MultiPathNodeInfoStd::MultiPathNodeInfo* father)
    {
        return (MultiPathNodeInfoStd::MultiPathNodeInfoDiffuseScattering*)father;
    }

    void FreeMultiPathNodeInfo(MultiPathNodeInfoStd::MultiPathNodeInfo* father)
    {
        switch (father->type)
        {
        case PropagationTypeStd::PropagationType::TransmittingAntenna:
        {
            auto ptr = MultiPathNodeInfo_ptr_to_MultiPathNodeInfoTransmittingAntenna_ptr(father);
            delete ptr;
            break;
        }
        case PropagationTypeStd::PropagationType::ReceiverAntenna:
        {
            auto ptr = MultiPathNodeInfo_ptr_to_MultiPathNodeInfoReceiverAntenna_ptr(father);
            delete ptr;
            break;
        }
        case PropagationTypeStd::PropagationType::Reflection:
        {
            auto ptr = MultiPathNodeInfo_ptr_to_MultiPathNodeInfoReflection_ptr(father);
            delete ptr;
            break;
        }
        case PropagationTypeStd::PropagationType::Transmission:
        {
            auto ptr = MultiPathNodeInfo_ptr_to_MultiPathNodeInfoTransmission_ptr(father);
            delete ptr;
            break;
        }
        case PropagationTypeStd::PropagationType::Diffraction:
        {
            auto ptr = MultiPathNodeInfo_ptr_to_MultiPathNodeInfoDiffraction_ptr(father);
            delete ptr;
            break;
        }
        case PropagationTypeStd::PropagationType::DiffuseScattering:
        {
            auto ptr = MultiPathNodeInfo_ptr_to_MultiPathNodeInfoDiffuseScattering_ptr(father);
            delete ptr;
            break;
        }
        case PropagationTypeStd::PropagationType::Null:
        {
            auto ptr = father;
            delete ptr;
            break;
        }

        default:
            break;
        }
    }

    void FreeMultiPathNodeInfo_vector(std::vector<MultiPathNodeInfoStd::MultiPathNodeInfo*>& path)
    {
        while (!path.empty()) {
            MultiPathNodeInfoStd::MultiPathNodeInfo* element = path.back();
            path.pop_back();
            FreeMultiPathNodeInfo(element);
        }
        path.clear();
    }


}