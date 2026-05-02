
#include"LxQOptimizationMaterialParameterGeneticAlgorithmConfig.h"

namespace OptimizationMaterialParameterGeneticAlgorithmConfigStd {

	OptimizationMaterialParameterGeneticAlgorithmConfig::OptimizationMaterialParameterGeneticAlgorithmConfig()
	{

        //种群数量
        this->populationNumber = 300;

        //种群遗传次数
        this->populationGenerationNumber = 300;

        //不进行交叉、变异的优秀个体数量
        this->retainedExcellentIndividualNumber = 5;

        //每次淘汰的个体数量，将会生成新的个体
        this->eliminatedIndividualsNumber = 150;

        //交叉概率
        this->crossoverProbability = 0.5;
        //变异概率
        this->mutationProbability = 0.5;

        //目标值
        this->searchRmseGoal = 1e-5;
	}

	OptimizationMaterialParameterGeneticAlgorithmConfig::~OptimizationMaterialParameterGeneticAlgorithmConfig()
	{
	}

}