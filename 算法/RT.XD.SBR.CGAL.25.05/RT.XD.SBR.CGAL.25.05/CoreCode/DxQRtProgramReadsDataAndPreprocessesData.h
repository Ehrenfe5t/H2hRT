#pragma once


#include"QzQDataInputCsvFileParameterConfig.h"
#include"HdQAntennaMIMO.h"
#include"HdQAntennaMIMOPath.h"
#include"LxQOptimizationMaterialParameterConfig.h"
#include"DxQScenarioDataInformation.h"
#include"DxQRayScenarioIntersectType.h"
#include"../0.DxQCalculateWaveImpactResponseDBmModule/CalculateWaveImpactResponseDBm.Output.h"

namespace RtProgramReadsDataAndPreprocessesDataStd {
    bool GeometricSpacePartitionProcessingMethod(
        int type, const Scenario3D& scenario, bool switchCorner, double cornerRadius);

    bool ReadScenarioPointByCsvModel(
        const std::string& fileName,
        std::vector<double>& x_data,
        std::vector<double>& y_data,
        std::vector<double>& z_data);

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
        std::vector<double>& structScenarioArrayScenarioTriangle3DIndexNZ_data);

    bool ReadScenarioCorner3DIndexByCsvModel(
        const std::string& fileName,
        std::vector<int>& structScenarioArrayScenarioCorner3DIndexP1Index_data,
        std::vector<int>& structScenarioArrayScenarioCorner3DIndexP2Index_data,
        std::vector<int>& structScenarioArrayScenarioCorner3DIndexP3Face0Index_data,
        std::vector<int>& structScenarioArrayScenarioCorner3DIndexP3FaceNIndex_data,
        std::vector<int>& structScenarioArrayScenarioCorner3DIndexFace0Index_data,
        std::vector<int>& structScenarioArrayScenarioCorner3DIndexFaceNIndex_data);

    bool ReadReceiverAntennaByCsvModel(
        const std::string& fileName,
        const std::vector<int>& transmittingAntennaIds,
        std::vector<ReceiverAntennaStd::ReceiverAntenna>& receiverAntennas);

    void InitAntennaMIMOAndAntennaMIMOPath(
        const AntennaMIMOStd::AntennaMIMO& antennaMIMO,
        AntennaMIMOPathStd::AntennaMIMOPath& antennaMIMOPath);
}