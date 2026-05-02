

#include"../Input.h"

namespace Geometry3DOperateStd {


    /// <summary>
    /// 传入的是一个>=0的数字
    /// </summary>
    /// <param name="d"></param>
    /// <returns></returns>
    bool IsZero(double d) {
        if (d <= GlobalConstantStd::Eps)
        {
            return true;
        }
        return false;
    }


    bool IsZeroAbs(double d) {
        return IsZero(abs(d));
    }



    /// <summary>
    /// 两个点是否相同,绝对相等
    /// </summary>
    /// <param name="a"></param>
    /// <param name="b"></param>
    /// <returns></returns>
    bool Equals_Point3D_Absolute(const Point3DStd::Point3D& a, const Point3DStd::Point3D& b) {
        if (a.x == b.x) {
            if (a.y == b.y) {
                if (a.z == b.z) {
                    return true;
                }
            }
        }
        return false;
    }

    /// <summary>
     /// 考虑计算误差计算两个点是否相等
     /// </summary>
     /// <param name="p1"></param>
     /// <param name="p2"></param>
     /// <param name="eps"></param>
     /// <returns></returns>
    bool Equals_Point3D(const Point3DStd::Point3D& p1, const Point3DStd::Point3D& p2)
    {
        Point3DStd::Point3D temp111 = SubPoint3DPoint3D(p1, p2);
        double dis = Length_Point3D(temp111);
        if (dis <= GlobalConstantStd::Eps)
        {
            return true;
        }
        return false;
    }


    /// <summary>
    /// 判断平面的法向量是否相同，180度或者0度都为真
    /// </summary>
    /// <param name="n1"></param>
    /// <param name="n2"></param>
    /// <param name="eps"></param>
    /// <returns></returns>
    bool Equals_Point3D_N(const Point3DStd::Point3D& n1, const Point3DStd::Point3D& n2)
    {
        if (Equals_Point3D(n1, n2))
        {
            return true;
        }
        Point3DStd::Point3D n = AddPoint3DPoint3D(n1, n2);

        double length = Length_Point3D(n);

        if (length <= GlobalConstantStd::Eps)
        {
            return true;
        }
        return false;
    }


}