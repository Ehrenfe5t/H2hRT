


#include"../Input.h"

namespace Geometry3DOperateStd {

    /// <summary>
    /// 计算一个三角形的法向量，单位化，如果是长度为0的向量提示结果
    /// </summary>
    /// <param name="triangle">三角形</param>
    /// <returns></returns>
    Point3DStd::Point3D NormalVector_Triangle_plus_unsafe(const Point3DStd::Point3D& p1, const Point3DStd::Point3D& p2, const Point3DStd::Point3D& p3)
    {
        Point3DStd::Point3D n1 = SubPoint3DPoint3D(p1, p2);
        Point3DStd::Point3D n2 = SubPoint3DPoint3D(p1, p3);
        Point3DStd::Point3D temp111 = CrossPoint3DPoint3D(n1, n2);
        return Normalization_Point3D(temp111);
    }

    Point3DStd::Point3D NormalVector_Triangle_unsafe(const Triangle3DStd::Triangle3D& triangle) {
        return NormalVector_Triangle_plus_unsafe(triangle.p1, triangle.p2, triangle.p3);
    }


    bool NormalVector_Triangle_plus_safe(const Point3DStd::Point3D& p1, const Point3DStd::Point3D& p2, const Point3DStd::Point3D& p3, Point3DStd::Point3D& res)
    {
        Point3DStd::Point3D n1 = SubPoint3DPoint3D(p1, p2);
        Point3DStd::Point3D n2 = SubPoint3DPoint3D(p1, p3);
        if (CrossPoint3DPoint3D_safe(n1, n2, res)) {
            res = Normalization_Point3D(res);
            return true;
        }
        return false;
    }

    bool NormalVector_Triangle_safe(const Triangle3DStd::Triangle3D& triangle, Point3DStd::Point3D& res) {
        return NormalVector_Triangle_plus_safe(triangle.p1, triangle.p2, triangle.p3, res);
    }


    /// <summary>
    /// 获取一个垂直与它的任意向量,以及归一化
    /// </summary>
    /// <param name="p"></param>
    /// <returns></returns>
    Point3DStd::Point3D NormalVector_Point3D(Point3DStd::Point3D& p) {
        double x1 = 0, y1 = 0, z1 = 0;
        if (p.x != 0) {
            y1 = 1;
            z1 = 1;
            x1 = -(p.y + p.z) / p.x;
            Point3DStd::Point3D temp111 = CreatePoint3D(x1, y1, z1);
            return Normalization_Point3D(temp111);
        }

        if (p.y != 0) {
            x1 = 1;
            z1 = 1;
            y1 = -(p.x + p.z) / p.y;
            Point3DStd::Point3D temp111 = CreatePoint3D(x1, y1, z1);
            return Normalization_Point3D(temp111);
        }

        if (p.z != 0) {
            x1 = 1;
            y1 = 1;
            z1 = -(p.z + p.y) / p.z;
            Point3DStd::Point3D temp111 = CreatePoint3D(x1, y1, z1);
            return Normalization_Point3D(temp111);
        }
        ProjectDependenciesStd::DisplayPromptOrReason("Point3D GetNByPoint3D(Point3D p) 0 0 0点无法获取一个垂直与它的向量、", true, __FILE__, __LINE__);
        return CreatePoint3D(x1, y1, z1);
    }

}