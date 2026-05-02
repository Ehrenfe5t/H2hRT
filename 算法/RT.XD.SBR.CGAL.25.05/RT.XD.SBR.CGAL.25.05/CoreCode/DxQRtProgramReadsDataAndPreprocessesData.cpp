
#include"DxQRtProgramReadsDataAndPreprocessesData.h"

#include"DxQVectorOperate.h"
#include"DxQStringToStructure.h"
#include"DxQRayScenarioIntersectType.h"
#include"DxQSpaceSubregionParameterConfig.h"
#include"DxQRayScenarioIntersect.h"

#include"QzQGeometry3DOperate.Equals.h"
#include"QzQGeometry3DIntersect.h"
#include"QzQFileBase.h"
#include"HdQAntennaPatternDatabase.h"
#include"HdQAntennaPatternJsonObject.h"
#include"HdQAntennaPatternJsonObjectJsonFile.h"
#include"HdQCalRunTime.h"
#include"LxQProjectDependencies.h"
#include"LxQMaterialObjectDatabase.h"
#include"LxQMultiLinearPolarization3DObjectDatabaseJsonOperate.h"
#include"LxQMultiLinearPolarization3DDatabase.h"

#include"../0.Ray3DIntersectGeometry3DElementsModule/0.Ray3DIntersectGeometry3DElementsModule.Output.h"


#include<set>
#include<unordered_set>
#include<unordered_map>
namespace RtProgramReadsDataAndPreprocessesDataStd {


    //bool GeometricSpacePartitionProcessingMethod(
    //    bool switchOfOfDiffraction,
    //    const RayScenarioIntersectTypeStd::RayScenarioIntersectType& type,
    //    double lengthOfPixel,
    //    const ScenarioObjectStd::ScenarioObject& scenarioObject,
    //    const TriangleAccelerateStructDatabaseStd::TriangleAccelerateStructDatabase& triangleAccelerateStructDatabase) {
    //
    //    SpaceSubregionParameterConfigStd::SpaceSubregionParameterConfig spaceSubregionParameterConfig;
    //    spaceSubregionParameterConfig.pixelParameterConfig.stepLength = lengthOfPixel;
    //
    //    if (type == RayScenarioIntersectTypeStd::RayScenarioIntersectType::Pixel3D
    //        || type == RayScenarioIntersectTypeStd::RayScenarioIntersectType::Pixel3D_SDF
    //        || type == RayScenarioIntersectTypeStd::RayScenarioIntersectType::BvhBall3D) {
    //        return RayScenarioIntersectStd::SetPixelType(
    //            type,
    //            switchOfOfDiffraction,
    //            spaceSubregionParameterConfig,
    //            scenarioObject,
    //            triangleAccelerateStructDatabase
    //        );
    //    }
    //    return true;
    //}


    bool GeometricSpacePartitionProcessingMethod(
        int type, const Scenario3D& scenario, bool switchCorner, double cornerRadius) {

        Ray3DIntersectGeometry3DStd::Initialize(type, scenario,  switchCorner,  cornerRadius);
        return true;
    }


    /// <summary>
    /// 初始化极化信息
    /// </summary>
    /// <param name="multiLinearPolarization3DDatabaseFileName"></param>
    void InitMultiLinearPolarization3DDatabaseByMultiLinearPolarization3DObjectDatabaseJsonFile(const std::string& multiLinearPolarization3DDatabaseFileName) {

        if (!FileOperateStd::ExistFile(multiLinearPolarization3DDatabaseFileName.c_str())) {
            MultiLinearPolarization3DObjectDatabaseStd::MultiLinearPolarization3DObjectDatabase multiLinearPolarization3DObjectDatabase1;
            MultiLinearPolarization3DDatabaseStd::InitDatabaseByMultiLinearPolarization3DObjectDatabase(multiLinearPolarization3DObjectDatabase1);
            MultiLinearPolarization3DObjectDatabaseJsonOperateStd::WriteMultiLinearPolarization3DDatabaseToJsonFile(multiLinearPolarization3DDatabaseFileName);
            return;
        }

        MultiLinearPolarization3DObjectDatabaseJsonStd::MultiLinearPolarization3DObjectDatabaseJson multiLinearPolarization3DObjectDatabaseJson;
        MultiLinearPolarization3DObjectDatabaseJsonOperateStd::ReadMultiLinearPolarization3DObjectDatabaseJsonByJsonFile(
            multiLinearPolarization3DDatabaseFileName.c_str(), multiLinearPolarization3DObjectDatabaseJson);

        MultiLinearPolarization3DObjectDatabaseStd::MultiLinearPolarization3DObjectDatabase multiLinearPolarization3DObjectDatabase1;
        multiLinearPolarization3DObjectDatabase1.AddAll(multiLinearPolarization3DObjectDatabaseJson.database);
        MultiLinearPolarization3DDatabaseStd::InitDatabaseByMultiLinearPolarization3DObjectDatabase(multiLinearPolarization3DObjectDatabase1);

    }

    void InitAntennaPatternDatabaseByAntennaPatternDatabaseJsonFile(const std::string& inputAntennaPatternDatabaseJsonFileName) {
        if (!FileOperateStd::ExistFile(inputAntennaPatternDatabaseJsonFileName.c_str())) {
            AntennaPatternJsonObjectStd::AntennaPatternJsonObject antennaPatternJsonObject;
            AntennaPatternObjectStd::AntennaPatternObject antennaPatternObject;
            antennaPatternJsonObject.antennaPatternObjects.emplace_back(antennaPatternObject);
            AntennaPatternJsonObjectStd::WriteAntennaPatternJsonObjectToJsonFile(inputAntennaPatternDatabaseJsonFileName.c_str(), antennaPatternJsonObject);
            
            return;
        }

        AntennaPatternJsonObjectStd::AntennaPatternJsonObject antennaPatternJsonObject;
        AntennaPatternJsonObjectStd::ReadAntennaPatternJsonObjectByJsonFile(inputAntennaPatternDatabaseJsonFileName.c_str(), antennaPatternJsonObject);

        AntennaPatternDatabaseStd::AddRangeAntennaPatternObject(antennaPatternJsonObject.antennaPatternObjects);
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


    bool ReadScenarioPointByCsvModel(
        const std::string& fileName,
        std::vector<double>& x_data,
        std::vector<double>& y_data,
        std::vector<double>& z_data) {

        if (!FileOperateStd::ExistFile(fileName.data())) {
            {
                std::ostringstream oss;
                oss << fileName << "文件不存在!";
                ProjectDependenciesStd::DisplayPromptOrReason(oss.str(), true, __FILE__, __LINE__);
            }
            return false;
        }
        std::ifstream file;
        file.open(fileName.c_str(), std::ios::in);
        double x = 0.0;
        double y = 0.0;
        double z = 0.0;
        std::string line;
        std::getline(file, line);


        while (true) {
            std::getline(file, line);
            int ret = sscanf_s(line.c_str(), "%lf,%lf,%lf", &x, &y, &z);
            if (ret <= 0) {
                //std::cout << "数据读取失败2222!" << std::endl;
                break;
            }
            //重复点不进行删除

            x_data.emplace_back(x);
            y_data.emplace_back(y);
            z_data.emplace_back(z);

            if (file.eof()) {
                break;
            }
        }
        file.close();


        return true;
    }


    bool ReadScenarioTriangle3DIndexByCsvModel(
        const std::string& fileName,
        std::vector<int>& structScenarioArrayScenarioTriangle3DIndexUpTypeNumber_data,
        std::vector<int>& structScenarioArrayScenarioTriangle3DIndexDownTypeNumber_data,
        std::vector<int>& structScenarioArrayScenarioTriangle3DIndexTriangleP1Index_data,
        std::vector<int>& structScenarioArrayScenarioTriangle3DIndexTriangleP2Index_data,
        std::vector<int>& structScenarioArrayScenarioTriangle3DIndexTriangleP3Index_data,
        std::vector<double>& structScenarioArrayScenarioTriangle3DIndexRoughness_data,
        std::vector<double>& structScenarioArrayScenarioTriangle3DIndexNX_data,
        std::vector<double>& structScenarioArrayScenarioTriangle3DIndexNY_data,
        std::vector<double>& structScenarioArrayScenarioTriangle3DIndexNZ_data) {

        if (!FileOperateStd::ExistFile(fileName.data())) {
            {
                std::ostringstream oss;
                oss << fileName << "文件不存在!";
                ProjectDependenciesStd::DisplayPromptOrReason(oss.str(), true, __FILE__, __LINE__);
            }
            return false;
        }
        std::ifstream file;
        file.open(fileName.c_str(), std::ios::in);
        int triangleP1Index = -1;
        int triangleP2Index = -1;
        int triangleP3Index = -1;
        double x = 0.0; double y = 0.0; double z = 0.0;
        int  upTypeNumber = 0;
        int downTypeNumber = 0;
        double roughness = 0.0f;
        std::string line;
        std::getline(file, line);
        int linCount = 1;


        while (true) {
            linCount++;
            std::getline(file, line);
            int ret = sscanf_s(line.c_str(), "%d,%d,%d,%d,%d,%lf,%lf,%lf,%lf", &triangleP1Index, &triangleP2Index, &triangleP3Index, &upTypeNumber, &downTypeNumber, &roughness, &x, &y, &z);
            if (ret <= 0) {
                //std::cout << "数据读取失败333!" << std::endl;
                break;
            }

            structScenarioArrayScenarioTriangle3DIndexUpTypeNumber_data.emplace_back(upTypeNumber);
            structScenarioArrayScenarioTriangle3DIndexDownTypeNumber_data.emplace_back(downTypeNumber);
            structScenarioArrayScenarioTriangle3DIndexTriangleP1Index_data.emplace_back(triangleP1Index);
            structScenarioArrayScenarioTriangle3DIndexTriangleP2Index_data.emplace_back(triangleP2Index);
            structScenarioArrayScenarioTriangle3DIndexTriangleP3Index_data.emplace_back(triangleP3Index);
            structScenarioArrayScenarioTriangle3DIndexRoughness_data.emplace_back(roughness);
            structScenarioArrayScenarioTriangle3DIndexNX_data.emplace_back(x);
            structScenarioArrayScenarioTriangle3DIndexNY_data.emplace_back(y);
            structScenarioArrayScenarioTriangle3DIndexNZ_data.emplace_back(z);

            if (file.eof()) {
                break;
            }
        }
        file.close();


        return true;
    }

    bool ReadScenarioCorner3DIndexByCsvModel(
        const std::string& fileName,
        std::vector<int>& structScenarioArrayScenarioCorner3DIndexP1Index_data,
        std::vector<int>& structScenarioArrayScenarioCorner3DIndexP2Index_data,
        std::vector<int>& structScenarioArrayScenarioCorner3DIndexP3Face0Index_data,
        std::vector<int>& structScenarioArrayScenarioCorner3DIndexP3FaceNIndex_data,
        std::vector<int>& structScenarioArrayScenarioCorner3DIndexFace0Index_data,
        std::vector<int>& structScenarioArrayScenarioCorner3DIndexFaceNIndex_data) {

        if (!FileOperateStd::ExistFile(fileName.data())) {
            {
                std::ostringstream oss;
                oss << fileName << "文件不存在!";
                ProjectDependenciesStd::DisplayPromptOrReason(oss.str(), true, __FILE__, __LINE__);
            }
            return false;
        }
        std::ifstream file;
        file.open(fileName.c_str(), std::ios::in);
        int startPoint3DIndex = -1;
        int endPoint3DIndex = -1;
        int triangleFace1Index = -1;
        int triangleFace2Index = -1;
        int p1Index = -1;
        int p2Index = -1;
        std::string line;
        std::getline(file, line);


        while (true) {
            std::getline(file, line);
            int ret = sscanf_s(line.c_str(), "%d,%d,%d,%d,%d,%d", &startPoint3DIndex, &endPoint3DIndex, &p1Index, &p2Index, &triangleFace1Index, &triangleFace2Index);
            if (ret <= 0) {
                //std::cout << "数据读取失败333!" << std::endl;
                break;
            }

            structScenarioArrayScenarioCorner3DIndexP1Index_data.emplace_back(startPoint3DIndex);
            structScenarioArrayScenarioCorner3DIndexP2Index_data.emplace_back(endPoint3DIndex);
            structScenarioArrayScenarioCorner3DIndexP3Face0Index_data.emplace_back(p1Index);
            structScenarioArrayScenarioCorner3DIndexP3FaceNIndex_data.emplace_back(p2Index);
            structScenarioArrayScenarioCorner3DIndexFace0Index_data.emplace_back(triangleFace1Index);
            structScenarioArrayScenarioCorner3DIndexFaceNIndex_data.emplace_back(triangleFace2Index);

            if (file.eof()) {
                break;
            }
        }
        file.close();
        return true;
    }


    bool ReadTransmittingAntennaByCsvModel(
        const std::string& fileName,
        std::vector<TransmittingAntennaStd::TransmittingAntenna>& transmittingAntennas) {

        if (!FileOperateStd::ExistFile(fileName.data())) {
            {
                std::ostringstream oss;
                oss << fileName << "文件不存在!";
                ProjectDependenciesStd::DisplayPromptOrReason(oss.str(), true, __FILE__, __LINE__);
            }
            return false;
        }
        std::ifstream file;
        file.open(fileName.c_str(), std::ios::in);
        int transmittingAntennaId = -1;
        double x = 0.0;
        double y = 0.0;
        double z = 0.0;
        double frequency = 1e9;
        double emissionPower = 0.0;
        int radiationPatternId = -1;
        int polarization3DModelId = -1;

        std::string line;
        std::getline(file, line);

        std::set<int> transmittingAntennaIdSet;

        //id,x(m),y(m),z(m),frequency(Hz),emissionPower(w),radiationPatternId,polarization3DModelId
        while (true) {
            std::getline(file, line);
            int ret = sscanf_s(
                line.c_str(), "%d,%lf,%lf,%lf,%lf,%lf,%d,%d",
                &transmittingAntennaId,
                &x, &y, &z,
                &frequency,
                &emissionPower,
                &radiationPatternId,
                &polarization3DModelId);
            if (ret <= 0) {
                //std::cout << "数据读取失败333!" << std::endl;
                break;
            }

            TransmittingAntennaStd::TransmittingAntenna transmittingAntenna;
            transmittingAntenna.transmittingAntennaId = transmittingAntennaId;
            transmittingAntenna.emissionPower = emissionPower;
            transmittingAntenna.antennaProperty.frequencys.emplace_back((long long)frequency);
            transmittingAntenna.antennaProperty.location.x = x;
            transmittingAntenna.antennaProperty.location.y = y;
            transmittingAntenna.antennaProperty.location.z = z;
            transmittingAntenna.antennaProperty.radiationPatternId = radiationPatternId;
            transmittingAntenna.antennaProperty.polarization3DModelId = polarization3DModelId;

            transmittingAntennas.emplace_back(transmittingAntenna);
            transmittingAntennaIdSet.insert(transmittingAntennaId);

            if (file.eof()) {
                break;
            }
        }
        file.close();

        if (transmittingAntennaIdSet.size() != transmittingAntennas.size()) {
            {
                std::ostringstream oss;
                oss << "注意发射机的id出现重复，可能在后续计算中导致错误！";
                ProjectDependenciesStd::DisplayPromptOrReason(oss.str(), false, __FILE__, __LINE__);
            }
        }

        return true;
    }

    /// <summary>
    /// 读取接收天线csv
    /// </summary>
    /// <param name="rxListFilename"></param>
    /// <param name="txList"></param>
    /// <param name="rxList"></param>
    /// <returns></returns>
    bool ReadReceiverAntennaByCsvModel(
        const std::string& fileName,
        const std::vector<int>& transmittingAntennaIds,
        std::vector<ReceiverAntennaStd::ReceiverAntenna>& receiverAntennas) {

        if (!FileOperateStd::ExistFile(fileName.data())) {
            {
                std::ostringstream oss;
                oss << fileName << "文件不存在!";
                ProjectDependenciesStd::DisplayPromptOrReason(oss.str(), true, __FILE__, __LINE__);
            }
            return false;
        }
        std::ifstream file;
        file.open(fileName.c_str(), std::ios::in);
        int receiverAntennaId = -1;
        double x = 0.0;
        double y = 0.0;
        double z = 0.0;
        double frequency = 1e9;
        double emissionPower = 0.0;
        int radiationPatternId = -1;
        int polarization3DModelId = -1;

        std::string line;
        std::getline(file, line);

        std::set<int> receiverAntennaIdSet;
        //csv文件没有从属关系，默认认为就是全部计算
        //id,x(m),y(m),z(m),frequency(Hz),radiationPatternId,polarization3DModelId
        while (true) {
            std::getline(file, line);
            int ret = sscanf_s(
                line.c_str(), "%d,%lf,%lf,%lf,%lf,%d,%d",
                &receiverAntennaId,
                &x, &y, &z,
                &frequency,
                &radiationPatternId,
                &polarization3DModelId);
            if (ret <= 0) {
                //std::cout << "数据读取失败333!" << std::endl;
                break;
            }

            ReceiverAntennaStd::ReceiverAntenna receiverAntenna;
            receiverAntenna.receiverAntennaId = receiverAntennaId;
            receiverAntenna.antennaProperty.frequencys.emplace_back((long long)frequency);
            receiverAntenna.antennaProperty.location.x = x;
            receiverAntenna.antennaProperty.location.y = y;
            receiverAntenna.antennaProperty.location.z = z;
            receiverAntenna.antennaProperty.radiationPatternId = radiationPatternId;
            receiverAntenna.antennaProperty.polarization3DModelId = polarization3DModelId;
            for (int i = 0; i < transmittingAntennaIds.size(); ++i) {
                receiverAntenna.transmittingAntennaIds.emplace_back(transmittingAntennaIds[i]);
            }

            receiverAntennas.emplace_back(receiverAntenna);
            receiverAntennaIdSet.insert(receiverAntennaId);
            if (file.eof()) {
                break;
            }
        }
        file.close();

        if (receiverAntennaIdSet.size() != receiverAntennas.size()) {
            {
                std::ostringstream oss;
                oss << "注意接收机的id出现重复，可能在后续计算中导致错误！";
                ProjectDependenciesStd::DisplayPromptOrReason(oss.str(), false, __FILE__, __LINE__);
            }

            for (int i = 0; i < receiverAntennas.size(); ++i) {
                receiverAntennas[i].receiverAntennaId = i;
            }

        }
        return true;
    }

    bool InitScenario(
        const std::string& inputScenarioPoint3DCsvFileName,
        const std::string& inputScenarioTriangle3DCsvFileName,
        const std::string& inputScenarioCorner3DCsvFileName,
        ScenarioObjectStd::ScenarioObject& scenarioObject) {

            {
                std::vector<double> x_data;
                std::vector<double> y_data;
                std::vector<double> z_data;
                if (!ReadScenarioPointByCsvModel(
                    inputScenarioPoint3DCsvFileName,
                    x_data,
                    y_data,
                    z_data)) {
                    return false;
                }

                for (int i = 0; i < x_data.size(); ++i) {
                    Point3DStd::Point3D location(x_data[i], y_data[i], z_data[i]);
                    scenarioObject.scenarioPoint3D.emplace_back(location);
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
                if (!ReadScenarioTriangle3DIndexByCsvModel(
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

                for (int i = 0; i < structScenarioArrayScenarioTriangle3DIndexUpTypeNumber_data.size(); ++i) {

                    ScenarioTriangle3DIndexStd::ScenarioTriangle3DIndex scenarioTriangle3DIndex;
                    scenarioTriangle3DIndex.TriangleP1Index = structScenarioArrayScenarioTriangle3DIndexTriangleP1Index_data[i];
                    scenarioTriangle3DIndex.TriangleP2Index = structScenarioArrayScenarioTriangle3DIndexTriangleP2Index_data[i];
                    scenarioTriangle3DIndex.TriangleP3Index = structScenarioArrayScenarioTriangle3DIndexTriangleP3Index_data[i];
                    scenarioTriangle3DIndex.UpTypeNumber = structScenarioArrayScenarioTriangle3DIndexUpTypeNumber_data[i];
                    scenarioTriangle3DIndex.DownTypeNumber = structScenarioArrayScenarioTriangle3DIndexDownTypeNumber_data[i];
                    scenarioTriangle3DIndex.Roughness = structScenarioArrayScenarioTriangle3DIndexRoughness_data[i];
                    scenarioTriangle3DIndex.n.x = structScenarioArrayScenarioTriangle3DIndexNX_data[i];
                    scenarioTriangle3DIndex.n.y = structScenarioArrayScenarioTriangle3DIndexNY_data[i];
                    scenarioTriangle3DIndex.n.z = structScenarioArrayScenarioTriangle3DIndexNZ_data[i];
                    scenarioObject.scenarioTriangle3DIndex.emplace_back(scenarioTriangle3DIndex);
                }

            }


            {

                std::vector<int> structScenarioArrayScenarioCorner3DIndexP1Index_data;
                std::vector<int> structScenarioArrayScenarioCorner3DIndexP2Index_data;
                std::vector<int> structScenarioArrayScenarioCorner3DIndexP3Face0Index_data;
                std::vector<int> structScenarioArrayScenarioCorner3DIndexP3FaceNIndex_data;
                std::vector<int> structScenarioArrayScenarioCorner3DIndexFace0Index_data;
                std::vector<int> structScenarioArrayScenarioCorner3DIndexFaceNIndex_data;
                if (!ReadScenarioCorner3DIndexByCsvModel(
                    inputScenarioCorner3DCsvFileName,
                    structScenarioArrayScenarioCorner3DIndexP1Index_data,
                    structScenarioArrayScenarioCorner3DIndexP2Index_data,
                    structScenarioArrayScenarioCorner3DIndexP3Face0Index_data,
                    structScenarioArrayScenarioCorner3DIndexP3FaceNIndex_data,
                    structScenarioArrayScenarioCorner3DIndexFace0Index_data,
                    structScenarioArrayScenarioCorner3DIndexFaceNIndex_data)) {
                    return false;
                }


                for (int i = 0; i < structScenarioArrayScenarioCorner3DIndexP1Index_data.size(); ++i) {

                    ScenarioCorner3DIndexStd::ScenarioCorner3DIndex scenarioCorner3DIndex;
                    scenarioCorner3DIndex.P1Index = structScenarioArrayScenarioCorner3DIndexP1Index_data[i];
                    scenarioCorner3DIndex.P2Index = structScenarioArrayScenarioCorner3DIndexP2Index_data[i];
                    scenarioCorner3DIndex.P3Face0Index = structScenarioArrayScenarioCorner3DIndexP3Face0Index_data[i];
                    scenarioCorner3DIndex.P3FaceNIndex = structScenarioArrayScenarioCorner3DIndexP3FaceNIndex_data[i];
                    scenarioCorner3DIndex.Face0Index = structScenarioArrayScenarioCorner3DIndexFace0Index_data[i];
                    scenarioCorner3DIndex.FaceNIndex = structScenarioArrayScenarioCorner3DIndexFaceNIndex_data[i];

                    scenarioObject.scenarioCorner3DIndex.emplace_back(scenarioCorner3DIndex);
                }
            }

            return true;
    }


    //读入数据并进行预处理，

    bool ReadReceiverAntennaByCsvModelByActualMeasurementData(
        long long frequency,
        const std::string& fileName,
        const std::vector<int>& transmittingAntennaIds,
        std::vector<ReceiverAntennaStd::ReceiverAntenna>& receiverAntennas,
        std::vector<double>& actualMeasurementData) {

        if (!FileOperateStd::ExistFile(fileName.data())) {
            {
                std::ostringstream oss;
                oss << fileName << "文件不存在!";
                ProjectDependenciesStd::DisplayPromptOrReason(oss.str(), true, __FILE__, __LINE__);
            }
            return false;
        }
        std::ifstream file;
        file.open(fileName.c_str(), std::ios::in);
        int receiverAntennaId = 0;
        double x = 0.0;
        double y = 0.0;
        double z = 0.0;
        double value = 0.0;
        int radiationPatternId = -1;
        int polarization3DModelId = -1;


        //csv文件没有从属关系，默认认为就是全部计算
        //id,x(m),y(m),z(m),frequency(Hz),radiationPatternId,polarization3DModelId
        while (true) {
            std::string line;
            std::getline(file, line);
            int ret = sscanf_s(
                line.c_str(), "%lf,%lf,%lf,%lf",
                &x, &y, &z,
                &value);
            if (ret <= 0) {
                //std::cout << "数据读取失败333!" << std::endl;
                break;
            }

            ReceiverAntennaStd::ReceiverAntenna receiverAntenna;
            receiverAntenna.receiverAntennaId = receiverAntennaId;
            receiverAntenna.antennaProperty.frequencys.emplace_back(frequency);
            receiverAntenna.antennaProperty.location.x = x;
            receiverAntenna.antennaProperty.location.y = y;
            receiverAntenna.antennaProperty.location.z = z;
            receiverAntenna.antennaProperty.radiationPatternId = radiationPatternId;
            receiverAntenna.antennaProperty.polarization3DModelId = polarization3DModelId;
            for (int i = 0; i < transmittingAntennaIds.size(); ++i) {
                receiverAntenna.transmittingAntennaIds.emplace_back(transmittingAntennaIds[i]);
            }

            receiverAntennas.emplace_back(receiverAntenna);
            actualMeasurementData.emplace_back(value);
            receiverAntennaId++;
            if (file.eof()) {
                break;
            }
        }
        file.close();

        
        return true;
    }

    

    /// <summary>
    /// 初始化antennaMIMOPath内存
    /// </summary>
    /// <param name="transmittingAntennas"></param>
    /// <param name="receiverAntennas"></param>
    /// <param name="antennaMIMO"></param>
    /// <param name="antennaMIMOPath"></param>
    void InitAntennaMIMOAndAntennaMIMOPath(
        const AntennaMIMOStd::AntennaMIMO& antennaMIMO,
        AntennaMIMOPathStd::AntennaMIMOPath& antennaMIMOPath) {

        for (int loop_index_transmittingAntennas = 0; loop_index_transmittingAntennas < antennaMIMO.antennaSIMOs.size(); ++loop_index_transmittingAntennas) {
            AntennaSIMOPathStd::AntennaSIMOPath antennaSIMOPath;
            antennaSIMOPath.transmittingAntennaId = antennaMIMO.antennaSIMOs[loop_index_transmittingAntennas].transmittingAntenna.transmittingAntennaId;

            AntennaSIMOStd::AntennaSIMO antennaSIMO;
            antennaSIMO.transmittingAntenna = antennaMIMO.antennaSIMOs[loop_index_transmittingAntennas].transmittingAntenna;

            auto receiverAntennas = antennaMIMO.antennaSIMOs[loop_index_transmittingAntennas].receiverAntennas;
            for (int loop_index_receiverAntennas = 0; loop_index_receiverAntennas < receiverAntennas.size(); ++loop_index_receiverAntennas) {
                //if (-1 != VectorOperateStd::ContainIntInVectorInt(
                //    antennaSIMOPath.transmittingAntennaId,
                //    receiverAntennas[loop_index_receiverAntennas].transmittingAntennaIds)) {
                //    AntennaSISOPathStd::AntennaSISOPath antennaSISOPath;
                //    antennaSISOPath.receiverAntennaId = receiverAntennas[loop_index_receiverAntennas].receiverAntennaId;
                //    antennaSISOPath.rx_location = receiverAntennas[loop_index_receiverAntennas].antennaProperty.location;
                //    antennaSIMOPath.paths.emplace_back(antennaSISOPath);
                //
                //    antennaSIMO.receiverAntennas.emplace_back(receiverAntennas[loop_index_receiverAntennas]);
                //}


                AntennaSISOPathStd::AntennaSISOPath antennaSISOPath;
                antennaSISOPath.receiverAntennaId = receiverAntennas[loop_index_receiverAntennas].receiverAntennaId;
                antennaSISOPath.rx_location = receiverAntennas[loop_index_receiverAntennas].antennaProperty.location;
                antennaSIMOPath.paths.emplace_back(antennaSISOPath);

                antennaSIMO.receiverAntennas.emplace_back(receiverAntennas[loop_index_receiverAntennas]);
            }
            antennaMIMOPath.paths.emplace_back(antennaSIMOPath);
        }

        AntennaMIMOStd::UpdataTransmittingAntennaDatabase(antennaMIMO);
    }


    bool RtProgramPreprocessesData(
        bool rebuildEdge,
        int ejectionsMaxTotalNumber,
        int ejectionsOfDiffractionMaxNumber,
        int geometricSpaceAccelerateType,
        double lengthOfPixel,
        double cornerRadius,
        const Scenario3D& scenario,
        const std::vector<TransmittingAntennaStd::TransmittingAntenna>& transmittingAntennas,
        const std::vector<ReceiverAntennaStd::ReceiverAntenna>& receiverAntennas,
        ScenarioObjectStd::ScenarioObject& scenarioObject,
        ScenarioDataInformationStd::ScenarioDataInformation& scenarioDataInformation,
        AntennaMIMOStd::AntennaMIMO& antennaMIMO,
        AntennaMIMOPathStd::AntennaMIMOPath& antennaMIMOPath,
        std::vector<std::vector<bool>>& triangle_triangle_sameside,
        std::vector<std::vector<bool>>& seg_seg_samepoint) {

        //3.预处理数据

        //3.1 重构边
        if (rebuildEdge) {
            ScenarioObjectStd::RebuildEdgeInformation(scenarioObject);
        }

        //3.2 预处理.计算边界值

        //scenarioObject的最大最小值必须包含天线
        {
            //更新最大值和最小值
            ScenarioObjectStd::ObtainScenarioBoundingBox(scenarioObject);

            for (int loop1 = 0; loop1 < transmittingAntennas.size(); ++loop1) {
                ScenarioObjectStd::ChangeScenarioMinMaxPoint3D(transmittingAntennas[loop1].antennaProperty.location, scenarioObject);
            }

            for (int loop2 = 0; loop2 < receiverAntennas.size(); ++loop2) {
                ScenarioObjectStd::ChangeScenarioMinMaxPoint3D(receiverAntennas[loop2].antennaProperty.location, scenarioObject);
            }
        }

        bool switchOfOfDiffraction = false;
        if (ejectionsMaxTotalNumber > 0 && ejectionsOfDiffractionMaxNumber > 0) {
            switchOfOfDiffraction = true;
        }
        if (!ScenarioDataInformationStd::InitScenarioDataInformationByScenarioObject(switchOfOfDiffraction,scenarioObject, scenarioDataInformation)) {
            return false;
        }

        //3.2 空间加速预处理

        if (geometricSpaceAccelerateType == 1) {
            if (!GeometricSpacePartitionProcessingMethod(
                geometricSpaceAccelerateType, 
                scenario, 
                switchOfOfDiffraction, 
                cornerRadius)) {
                return false;
            }
        }


        //内存初始化
        //InitAntennaMIMOAndAntennaMIMOPath(transmittingAntennas, receiverAntennas, antennaMIMO, antennaMIMOPath);

        CalRunTimeStd::CalRunTime CCC(true);
        if (switchOfOfDiffraction) {

            {
                //size_t seg_seg_size = scenarioObject.scenarioCorner3DIndex.size();
                //seg_seg_samepoint.resize(seg_seg_size, std::vector<bool>(seg_seg_size, false));
                //
                //std::vector<std::vector<bool>> seg_seg_samepoint_visited(seg_seg_size, std::vector<bool>(seg_seg_size, false));
                //
                //
                //for (int loop_1 = 0; loop_1 < seg_seg_size; ++loop_1) {
                //    auto seg_index_1 = scenarioObject.scenarioCorner3DIndex[loop_1];
                //    auto seg_1_start = scenarioObject.scenarioPoint3D[seg_index_1.P1Index];
                //    auto seg_1_end = scenarioObject.scenarioPoint3D[seg_index_1.P2Index];
                //    for (int loop_2 = 0; loop_2 < seg_seg_size; ++loop_2) {
                //        if (seg_seg_samepoint_visited[loop_1][loop_2]) {
                //            continue;
                //        }
                //
                //        if (loop_1 == loop_2) {
                //            seg_seg_samepoint[loop_1][loop_2] = true;
                //            seg_seg_samepoint_visited[loop_1][loop_2] = true;
                //        }
                //        else {
                //            bool state = false;
                //            {
                //                auto seg_index_2 = scenarioObject.scenarioCorner3DIndex[loop_2];
                //                auto seg_2_start = scenarioObject.scenarioPoint3D[seg_index_2.P1Index];
                //                auto seg_2_end = scenarioObject.scenarioPoint3D[seg_index_2.P2Index];
                //                if (Geometry3DIntersectStd::Intersect_LineSegment3D_LineSegment3D_plus(seg_2_start, seg_2_end, seg_1_start, seg_1_end)) {
                //                    state = true;
                //                }
                //
                //            }
                //
                //            seg_seg_samepoint[loop_1][loop_2] = state;
                //            seg_seg_samepoint[loop_2][loop_1] = state;
                //            seg_seg_samepoint_visited[loop_1][loop_2] = true;
                //            seg_seg_samepoint_visited[loop_2][loop_1] = true;
                //        }
                //
                //    }
                //}
            }
            
            {
               size_t seg_seg_size = scenarioObject.scenarioCorner3DIndex.size();
               seg_seg_samepoint.resize(seg_seg_size, std::vector<bool>(seg_seg_size, false));
               
               std::vector<std::vector<bool>> seg_seg_samepoint_visited(seg_seg_size, std::vector<bool>(seg_seg_size, false));
               
               std::unordered_map<int, std::unordered_set<int>> point_seg_index_map;
               
               for (int loop_1 = 0; loop_1 < seg_seg_size; ++loop_1) {
                   auto seg_index_1 = scenarioObject.scenarioCorner3DIndex[loop_1];
                   point_seg_index_map[seg_index_1.P1Index].insert(loop_1);
                   point_seg_index_map[seg_index_1.P2Index].insert(loop_1);
               
                   seg_seg_samepoint[loop_1][loop_1] = true;
                   seg_seg_samepoint_visited[loop_1][loop_1] = true;
               }
               
               for (auto& ele_1 : point_seg_index_map) {
                   std::unordered_set<int> seg_index = ele_1.second;
                   if (seg_index.size() > 1) {
                       std::vector<int> index_seg_vector;
                       index_seg_vector.insert(index_seg_vector.end(), seg_index.begin(), seg_index.end());
               
                       for (int loop_1 = 0; loop_1 < (int)index_seg_vector.size() - 1; ++loop_1) {
                           int seg_1_index = index_seg_vector[loop_1];
                           for (int loop_2 = loop_1 + 1; loop_2 < index_seg_vector.size(); ++loop_2) {
                               int seg_2_index = index_seg_vector[loop_2];
                               if (seg_seg_samepoint_visited[loop_1][loop_2]) {
                                   continue;
                               }
                               bool state = false;
                               {
                                   auto seg_index_1 = scenarioObject.scenarioCorner3DIndex[seg_1_index];
                                   auto seg_1_start = scenarioObject.scenarioPoint3D[seg_index_1.P1Index];
                                   auto seg_1_end = scenarioObject.scenarioPoint3D[seg_index_1.P2Index];
               
                                   auto seg_index_2 = scenarioObject.scenarioCorner3DIndex[seg_2_index];
                                   auto seg_2_start = scenarioObject.scenarioPoint3D[seg_index_2.P1Index];
                                   auto seg_2_end = scenarioObject.scenarioPoint3D[seg_index_2.P2Index];
                                   if (Geometry3DIntersectStd::Intersect_LineSegment3D_LineSegment3D_plus(seg_2_start, seg_2_end, seg_1_start, seg_1_end)) {
                                       state = true;
                                   }
               
                               }
               
                               seg_seg_samepoint[loop_1][loop_2] = state;
                               seg_seg_samepoint[loop_2][loop_1] = state;
                               seg_seg_samepoint_visited[loop_1][loop_2] = true;
                               seg_seg_samepoint_visited[loop_2][loop_1] = true;
                           }
                       }
               
                   }
               }

            }

        }

        {
            {
                //size_t triangle_triangle_size = scenarioObject.scenarioTriangle3DIndex.size();
                //triangle_triangle_sameside.resize(triangle_triangle_size, std::vector<bool>(triangle_triangle_size, false));
                //
                //std::vector<std::vector<bool>> triangle_triangle_sameside_visited(triangle_triangle_size, std::vector<bool>(triangle_triangle_size, false));
                //
                //
                //for (int loop_1 = 0; loop_1 < triangle_triangle_size; ++loop_1) {
                //    auto triangle_index_1 = scenarioObject.scenarioTriangle3DIndex[loop_1];
                //    auto triangle_index_1_p1 = triangle_index_1.TriangleP1Index;
                //    auto triangle_index_1_p2 = triangle_index_1.TriangleP2Index;
                //    auto triangle_index_1_p3 = triangle_index_1.TriangleP3Index;
                //
                //    auto triangle_1_n = triangle_index_1.n;
                //
                //    for (int loop_2 = 0; loop_2 < triangle_triangle_size; ++loop_2) {
                //        if (triangle_triangle_sameside_visited[loop_1][loop_2]) {
                //            continue;
                //        }
                //
                //        if (loop_1 == loop_2) {
                //            triangle_triangle_sameside[loop_1][loop_2] = true;
                //            triangle_triangle_sameside_visited[loop_1][loop_2] = true;
                //        }
                //        else {
                //            bool state = false;
                //            {
                //                auto triangle_index_2 = scenarioObject.scenarioTriangle3DIndex[loop_2];
                //                auto triangle_index_2_p1 = triangle_index_2.TriangleP1Index;
                //                auto triangle_index_2_p2 = triangle_index_2.TriangleP2Index;
                //                auto triangle_index_2_p3 = triangle_index_2.TriangleP3Index;
                //                std::unordered_set<int> num_index_set;
                //                num_index_set.insert(triangle_index_1_p1);
                //                num_index_set.insert(triangle_index_1_p2);
                //                num_index_set.insert(triangle_index_1_p3);
                //
                //                num_index_set.insert(triangle_index_2_p1);
                //                num_index_set.insert(triangle_index_2_p2);
                //                num_index_set.insert(triangle_index_2_p3);
                //                if (num_index_set.size() < 5) {
                //                    state = true;
                //                    auto triangle_2_n = triangle_index_2.n;
                //                    if (!Geometry3DOperateStd::Equals_Point3D_N(triangle_1_n, triangle_2_n)) {
                //                        state = false;
                //                    }
                //                }
                //
                //
                //            }
                //
                //            triangle_triangle_sameside[loop_1][loop_2] = state;
                //            triangle_triangle_sameside[loop_2][loop_1] = state;
                //            triangle_triangle_sameside_visited[loop_1][loop_2] = true;
                //            triangle_triangle_sameside_visited[loop_2][loop_1] = true;
                //        }
                //
                //    }
                //}
            }

            {
                size_t triangle_triangle_size = scenarioObject.scenarioTriangle3DIndex.size();
                triangle_triangle_sameside.resize(triangle_triangle_size, std::vector<bool>(triangle_triangle_size, false));

                std::vector<std::vector<bool>> triangle_triangle_sameside_visited(triangle_triangle_size, std::vector<bool>(triangle_triangle_size, false));

                std::unordered_map<int, std::unordered_set<int>> point_triangle_index_map;
                for (int loop_1 = 0; loop_1 < triangle_triangle_size; ++loop_1) {
                    auto triangle_index_1 = scenarioObject.scenarioTriangle3DIndex[loop_1];
                    auto triangle_index_1_p1 = triangle_index_1.TriangleP1Index;
                    auto triangle_index_1_p2 = triangle_index_1.TriangleP2Index;
                    auto triangle_index_1_p3 = triangle_index_1.TriangleP3Index;
                    point_triangle_index_map[triangle_index_1_p1].insert(loop_1);
                    point_triangle_index_map[triangle_index_1_p2].insert(loop_1);
                    point_triangle_index_map[triangle_index_1_p3].insert(loop_1);

                    triangle_triangle_sameside[loop_1][loop_1] = true;
                    triangle_triangle_sameside_visited[loop_1][loop_1] = true;
                }

                for (auto& ele_1 : point_triangle_index_map) {
                    std::unordered_set<int> triangle_index = ele_1.second;
                    if (triangle_index.size() > 1) {
                        std::vector<int> index_triangle_vector;
                        index_triangle_vector.insert(index_triangle_vector.end(), triangle_index.begin(), triangle_index.end());

                        for (int loop_1 = 0; loop_1 < (int)index_triangle_vector.size()-1; ++loop_1) {
                            int triangle_1_index = index_triangle_vector[loop_1];
                            for (int loop_2 = loop_1 + 1; loop_2 < index_triangle_vector.size(); ++loop_2) {
                                int triangle_2_index = index_triangle_vector[loop_2];
                                if (triangle_triangle_sameside_visited[loop_1][loop_2]) {
                                    continue;
                                }
                                bool state = false;
                                {
                                    auto triangle_index_1 = scenarioObject.scenarioTriangle3DIndex[triangle_1_index];
                                    auto triangle_index_1_p1 = triangle_index_1.TriangleP1Index;
                                    auto triangle_index_1_p2 = triangle_index_1.TriangleP2Index;
                                    auto triangle_index_1_p3 = triangle_index_1.TriangleP3Index;
                                    auto triangle_index_2 = scenarioObject.scenarioTriangle3DIndex[triangle_2_index];
                                    auto triangle_index_2_p1 = triangle_index_2.TriangleP1Index;
                                    auto triangle_index_2_p2 = triangle_index_2.TriangleP2Index;
                                    auto triangle_index_2_p3 = triangle_index_2.TriangleP3Index;
                                    std::unordered_set<int> num_index_set;
                                    num_index_set.insert(triangle_index_1_p1);
                                    num_index_set.insert(triangle_index_1_p2);
                                    num_index_set.insert(triangle_index_1_p3);

                                    num_index_set.insert(triangle_index_2_p1);
                                    num_index_set.insert(triangle_index_2_p2);
                                    num_index_set.insert(triangle_index_2_p3);
                                    if (num_index_set.size() < 5) {
                                        state = true;
                                        auto triangle_1_n = triangle_index_1.n;
                                        auto triangle_2_n = triangle_index_2.n;
                                        if (!Geometry3DOperateStd::Equals_Point3D_N(triangle_1_n, triangle_2_n)) {
                                            state = false;
                                        }
                                    }


                                }

                                triangle_triangle_sameside[loop_1][loop_2] = state;
                                triangle_triangle_sameside[loop_2][loop_1] = state;
                                triangle_triangle_sameside_visited[loop_1][loop_2] = true;
                                triangle_triangle_sameside_visited[loop_2][loop_1] = true;
                            }
                        }

                    }
                }

            }
        }

        return true;
    }




}