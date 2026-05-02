
#include"XdQRtSbr3D.h"
#include"HdQAntennaMIMOPathFileOutput.h"
#include"HdQAntennaMIMOPathElectricFieldFileOutput.h"
#include"HdQCalRunTime.h"
#include"HdQCalAntennaPathElectricField.Interface.h"
#include"QzQGlobalConstant.h"
#include"QzQGeometry3DOperate.Point3D.h"
#include"QzQGeometry3DIntersect.h"
#include"LxQMultiPathNodeInfoOperate.Ptr.h"
#include"LxQProjectDependencies.h"
#include"DxQRayScenarioIntersect.h"
#include"DxQRtSbr3DForRay3DParameterConfigJsonFile.h"
#include"DxQRtProgramReadsDataAndPreprocessesData.h"
#include"DxQRayTracingGeometricPathNodeSIMO.h"
#include"DxQSbrRayGeneratedByTransmittingAntenna.h"
#include"DxQSbrRayGeneratedByTransmission.h"
#include"DxQSbrRayGeneratedByReflection.h"
#include"DxQThreadPool.h"
#include"DxQThreadPool.h"
#include"../0.DxQCalculateWaveImpactResponseDBmModule/CalculateWaveImpactResponseDBm.Output.h"
#include"DxQRtProgramReadsDataAndPreprocessesData.h"

#include"QzQFileBase.h"
#include"DxQStringToStructure.h"
#include"LxQMultiLinearPolarization3DObjectDatabaseJson.h"
#include"LxQMultiLinearPolarization3DObjectDatabaseJsonOperate.h"

#include"DxQTransmittingAntennaDatabaseJsonFile.h"
#include"DxQTransmittingAntennaDatabaseJsonFileJsonFile.h"

#include"HdQAntennaPatternJsonObject.h"
#include"HdQAntennaPatternJsonObjectJsonFile.h"
#include"QzQDirectoryOperate.h"

#include<string.h>


namespace SY_2025_06 {
	void RT_Init_Sbr3DFindPathConfig(const RtSbr3DForRay3DParameterConfigStd::RtSbr3DForRay3DParameterConfig& rtSbr3DForRay3DParameterConfig,
		Sbr3DFindPathConfig& config) {
		config.diffuseScatteringAr = rtSbr3DForRay3DParameterConfig.rtSbr3DForRay3DPrivateParameterConfig.diffuseScatteringParameter.diffuseScatteringAr;
		config.diffuseScatteringCoefficient = rtSbr3DForRay3DParameterConfig.rtSbr3DForRay3DPrivateParameterConfig.diffuseScatteringParameter.diffuseScatteringCoefficient;
		config.diffuseScatteringRayleighRange = rtSbr3DForRay3DParameterConfig.rtSbr3DForRay3DPrivateParameterConfig.diffuseScatteringParameter.diffuseScatteringRayleighRange;
		config.ejectionsMaxTotalNumber = rtSbr3DForRay3DParameterConfig.rayEjectionParameterConfig.ejectionsMaxTotalNumber;
		config.ejectionsOfDiffractionMaxNumber = rtSbr3DForRay3DParameterConfig.rayEjectionParameterConfig.ejectionsOfDiffractionMaxNumber;
		config.ejectionsOfDiffuseScatteringMaxNumber = rtSbr3DForRay3DParameterConfig.rayEjectionParameterConfig.ejectionsOfDiffuseScatteringMaxNumber;
		config.ejectionsOfReflectionMaxNumber = rtSbr3DForRay3DParameterConfig.rayEjectionParameterConfig.ejectionsOfReflectionMaxNumber;
		config.ejectionsOfTransmissionMaxNumber = rtSbr3DForRay3DParameterConfig.rayEjectionParameterConfig.ejectionsOfTransmissionMaxNumber;

		config.gapDiffractionRad = rtSbr3DForRay3DParameterConfig.rtSbr3DForRay3DPrivateParameterConfig.gapDiffractionRad;
		config.gapDiffuseScatteringAzimuth = rtSbr3DForRay3DParameterConfig.rtSbr3DForRay3DPrivateParameterConfig.gapDiffuseScatteringAzimuth;
		config.gapDiffuseScatteringPitchAngle = rtSbr3DForRay3DParameterConfig.rtSbr3DForRay3DPrivateParameterConfig.gapDiffuseScatteringPitchAngle;
		config.geometricSpaceAccelerateType = rtSbr3DForRay3DParameterConfig.geometricSpaceAccelerateParameterConfig.geometricSpaceAccelerateType;

		config.multithreadConfigSwitchOfMultithread = rtSbr3DForRay3DParameterConfig.multithreadParameterConfig.multithreadConfigSwitchOfMultithread;
		config.powerThreshold = rtSbr3DForRay3DParameterConfig.commonParameterConfig.powerThreshold;
		config.radiusCorner = rtSbr3DForRay3DParameterConfig.rtSbr3DForRay3DPrivateParameterConfig.radiusCorner;
		config.radiusRx = rtSbr3DForRay3DParameterConfig.rtSbr3DForRay3DPrivateParameterConfig.radiusRx;
		config.rayNumber = (double)rtSbr3DForRay3DParameterConfig.rtSbr3DForRay3DPrivateParameterConfig.rayNumber;
		config.rebuildEdge = rtSbr3DForRay3DParameterConfig.commonParameterConfig.rebuildEdge;
		config.switchOfLos = rtSbr3DForRay3DParameterConfig.rayEjectionParameterConfig.switchOfLos;


	}


	bool ReadDataByMaterialDataInputConfig(
		const std::string& inputMaterialTableCsvFileName,
		std::vector<MaterialObjectStd::MaterialObject>& materials) {

		auto fileName = inputMaterialTableCsvFileName;
		std::vector<std::vector<std::string>> allFields;
		if (!FileOperateStd::LoadCsvFile(fileName, allFields)) {
			return false;
		}



		for (int i = 1; i < allFields.size(); i++) {
			if (allFields[i].size() < 8) {
				ProjectDependenciesStd::DisplayPromptOrReason("文件格式不正确.", true, __FILE__, __LINE__);
				return false;
			}
			//int id = StringToStructureStd::StringToInt(allFields[i][0]);
			std::string name = allFields[i][1];
			int typeNumber = StringToStructureStd::StringToInt(allFields[i][2]);
			long long frequency = (long long)StringToStructureStd::StringToDouble(allFields[i][3]);
			double relativePermittivity = StringToStructureStd::StringToDouble(allFields[i][4]);
			double conductivity = StringToStructureStd::StringToDouble(allFields[i][5]);
			//double relativePermeability = StringToStructureStd::StringToDouble(allFields[i][6]);
			//double magnetoconductivity = StringToStructureStd::StringToDouble(allFields[i][7]);

			MaterialObjectStd::MaterialObject materialObject;
			materialObject.typeNumber = typeNumber;
			materialObject.frequency = frequency;
			materialObject.relativePermittivity = relativePermittivity;
			materialObject.conductivity = conductivity;


			std::string str = name;
			std::copy(str.begin(), str.end(), materialObject.materialName);
			materialObject.materialName[str.length()] = '\0';

			materials.emplace_back(materialObject);
		}


		return true;
	}


	bool RT_Init_MaterialSet(const std::string& inputMaterialTableCsvFileName, MaterialSet& materialSet) {
		std::vector<MaterialObjectStd::MaterialObject> materials;
		if (!ReadDataByMaterialDataInputConfig(inputMaterialTableCsvFileName, materials)) {
			return false;
		}

		materialSet.size = (int)materials.size();

		materialSet.materials = (Material*)malloc(materialSet.size * sizeof(Material));
		if (materialSet.materials == NULL) {
			return false;
		}
		for (int i = 0; i < materialSet.size; i++) {
			materialSet.materials[i].materialTypeNumber = materials[i].typeNumber;
			materialSet.materials[i].frequency = materials[i].frequency;
			materialSet.materials[i].relativePermittivity = materials[i].relativePermittivity;
			materialSet.materials[i].conductivity = materials[i].conductivity;
		}
		return true;
	}


	bool RT_Init_Scenario3D(
		const std::string& inputScenarioPoint3DCsvFileName,
		const std::string& inputScenarioTriangle3DCsvFileName,
		const std::string& inputScenarioCorner3DCsvFileName,
		Scenario3D& scenario) {


			{
				std::vector<double> x_data;
				std::vector<double> y_data;
				std::vector<double> z_data;
				if (!RtProgramReadsDataAndPreprocessesDataStd::ReadScenarioPointByCsvModel(
					inputScenarioPoint3DCsvFileName,
					x_data,
					y_data,
					z_data)) {
					return false;
				}

				scenario.pointsCount = (int)x_data.size();
				if (scenario.pointsCount > 0) {
					scenario.scenario_point3d_set = (Point3D*)malloc(scenario.pointsCount * sizeof(Point3D));
					if (scenario.scenario_point3d_set == NULL) {
						return false;
					}
					for (int i = 0; i < scenario.pointsCount; ++i) {

						scenario.scenario_point3d_set[i].x = x_data[i];
						scenario.scenario_point3d_set[i].y = y_data[i];
						scenario.scenario_point3d_set[i].z = z_data[i];

					}
				}



			}


			{
				std::vector<int> structScenarioArrayScenarioTriangle3DIndexUpTypeNumber_data;
				std::vector<int> structScenarioArrayScenarioTriangle3DIndexDownTypeNumber_data;
				std::vector<int> structScenarioArrayScenarioTriangle3DIndexTriangleP1Index_data;
				std::vector<int> structScenarioArrayScenarioTriangle3DIndexTriangleP2Index_data;
				std::vector<int> structScenarioArrayScenarioTriangle3DIndexTriangleP3Index_data;
				std::vector<double> structScenarioArrayScenarioTriangle3DIndexRoughness_data;
				std::vector<double> structScenarioArrayScenarioTriangle3DIndexNX_data;
				std::vector<double> structScenarioArrayScenarioTriangle3DIndexNY_data;
				std::vector<double> structScenarioArrayScenarioTriangle3DIndexNZ_data;
				if (!RtProgramReadsDataAndPreprocessesDataStd::ReadScenarioTriangle3DIndexByCsvModel(
					inputScenarioTriangle3DCsvFileName,
					structScenarioArrayScenarioTriangle3DIndexUpTypeNumber_data,
					structScenarioArrayScenarioTriangle3DIndexDownTypeNumber_data,
					structScenarioArrayScenarioTriangle3DIndexTriangleP1Index_data,
					structScenarioArrayScenarioTriangle3DIndexTriangleP2Index_data,
					structScenarioArrayScenarioTriangle3DIndexTriangleP3Index_data,
					structScenarioArrayScenarioTriangle3DIndexRoughness_data,
					structScenarioArrayScenarioTriangle3DIndexNX_data,
					structScenarioArrayScenarioTriangle3DIndexNY_data,
					structScenarioArrayScenarioTriangle3DIndexNZ_data)) {
					return false;
				}

				scenario.trianglesCount = (int)structScenarioArrayScenarioTriangle3DIndexUpTypeNumber_data.size();
				if (scenario.trianglesCount > 0) {
					scenario.scenario_triangle3d_set = (Triangle3D*)malloc(scenario.trianglesCount * sizeof(Triangle3D));
					if (scenario.scenario_triangle3d_set == NULL) {
						return false;
					}
					for (int i = 0; i < scenario.trianglesCount; ++i) {

						scenario.scenario_triangle3d_set[i].triangleP1Index = structScenarioArrayScenarioTriangle3DIndexTriangleP1Index_data[i];
						scenario.scenario_triangle3d_set[i].triangleP2Index = structScenarioArrayScenarioTriangle3DIndexTriangleP2Index_data[i];
						scenario.scenario_triangle3d_set[i].triangleP3Index = structScenarioArrayScenarioTriangle3DIndexTriangleP3Index_data[i];
						scenario.scenario_triangle3d_set[i].topMaterialTypeNumber = structScenarioArrayScenarioTriangle3DIndexUpTypeNumber_data[i];
						scenario.scenario_triangle3d_set[i].bottomMaterialTypeNumber = structScenarioArrayScenarioTriangle3DIndexDownTypeNumber_data[i];
						scenario.scenario_triangle3d_set[i].roughness = (float)structScenarioArrayScenarioTriangle3DIndexRoughness_data[i];
						scenario.scenario_triangle3d_set[i].n.x = structScenarioArrayScenarioTriangle3DIndexNX_data[i];
						scenario.scenario_triangle3d_set[i].n.y = structScenarioArrayScenarioTriangle3DIndexNY_data[i];
						scenario.scenario_triangle3d_set[i].n.z = structScenarioArrayScenarioTriangle3DIndexNZ_data[i];
					}
				}


			}


			{

				std::vector<int> structScenarioArrayScenarioCorner3DIndexP1Index_data;
				std::vector<int> structScenarioArrayScenarioCorner3DIndexP2Index_data;
				std::vector<int> structScenarioArrayScenarioCorner3DIndexP3Face0Index_data;
				std::vector<int> structScenarioArrayScenarioCorner3DIndexP3FaceNIndex_data;
				std::vector<int> structScenarioArrayScenarioCorner3DIndexFace0Index_data;
				std::vector<int> structScenarioArrayScenarioCorner3DIndexFaceNIndex_data;
				if (!RtProgramReadsDataAndPreprocessesDataStd::ReadScenarioCorner3DIndexByCsvModel(
					inputScenarioCorner3DCsvFileName,
					structScenarioArrayScenarioCorner3DIndexP1Index_data,
					structScenarioArrayScenarioCorner3DIndexP2Index_data,
					structScenarioArrayScenarioCorner3DIndexP3Face0Index_data,
					structScenarioArrayScenarioCorner3DIndexP3FaceNIndex_data,
					structScenarioArrayScenarioCorner3DIndexFace0Index_data,
					structScenarioArrayScenarioCorner3DIndexFaceNIndex_data)) {
					return false;
				}

				scenario.cornersCount = (int)structScenarioArrayScenarioCorner3DIndexP1Index_data.size();
				if (scenario.cornersCount > 0) {
					scenario.scenario_corner3d_set = (Corner3D*)malloc(scenario.cornersCount * sizeof(Corner3D));
					if (scenario.scenario_corner3d_set == NULL) {
						return false;
					}
					for (int i = 0; i < scenario.cornersCount; ++i) {

						scenario.scenario_corner3d_set[i].p1Index = structScenarioArrayScenarioCorner3DIndexP1Index_data[i];
						scenario.scenario_corner3d_set[i].p2Index = structScenarioArrayScenarioCorner3DIndexP2Index_data[i];
						scenario.scenario_corner3d_set[i].p3Face1Index = structScenarioArrayScenarioCorner3DIndexP3Face0Index_data[i];
						scenario.scenario_corner3d_set[i].p3Face2Index = structScenarioArrayScenarioCorner3DIndexP3FaceNIndex_data[i];
						scenario.scenario_corner3d_set[i].face1Index = structScenarioArrayScenarioCorner3DIndexFace0Index_data[i];
						scenario.scenario_corner3d_set[i].face2Index = structScenarioArrayScenarioCorner3DIndexFaceNIndex_data[i];

					}
				}

			}


		return true;
	}


	/// <summary>
	/// 初始化极化信息
	/// </summary>
	/// <param name="multiLinearPolarization3DDatabaseFileName"></param>
	void InitMultiLinearPolarization3DDatabaseByMultiLinearPolarization3DObjectDatabaseJsonFile(
		const std::string& multiLinearPolarization3DDatabaseFileName,
		int polarization3DModelId,
		AntennaPolarization3DModel& antennaPolarization3DModel) {

		if (!FileOperateStd::ExistFile(multiLinearPolarization3DDatabaseFileName.c_str())) {
			return;
		}

		MultiLinearPolarization3DObjectDatabaseJsonStd::MultiLinearPolarization3DObjectDatabaseJson multiLinearPolarization3DObjectDatabaseJson;
		MultiLinearPolarization3DObjectDatabaseJsonOperateStd::ReadMultiLinearPolarization3DObjectDatabaseJsonByJsonFile(
			multiLinearPolarization3DDatabaseFileName.c_str(), multiLinearPolarization3DObjectDatabaseJson);
		
		for (int i = 0;i< multiLinearPolarization3DObjectDatabaseJson.database.size();++i) {
			if (polarization3DModelId == multiLinearPolarization3DObjectDatabaseJson.database[i].polarization3DModelId) {
				antennaPolarization3DModel.polarization3DModelId = multiLinearPolarization3DObjectDatabaseJson.database[i].polarization3DModelId;
				antennaPolarization3DModel.size = (int)multiLinearPolarization3DObjectDatabaseJson.database[i].multiLinearPolarization3D.size();
				antennaPolarization3DModel.multiLinearPolarization3D = (OneAntennaLinearPolarization3D*)malloc(antennaPolarization3DModel.size * sizeof(OneAntennaLinearPolarization3D));
				if (antennaPolarization3DModel.multiLinearPolarization3D == NULL) {
					std::cout << "malloc OneAntennaLinearPolarization3D failed!" << std::endl;
					return;
				}
				for (int j = 0; j < antennaPolarization3DModel.size; ++j) {
					antennaPolarization3DModel.multiLinearPolarization3D[j].weight = multiLinearPolarization3DObjectDatabaseJson.database[i].multiLinearPolarization3D[j].weight;
					antennaPolarization3DModel.multiLinearPolarization3D[j].linearPolarization3DObject.phi0 = multiLinearPolarization3DObjectDatabaseJson.database[i].multiLinearPolarization3D[j].linearPolarization3DObject.phi0;
					antennaPolarization3DModel.multiLinearPolarization3D[j].linearPolarization3DObject.vec.x = multiLinearPolarization3DObjectDatabaseJson.database[i].multiLinearPolarization3D[j].linearPolarization3DObject.vec.x;
					antennaPolarization3DModel.multiLinearPolarization3D[j].linearPolarization3DObject.vec.y = multiLinearPolarization3DObjectDatabaseJson.database[i].multiLinearPolarization3D[j].linearPolarization3DObject.vec.y;
					antennaPolarization3DModel.multiLinearPolarization3D[j].linearPolarization3DObject.vec.z = multiLinearPolarization3DObjectDatabaseJson.database[i].multiLinearPolarization3D[j].linearPolarization3DObject.vec.z;
				}
				return;
			}
		}

	}


	void InitAntennaPatternDatabaseByAntennaPatternDatabaseJsonFile2(
		const std::string& inputAntennaPatternDatabaseJsonFileName,
		int radiationPatternId,
		AntennaRadiationPattern3DModel& antennaRadiationPattern3DModel) {
		if (!FileOperateStd::ExistFile(inputAntennaPatternDatabaseJsonFileName.c_str())) {
			return;
		}

		antennaRadiationPattern3DModel.radiationPatternId;
		AntennaPatternJsonObjectStd::AntennaPatternJsonObject antennaPatternJsonObject;
		AntennaPatternJsonObjectStd::ReadAntennaPatternJsonObjectByJsonFile(inputAntennaPatternDatabaseJsonFileName.c_str(), antennaPatternJsonObject);

		for (int i = 0; i < antennaPatternJsonObject.antennaPatternObjects.size(); ++i) {
			if (radiationPatternId == antennaPatternJsonObject.antennaPatternObjects[i].radiationPatternId) {
				antennaRadiationPattern3DModel.radiationPatternId = antennaPatternJsonObject.antennaPatternObjects[i].radiationPatternId;
				antennaRadiationPattern3DModel.rows = 360;
				antennaRadiationPattern3DModel.columns = 181;
				antennaRadiationPattern3DModel.radiationPattern = (double*)malloc(antennaRadiationPattern3DModel.rows * antennaRadiationPattern3DModel.columns * sizeof(double));
				if (antennaRadiationPattern3DModel.radiationPattern == NULL) {
					return;
				}

				for (int j = 0; j < antennaRadiationPattern3DModel.rows; ++j) {
					for (int k = 0; k < antennaRadiationPattern3DModel.columns; ++k) {
						antennaRadiationPattern3DModel.radiationPattern[j * antennaRadiationPattern3DModel.columns + k] = antennaPatternJsonObject.antennaPatternObjects[i].radiationPattern[j][k];
					}
				}

				return;
			}
		}

	}

	/// <summary>
   /// 天线模块的读取
   /// </summary>
   /// <returns></returns>
	bool RT_Init_TransmitterAntenna(
		const std::string& fileName,
		AntennaPolarization3DModel& antennaPolarization3DModel,
		AntennaRadiationPattern3DModel& antennaRadiationPattern3DModel,
		TransmitterAntenna& transmitterAntenna) {

		TransmittingAntennaDatabaseJsonFileStd::TransmittingAntennaDatabaseJsonFile transmittingAntennaDatabaseJsonFile;
		//json反序列化，不存在则生成一个模板

		
		if (!TransmittingAntennaDatabaseJsonFileStd::ReadTransmittingAntennaDatabaseJsonFileByJsonFile(fileName.c_str(), transmittingAntennaDatabaseJsonFile)) {
			transmittingAntennaDatabaseJsonFile.transmittingAntennaJsonFiles.emplace_back(TransmittingAntennaJsonFileStd::TransmittingAntennaJsonFile());
			TransmittingAntennaDatabaseJsonFileStd::WriteTransmittingAntennaDatabaseJsonFileToJsonFile(fileName.c_str(), transmittingAntennaDatabaseJsonFile);
			std::cout << "(" << fileName << ")文件不存在,缺少天线配置文件!当前已经生成模板!" << std::endl;
			return false;
		}
		if (transmittingAntennaDatabaseJsonFile.transmittingAntennaJsonFiles.size() < 1) {
			return false;
		}

		//当前有效的只有第一个天线数据
		auto transmittingAntennaJsonFile = transmittingAntennaDatabaseJsonFile.transmittingAntennaJsonFiles[0];
		int polarization3DModelId = transmittingAntennaJsonFile.polarization3DModelId;
		{
			//极化
			InitMultiLinearPolarization3DDatabaseByMultiLinearPolarization3DObjectDatabaseJsonFile(
				transmittingAntennaDatabaseJsonFile.inputPolarization3DDatabaseJsonFileName,
				polarization3DModelId,
				antennaPolarization3DModel);
		}
		int radiationPatternId = transmittingAntennaJsonFile.radiationPatternId;
		{
			//天线方向图
			InitAntennaPatternDatabaseByAntennaPatternDatabaseJsonFile2(
				transmittingAntennaDatabaseJsonFile.inputAntennaPatternDatabaseJsonFileName, 
				radiationPatternId,
				antennaRadiationPattern3DModel);
		}

		std::vector<ReceiverAntennaStd::ReceiverAntenna> receiverAntennas;
		std::vector<int> transmittingAntennaIdSet;
		transmittingAntennaIdSet.emplace_back(transmittingAntennaJsonFile.transmittingAntennaId);
		if (!RtProgramReadsDataAndPreprocessesDataStd::ReadReceiverAntennaByCsvModel(
			transmittingAntennaJsonFile.inputReceivingAntennaCsvFileName, transmittingAntennaIdSet, receiverAntennas)) {
			return false;
		}

		transmitterAntenna.frequencyBandwidth.size = (int)transmittingAntennaJsonFile.frequencys.size();
		if (transmitterAntenna.frequencyBandwidth.size < 1) {
			return false;
		}
		transmitterAntenna.frequencyBandwidth.frequencys = (long long*)malloc(transmitterAntenna.frequencyBandwidth.size * sizeof(long long));
		if (transmitterAntenna.frequencyBandwidth.frequencys == NULL) {
			return false;
		}
		for (int i = 0; i < transmitterAntenna.frequencyBandwidth.size; ++i) {
			transmitterAntenna.frequencyBandwidth.frequencys[i] = transmittingAntennaJsonFile.frequencys[i];
		}
		transmitterAntenna.location.x = transmittingAntennaJsonFile.center_location_x;
		transmitterAntenna.location.y = transmittingAntennaJsonFile.center_location_y;
		transmitterAntenna.location.z = transmittingAntennaJsonFile.center_location_z;
		transmitterAntenna.materialTypeNumber = transmittingAntennaJsonFile.materialTypeNumber;
		transmitterAntenna.polarization3DModelId = polarization3DModelId;
		transmitterAntenna.radiationPatternId = radiationPatternId;
		transmitterAntenna.receiversCount = (int)receiverAntennas.size();
		if (transmitterAntenna.receiversCount < 1) {
			return false;
		}
		transmitterAntenna.receivers = (ReceiverAntenna*)malloc(transmitterAntenna.receiversCount * sizeof(ReceiverAntenna));
		if (transmitterAntenna.receivers == NULL) {
			return false;
		}
		for (int i = 0; i < transmitterAntenna.receiversCount; ++i) {
			transmitterAntenna.receivers[i].location.x = receiverAntennas[i].antennaProperty.location.x;
			transmitterAntenna.receivers[i].location.y = receiverAntennas[i].antennaProperty.location.y;
			transmitterAntenna.receivers[i].location.z = receiverAntennas[i].antennaProperty.location.z;
			transmitterAntenna.receivers[i].receiverAntennaId = receiverAntennas[i].receiverAntennaId;
		}
		transmitterAntenna.transmitPower = transmittingAntennaJsonFile.emissionPower;
		transmitterAntenna.transmitterAntennaId = transmittingAntennaJsonFile.transmittingAntennaId;


		return true;
	}

}




namespace SIMOGlobal_2025_06Std{


	RtSbr3DForRay3DPrivateParameterConfigStd::RtSbr3DForRay3DPrivateParameterConfig globalRtSbr3DForRay3DPrivateParameterConfig;
	RayEjectionParameterConfigStd::RayEjectionParameterConfig globalRayEjectionParameterConfig;

	void SetRtSbr3DForRay3DPrivateParameterConfig(
		const RtSbr3DForRay3DPrivateParameterConfigStd::RtSbr3DForRay3DPrivateParameterConfig& rtSbr3DForRay3DPrivateParameterConfig,
		const RayEjectionParameterConfigStd::RayEjectionParameterConfig& rayEjectionParameterConfig) {
		globalRtSbr3DForRay3DPrivateParameterConfig = rtSbr3DForRay3DPrivateParameterConfig;
		globalRayEjectionParameterConfig = rayEjectionParameterConfig;
	}

	//每次进行SIMO计算时都应该进行的赋值运算

	TriangleAccelerateStructDatabaseStd::TriangleAccelerateStructDatabase* globalTriangleAccelerateStructDatabase_ptr = NULL;
	TransmittingAntennaStd::TransmittingAntenna* globalTransmittingAntenna_ptr = NULL;
	int* globalReceiverAntennaId = NULL;
	Point3DStd::Point3D* globalReceiverAntennaLocation = NULL;

	size_t globalReceiverAntennas_size = 0;

	Point3DStd::Point3D* globalRayVecs = NULL;
	size_t globalRayVecs_size = 0;
	RayTracingGeometricPathNodeSIMOStd::RayTracingGeometricPathNodeSIMO* globalRayTracingGeometricPathNodeSIMO = NULL;
	ScenarioObjectStd::ScenarioObject* globalScenarioObject_ptr = NULL;



	void FreeSIMOGlobal_ReceiverAntenna() {
		if (globalReceiverAntennas_size == 0) {
			globalReceiverAntennas_size = 0;
			globalReceiverAntennaId = NULL;
			globalReceiverAntennaLocation = NULL;
			return;
		}
		delete globalReceiverAntennaId;
		delete globalReceiverAntennaLocation;
		globalReceiverAntennas_size = 0;
	}


	void FreeSIMOGlobal_RayVec() {
		if (globalRayVecs_size == 0) {
			globalRayVecs_size = 0;
			globalRayVecs = NULL;
			return;
		}
		delete globalRayVecs;
		globalRayVecs_size = 0;
	}

	void FreeSIMOGlobal() {
		FreeSIMOGlobal_ReceiverAntenna();
		FreeSIMOGlobal_RayVec();
	}

	bool SetSIMOGlobal_RayVec(const std::vector<Point3DStd::Point3D>& rayVecs) {

		FreeSIMOGlobal_RayVec();

		globalRayVecs_size = rayVecs.size();

		if (globalRayVecs_size < 1) {
			return false;
		}
		globalRayVecs = (Point3DStd::Point3D*)malloc(globalRayVecs_size *sizeof(Point3DStd::Point3D));
		if (globalRayVecs == NULL) {
			return false;
		}

		for (size_t i = 0; i < globalRayVecs_size; ++i) {
			globalRayVecs[i] = rayVecs[i];
		}

		return true;
	}


	bool SetSIMOGlobal_ReceiverAntenna(const std::vector<ReceiverAntennaStd::ReceiverAntenna>& receiverAntennas) {

		FreeSIMOGlobal_ReceiverAntenna();

		globalReceiverAntennas_size = receiverAntennas.size();

		if (globalReceiverAntennas_size < 1) {
			return false;
		}
		globalReceiverAntennaLocation = (Point3DStd::Point3D*)malloc(globalReceiverAntennas_size * sizeof(Point3DStd::Point3D));
		if (globalReceiverAntennaLocation == NULL) {
			return false;
		}
		globalReceiverAntennaId = (int*)malloc(globalReceiverAntennas_size * sizeof(int));
		if (globalReceiverAntennaId == NULL) {
			return false;
		}

		Point2DStd::Point2D min2D(GlobalConstantStd::BoundingBoxLength, GlobalConstantStd::BoundingBoxLength);
		Point2DStd::Point2D max2D(-GlobalConstantStd::BoundingBoxLength, -GlobalConstantStd::BoundingBoxLength);
		std::vector<Point3DStd::Point3D> rx_location;
		for (size_t i = 0; i < globalReceiverAntennas_size; ++i) {
			globalReceiverAntennaId[i] = receiverAntennas[i].receiverAntennaId;
			globalReceiverAntennaLocation[i] = receiverAntennas[i].antennaProperty.location;
			rx_location.emplace_back(globalReceiverAntennaLocation[i]);

			if (min2D.x > globalReceiverAntennaLocation[i].x) {
				min2D.x = globalReceiverAntennaLocation[i].x;
			}
			if (min2D.y > globalReceiverAntennaLocation[i].y) {
				min2D.y = globalReceiverAntennaLocation[i].y;
			}

			if (max2D.x < globalReceiverAntennaLocation[i].x) {
				max2D.x = globalReceiverAntennaLocation[i].x;
			}
			if (max2D.y < globalReceiverAntennaLocation[i].y) {
				max2D.y = globalReceiverAntennaLocation[i].y;
			}
		}


		double cube_size = 0.05;
		{
			double max_len = max2D.x - min2D.x;
			double max_len2 = max2D.y - min2D.y;
			if (max_len < max_len2) {
				max_len = max_len2;
			}
			cube_size = max_len / 99.0;
		}

		if (cube_size < globalRtSbr3DForRay3DPrivateParameterConfig.radiusRx) {
			cube_size = globalRtSbr3DForRay3DPrivateParameterConfig.radiusRx * 1.003;
		}

		return true;
	}

	bool SetSIMOGlobal(
		const std::vector<Point3DStd::Point3D>& rayVecs,
		ScenarioDataInformationStd::ScenarioDataInformation& scenarioDataInformation,
		AntennaSIMOStd::AntennaSIMO& antennaSIMO,
		RayTracingGeometricPathNodeSIMOStd::RayTracingGeometricPathNodeSIMO& rayTracingGeometricPathNodeSIMO) {

		globalTriangleAccelerateStructDatabase_ptr = &scenarioDataInformation.triangleAccelerateStructDatabase;
		globalScenarioObject_ptr = &scenarioDataInformation.scenarioObject;

		globalTransmittingAntenna_ptr = &antennaSIMO.transmittingAntenna;
		globalRayTracingGeometricPathNodeSIMO = &rayTracingGeometricPathNodeSIMO;

		if (!SetSIMOGlobal_ReceiverAntenna(antennaSIMO.receiverAntennas)) {
			return false;
		}

		if (!SetSIMOGlobal_RayVec(rayVecs)) {
			return false;
		}

		return true;

	}

}


namespace InteractionBetweenRayAndRx_2025_06Std {


	void IsArriveReceiverAntennasByConeMethod_ShadowPoint_1(
		double rayScenarioIntersectResultDistance,
		const Ray3DStd::Ray3D& ray,
		std::vector<int>& indexRxList) {
		//方法1.直接暴力求解，每次和所有的接收球依次计算，这样如果接收天线数量特别大，例如面型预测时，接受数量上10万，甚至100万，这种方法就比较慢
		for (int i = 0; i < SIMOGlobal_2025_06Std::globalReceiverAntennas_size; i++) {

			//if (Geometry3DIntersectStd::ToDetermineWhetherTheReceiverCollidesWithTheRayByUsingTheCylindricalMethod(
			//	rayScenarioIntersectResultDistance,
			//	SIMOGlobalStd::globalRtSbr3DForRay3DPrivateParameterConfig.radiusRx,
			//	ray.o,
			//	ray.vec,
			//	SIMOGlobalStd::globalReceiverAntennaLocation[i])) {
			//	indexRxList.emplace_back(i);
			//}
		}
	}

	void IsArriveReceiverAntennasByConeMethod_ShadowPoint_2(
		double rayScenarioIntersectResultDistance,
		const Ray3DStd::Ray3D& ray,
		const std::vector<int>& maybe_indexRxList,
		std::vector<int>& indexRxList) {
		//方法1.直接暴力求解，每次和所有的接收球依次计算，这样如果接收天线数量特别大，例如面型预测时，接受数量上10万，甚至100万，这种方法就比较慢
		for (int loop = 0; loop < maybe_indexRxList.size(); loop++) {
			int i = maybe_indexRxList[loop];
			//if (Geometry3DIntersectStd::ToDetermineWhetherTheReceiverCollidesWithTheRayByUsingTheCylindricalMethod(
			//	rayScenarioIntersectResultDistance,
			//	SIMOGlobalStd::globalRtSbr3DForRay3DPrivateParameterConfig.radiusRx,
			//	ray.o,
			//	ray.vec,
			//	SIMOGlobalStd::globalReceiverAntennaLocation[i])) {
			//	indexRxList.emplace_back(i);
			//}
		}
	}


	/// <summary>
	/// 所有天线和当前这跟射线的交互结果
	/// </summary>
	/// <param name="rayScenarioIntersectResultDistance"></param>
	/// <param name="ray"></param>
	/// <param name="indexRxList"></param>
	void IsArriveReceiverAntennasByConeMethod_ShadowPoint(
		double rayScenarioIntersectResultDistance,
		const Ray3DStd::Ray3D& ray, 
		std::vector<int>& indexRxList) {

		IsArriveReceiverAntennasByConeMethod_ShadowPoint_1(rayScenarioIntersectResultDistance, ray, indexRxList);

	}

	void InteractionBetweenRayAndRx_base(
		double rayScenarioIntersectResultDistance,
		const Ray3DStd::Ray3D& ray,
		const std::vector<RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode>& root) {

		if (!SIMOGlobal_2025_06Std::globalRayEjectionParameterConfig.switchOfLos) {
			if (root.size() < 2) {
				return;
			}
		}

		//碰撞的接收天线索引
		std::vector<int> indexRxList;
		//std::vector<Point3DStd::Point3D> indexRxList_location;
		//3.计算射线是否被接收天线接收
		IsArriveReceiverAntennasByConeMethod_ShadowPoint(rayScenarioIntersectResultDistance, ray, indexRxList);

		for (int i = 0; i < indexRxList.size(); i++) {
			int rx_loop_index = indexRxList[i];
			Point3DStd::Point3D receiverAntenna_location = SIMOGlobal_2025_06Std::globalReceiverAntennaLocation[rx_loop_index];
			int receiverAntenna_id = SIMOGlobal_2025_06Std::globalReceiverAntennaId[rx_loop_index];

			RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode inforx(
				PropagationTypeStd::PropagationType::ReceiverAntenna, receiverAntenna_id, receiverAntenna_location);// (rx);

			//auto pre_location = root[(int)root.size() - 1].location;

			//double distance = Geometry3DOperateStd::GetDistancePoint3DPoint3D(receiverAntenna_location, pre_location);
			//Point3DStd::Point3D rx_ray_vec_old = Geometry3DOperateStd::SubPoint3DPoint3D(pre_location, receiverAntenna_location);
			//Point3DStd::Point3D rx_ray_vec;
			//if (!Geometry3DOperateStd::Normalization_Point3D_safe(rx_ray_vec_old, rx_ray_vec)) {
			//	continue;
			//}
			//
			//Ray3DStd::Ray3D rx_ray(receiverAntenna_location, rx_ray_vec);
			//Ray3DTriangle3DIntersectResultStd::Ray3DTriangle3DIntersectResult rx_ray3DTriangle3DIntersectResult = RayScenarioIntersectStd::RayScenarioTriangle3DIntersect(
			//	rx_ray, scenarioDataInformation.triangleAccelerateStructDatabase);
			//if (distance > GlobalConstantStd::Eps + rx_ray3DTriangle3DIntersectResult.distance) {
			//	continue;
			//}

			std::vector<RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode> path((int)root.size() + 1);
			for (size_t j = 0; j < root.size(); j++) {
				path[j] = root[j];
			}
			path[root.size()] = inforx;

			SIMOGlobal_2025_06Std::globalRayTracingGeometricPathNodeSIMO->AddPath(rx_loop_index, path);
		}
	}


	//InteractionBetweenRayAndRxObject
	class InteractionBetweenRayAndRxObject
	{
	public:
		double rayScenarioIntersectResultDistance;
		Ray3DStd::Ray3D ray;
		std::vector<RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode> root;
		InteractionBetweenRayAndRxObject();
		~InteractionBetweenRayAndRxObject();

	private:

	};

	InteractionBetweenRayAndRxObject::InteractionBetweenRayAndRxObject()
	{
		this->rayScenarioIntersectResultDistance = GlobalConstantStd::BoundingBoxLength;
	}

	InteractionBetweenRayAndRxObject::~InteractionBetweenRayAndRxObject()
	{
	}

	//ThreadPoolStd::ThreadPool interactionBetweenRayAndRx_threadPool;

	void InteractionBetweenRayAndRx_Core(InteractionBetweenRayAndRxObject& obj) {
		InteractionBetweenRayAndRx_base(obj.rayScenarioIntersectResultDistance, obj.ray, obj.root);
	}

	std::vector<Ray3DStd::Ray3D> interactionBetweenRayAndRx_ray;
	std::vector<double> interactionBetweenRayAndRx_rayScenarioIntersectResultDistance;
	std::vector<std::vector<RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode>> interactionBetweenRayAndRx_nodes;


	std::mutex interactionBetweenRayAndRx_mtx;
	void InteractionBetweenRayAndRx(
		const Ray3DStd::Ray3D& ray,
		const Ray3DScenario3DIntersectResultStd::Ray3DScenario3DIntersectResult& rayScenarioIntersectResult,
		const std::vector<RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode>& root) {

		//这里认为天线的路径如果没有被三角形遮挡，那么也不会被棱边遮挡
		double rayScenarioIntersectResultDistance = rayScenarioIntersectResult.ray3DTriangle3DIntersectResult.distance;

		{
			////这种思路消耗内存
			//interactionBetweenRayAndRx_mtx.lock();
			//
			//interactionBetweenRayAndRx_ray.emplace_back(ray);
			//interactionBetweenRayAndRx_rayScenarioIntersectResultDistance.emplace_back(rayScenarioIntersectResultDistance);
			//std::vector<RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode> nodes;
			//nodes.insert(nodes.end(), root.begin(), root.end());
			//interactionBetweenRayAndRx_nodes.emplace_back(nodes);
			//
			//interactionBetweenRayAndRx_mtx.unlock();
		}

		{
			//InteractionBetweenRayAndRxObject interactionBetweenRayAndRxObject;
			//interactionBetweenRayAndRxObject.ray = ray;
			//interactionBetweenRayAndRxObject.rayScenarioIntersectResultDistance = rayScenarioIntersectResultDistance;
			//interactionBetweenRayAndRxObject.root.insert(interactionBetweenRayAndRxObject.root.end(), root.begin(), root.end());
			//interactionBetweenRayAndRx_threadPool.submit(InteractionBetweenRayAndRx_Core, interactionBetweenRayAndRxObject);
		}


		//return;
		InteractionBetweenRayAndRx_base(rayScenarioIntersectResultDistance, ray,root);
	}
}

namespace RtSbr3DForRay3DFindPathSingleThread_2025_06Std {

	/// <summary>
	/// 计算入射角
	/// </summary>
	/// <returns></returns>
	double GetThetaI(const Point3DStd::Point3D& v1, const Point3DStd::Point3D& n1) {
		if (Geometry3DOperateStd::DotPoint3DPoint3D(n1, v1) > 0) {
			return Geometry3DOperateStd::GetAnglePoint3DPoint3D(v1, n1);
		}
		else {
			Point3DStd::Point3D v2(-v1.x, -v1.y, -v1.z);
			return Geometry3DOperateStd::GetAnglePoint3DPoint3D(v2, n1);
		}
	}


	void SbrFindGeometricPathSIMO(
		const RayEjectionParameterConfigStd::RayEjectionParameterConfig& rayEjectionParameterConfig,
		const Ray3DStd::Ray3D& ray,
		std::vector<RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode>& root) {
		if (rayEjectionParameterConfig.ejectionsMaxTotalNumber < 1) {
			return;
		}
		bool transmission = true;
		if (rayEjectionParameterConfig.ejectionsOfTransmissionMaxNumber < 1) {
			transmission = false;
		}
		bool reflect = true;
		if (rayEjectionParameterConfig.ejectionsOfReflectionMaxNumber < 1) {
			reflect = false;
		}

		//计算射线的碰撞结果
		Ray3DScenario3DIntersectResultStd::Ray3DScenario3DIntersectResult rayScenarioIntersectResult;

		rayScenarioIntersectResult.ray3DTriangle3DIntersectResult =
			RayScenarioIntersectStd::RayScenarioTriangle3DIntersect(ray, SIMOGlobal_2025_06Std::globalTriangleAccelerateStructDatabase_ptr);

		InteractionBetweenRayAndRx_2025_06Std::InteractionBetweenRayAndRx(ray, rayScenarioIntersectResult, root);

		if (rayScenarioIntersectResult.ray3DTriangle3DIntersectResult.index == -1
			&& rayScenarioIntersectResult.ray3DCorner3DIntersectResult.index == -1) {
			return;
		}

		if (rayScenarioIntersectResult.ray3DTriangle3DIntersectResult.index != -1) {

			if (reflect) {
				
				auto curScenarioTriangleN = SIMOGlobal_2025_06Std::globalTriangleAccelerateStructDatabase_ptr->triangleAccelerateStructs[rayScenarioIntersectResult.ray3DTriangle3DIntersectResult.index].scenarioTriangleN;

				Ray3DStd::Ray3D newRay(Point3DStd::Point3D(0, 0, 0), Point3DStd::Point3D(0, 0, 0));
				double thetai;
				//根据当前的碰撞点计算反射信息
				if (SbrRayGeneratedByReflectionStd::BuildReflectionRay(
					ray.o,
					rayScenarioIntersectResult.ray3DTriangle3DIntersectResult.point,
					curScenarioTriangleN,
					newRay,
					thetai)) {
					RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode infoReflection(
						PropagationTypeStd::PropagationType::Reflection,
						rayScenarioIntersectResult.ray3DTriangle3DIntersectResult.index,
						rayScenarioIntersectResult.ray3DTriangle3DIntersectResult.point);

					root.emplace_back(infoReflection);

					RayEjectionParameterConfigStd::RayEjectionParameterConfig newRayEjectionParameterConfig;

					newRayEjectionParameterConfig.ejectionsMaxTotalNumber = rayEjectionParameterConfig.ejectionsMaxTotalNumber - 1;
					newRayEjectionParameterConfig.ejectionsOfDiffractionMaxNumber = rayEjectionParameterConfig.ejectionsOfDiffractionMaxNumber;
					newRayEjectionParameterConfig.ejectionsOfDiffuseScatteringMaxNumber = rayEjectionParameterConfig.ejectionsOfDiffuseScatteringMaxNumber;
					newRayEjectionParameterConfig.ejectionsOfReflectionMaxNumber = rayEjectionParameterConfig.ejectionsOfReflectionMaxNumber - 1;
					newRayEjectionParameterConfig.ejectionsOfTransmissionMaxNumber = rayEjectionParameterConfig.ejectionsOfTransmissionMaxNumber;
					newRayEjectionParameterConfig.switchOfLos = rayEjectionParameterConfig.switchOfLos;

					SbrFindGeometricPathSIMO(
						newRayEjectionParameterConfig,
						newRay,
						root);
					root.pop_back();
				}
			}
			if (transmission) {

				auto triangleAccelerateStruct = SIMOGlobal_2025_06Std::globalTriangleAccelerateStructDatabase_ptr->triangleAccelerateStructs[rayScenarioIntersectResult.ray3DTriangle3DIntersectResult.index];
				auto curScenarioTriangleN = triangleAccelerateStruct.scenarioTriangleN;

				int faceUpTypeNumber = triangleAccelerateStruct.upTypeNumber;
				int faceDownTypeNumber = triangleAccelerateStruct.downTypeNumber;

				Ray3DStd::Ray3D newRay(Point3DStd::Point3D(0, 0, 0), Point3DStd::Point3D(0, 0, 0));
				double thetai;
				if (SIMOGlobal_2025_06Std::globalRtSbr3DForRay3DPrivateParameterConfig.realWorldRefraction) {
					//根据当前的碰撞点计算透射信息
					if (SbrRayGeneratedByTransmissionStd::BuildTransmissionRay_RayTracingGeometricPathNode(
						faceUpTypeNumber,
						faceDownTypeNumber,
						SIMOGlobal_2025_06Std::globalTransmittingAntenna_ptr->antennaProperty.frequencys[0],
						ray.o,
						rayScenarioIntersectResult.ray3DTriangle3DIntersectResult.point,
						curScenarioTriangleN,
						newRay,
						thetai)) {

						RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode infoTransmission(
							PropagationTypeStd::PropagationType::Transmission,
							rayScenarioIntersectResult.ray3DTriangle3DIntersectResult.index,
							rayScenarioIntersectResult.ray3DTriangle3DIntersectResult.point);
						root.emplace_back(infoTransmission);

						RayEjectionParameterConfigStd::RayEjectionParameterConfig newRayEjectionParameterConfig;

						newRayEjectionParameterConfig.ejectionsMaxTotalNumber = rayEjectionParameterConfig.ejectionsMaxTotalNumber - 1;
						newRayEjectionParameterConfig.ejectionsOfDiffractionMaxNumber = rayEjectionParameterConfig.ejectionsOfDiffractionMaxNumber;
						newRayEjectionParameterConfig.ejectionsOfDiffuseScatteringMaxNumber = rayEjectionParameterConfig.ejectionsOfDiffuseScatteringMaxNumber;
						newRayEjectionParameterConfig.ejectionsOfReflectionMaxNumber = rayEjectionParameterConfig.ejectionsOfReflectionMaxNumber;
						newRayEjectionParameterConfig.ejectionsOfTransmissionMaxNumber = rayEjectionParameterConfig.ejectionsOfTransmissionMaxNumber - 1;
						newRayEjectionParameterConfig.switchOfLos = rayEjectionParameterConfig.switchOfLos;


						SbrFindGeometricPathSIMO(
							newRayEjectionParameterConfig,
							newRay,
							root);
						root.pop_back();
					}
				}
				else {

					newRay.o = rayScenarioIntersectResult.ray3DTriangle3DIntersectResult.point;
					newRay.vec = ray.vec;
					thetai = GetThetaI(ray.vec, curScenarioTriangleN);
					RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode infoTransmission(
						PropagationTypeStd::PropagationType::Transmission,
						rayScenarioIntersectResult.ray3DTriangle3DIntersectResult.index,
						rayScenarioIntersectResult.ray3DTriangle3DIntersectResult.point);
					root.emplace_back(infoTransmission);

					RayEjectionParameterConfigStd::RayEjectionParameterConfig newRayEjectionParameterConfig;

					newRayEjectionParameterConfig.ejectionsMaxTotalNumber = rayEjectionParameterConfig.ejectionsMaxTotalNumber - 1;
					newRayEjectionParameterConfig.ejectionsOfDiffractionMaxNumber = rayEjectionParameterConfig.ejectionsOfDiffractionMaxNumber;
					newRayEjectionParameterConfig.ejectionsOfDiffuseScatteringMaxNumber = rayEjectionParameterConfig.ejectionsOfDiffuseScatteringMaxNumber;
					newRayEjectionParameterConfig.ejectionsOfReflectionMaxNumber = rayEjectionParameterConfig.ejectionsOfReflectionMaxNumber;
					newRayEjectionParameterConfig.ejectionsOfTransmissionMaxNumber = rayEjectionParameterConfig.ejectionsOfTransmissionMaxNumber - 1;
					newRayEjectionParameterConfig.switchOfLos = rayEjectionParameterConfig.switchOfLos;


					SbrFindGeometricPathSIMO(
						newRayEjectionParameterConfig,
						newRay,
						root);
					root.pop_back();
				}

			}
		}


	}




	void RtSbr3DForRay3DFindPathSingleThread_SIMO() {


		size_t index_gap_10 = (SIMOGlobal_2025_06Std::globalRayVecs_size / 50);

		for (size_t loop_vec_index = 0; loop_vec_index < SIMOGlobal_2025_06Std::globalRayVecs_size;++loop_vec_index) {

			Ray3DStd::Ray3D ray(SIMOGlobal_2025_06Std::globalTransmittingAntenna_ptr->antennaProperty.location,
				SIMOGlobal_2025_06Std::globalRayVecs[loop_vec_index]);

			std::vector<RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode> root;
			RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode transmittingAntennaNode(
				PropagationTypeStd::PropagationType::TransmittingAntenna,
				SIMOGlobal_2025_06Std::globalTransmittingAntenna_ptr->transmittingAntennaId,
				SIMOGlobal_2025_06Std::globalTransmittingAntenna_ptr->antennaProperty.location);

			RayEjectionParameterConfigStd::RayEjectionParameterConfig newRayEjectionParameterConfig;
			newRayEjectionParameterConfig.ejectionsMaxTotalNumber = SIMOGlobal_2025_06Std::globalRayEjectionParameterConfig.ejectionsMaxTotalNumber;
			newRayEjectionParameterConfig.ejectionsOfDiffractionMaxNumber = SIMOGlobal_2025_06Std::globalRayEjectionParameterConfig.ejectionsOfDiffractionMaxNumber;
			newRayEjectionParameterConfig.ejectionsOfDiffuseScatteringMaxNumber = SIMOGlobal_2025_06Std::globalRayEjectionParameterConfig.ejectionsOfDiffuseScatteringMaxNumber;
			newRayEjectionParameterConfig.ejectionsOfReflectionMaxNumber = SIMOGlobal_2025_06Std::globalRayEjectionParameterConfig.ejectionsOfReflectionMaxNumber;
			newRayEjectionParameterConfig.ejectionsOfTransmissionMaxNumber = SIMOGlobal_2025_06Std::globalRayEjectionParameterConfig.ejectionsOfTransmissionMaxNumber;
			newRayEjectionParameterConfig.switchOfLos = SIMOGlobal_2025_06Std::globalRayEjectionParameterConfig.switchOfLos;

			root.emplace_back(transmittingAntennaNode);
			SbrFindGeometricPathSIMO(
				newRayEjectionParameterConfig,
				ray,
				root);
			root.pop_back();
			if (loop_vec_index % index_gap_10 == index_gap_10 - 1) {
				{
					std::ostringstream oss;
					oss << loop_vec_index << "/" << SIMOGlobal_2025_06Std::globalRayVecs_size;
					ProjectDependenciesStd::DisplayPromptOrReason(oss.str(), false, __FILE__, __LINE__);
				}
			}
		}

	}

}

namespace RtSbr3DForRay3DFindPathMultiThread_2025_06Std {


	typedef struct DefMultiThread_RtSbr3DForRay3DFindPathMultiThread_SIMO {
		int start;
		int end;

	}MultiThread_RtSbr3DForRay3DFindPathMultiThread_SIMO;

	void RtSbr3DForRay3DFindPathMultiThread_SIMO_Core(MultiThread_RtSbr3DForRay3DFindPathMultiThread_SIMO& obj) {

		auto rayEjectionParameterConfig = SIMOGlobal_2025_06Std::globalRayEjectionParameterConfig;

		for (size_t loop_vec_index = obj.start; loop_vec_index < SIMOGlobal_2025_06Std::globalRayVecs_size && loop_vec_index < obj.end; ++loop_vec_index) {
			Ray3DStd::Ray3D ray(SIMOGlobal_2025_06Std::globalTransmittingAntenna_ptr->antennaProperty.location, SIMOGlobal_2025_06Std::globalRayVecs[loop_vec_index]);
			std::vector<RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode> root;
			RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode transmittingAntennaNode(
				PropagationTypeStd::PropagationType::TransmittingAntenna,
				SIMOGlobal_2025_06Std::globalTransmittingAntenna_ptr->transmittingAntennaId,
				SIMOGlobal_2025_06Std::globalTransmittingAntenna_ptr->antennaProperty.location);

			RayEjectionParameterConfigStd::RayEjectionParameterConfig newRayEjectionParameterConfig;
			newRayEjectionParameterConfig.ejectionsMaxTotalNumber = rayEjectionParameterConfig.ejectionsMaxTotalNumber;
			newRayEjectionParameterConfig.ejectionsOfDiffractionMaxNumber = rayEjectionParameterConfig.ejectionsOfDiffractionMaxNumber;
			newRayEjectionParameterConfig.ejectionsOfDiffuseScatteringMaxNumber = rayEjectionParameterConfig.ejectionsOfDiffuseScatteringMaxNumber;
			newRayEjectionParameterConfig.ejectionsOfReflectionMaxNumber = rayEjectionParameterConfig.ejectionsOfReflectionMaxNumber;
			newRayEjectionParameterConfig.ejectionsOfTransmissionMaxNumber = rayEjectionParameterConfig.ejectionsOfTransmissionMaxNumber;
			newRayEjectionParameterConfig.switchOfLos = rayEjectionParameterConfig.switchOfLos;

			root.emplace_back(transmittingAntennaNode);
			RtSbr3DForRay3DFindPathSingleThread_2025_06Std::SbrFindGeometricPathSIMO(
				newRayEjectionParameterConfig,
				ray,
				root);
			root.pop_back();
		}

	}


    void RtSbr3DForRay3DFindPathMultiThread_SIMO(
		const MultithreadParameterConfigStd::MultithreadParameterConfig& multithreadParameterConfig) {

		int indexGap = multithreadParameterConfig.multithreadConfigThreadOneCpuCalNum;

		std::vector<int> starts;
		for (int i = 0; i < SIMOGlobal_2025_06Std::globalRayVecs_size; i = i + indexGap) {
			//if (i<809 ||i>810) {
			//	continue;
			//}
			starts.emplace_back(i);
		}


		ThreadPoolStd::ThreadPool threadPool;


		for (int i = 0; i < starts.size(); ++i) {
			MultiThread_RtSbr3DForRay3DFindPathMultiThread_SIMO multiThread_RtSbr3DForRay3DFindPathMultiThread_SIMO;
			multiThread_RtSbr3DForRay3DFindPathMultiThread_SIMO.start = starts[i];
			multiThread_RtSbr3DForRay3DFindPathMultiThread_SIMO.end = starts[i] + indexGap;

			threadPool.submit(RtSbr3DForRay3DFindPathMultiThread_SIMO_Core, multiThread_RtSbr3DForRay3DFindPathMultiThread_SIMO);

		}

		threadPool.join();

    }
}

namespace RtSbr3DForRay3DFindPath_2025_06Std {

    /// <summary>
    /// 
    /// </summary>
    /// <param name="deduplicateRadius"></param>
    /// <param name="multithreadParameterConfig"></param>
    /// <param name="rayEjectionParameterConfig"></param>
    /// <param name="rayVecs"></param>
    /// <param name="scenarioDataInformation"></param>
    /// <param name="antennaSIMO"></param>
    /// <param name="antennaSIMOPath"></param>
    void RtSbr3DForRay3DFindPath_SIMO(
		double deduplicateRadius,
		const std::vector<std::vector<bool>>& triangle_triangle_sameside,
		const std::vector<std::vector<bool>>& seg_seg_samepoint,
		const MultithreadParameterConfigStd::MultithreadParameterConfig& multithreadParameterConfig,
        ScenarioDataInformationStd::ScenarioDataInformation& scenarioDataInformation,
        AntennaSIMOStd::AntennaSIMO& antennaSIMO,
        AntennaSIMOPathStd::AntennaSIMOPath& antennaSIMOPath) {


		size_t transmitterRayNumber = SIMOGlobal_2025_06Std::globalRtSbr3DForRay3DPrivateParameterConfig.rayNumber;

		std::vector<Point3DStd::Point3D> rayVecs; {
			{
				std::ostringstream oss;
				oss << "初始化射线束";
				ProjectDependenciesStd::DisplayPromptOrReason(oss.str(), false, __FILE__, __LINE__);
			}
			CalRunTimeStd::CalRunTime calRunTime(true);
			SbrRayGeneratedByTransmittingAntennaStd::InitSBRLaunchRayVec(transmitterRayNumber, rayVecs);
		}

		//所有单核或者多核的射线跟踪计算都在这里进行

		//准备数据

		RayTracingGeometricPathNodeSIMOStd::RayTracingGeometricPathNodeSIMO rayTracingGeometricPathNodeSIMO(antennaSIMO.transmittingAntenna, antennaSIMO.receiverAntennas);

		if (!SIMOGlobal_2025_06Std::SetSIMOGlobal(rayVecs, scenarioDataInformation, antennaSIMO, rayTracingGeometricPathNodeSIMO)) {
			return;
		}



		if (multithreadParameterConfig.multithreadConfigSwitchOfMultithread) {
			RtSbr3DForRay3DFindPathMultiThread_2025_06Std::RtSbr3DForRay3DFindPathMultiThread_SIMO(multithreadParameterConfig);
		}
		else {
			RtSbr3DForRay3DFindPathSingleThread_2025_06Std::RtSbr3DForRay3DFindPathSingleThread_SIMO();
		}

		{
			//CalRunTimeStd::CalRunTime CalRunTime(true);
			//std::cout << "与接收机交互" << std::endl;

			//InteractionBetweenRayAndRxStd::interactionBetweenRayAndRx_threadPool.join();
		}
		//std::cout << rayTracingGeometricPathNodeSIMO.rayTracingGeometricPathNodeSISOs[0].paths.size() << std::endl;
		{
			CalRunTimeStd::CalRunTime CalRunTime(true);
			std::cout << "多径转化(包括删除重复路径和计算节点信息)" << std::endl;

			//if (deduplicateRadius>0.001) {
			//	//在这里开启筛选会导致临界花纹
			//	RayTracingGeometricPathNodeSIMOStd::DeleteSamePath_SIMO(
			//		deduplicateRadius, triangle_triangle_sameside, seg_seg_samepoint, rayTracingGeometricPathNodeSIMO);
			//}
			//RayTracingGeometricPathNodeSIMOStd::RayTracingGeometricPathNodeToMultiPathNodeInfo_SIMO(scenarioDataInformation, rayTracingGeometricPathNodeSIMO, antennaSIMOPath);

		}
		
		//std::cout << antennaSIMOPath.paths[0].paths.size() << std::endl;

		SIMOGlobal_2025_06Std::FreeSIMOGlobal();

    }

    void RtSbr3DForRay3DFindPath_MIMO(
		double deduplicateRadius,
		const std::vector<std::vector<bool>>& triangle_triangle_sameside,
		const std::vector<std::vector<bool>>& seg_seg_samepoint,
		const MultithreadParameterConfigStd::MultithreadParameterConfig& multithreadParameterConfig,
        ScenarioDataInformationStd::ScenarioDataInformation& scenarioDataInformation,
        AntennaMIMOStd::AntennaMIMO& antennaMIMO,
        AntennaMIMOPathStd::AntennaMIMOPath& antennaMIMOPath) {

		for (int loop_transmittingAntenna_index = 0; loop_transmittingAntenna_index < antennaMIMO.antennaSIMOs.size();++loop_transmittingAntenna_index) {
			AntennaSIMOStd::AntennaSIMO antennaSIMO = antennaMIMO.antennaSIMOs[loop_transmittingAntenna_index];
			RtSbr3DForRay3DFindPath_SIMO(
				deduplicateRadius,
				triangle_triangle_sameside,
				seg_seg_samepoint,
				multithreadParameterConfig,
				scenarioDataInformation,
				antennaSIMO,
				antennaMIMOPath.paths[loop_transmittingAntenna_index]);

		}


    }

    void RtSbr3DForRay3DFindPath(
		double deduplicateRadius,
		const std::vector<std::vector<bool>>& triangle_triangle_sameside,
		const std::vector<std::vector<bool>>& seg_seg_samepoint,
		const MultithreadParameterConfigStd::MultithreadParameterConfig& multithreadParameterConfig,
		const RayEjectionParameterConfigStd::RayEjectionParameterConfig& rayEjectionParameterConfig,
		const RtSbr3DForRay3DPrivateParameterConfigStd::RtSbr3DForRay3DPrivateParameterConfig& rtSbr3DForRay3DPrivateParameterConfig,
        ScenarioDataInformationStd::ScenarioDataInformation& scenarioDataInformation,
        AntennaMIMOStd::AntennaMIMO& antennaMIMO,
        AntennaMIMOPathStd::AntennaMIMOPath& antennaMIMOPath) {

		SIMOGlobal_2025_06Std::SetRtSbr3DForRay3DPrivateParameterConfig(rtSbr3DForRay3DPrivateParameterConfig, rayEjectionParameterConfig);

		RtSbr3DForRay3DFindPath_MIMO(
			deduplicateRadius,
			triangle_triangle_sameside,
			seg_seg_samepoint,
			multithreadParameterConfig,
			scenarioDataInformation,
			antennaMIMO,
			antennaMIMOPath);

    }
}

namespace XdQRtSbr3DStd {


    bool InitRtSbr3DForRay3DParameterConfig(const std::string& fileName, RtSbr3DForRay3DParameterConfigStd::RtSbr3DForRay3DParameterConfig& rtSbr3DForRay3DParameterConfig) {
        //授权检验

        if (!RtSbr3DForRay3DParameterConfigStd::ReadRtSbr3DForRay3DParameterConfigByJsonFile(fileName.c_str(), rtSbr3DForRay3DParameterConfig)) {
            RtSbr3DForRay3DParameterConfigStd::WriteRtSbr3DForRay3DParameterConfigToJsonFile(fileName.c_str(), rtSbr3DForRay3DParameterConfig);
            return false;
        }

        return true;
    }

	//bool XdQRtSbr3DStart() {
	//
    //    
    //    CalRunTimeStd::CalRunTime calRunTime(false);
	//
	//
    //    const std::string RtSbr3DForRay3D_Config_FileName = "RtSbr3DForRay3D.Config.json";
	//
    //    //1.配置文件的操作
    //    RtSbr3DForRay3DParameterConfigStd::RtSbr3DForRay3DParameterConfig rtSbr3DForRay3DParameterConfig;
    //    if (!InitRtSbr3DForRay3DParameterConfig(RtSbr3DForRay3D_Config_FileName, rtSbr3DForRay3DParameterConfig)) {
    //        return false;
    //    }
    //    ScenarioDataInformationStd::ScenarioDataInformation scenarioDataInformation;
    //    AntennaMIMOStd::AntennaMIMO antennaMIMO;
    //    AntennaMIMOPathStd::AntennaMIMOPath antennaMIMOPath;
	//
    //    //2.读入数据
	//
	//	std::vector<std::vector<bool>> triangle_triangle_sameside;
	//	std::vector<std::vector<bool>> seg_seg_samepoint;
    //    if (!RtProgramReadsDataAndPreprocessesDataStd::RtProgramReadsDataAndPreprocessesData(
    //        rtSbr3DForRay3DParameterConfig.commonParameterConfig.rebuildEdge,
    //        rtSbr3DForRay3DParameterConfig.rayEjectionParameterConfig.ejectionsMaxTotalNumber,
    //        rtSbr3DForRay3DParameterConfig.rayEjectionParameterConfig.ejectionsOfDiffractionMaxNumber,
    //        rtSbr3DForRay3DParameterConfig.geometricSpaceAccelerateParameterConfig.geometricSpaceAccelerateType,
    //        rtSbr3DForRay3DParameterConfig.geometricSpaceAccelerateParameterConfig.lengthOfPixel,
    //        rtSbr3DForRay3DParameterConfig.dataInputCsvFileParameterConfig,
    //        scenarioDataInformation,
    //        antennaMIMO,
    //        antennaMIMOPath, triangle_triangle_sameside, seg_seg_samepoint)) {
    //        return false;
    //    }
	//
    //    //4.计算路径
	//
    //    //设置计算精度
    //    double time_calPath_start = calRunTime.GetTimeAll();
    //    //基于正向射线根据计算路径
	//
    //    {
    //        GlobalConstantStd::SetAirSubstanceType(rtSbr3DForRay3DParameterConfig.commonParameterConfig.airSubstanceType);
    //        //计算路径
    //        //
	//		RtSbr3DForRay3DFindPathStd::RtSbr3DForRay3DFindPath(
	//			rtSbr3DForRay3DParameterConfig.commonParameterConfig.deduplicateRadius,
	//			triangle_triangle_sameside,
	//			seg_seg_samepoint,
	//			rtSbr3DForRay3DParameterConfig.multithreadParameterConfig,
	//			rtSbr3DForRay3DParameterConfig.rayEjectionParameterConfig,
	//			rtSbr3DForRay3DParameterConfig.rtSbr3DForRay3DPrivateParameterConfig,
	//			scenarioDataInformation,
	//			antennaMIMO,
	//			antennaMIMOPath);
    //    }
	//
    //    double time_calPath_end = calRunTime.GetTimeAll();
	//
    //    //5.路径进行去重
    //    double time_findCenterMultipathByDeduplicateRadius_start = calRunTime.GetTimeAll();
	//	//正向已经去重,这里的去重方法 应当舍弃，不是最优秀的版本
    //    //AntennaMIMOPathStd::FindCenterMultipathByDeduplicateRadius(
    //    //    antennaMIMOPath, rtSbr3DForRay3DParameterConfig.commonParameterConfig.deduplicateRadius);
    //    double time_findCenterMultipathByDeduplicateRadius_end = calRunTime.GetTimeAll();
	//
    //    //7.计算电场
    //    double time_calElectricField_start = calRunTime.GetTimeAll();
	//
	//	//CalAntennaPathElectricFieldInterfaceStd::CalantennaMIMOPathElectricField(true, antennaMIMOPath);
	//
    //    double time_calElectricField_end = calRunTime.GetTimeAll();
	//
    //    double time_writeTxt_start = calRunTime.GetTimeAll();
	//
    //    //6.路径进行json输出
    //    AntennaMIMOPathFileOutputStd::AntennaMIMOPathFileOutput(
    //        rtSbr3DForRay3DParameterConfig.dataOutputParameterConfig.switchOfPathInfo,
    //        rtSbr3DForRay3DParameterConfig.dataOutputParameterConfig.outPutDirectoryPathName,
    //        antennaMIMOPath);
	//
    //    //8.输出计算结果
    //    //AntennaMIMOPathElectricFieldFileOutputStd::AntennaMIMOPathElectricFieldFileOutput(
    //    //    rtSbr3DForRay3DParameterConfig.commonParameterConfig.electricFieldCalculationMode,
    //    //    rtSbr3DForRay3DParameterConfig.commonParameterConfig.energyOutputMode,
    //    //    rtSbr3DForRay3DParameterConfig.dataOutputParameterConfig.switchOfBigChannelParameterInfo,
    //    //    rtSbr3DForRay3DParameterConfig.dataOutputParameterConfig.switchOfSmallChannelParameterInfo,
    //    //    rtSbr3DForRay3DParameterConfig.dataOutputParameterConfig.switchOfStatisticChannelParameterInfo,
    //    //    rtSbr3DForRay3DParameterConfig.dataOutputParameterConfig.outPutDirectoryPathName,
    //    //    rtSbr3DForRay3DParameterConfig.commonParameterConfig.powerThreshold);
	//
    //    //多信号源叠加
    //    //AntennaMIMOPathElectricFieldFileOutputStd::MultipleSignalSourceSuperpositionOutput(
    //    //    rtSbr3DForRay3DParameterConfig.commonParameterConfig.electricFieldCalculationMode,
    //    //    rtSbr3DForRay3DParameterConfig.commonParameterConfig.energyOutputMode,
    //    //    rtSbr3DForRay3DParameterConfig.dataOutputParameterConfig.switchOfMultipleSignalSourceSuperposition,
    //    //    rtSbr3DForRay3DParameterConfig.dataOutputParameterConfig.outPutDirectoryPathName,
    //    //    rtSbr3DForRay3DParameterConfig.commonParameterConfig.powerThreshold);
	//
    //    double time_writeTxt_end = calRunTime.GetTimeAll();
	//
    //    //9.输出日志，主要输出配置文件和运算时间
    //    std::string logContext = RtSbr3DForRay3DParameterConfigStd::RtSbr3DForRay3DParameterConfigToJsonString(rtSbr3DForRay3DParameterConfig);
	//
    //    double time_run_all = calRunTime.GetTimeAll();
	//
    //    {
    //        std::ostringstream oss;
    //        oss << std::endl << std::endl;
    //        oss << "几何寻径用时为 " << (time_calPath_end - time_calPath_start) << " ms." << std::endl;
    //        oss << "寻找中心簇用时为 " << (time_findCenterMultipathByDeduplicateRadius_end - time_findCenterMultipathByDeduplicateRadius_start) << " ms." << std::endl;
    //        oss << "电场计算用时为 " << (time_calElectricField_end - time_calElectricField_start) << " ms." << std::endl;
    //        oss << "输出结果用时为 " << (time_writeTxt_end - time_writeTxt_start) << " ms." << std::endl;
    //        oss << "程序运行总时长为 " << time_run_all << " ms." << std::endl;
    //        logContext.append(oss.str());
    //    }
	//
	//
    //    AntennaMIMOPathElectricFieldFileOutputStd::OutPutLog(
    //        rtSbr3DForRay3DParameterConfig.dataOutputParameterConfig.outPutDirectoryPathName,
    //        rtSbr3DForRay3DParameterConfig.dataOutputParameterConfig.outPutLogTxtFileName,
    //        logContext);
	//
    //    //10.内存释放，程序结束
	//
    //    {
	//
    //        //所有的多径节点最终被统一释放内存
    //        MultiPathNodeInfoOperateStd::FreeMultiPathNodeInfo_vector_all();
	//
    //    }
	//
    //    {
    //        std::ostringstream oss;
    //        oss << "程序运行总时长为 " << calRunTime.GetTimeAll() << " ms.";
    //        ProjectDependenciesStd::DisplayPromptOrReason(oss.str(), false, __FILE__, __LINE__);
    //    }
    //    
    //    return true;
    //}



	bool XdQRtSbr3DStart() {


		CalRunTimeStd::CalRunTime calRunTime(false);


		const std::string RtSbr3DForRay3D_Config_FileName = "RtSbr3DForRay3D.Config.json";


		//1.配置文件的操作
		RtSbr3DForRay3DParameterConfigStd::RtSbr3DForRay3DParameterConfig rtSbr3DForRay3DParameterConfig;
		if (!InitRtSbr3DForRay3DParameterConfig(RtSbr3DForRay3D_Config_FileName, rtSbr3DForRay3DParameterConfig)) {
			return false;
		}


		bool success = false;
		Sbr3DFindPathConfig config;
		MaterialSet materialSet;
		Scenario3D scenario;
		AntennaPolarization3DModel antennaPolarization3DModel;
		AntennaRadiationPattern3DModel antennaRadiationPattern3DModel;
		TransmitterAntenna transmitterAntenna;
		{
			SY_2025_06::RT_Init_Sbr3DFindPathConfig(rtSbr3DForRay3DParameterConfig, config);
			bool init_state = SY_2025_06::RT_Init_MaterialSet(rtSbr3DForRay3DParameterConfig.dataInputCsvFileParameterConfig.inputMaterialTableCsvFileName, materialSet);
			if (!init_state) {
				return false;
			}
			init_state = SY_2025_06::RT_Init_Scenario3D(
				rtSbr3DForRay3DParameterConfig.dataInputCsvFileParameterConfig.inputScenarioPoint3DCsvFileName,
				rtSbr3DForRay3DParameterConfig.dataInputCsvFileParameterConfig.inputScenarioTriangle3DCsvFileName,
				rtSbr3DForRay3DParameterConfig.dataInputCsvFileParameterConfig.inputScenarioCorner3DCsvFileName,
				scenario);
			if (!init_state) {
				return false;
			}
			init_state = SY_2025_06::RT_Init_TransmitterAntenna(
				rtSbr3DForRay3DParameterConfig.dataInputCsvFileParameterConfig.inputTransmittingAntennaDatabaseJsonFileName, antennaPolarization3DModel, antennaRadiationPattern3DModel, transmitterAntenna);
			if (!init_state) {
				return false;
			}
		}

		double time_calElectricField_start, time_calElectricField_end;

		time_calElectricField_start = calRunTime.GetTimeAll();
		
		RtoiOutputInformation* rtoiOutputInformation = RTSbr3DCircularPolarization3DPtr(
			config,
			materialSet,
			scenario,
			antennaPolarization3DModel,
			antennaRadiationPattern3DModel,
			transmitterAntenna,
			success);


		time_calElectricField_end = calRunTime.GetTimeAll();


		double time_writeTxt_start = calRunTime.GetTimeAll();

		if (success) {
			if (rtSbr3DForRay3DParameterConfig.dataOutputParameterConfig.switchOfBigChannelParameterInfo) {
				RtoiOutputPowerInformationToCsvFile(*rtoiOutputInformation, rtSbr3DForRay3DParameterConfig.dataOutputParameterConfig.outPutDirectoryPathName);
			}
			if (rtSbr3DForRay3DParameterConfig.dataOutputParameterConfig.switchOfSmallChannelParameterInfo) {
				RtoiOutputImpactResponseInformationToCsvFile(*rtoiOutputInformation, rtSbr3DForRay3DParameterConfig.dataOutputParameterConfig.outPutDirectoryPathName);
			}

			if (rtSbr3DForRay3DParameterConfig.dataOutputParameterConfig.switchOfPathInfo) {
				std::string pathInformationJsonFileName = rtSbr3DForRay3DParameterConfig.dataOutputParameterConfig.outPutDirectoryPathName;

				FileOperateStd::CreateNewDirectorys(rtSbr3DForRay3DParameterConfig.dataOutputParameterConfig.outPutDirectoryPathName);
				pathInformationJsonFileName.append("\\pathInformation.json");
				RtoiOutputInformationToJsonFile(*rtoiOutputInformation, pathInformationJsonFileName);
			}
		}
		
		double time_writeTxt_end = calRunTime.GetTimeAll();

		// 输出日志，主要输出配置文件和运算时间
		std::string logContext = RtSbr3DForRay3DParameterConfigStd::RtSbr3DForRay3DParameterConfigToJsonString(rtSbr3DForRay3DParameterConfig);

		double time_run_all = calRunTime.GetTimeAll();

		{
			std::ostringstream oss;
			oss << std::endl << std::endl;
			//oss << "电场计算用时为 " << (time_calElectricField_end - time_calElectricField_start) << " ms." << std::endl;
			//oss << "输出结果用时为 " << (time_writeTxt_end - time_writeTxt_start) << " ms." << std::endl;
			oss << "程序运行总时长为 " << time_run_all << " ms." << std::endl;
			logContext.append(oss.str());
		}


		AntennaMIMOPathElectricFieldFileOutputStd::OutPutLog(
			rtSbr3DForRay3DParameterConfig.dataOutputParameterConfig.outPutDirectoryPathName,
			rtSbr3DForRay3DParameterConfig.dataOutputParameterConfig.outPutLogTxtFileName,
			logContext);

		// 内存释放，程序结束

		{

			//所有的多径节点最终被统一释放内存
			MultiPathNodeInfoOperateStd::FreeMultiPathNodeInfo_vector_all();

		}

		{
			std::ostringstream oss;
			oss << "程序运行总时长为 " << time_run_all << " ms.";
			ProjectDependenciesStd::DisplayPromptOrReason(oss.str(), false, __FILE__, __LINE__);
		}

		return true;
	}

}