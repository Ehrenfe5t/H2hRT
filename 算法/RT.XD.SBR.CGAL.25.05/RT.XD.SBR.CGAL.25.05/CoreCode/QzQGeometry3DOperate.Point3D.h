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




namespace Geometry3DOperateStd {

    
    INTERFACE_API int AddPointAndReturnIndex(const Point3DStd::Point3D& point, std::vector<Point3DStd::Point3D>& points);
    INTERFACE_API Point3DStd::Point3D AddPoint3DPoint3D(const Point3DStd::Point3D& p1, const Point3DStd::Point3D& p2);
    INTERFACE_API Point3DStd::Point3D AddPoint3DPoint3DPoint3D(const Point3DStd::Point3D& p1, const Point3DStd::Point3D& p2, const Point3DStd::Point3D& p3);
    INTERFACE_API Point3DStd::Point3D SubPoint3DPoint3D(const Point3DStd::Point3D& p1, const Point3DStd::Point3D& p2);
    INTERFACE_API Point3DStd::Point3D MulDoublePoint3D(double d, const Point3DStd::Point3D& p);
    INTERFACE_API double DotPoint3DPoint3D(const Point3DStd::Point3D& p1, const Point3DStd::Point3D& p2);
    INTERFACE_API Point3DStd::Point3D CrossPoint3DPoint3D(const Point3DStd::Point3D& p1, const Point3DStd::Point3D& p2);
    INTERFACE_API bool CrossPoint3DPoint3D_safe(const Point3DStd::Point3D& p1, const Point3DStd::Point3D& p2, Point3DStd::Point3D& res);

    INTERFACE_API bool Point3DInCoordinateSystemXYZ01(const Point3DStd::Point3D& p);
    INTERFACE_API bool Point3DInCoordinateSystemXYZ02(const Point3DStd::Point3D& p);
    INTERFACE_API bool Point3DInCoordinateSystemXYZ03(const Point3DStd::Point3D& p);
    INTERFACE_API bool Point3DInCoordinateSystemXYZ04(const Point3DStd::Point3D& p);
    

    INTERFACE_API double GetAnglePoint3DPoint3D(const Point3DStd::Point3D& a, const Point3DStd::Point3D& b);
    INTERFACE_API double GetAnglePoint3DPoint3D_unsafe(const Point3DStd::Point3D& a, const Point3DStd::Point3D& b);
    INTERFACE_API double GetThetaI(const Point3DStd::Point3D& v1, const Point3DStd::Point3D& n1);
    INTERFACE_API double GetAngle(const Point3DStd::Point3D& ab, const Point3DStd::Point3D& ac);
    INTERFACE_API void CalThetaAndPhi(const Point3DStd::Point3D& unit_vec, double& theta, double& phi);
     
    INTERFACE_API Point3DStd::Point3D CalTransmissionRayVec(bool dotFlag, const Point3DStd::Point3D& a, const Point3DStd::Point3D& b, const Point3DStd::Point3D& n, double thetai, double thetat);

    INTERFACE_API void ChangeSenceMinPoint3D_x(double x, Point3DStd::Point3D& res);
    INTERFACE_API void ChangeSenceMinPoint3D_y(double y, Point3DStd::Point3D& res);
    INTERFACE_API void ChangeSenceMinPoint3D_z(double z, Point3DStd::Point3D& res);
    INTERFACE_API void ChangeSenceMinPoint3D_point(const Point3DStd::Point3D& p, Point3DStd::Point3D& res);
    INTERFACE_API void ChangeSenceMaxPoint3D_x(double x, Point3DStd::Point3D& res);
    INTERFACE_API void ChangeSenceMaxPoint3D_y(double y, Point3DStd::Point3D& res);
    INTERFACE_API void ChangeSenceMaxPoint3D_z(double z, Point3DStd::Point3D& res);
    INTERFACE_API void ChangeSenceMaxPoint3D_point(const Point3DStd::Point3D& p, Point3DStd::Point3D& res);

    INTERFACE_API void ChangeSenceMinPoint3D_x_noPadding(double x, Point3DStd::Point3D& res);
    INTERFACE_API void ChangeSenceMinPoint3D_y_noPadding(double y, Point3DStd::Point3D& res);
    INTERFACE_API void ChangeSenceMinPoint3D_z_noPadding(double z, Point3DStd::Point3D& res);
    INTERFACE_API void ChangeSenceMinPoint3D_point_noPadding(const Point3DStd::Point3D& p, Point3DStd::Point3D& res);
    INTERFACE_API void ChangeSenceMaxPoint3D_x_noPadding(double x, Point3DStd::Point3D& res);
    INTERFACE_API void ChangeSenceMaxPoint3D_y_noPadding(double y, Point3DStd::Point3D& res);
    INTERFACE_API void ChangeSenceMaxPoint3D_z_noPadding(double z, Point3DStd::Point3D& res);
    INTERFACE_API void ChangeSenceMaxPoint3D_point_noPadding(const Point3DStd::Point3D& p, Point3DStd::Point3D& res);
    


    /// <summary>
    /// 计算镜像点,法相是任意的即可，
    /// </summary>
    /// <param name="point"></param>
    /// <param name="face"></param>
    /// <param name="eps"></param>
    /// <param name="res"></param>
    /// <returns></returns>
    INTERFACE_API bool GetMirrorPoint3D(const Point3DStd::Point3D& point, const Point3DStd::Point3D& triangle_p1,
        const Point3DStd::Point3D& n, Point3DStd::Point3D& res);

}