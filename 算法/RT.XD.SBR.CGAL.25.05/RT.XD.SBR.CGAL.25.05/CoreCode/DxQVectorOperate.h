#pragma once

#include<vector>

namespace VectorOperateStd {
    int ContainIntInVectorInt(int value, const std::vector<int>& list);


    int ContainUshortInVectorUshort(unsigned short value, const std::vector<unsigned short>& list);

    int ContainShortInVectorShort(short value, const std::vector<short>& list);

    void IntVectorAddIntVectorRemoveDuplicateElements(const std::vector<int>& list1, std::vector<int>& list2);

    void UshortVectorAddUshortVectorRemoveDuplicateElements(const std::vector<unsigned short>& list1, std::vector<unsigned short>& list2);

    void ShortVectorAddShortVectorRemoveDuplicateElements(const std::vector<short>& list1, std::vector<short>& list2);



    double GetMean(const std::vector<double>& data);

    double CalMin(const std::vector<double>& data);

    double CalMax(const std::vector<double>& data);

    /// <summary>
    /// 计算RMSE
    /// </summary>
    /// <param name="powerMea"></param>
    /// <param name="powerVectorSim"></param>
    /// <returns></returns>
    double GetRmse(const std::vector<double>& powerMea, const std::vector<double>& powerVectorSim);

    /// <summary>
    /// 均值相等后计算RMSE
    /// </summary>
    /// <param name="powerMea"></param>
    /// <param name="powerVectorSim"></param>
    /// <returns></returns>
    double GetRmseFrontMean(const std::vector<double>& powerMea, const std::vector<double>& powerVectorSim);

    /// <summary>
    /// 利用实测数据先对齐最大值，将此时比实测结果最小的值还低的结果改为实测值的最小值，然后按均值平移
    /// </summary>
    /// <param name="powerSim"></param>
    /// <param name="powerMea"></param>
    /// <returns></returns>
    std::vector<double> UseTheMinValueAsTheBaseNoiseToOffsetTheImpactOfTxPowerByVector(const std::vector<double>& powerSim, const std::vector<double>& powerMea);

    std::vector<double> UseTheMinValueAsTheBaseNoiseToOffsetTheImpactOfTxPower(const std::vector<double>& powerSim,
        double meanMeaValue, double minValueMea, double maxValueMea);


}