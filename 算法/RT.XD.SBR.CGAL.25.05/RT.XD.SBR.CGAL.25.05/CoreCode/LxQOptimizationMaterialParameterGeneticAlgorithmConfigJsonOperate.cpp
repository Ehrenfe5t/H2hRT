
#include"LxQOptimizationMaterialParameterGeneticAlgorithmConfigJsonOperate.h"

#include"LxQOptimizationMaterialParameterConfigJsonOperate.h"
#include"LxQMaterialParameterChromDataBoundaryConfigJsonOperate.h"

namespace OptimizationMaterialParameterGeneticAlgorithmConfigStd {
    /// <summary>
    /// 从json字符串获取点
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void from_json(const nlohmann::json& j, OptimizationMaterialParameterGeneticAlgorithmConfigStd::OptimizationMaterialParameterGeneticAlgorithmConfig& obj) {
        {
            auto jsonObject = j.at("crossoverProbability");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.crossoverProbability);
            }
        }
        {
            auto jsonObject = j.at("eliminatedIndividualsNumber");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.eliminatedIndividualsNumber);
            }
        }
        {
            auto jsonObject = j.at("mutationProbability");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.mutationProbability);
            }
        }
        {
            auto jsonObject = j.at("optimizationMaterialParameterConfig");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.optimizationMaterialParameterConfig);
            }
        }
        {
            auto jsonObject = j.at("populationGenerationNumber");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.populationGenerationNumber);
            }
        }
        {
            auto jsonObject = j.at("populationNumber");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.populationNumber);
            }
        }
        {
            auto jsonObject = j.at("retainedExcellentIndividualNumber");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.retainedExcellentIndividualNumber);
            }
        }
        {
            auto jsonObject = j.at("useBestMaterialParameterIndividual");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.useBestMaterialParameterIndividual);
            }
        }
        {
            auto jsonObject = j.at("usingRoadTestData");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.usingRoadTestData);
            }
        }
        {
            auto jsonObject = j.at("searchRmseGoal");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.searchRmseGoal);
            }
        }
        {
            auto jsonObject = j.at("bestMaterialParameterIndividualCsvFileName");
            if (!jsonObject.is_null()) {
                jsonObject.get_to(obj.bestMaterialParameterIndividualCsvFileName);
            }
        }
        {
            for (auto& object : j["dataBoundarys"]) {
                MaterialParameterChromDataBoundaryConfigStd::MaterialParameterChromDataBoundaryConfig value;
                from_json(object, value);
                obj.materialParameterChromDataBoundaryConfigs.emplace_back(value);
            }
        }
    }

    /// <summary>
    /// 将点对象转化为json字符串
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void to_json(nlohmann::json& j, const OptimizationMaterialParameterGeneticAlgorithmConfigStd::OptimizationMaterialParameterGeneticAlgorithmConfig& obj) {

        j["crossoverProbability"] = obj.crossoverProbability;
        j["eliminatedIndividualsNumber"] = obj.eliminatedIndividualsNumber;
        j["mutationProbability"] = obj.mutationProbability;
        j["optimizationMaterialParameterConfig"] = obj.optimizationMaterialParameterConfig;
        j["populationGenerationNumber"] = obj.populationGenerationNumber;
        j["populationNumber"] = obj.populationNumber;
        j["retainedExcellentIndividualNumber"] = obj.retainedExcellentIndividualNumber;
        j["searchRmseGoal"] = obj.searchRmseGoal;

        j["useBestMaterialParameterIndividual"] = obj.useBestMaterialParameterIndividual;
        j["usingRoadTestData"] = obj.usingRoadTestData;
        j["bestMaterialParameterIndividualCsvFileName"] = obj.bestMaterialParameterIndividualCsvFileName;

        j["dataBoundarys"];
        for (auto& value : obj.materialParameterChromDataBoundaryConfigs) {
            nlohmann::json jf;
            to_json(jf, value);
            j["dataBoundarys"].emplace_back(jf);
        }
    }
}