
#include"LxQMathOperate.h"

#include"QzQGlobalConstant.h"
#include"LxQProjectDependencies.h"
#include<math.h>
#include<iostream>

namespace MathOperateStd {

    /// <summary>
    /// 考虑计算误差，判读一个属是否为0，误差为参数
    /// </summary>
    /// <param name="d"></param>
    /// <param name="eps"></param>
    /// <returns>1相等，0不相等</returns>
    int OneNumberIsZeroByEps(double d)
    {
        /*if (eps < 0)
        {
            std::cout << "OneNumberIsZeroByEps(double d,double eps)传入了非法参数\n";
            return 0;
        }*/
        if (d > -GlobalConstantStd::Eps && d < GlobalConstantStd::Eps)
        {
            return 1;
        }
        return 0;
    }

    int OneNumberIsZeroByEpsPlus(double d)
    {
        /*if (eps < 0)
        {
            std::cout << "OneNumberIsZeroByEps(double d,double eps)传入了非法参数\n";
            return 0;
        }*/
        if (d > -1e-12 && d < 1e-12)
        {
            return 1;
        }
        return 0;
    }


    double GetAngleByXY(double x, double y) {
        if (OneNumberIsZeroByEps(x) == 1) {
            if (y > 0.0) {
                return 0.5 * GlobalConstantStd::Pi;
            }
            else if (y < 0.0) {
                return 1.5 * GlobalConstantStd::Pi;
            }
            //std::cout << "file =" << __FILE__ << "\tline =" << __LINE__ << "\t";
            //std::cout << "GetAngleByXY warning 1!\n";
            return 0;
        }
        if (OneNumberIsZeroByEps(y) == 1) {
            if (x > 0.0) {
                return 0.0;
            }
            else if (x < 0.0) {
                return GlobalConstantStd::Pi;
            }
            std::cout << "GetAngleByXY warning 2!\n";
            return 0;
        }
        double ddd = atan2(y, x);
        if (y < 0) {
            return 2 * GlobalConstantStd::Pi + ddd;
        }
        return ddd;
    }

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
    int GetThetaT(double relativePermittivity1, double relativePermeability1, double relativePermittivity2, double relativePermeability2, double thetai, double& thetat) {
        if (OneNumberIsZeroByEps(thetai) == 1) {
            thetat = 0.0;
            return 1;
        }
        double t1 = relativePermeability1 * relativePermittivity2;
        if (OneNumberIsZeroByEps(t1) == 1) {
            ProjectDependenciesStd::DisplayPromptOrReason("GetThetaT,计算折射角时出现分母为：0.", true, __FILE__, __LINE__);
            return 0;
        }
        double x = sqrt(relativePermittivity1 * relativePermeability2 / t1) * sin(thetai);
        if (OneNumberIsZeroByEps(x - 1.0f) == 1) {
            thetat = asin(1.0f);
            return 1;
        }
        if (OneNumberIsZeroByEps(x + 1.0f) == 1) {
            thetat = asin(-1.0f);
            return 1;
        }
        if (x < -1 - GlobalConstantStd::Eps) {
            thetat = 0.0;
            return 0;
        }
        if (x > 1 + GlobalConstantStd::Eps) {
            thetat = 0.0;
            return 0;
        }
        thetat = asin(x);
        return 1;
    }

    int GetThetaTPlus(double n2n1, double thetai, double& thetat) {
        if (OneNumberIsZeroByEps(thetai) == 1) {
            thetat = 0.0;
            return 1;
        }
        double x = sqrt(n2n1) * sin(thetai);
        if (OneNumberIsZeroByEps(x - 1.0f) == 1) {
            thetat = asin(1.0f);
            return 1;
        }
        if (OneNumberIsZeroByEps(x + 1.0f) == 1) {
            thetat = asin(-1.0f);
            return 1;
        }
        if (x < -1 - GlobalConstantStd::Eps) {
            thetat = 0.0;
            return 0;
        }
        if (x > 1 + GlobalConstantStd::Eps) {
            thetat = 0.0;
            return 0;
        }
        thetat = asin(x);
        return 1;
    }


    int GetThetaIByThetaT_unsafe(double e1, double mu1, double e2, double mu2, double thetat, double& thetai) {
        double t1 = mu2 * e2;
        if (OneNumberIsZeroByEps(t1) == 1) {
            ProjectDependenciesStd::DisplayPromptOrReason("GetThetaIByThetaT_unsafe,计算折射角时出现分母为：0.", true, __FILE__, __LINE__);
            return 0;
        }
        double tempd1 = sqrt(e1 * mu1 / t1);

        if (OneNumberIsZeroByEps(tempd1) == 1) {
            ProjectDependenciesStd::DisplayPromptOrReason("GetThetaIByThetaT_unsafe,计算折射角时出现分母为：0.", true, __FILE__, __LINE__);
            return 0;
        }

        double x = sin(thetat) / tempd1;
        if (OneNumberIsZeroByEps(x - 1.0f) == 1) {
            thetai = asin(1.0f);
            return 1;
        }
        if (OneNumberIsZeroByEps(x + 1.0f) == 1) {
            thetai = asin(-1.0f);
            return 1;
        }
        if (x < -1 - GlobalConstantStd::Eps) {
            thetai = 0.0;
            return 0;
        }
        if (x > 1 + GlobalConstantStd::Eps) {
            thetai = 0.0;
            return 0;
        }
        thetai = asin(x);
        return 1;
    }

    double CalMartix33(double num[3][3]) {
        return num[0][0] * num[1][1] * num[2][2] +
            num[0][1] * num[1][2] * num[2][0] +
            num[0][2] * num[1][0] * num[2][1] -
            num[2][0] * num[1][1] * num[0][2] -
            num[2][1] * num[1][2] * num[0][0] -
            num[1][0] * num[0][1] * num[2][2];
    }

    /// <summary>
    /// 计算4*4的行列式结果,此处是JvZhen函数的输入值，此处需要输入一个二维矩阵。
    /// </summary>
    /// <param name="axx"></param>
    /// <returns></returns>
    double CalDeterminant4_4(double axx[4][4])
    {
        int a, b, c, d;//此处的逻辑为比方说一个4x4的矩阵竖列循环排列
        //{1,2,3,4}那么a对应[1,1,1,1]这一列
        //{1,2,3,4}    b对应[2,2,2,2]这一列
        //{1,2,3,4}    c对应[3,3,3,3]这一列
        //{1,2,3,4}    d对应[4,4,4,4]这一列
        int i = 0, k = 0, j = 0;
        double res = 0;//i,j,k为算数需要的工具数字。这类东西我
        //一般不太愿意用于过长的命名，影响思路。
        // 但是还是推荐写出目的名字。
        //因为后期计算量大的时候名称指代不清实在闹人眼睛。
        for (a = 1; a <= 4; a++)
        {
            for (b = 1; b <= 4; b++)
            {
                for (c = 1; c <= 4; c++)
                {
                    if (b == a) { break; }//作用同下
                    for (d = 1; d <= 4; d++)
                    {
                        if (c == b || c == a) { break; }      //去掉不必要的计算，如果a=b或者
                        //两两相等的话就不需要遍历下去了，
                        //调试的时候你就会明白，
                        //break能拯救你的时间
//这是一个判定式，原理很简单整个循环4次嵌套，筛选出需要的。筛选的依据在下面*1中解释。
                        if (d != c && d != b && d != a && c != b && c != a && b != a)
                        {
                            //printf("| %d,%d,%d,%d,|",a,b,c,d);此处是打印出行列式的各项，
                            //在此处后面的代码本来是需要做奇偶排列的筛选但是，
                            //就排列有捷径可循在下面*2处解释
                            i++;
                            k = i % 4;
                            if (k == 0 || k == 1)
                            {
                                res = res + axx[0][a - 1] * axx[1][b - 1] * axx[2][c - 1] * axx[3][d - 1];
                            }

                            else if (k == 2 || k == 3)
                            {
                                res = res - axx[0][a - 1] * axx[1][b - 1] * axx[2][c - 1] * axx[3][d - 1];
                            }
                        }
                    }
                }
            }
        }
        return res;
    }
}



