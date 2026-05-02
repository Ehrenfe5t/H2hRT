

#include"../Input.h"

namespace Geometry2DOperateStd {

    /// <summary>
     /// 创建一个点坐标
     /// </summary>
     /// <param name="x"></param>
     /// <param name="y"></param>
     /// <param name="z"></param>
     /// <returns></returns>
    Point2DStd::Point2D CreatePoint2D(double x, double y) {
        Point2DStd::Point2D res;
        res.x = x;
        res.y = y;
        return res;
    }


    /// <summary>
    /// 计算两个点的差
    /// </summary>
    /// <param name="p1">点1</param>
    /// <param name="p2">点2</param>
    /// <returns>两个点的差</returns>
    Point2DStd::Point2D SubPoint2DPoint2D(const Point2DStd::Point2D& p1, const Point2DStd::Point2D& p2)
    {
        return CreatePoint2D(p1.x - p2.x, p1.y - p2.y);
    }

    /// <summary>
     /// 点乘计算
     /// </summary>
     /// <param name="p1"></param>
     /// <param name="p2"></param>
     /// <returns></returns>
    double DotPoint2DPoint2D(const Point2DStd::Point2D& p1, const Point2DStd::Point2D& p2)
    {
        return p1.x * p2.x + p1.y * p2.y;
    }
    /// <summary>
    /// 将一个向量放大或缩小
    /// </summary>
    /// <param name="d">zoom</param>
    /// <param name="p">向量</param>
    /// <returns>缩放后的向量</returns>
    Point2DStd::Point2D MulDoublePoint2D(double d, const Point2DStd::Point2D& p)
    {
        return CreatePoint2D(d * p.x, d * p.y);
    }

    /// <summary>
    /// 计算向量长度
    /// </summary>
    /// <param name="p"></param>
    /// <returns></returns>
    double Length_Point2D(const Point2DStd::Point2D& p)
    {
        return sqrt(DotPoint2DPoint2D(p, p));
    }

    Point2DStd::Point2D Normalization_Point2D_unsafe(const Point2DStd::Point2D& p)
    {
        double len = Length_Point2D(p);
        return MulDoublePoint2D(1 / len, p);
    }
    bool Normalization_Point2D_safe(const Point2DStd::Point2D& p, Point2DStd::Point2D& res)
    {
        double len = Length_Point2D(p);
        if (len < GlobalConstantStd::MaxEps)
        {
            return false;
        }
        res = MulDoublePoint2D(1 / len, p);
        return true;
    }


    Point2DStd::Point2D AddPoint2DPoint2D(const Point2DStd::Point2D& p1, const Point2DStd::Point2D& p2)
    {
        return CreatePoint2D(p1.x + p2.x, p1.y + p2.y);
    }

    bool Equals_Point2D(const Point2DStd::Point2D& p1, const Point2DStd::Point2D& p2)
    {
        Point2DStd::Point2D temp111 = SubPoint2DPoint2D(p1, p2);
        double dis = Length_Point2D(temp111);
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
    bool Equals_Point2D_N(const Point2DStd::Point2D& n1, const Point2DStd::Point2D& n2)
    {
        if (Equals_Point2D(n1, n2))
        {
            return true;
        }
        Point2DStd::Point2D n = AddPoint2DPoint2D(n1, n2);

        double length = Length_Point2D(n);

        if (length <= GlobalConstantStd::Eps)
        {
            return true;
        }
        return false;
    }


    double GetAnglePoint2DPoint2D_unsafe(const Point2DStd::Point2D& a, const Point2DStd::Point2D& b)
    {
        double f = Length_Point2D(a) * Length_Point2D(b);
        double d1 = DotPoint2DPoint2D(a, b);
        return acos(abs(d1) / f);
    }

    double GetDistancePoint2DLine2D_plus_unsafe(
        const Point2DStd::Point2D& p, 
        const Point2DStd::Point2D& lineO, 
        const Point2DStd::Point2D& lineVec)
    {
        Point2DStd::Point2D op = SubPoint2DPoint2D(p,lineO);
        double op_len = Length_Point2D(op);

        if (op_len < GlobalConstantStd::Eps)
        {
            return 0.0;
        }

        double theta = GetAnglePoint2DPoint2D_unsafe(op, lineVec);
        return op_len * sin(theta);

    }

    double Cross(const Point2DStd::Point2D& A, const Point2DStd::Point2D& B)
    {
        return A.x * B.y - B.x * A.y;
    }
    Point2DStd::Point2D Intersect_Line2D_Line2D_plus_unsafe(
        const Point2DStd::Point2D& A, 
        const Point2DStd::Point2D& v1_normalized,
        const Point2DStd::Point2D& B, 
        const Point2DStd::Point2D& v2_normalized,
        double& distance)
    {
        Point2DStd::Point2D v =SubPoint2DPoint2D(B , A) ;
        double m = Cross(v, v1_normalized);
        double n = Cross(v1_normalized, v2_normalized);
        double t = m / n;
        distance = t;
        Point2DStd::Point2D res = AddPoint2DPoint2D(B,MulDoublePoint2D(t, v2_normalized));
        return res;
    }

}

namespace Geometry3DOperateStd {

    /// <summary>
    /// 计算点的单位向量
    /// </summary>
    /// <param name="p">点</param>
    /// <returns>点的单位向量</returns>
    Point3DStd::Point3D Normalization_Point3D(const Point3DStd::Point3D& p)
    {
        double len = Length_Point3D(p);
        if (len < GlobalConstantStd::Eps * GlobalConstantStd::Eps)
        {
            ProjectDependenciesStd::DisplayPromptOrReason("Point3DNormalization传入了非法参数", true, __FILE__, __LINE__);
            return CreatePoint3D(0.0, 0.0, 0.0);
        }
        return MulDoublePoint3D(1 / len, p);
    }

    Point3DStd::Point3D Normalization_Point3D_unsafe(const Point3DStd::Point3D& p)
    {
        double len = Length_Point3D(p);
        return MulDoublePoint3D(1 / len, p);
    }

    

    bool Normalization_Point3D_safe(const Point3DStd::Point3D& p, Point3DStd::Point3D& res)
    {
        double len = Length_Point3D(p);
        if (len < GlobalConstantStd::MaxEps)
        {
            return false;
        }
        AssignmentPoint3DPoint3D(res, MulDoublePoint3D(1 / len, p));
        return true;
    }
}