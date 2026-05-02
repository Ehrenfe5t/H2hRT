#pragma once



namespace MathOperateStd {

    /// <summary>
    /// 考虑计算误差，判读一个属是否为0，误差为参数
    /// </summary>
    /// <param name="d"></param>
    /// <param name="eps"></param>
    /// <returns>1相等，0不相等</returns>
    int OneNumberIsZeroByEps(double d);
    int OneNumberIsZeroByEpsPlus(double d);

    double GetAngleByXY(double x, double y);

    /// <summary>
    /// 计算折射角
    /// </summary>
    /// <param name="e1"></param>
    /// <param name="mu1"></param>
    /// <param name="e2"></param>
    /// <param name="mu2"></param>
    /// <param name="thetai"></param>
    /// <param name="eps"></param>
    /// <param name="thetat"></param>
    /// <returns></returns>
    int GetThetaT(double e1, double mu1, double e2, double mu2, double thetai, double& thetat);
    int GetThetaTPlus(double n2n1, double thetai, double& thetat);
    int GetThetaIByThetaT_unsafe(double e1, double mu1, double e2, double mu2, double thetat, double& thetai);

    double CalMartix33(double num[3][3]);

    double CalDeterminant4_4(double axx[4][4]);
}