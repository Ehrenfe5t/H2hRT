
#include"DxQVectorOperate.h"
#include"LxQProjectDependencies.h"


namespace VectorOperateStd {
    int ContainIntInVectorInt(int value, const std::vector<int>& list) {
        for (int i = 0; i < list.size(); i++) {
            if (list[i] == value) {
                return i;
            }
        }
        return -1;
    }


    int ContainUshortInVectorUshort(unsigned short value, const std::vector<unsigned short>& list) {
        for (int i = 0; i < list.size(); i++) {
            if (list[i] == value) {
                return i;
            }
        }
        return -1;
    }

    int ContainShortInVectorShort(short value, const std::vector<short>& list) {
        for (int i = 0; i < list.size(); i++) {
            if (list[i] == value) {
                return i;
            }
        }
        return -1;
    }

    void IntVectorAddIntVectorRemoveDuplicateElements(const std::vector<int>& list1, std::vector<int>& list2) {
        for (int i = 0; i < list1.size(); ++i) {
            if (-1 == ContainIntInVectorInt(list1[i], list2)) {
                list2.push_back(list1[i]);
            }
        }
    }


    void UshortVectorAddUshortVectorRemoveDuplicateElements(const std::vector<unsigned short>& list1, std::vector<unsigned short>& list2) {
        for (int i = 0; i < list1.size(); ++i) {
            if (-1 == ContainUshortInVectorUshort(list1[i], list2)) {
                list2.push_back(list1[i]);
            }
        }
    }

    void ShortVectorAddShortVectorRemoveDuplicateElements(const std::vector<short>& list1, std::vector<short>& list2) {
        for (int i = 0; i < list1.size(); ++i) {
            if (-1 == ContainShortInVectorShort(list1[i], list2)) {
                list2.push_back(list1[i]);
            }
        }
    }



    double GetMean(const std::vector<double>& data) {
        double sum = 0;
        for (int i = 0; i < data.size(); i++) {
            sum += data[i];
        }
        return sum /= data.size();
    }


    double CalMin(const std::vector<double>& data) {
        double sum = 1e308;
        for (int i = 0; i < data.size(); i++) {
            if (sum > data[i]) {
                sum = data[i];
            }
        }
        return sum;
    }
    double CalMax(const std::vector<double>& data) {
        double sum = -1e308;
        for (int i = 0; i < data.size(); i++) {
            if (sum < data[i]) {
                sum = data[i];
            }
        }
        return sum;
    }

    /// <summary>
    /// 计算RMSE
    /// </summary>
    /// <param name="powerMea"></param>
    /// <param name="powerVectorSim"></param>
    /// <returns></returns>
    double GetRmse(const std::vector<double>& powerMea, const std::vector<double>& powerVectorSim) {
        if (powerMea.size() != powerVectorSim.size()) {
            ProjectDependenciesStd::DisplayPromptOrReason("计算Rmse的两个数据集必须样本数量相同.", true, __FILE__, __LINE__);
            return 11111110.0;
        }
        double sum = 0.0;
        for (int i = 0; i < powerMea.size(); i++) {
            double d = powerMea[i] - powerVectorSim[i];
            sum += d * d;
        }
        sum /= powerMea.size();
        return sqrt(sum);
    }

    /// <summary>
    /// 均值相等后计算RMSE
    /// </summary>
    /// <param name="powerMea"></param>
    /// <param name="powerVectorSim"></param>
    /// <returns></returns>
    double GetRmseFrontMean(const std::vector<double>& powerMea, const std::vector<double>& powerVectorSim) {
        double meanMea = GetMean(powerMea);
        double meanSim = GetMean(powerVectorSim);
        double temp = (meanSim - meanMea);
        std::vector<double> powerSim;
        for (int i = 0; i < powerVectorSim.size(); i++) {
            powerSim.push_back(powerVectorSim[i] - temp);
        }
        return GetRmse(powerMea, powerSim);
    }


    /// <summary>
    /// 利用实测数据先对齐最大值，将此时比实测结果最小的值还低的结果改为实测值的最小值，然后按均值平移
    /// </summary>
    /// <param name="powerSim"></param>
    /// <param name="powerMea"></param>
    /// <returns></returns>
    std::vector<double> UseTheMinValueAsTheBaseNoiseToOffsetTheImpactOfTxPowerByVector(const std::vector<double>& powerSim, const std::vector<double>& powerMea) {
        std::vector<double> res;
        double meanMeaValue = GetMean(powerMea);
        double minValueMea = CalMin(powerMea);
        double maxValueMea = CalMax(powerMea);
        double maxValueSim = CalMax(powerSim);
        //把最大值对齐
        std::vector<double> temp1;
        std::vector<int> minIndex;
        double temp0 = maxValueSim - maxValueMea;
        for (int i = 0; i < powerSim.size(); i++) {
            temp1.push_back(powerSim[i] - temp0);
            if (temp1[i] < minValueMea) {
                temp1[i] = minValueMea;
            }
        }

        double meanSimValue = GetMean(powerSim);
        double temp2 = meanSimValue - meanMeaValue;
        //先经过第一次平移
        for (int i = 0; i < temp1.size(); i++) {
            res.push_back(temp1[i] - temp2);
        }
        return res;
    }

    std::vector<double> UseTheMinValueAsTheBaseNoiseToOffsetTheImpactOfTxPower(const std::vector<double>& powerSim,
        double meanMeaValue, double minValueMea, double maxValueMea) {
        std::vector<double> res;
        double maxValueSim = CalMax(powerSim);
        //把最大值对齐
        std::vector<double> temp1;

        double temp0 = maxValueSim - maxValueMea;
        for (int i = 0; i < powerSim.size(); i++) {
            temp1.push_back(powerSim[i] - temp0);
            if (temp1[i] < minValueMea) {
                temp1[i] = minValueMea;
            }
        }

        double meanSimValue = GetMean(temp1);
        double temp2 = meanSimValue - meanMeaValue;
        //先经过第一次平移
        for (int i = 0; i < temp1.size(); i++) {
            res.push_back(temp1[i] - temp2);
        }
        return res;
    }


}