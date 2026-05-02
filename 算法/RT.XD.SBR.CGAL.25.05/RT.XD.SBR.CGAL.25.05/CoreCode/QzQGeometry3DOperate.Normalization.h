#pragma once

#include"HdQBall3D.h"
#include"HdQBaseCoordinateSystem.h"
#include"HdQBoundingBox3D.h"
#include"HdQConfig.h"
#include"LxQLine3D.h"
#include"LxQLineSegment3D.h"
#include"LxQPlane3D.h"
#include"LxQPoint2I.h"
#include"LxQPoint3DI.h"
#include"LxQPoint3F.h"
#include"LxQPoint3I.h"
#include"LxQPoint2D.h"
#include"LxQPoint3D.h"
#include"DxQRay3D.h"
#include"DxQTriangle3D.h"
#include"DxQTriangleI.h"


namespace Geometry2DOperateStd {

    /// <summary>
     /// 创建一个点坐标
     /// </summary>
     /// <param name="x"></param>
     /// <param name="y"></param>
     /// <param name="z"></param>
     /// <returns></returns>
    INTERFACE_API Point2DStd::Point2D CreatePoint2D(double x, double y);


    /// <summary>
    /// 计算两个点的差
    /// </summary>
    /// <param name="p1">点1</param>
    /// <param name="p2">点2</param>
    /// <returns>两个点的差</returns>
    INTERFACE_API Point2DStd::Point2D SubPoint2DPoint2D(const Point2DStd::Point2D& p1, const Point2DStd::Point2D& p2);

    /// <summary>
     /// 点乘计算
     /// </summary>
     /// <param name="p1"></param>
     /// <param name="p2"></param>
     /// <returns></returns>
    INTERFACE_API double DotPoint2DPoint2D(const Point2DStd::Point2D& p1, const Point2DStd::Point2D& p2);
    /// <summary>
    /// 将一个向量放大或缩小
    /// </summary>
    /// <param name="d">zoom</param>
    /// <param name="p">向量</param>
    /// <returns>缩放后的向量</returns>
    INTERFACE_API Point2DStd::Point2D MulDoublePoint2D(double d, const Point2DStd::Point2D& p);

    /// <summary>
    /// 计算向量长度
    /// </summary>
    /// <param name="p"></param>
    /// <returns></returns>
    INTERFACE_API double Length_Point2D(const Point2DStd::Point2D& p);
    INTERFACE_API Point2DStd::Point2D Normalization_Point2D_unsafe(const Point2DStd::Point2D& p);
    INTERFACE_API bool Normalization_Point2D_safe(const Point2DStd::Point2D& p, Point2DStd::Point2D& res);

    INTERFACE_API Point2DStd::Point2D AddPoint2DPoint2D(const Point2DStd::Point2D& p1, const Point2DStd::Point2D& p2);

    INTERFACE_API bool Equals_Point2D(const Point2DStd::Point2D& p1, const Point2DStd::Point2D& p2);
    INTERFACE_API bool Equals_Point2D_N(const Point2DStd::Point2D& n1, const Point2DStd::Point2D& n2);


    INTERFACE_API double GetAnglePoint2DPoint2D_unsafe(const Point2DStd::Point2D& a, const Point2DStd::Point2D& b);

    INTERFACE_API double GetDistancePoint2DLine2D_plus_unsafe(
        const Point2DStd::Point2D& p,
        const Point2DStd::Point2D& lineO,
        const Point2DStd::Point2D& lineVec);

    INTERFACE_API double Cross(const Point2DStd::Point2D& A, const Point2DStd::Point2D& B);

    /// <summary>
    /// 这里的距离时直线2到端点的距离,可能是负值
    /// </summary>
    /// <param name="A"></param>
    /// <param name="v1_normalized"></param>
    /// <param name="B"></param>
    /// <param name="v2_normalized"></param>
    /// <param name="distance"></param>
    /// <returns></returns>
    INTERFACE_API Point2DStd::Point2D Intersect_Line2D_Line2D_plus_unsafe(
        const Point2DStd::Point2D& A,
        const Point2DStd::Point2D& v1_normalized,
        const Point2DStd::Point2D& B,
        const Point2DStd::Point2D& v2_normalized,
        double& distance);

}
/// <summary>
/// 归一化
/// </summary>
namespace Geometry3DOperateStd {

    INTERFACE_API Point3DStd::Point3D Normalization_Point3D(const Point3DStd::Point3D& p);
    INTERFACE_API Point3DStd::Point3D Normalization_Point3D_unsafe(const Point3DStd::Point3D& p);
    INTERFACE_API bool Normalization_Point3D_safe(const Point3DStd::Point3D& p, Point3DStd::Point3D& res);

}