#pragma once


namespace GlobalConstantStd {

    /// <summary>
    /// 计算误差
    /// </summary>
    const double Eps = 1e-6;
    /// <summary>
    /// 更高精度
    /// </summary>
    const double MoreEps = 1e-9;
    /// <summary>
    /// 更高精度
    /// </summary>
    const double MaxEps = 1e-12;
    /// <summary>
    /// 圆周率
    /// </summary>
    const double Pi = 3.141592653589793238462643383279;
    /// <summary>
    /// 光速
    /// </summary>
    const double C = 299792458.0;

    /// <summary>
    /// 计算边界范围上限
    /// </summary>
    const double BoundingBoxLength = 1e9;

    /// <summary>
    /// 自由空间特征阻抗
    /// </summary>
    const double FreeSpaceCharacteristicImpedance = 376.99111843077515;

    /// <summary>
    /// 真空介电常数
    /// </summary>
    const double RelativePermittivity_0 = 8.854187817 * 1e-12;

    /// <summary>
    /// 真空磁导率
    /// </summary>
    const double RelativePermeability_0 = 1.256637061435917e-06;

    const bool checkReflectCoefficient = true;



    int GetAirSubstanceType();
    void SetAirSubstanceType(int type);

    double GetBoundingBoxPadding();
    void SetBoundingBoxPadding(double padding);

    double GetNumberOfDiscreteBoundingBox();

    void SetNumberOfDiscreteBoundingBox(double numberOfDiscreteBoundingBox_value);
}