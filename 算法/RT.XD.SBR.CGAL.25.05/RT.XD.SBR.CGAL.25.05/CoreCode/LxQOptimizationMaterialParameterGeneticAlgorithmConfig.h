#pragma once


#include"LxQMaterialParameterChromDataBoundaryConfig.h"
#include"LxQOptimizationMaterialParameterConfig.h"

namespace OptimizationMaterialParameterGeneticAlgorithmConfigStd {

	class OptimizationMaterialParameterGeneticAlgorithmConfig
	{
	public:

        /// <summary>
        /// 种群数量
        /// </summary>
        int populationNumber;

        /// <summary>
        /// 种群遗传次数
        /// </summary>
        int populationGenerationNumber;

        //不进行交叉、变异的优秀个体数量
        int retainedExcellentIndividualNumber;

        /// <summary>
        /// 每次淘汰的个体数量，将会生成新的个体
        /// </summary>
        int eliminatedIndividualsNumber;

        /// <summary>
        /// 交叉概率
        /// </summary>
        double crossoverProbability ;

        /// <summary>
        /// 变异概率
        /// </summary>
        double mutationProbability;

        /// <summary>
        /// 目标值
        /// </summary>
        double searchRmseGoal;

        bool useBestMaterialParameterIndividual;

        bool usingRoadTestData;

        std::string bestMaterialParameterIndividualCsvFileName;

        std::vector<MaterialParameterChromDataBoundaryConfigStd::MaterialParameterChromDataBoundaryConfig> materialParameterChromDataBoundaryConfigs;

		OptimizationMaterialParameterConfigStd::OptimizationMaterialParameterConfig optimizationMaterialParameterConfig;

		OptimizationMaterialParameterGeneticAlgorithmConfig();
		~OptimizationMaterialParameterGeneticAlgorithmConfig();

	private:

	};

}

