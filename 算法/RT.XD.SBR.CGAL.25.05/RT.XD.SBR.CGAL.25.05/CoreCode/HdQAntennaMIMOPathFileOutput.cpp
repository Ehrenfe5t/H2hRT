

#include"HdQAntennaMIMOPathFileOutput.h"

#include"QzQDirectoryOperate.h"

#include"QzQJsonFileOperateBase.h"

#include"LxQMultiPathNodeInfoOperate.Ptr.h"


namespace MultiPathNodeInfoStd{
    void from_json(const nlohmann::json& j, MultiPathNodeInfoStd::MultiPathNodeInfo& obj) {
        {
            auto jsonObject = j.at("type");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.type);
            }
        }
        {
            auto jsonObject = j.at("location");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.location);
            }
        }
    }
    void to_json(nlohmann::json& j, const MultiPathNodeInfoStd::MultiPathNodeInfo& obj) {

        j["type"] = obj.type;
        j["location"] = obj.location;
    }
}

namespace AntennaSISOOnePathStd{

    /// <summary>
    /// 从json字符串获取点
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void from_json(const nlohmann::json& j, AntennaSISOOnePathStd::AntennaSISOOnePath& obj) {

        {
            for (auto& object : j["path"]) {
                MultiPathNodeInfoStd::MultiPathNodeInfo value;
                from_json(object, value);

                MultiPathNodeInfoStd::MultiPathNodeInfo* ptr;
                if (value.type == PropagationTypeStd::PropagationType::TransmittingAntenna) {
                    MultiPathNodeInfoStd::MultiPathNodeInfoTransmittingAntenna node;
                    from_json(object, node);
                    ptr = MultiPathNodeInfoOperateStd::CreateMultiPathNodeInfoTransmittingAntenna(
                        node.baseStationAntennaID, node.location);
                }
                else if (value.type == PropagationTypeStd::PropagationType::ReceiverAntenna) {
                    MultiPathNodeInfoStd::MultiPathNodeInfoReceiverAntenna node;
                    from_json(object, node);
                    ptr = MultiPathNodeInfoOperateStd::CreateMultiPathNodeInfoReceiverAntenna(
                        node.antennaID, node.location);
                }
                else if (value.type == PropagationTypeStd::PropagationType::Reflection) {
                    MultiPathNodeInfoStd::MultiPathNodeInfoReflection node;
                    from_json(object, node);
                    ptr = MultiPathNodeInfoOperateStd::CreateMultiPathNodeInfoReflection(
                        node.upObjectType, node.downObjectType, node.thetai, node.location, node.normalVector);
                }
                else if (value.type == PropagationTypeStd::PropagationType::Transmission) {
                    MultiPathNodeInfoStd::MultiPathNodeInfoTransmission node;
                    from_json(object, node);
                    ptr = MultiPathNodeInfoOperateStd::CreateMultiPathNodeInfoTransmission(
                        node.upObjectType, node.downObjectType, node.thetai, node.location, node.normalVector);
                }
                else if (value.type == PropagationTypeStd::PropagationType::Diffraction) {
                    MultiPathNodeInfoStd::MultiPathNodeInfoDiffraction node;
                    from_json(object, node);
                    ptr = MultiPathNodeInfoOperateStd::CreateMultiPathNodeInfoDiffraction(
                        node.upObjectType, node.downObjectType, 
                        node.beta, node.phiE, node.phi1, node.phi2,
                        node.location, node.normalVector);
                }
                else if (value.type == PropagationTypeStd::PropagationType::DiffuseScattering) {
                    MultiPathNodeInfoStd::MultiPathNodeInfoDiffuseScattering node;
                    from_json(object, node);
                    ptr = MultiPathNodeInfoOperateStd::CreateMultiPathNodeInfoDiffuseScattering(
                        node.upObjectType, node.downObjectType,
                        node.widthCoefficientOfDiffuseLobe, node.roughness, node.thetai, node.beta, node.area, node.scatteringCoefficient,
                        node.location, node.normalVector);
                }

                
                obj.path.emplace_back(ptr);
            }
        }

    }
    /// <summary>
    /// 将点对象转化为json字符串
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void to_json(nlohmann::json& j, const AntennaSISOOnePathStd::AntennaSISOOnePath& obj) {

        j["path"];
        for (auto& value : obj.path) {
            nlohmann::json jf;
            if (value->type == PropagationTypeStd::PropagationType::TransmittingAntenna) {
                MultiPathNodeInfoStd::MultiPathNodeInfoTransmittingAntenna* ptr = 
                    MultiPathNodeInfoOperateStd::MultiPathNodeInfo_ptr_to_MultiPathNodeInfoTransmittingAntenna_ptr(value);
                to_json(jf, *ptr);
            }
            else if (value->type == PropagationTypeStd::PropagationType::ReceiverAntenna) {
                MultiPathNodeInfoStd::MultiPathNodeInfoReceiverAntenna* ptr =
                    MultiPathNodeInfoOperateStd::MultiPathNodeInfo_ptr_to_MultiPathNodeInfoReceiverAntenna_ptr(value);
                to_json(jf, *ptr);
            }
            else if (value->type == PropagationTypeStd::PropagationType::Reflection) {
                MultiPathNodeInfoStd::MultiPathNodeInfoReflection* ptr =
                    MultiPathNodeInfoOperateStd::MultiPathNodeInfo_ptr_to_MultiPathNodeInfoReflection_ptr(value);
                to_json(jf, *ptr);
            }
            else if (value->type == PropagationTypeStd::PropagationType::Transmission) {
                MultiPathNodeInfoStd::MultiPathNodeInfoTransmission* ptr =
                    MultiPathNodeInfoOperateStd::MultiPathNodeInfo_ptr_to_MultiPathNodeInfoTransmission_ptr(value);
                to_json(jf, *ptr);
            }
            else if (value->type == PropagationTypeStd::PropagationType::Diffraction) {
                MultiPathNodeInfoStd::MultiPathNodeInfoDiffraction* ptr =
                    MultiPathNodeInfoOperateStd::MultiPathNodeInfo_ptr_to_MultiPathNodeInfoDiffraction_ptr(value);
                to_json(jf, *ptr);
            }
            else if (value->type == PropagationTypeStd::PropagationType::DiffuseScattering) {
                MultiPathNodeInfoStd::MultiPathNodeInfoDiffuseScattering* ptr =
                    MultiPathNodeInfoOperateStd::MultiPathNodeInfo_ptr_to_MultiPathNodeInfoDiffuseScattering_ptr(value);
                to_json(jf, *ptr);
            }
            j["path"].push_back(jf);
        }
    }

}

namespace AntennaSISOPathStd {


    /// <summary>
    /// 从json字符串获取点
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void from_json(const nlohmann::json& j, AntennaSISOPathStd::AntennaSISOPath& obj) {
       
        {
            for (auto& object : j["paths"]) {
				AntennaSISOOnePathStd::AntennaSISOOnePath value;
                from_json(object, value);
                obj.paths.emplace_back(value);
            }
        }

    }
    /// <summary>
    /// 将点对象转化为json字符串
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void to_json(nlohmann::json& j, const AntennaSISOPathStd::AntennaSISOPath& obj) {

        j["paths"];
        for (auto& value : obj.paths) {
            nlohmann::json jf;
            to_json(jf, value);
            j["paths"].push_back(jf);
        }
    }



}

namespace AntennaMIMOPathFileOutputStd {

	void AntennaMIMOPathFileOutput_SISO(
		int transmittingAntennaId,
		const std::string& outPutDirectoryPathName,
		const AntennaSISOPathStd::AntennaSISOPath& antennaSISOPath) {

		
		std::string fileName;
		{
			std::ostringstream oss;
			oss << outPutDirectoryPathName << "\\GeometryPathInformation.Tx[" << transmittingAntennaId << "]AndRx[" << antennaSISOPath.receiverAntennaId << "].json";
			fileName = oss.str();
		}

		nlohmann::json jf;
		to_json(jf, antennaSISOPath);
		
		JsonFileOperateBaseStd::WriteJsonStringToJsonFile(fileName.c_str(), jf);
	}

	void AntennaMIMOPathFileOutput_SIMO(
		const std::string& outPutDirectoryPathName,
		const AntennaSIMOPathStd::AntennaSIMOPath& antennaSIMOPath) {

		for (int loop_index = 0; loop_index < antennaSIMOPath.paths.size(); ++loop_index) {
			AntennaMIMOPathFileOutput_SISO(
				antennaSIMOPath.transmittingAntennaId,
				outPutDirectoryPathName,
				antennaSIMOPath.paths[loop_index]);
		}
	}

	void AntennaMIMOPathFileOutput(
		bool switchOfPathInfo,
		const std::string& outPutDirectoryPathName,
		const AntennaMIMOPathStd::AntennaMIMOPath& antennaMIMOPath) {
		if (!switchOfPathInfo) {
			return;
		}
		std::string newDirectoryPathName = outPutDirectoryPathName + "\\GeometryPathInformation";
		FileOperateStd::CreateNewDirectorys(newDirectoryPathName);

		for (int loop_index = 0; loop_index < antennaMIMOPath.paths.size(); ++loop_index) {
			AntennaMIMOPathFileOutput_SIMO(
				newDirectoryPathName,
				antennaMIMOPath.paths[loop_index]);
		}

	}

}