


#include"../Input.h"

namespace Geometry3DOperateStd {


    int AddPointAndReturnIndex(const Point3DStd::Point3D& point, std::vector<Point3DStd::Point3D>& points) {

        for (int i = 0; i < points.size(); ++i) {
            if (GetDistancePoint3DPoint3D(point, points[i]) < GlobalConstantStd::Eps) {
                return i;
            }
        }
        int index = (int)points.size();
        points.emplace_back(point);
        return index;
    }


   




    /// <summary>
    /// x>0,y>0,z>0,是否在第1卦象
    /// </summary>
    /// <param name="p"></param>
    /// <returns></returns>
    bool Point3DInCoordinateSystemXYZ01(const Point3DStd::Point3D& p) {
        if (p.x > 0) {
            if (p.y > 0) {
                if (p.z > 0) {
                    return true;
                }
            }
        }
        return false;
    }

    /// <summary>
    /// x<0,y>0,z>0,是否在第2卦象
    /// </summary>
    /// <param name="p"></param>
    /// <returns></returns>
    bool Point3DInCoordinateSystemXYZ02(const Point3DStd::Point3D& p) {
        if (p.x < 0) {
            if (p.y > 0) {
                if (p.z > 0) {
                    return true;
                }
            }
        }
        return false;
    }

    /// <summary>
    /// x<0,y<0,z>0,是否在第3卦象
    /// </summary>
    /// <param name="p"></param>
    /// <returns></returns>
    bool Point3DInCoordinateSystemXYZ03(const Point3DStd::Point3D& p) {
        if (p.x < 0) {
            if (p.y < 0) {
                if (p.z > 0) {
                    return true;
                }
            }
        }
        return false;
    }


    /// <summary>
    /// x>0,y<0,z>0,是否在第4卦象
    /// </summary>
    /// <param name="p"></param>
    /// <returns></returns>
    bool Point3DInCoordinateSystemXYZ04(const Point3DStd::Point3D& p) {
        if (p.x > 0) {
            if (p.y < 0) {
                if (p.z > 0) {
                    return true;
                }
            }
        }
        return false;
    }


    /// <summary>
    /// 计算两个点的叉乘
    /// </summary>
    /// <param name="p1">点1</param>
    /// <param name="p2">点2</param>
    /// <returns>两个点的叉乘</returns>
    Point3DStd::Point3D CrossPoint3DPoint3D(const Point3DStd::Point3D& p1, const Point3DStd::Point3D& p2)
    {
        double l = p1.x;
        double m = p1.y;
        double n = p1.z;
        double o = p2.x;
        double p = p2.y;
        double q = p2.z;
        Point3DStd::Point3D res = CreatePoint3D(m * q - n * p, n * o - l * q, l * p - m * o);

        return res;
    }

    bool CrossPoint3DPoint3D_safe(const Point3DStd::Point3D& p1, const Point3DStd::Point3D& p2, Point3DStd::Point3D& res)
    {
        double l = p1.x;
        double m = p1.y;
        double n = p1.z;
        double o = p2.x;
        double p = p2.y;
        double q = p2.z;
        Point3DStd::Point3D res2 = CreatePoint3D(m * q - n * p, n * o - l * q, l * p - m * o);
        if (Length_Point3D(res2) > 0) {
            AssignmentPoint3DPoint3D(res, res2);
            return true;
        }
        return false;
    }

    /// <summary>
    /// 计算两个点的差
    /// </summary>
    /// <param name="p1">点1</param>
    /// <param name="p2">点2</param>
    /// <returns>两个点的差</returns>
    Point3DStd::Point3D SubPoint3DPoint3D(const Point3DStd::Point3D& p1, const Point3DStd::Point3D& p2)
    {
        return CreatePoint3D(p1.x - p2.x, p1.y - p2.y, p1.z - p2.z);
    }

    /// <summary>
     /// 点乘计算
     /// </summary>
     /// <param name="p1"></param>
     /// <param name="p2"></param>
     /// <returns></returns>
    double DotPoint3DPoint3D(const Point3DStd::Point3D& p1, const Point3DStd::Point3D& p2)
    {
        return p1.x * p2.x + p1.y * p2.y + p1.z * p2.z;
    }
    /// <summary>
    /// 将一个向量放大或缩小
    /// </summary>
    /// <param name="d">zoom</param>
    /// <param name="p">向量</param>
    /// <returns>缩放后的向量</returns>
    Point3DStd::Point3D MulDoublePoint3D(double d, const Point3DStd::Point3D& p)
    {
        return CreatePoint3D(d * p.x, d * p.y, d * p.z);
    }


    void ChangeSenceMinPoint3D_x(double x, Point3DStd::Point3D& res) {
        if (x < res.x) {
            res.x = x - GlobalConstantStd::GetBoundingBoxPadding();
        }
    }
    void ChangeSenceMinPoint3D_y(double y, Point3DStd::Point3D& res) {
        if (y < res.y) {
            res.y = y - GlobalConstantStd::GetBoundingBoxPadding();
        }
    }
    void ChangeSenceMinPoint3D_z(double z, Point3DStd::Point3D& res) {
        if (z < res.z) {
            res.z = z - GlobalConstantStd::GetBoundingBoxPadding();
        }
    }

    void ChangeSenceMinPoint3D_point(const Point3DStd::Point3D& p, Point3DStd::Point3D& res) {
        ChangeSenceMinPoint3D_x(p.x, res);
        ChangeSenceMinPoint3D_y(p.y, res);
        ChangeSenceMinPoint3D_z(p.z, res);
    }


    void ChangeSenceMaxPoint3D_x(double x, Point3DStd::Point3D& res) {
        if (x > res.x) {
            res.x = x + GlobalConstantStd::GetBoundingBoxPadding();
        }
    }
    void ChangeSenceMaxPoint3D_y(double y, Point3DStd::Point3D& res) {
        if (y > res.y) {
            res.y = y + GlobalConstantStd::GetBoundingBoxPadding();
        }
    }
    void ChangeSenceMaxPoint3D_z(double z, Point3DStd::Point3D& res) {
        if (z > res.z) {
            res.z = z + GlobalConstantStd::GetBoundingBoxPadding();
        }
    }


    void ChangeSenceMaxPoint3D_point(const Point3DStd::Point3D& p, Point3DStd::Point3D& res) {
        ChangeSenceMaxPoint3D_x(p.x, res);
        ChangeSenceMaxPoint3D_y(p.y, res);
        ChangeSenceMaxPoint3D_z(p.z, res);
    }



    /// <summary>
    /// 计算两个点的和
    /// </summary>
    /// <param name="p1">点1</param>
    /// <param name="p2">点2</param>
    /// <returns>两个点的和</returns>
    Point3DStd::Point3D AddPoint3DPoint3D(const Point3DStd::Point3D& p1, const Point3DStd::Point3D& p2)
    {
        return CreatePoint3D(p1.x + p2.x, p1.y + p2.y, p1.z + p2.z);
    }




    double GetAnglePoint3DPoint3D(const Point3DStd::Point3D& a, const Point3DStd::Point3D& b)
    {
        double f = Length_Point3D(a) * Length_Point3D(b);
        if (f <= GlobalConstantStd::Eps)
        {
            const Point3DStd::Point3D a2 = Normalization_Point3D(a);
            const Point3DStd::Point3D b2 = Normalization_Point3D(b);
            f = Length_Point3D(a2) * Length_Point3D(b2);
            if (f <= GlobalConstantStd::Eps)
            {
                ProjectDependenciesStd::DisplayPromptOrReason("GetAnglePoint3DPoint3D，向量长度为0无法计算夹角大小", true, __FILE__, __LINE__);
                return 0.0;
            }
        }
        double d1 = DotPoint3DPoint3D(a, b);
        return acos(abs(d1) / f);
    }

    double GetAnglePoint3DPoint3D_unsafe(const Point3DStd::Point3D& a, const Point3DStd::Point3D& b)
    {
        double f = Length_Point3D(a) * Length_Point3D(b);
        double d1 = DotPoint3DPoint3D(a, b);
        return acos(abs(d1) / f);
    }

    


    Point3DStd::Point3D AddPoint3DPoint3DPoint3D(const Point3DStd::Point3D& p1, const Point3DStd::Point3D& p2, const Point3DStd::Point3D& p3)
    {
        return CreatePoint3D(p1.x + p2.x + p3.x, p1.y + p2.y + p3.y, p1.z + p2.z + p3.z);
    }

    Point3DStd::Point3D GetRelativeXYZIN(
        const Point3DStd::Point3D& o1,
        const Point3DStd::Point3D& ex,
        const Point3DStd::Point3D& ey,
        const Point3DStd::Point3D& ez,
        const Point3DStd::Point3D& p0) {
        auto p1 = MulDoublePoint3D(p0.x, ex);
        auto p2 = MulDoublePoint3D(p0.y, ey);
        auto p3 = MulDoublePoint3D(p0.z, ez);
        auto p4 = AddPoint3DPoint3DPoint3D(p1, p2, p3);
        return AddPoint3DPoint3D(o1, p4);
    }

    


    

    


    void CalThetaAndPhi(const Point3DStd::Point3D& unit_vec, double& theta, double& phi) {

        theta = std::acos(unit_vec.z);
        if (unit_vec.x == 0.0 && unit_vec.y == 0.0) {
            phi = 0.0;
        }
        else {
            phi = std::atan2(unit_vec.y, unit_vec.x);
            if (phi < 0) {
                phi = phi + 2 * GlobalConstantStd::Pi;
            }
        }

    }


    /// <summary>
    /// 计算入射角
    /// </summary>
    /// <returns></returns>
    double GetThetaI(const Point3DStd::Point3D& v1, const Point3DStd::Point3D& n1) {
        if (DotPoint3DPoint3D(n1, v1) > 0) {
            return GetAnglePoint3DPoint3D(v1, n1);
        }
        else {
            Point3DStd::Point3D v2(-v1.x, -v1.y, -v1.z);
            return GetAnglePoint3DPoint3D(v2, n1);
        }
    }


    Point3DStd::Point3D CalTransmissionRayVec(
        bool dotFlag,
        const Point3DStd::Point3D& a,
        const Point3DStd::Point3D& b,
        const Point3DStd::Point3D& n,
        double thetai,
        double thetat) {
        Point3DStd::Point3D ab = SubPoint3DPoint3D(b, a);
        double lenAB = Length_Point3D(ab);
        if (thetai < GlobalConstantStd::Eps) {
            return MulDoublePoint3D(1 / lenAB, ab);
        }

        //这个折射过程是tx从面元上表面进入下表面
        if (dotFlag) {
            Point3DStd::Point3D bd = MulDoublePoint3D(lenAB * cos(thetai), n);
            Point3DStd::Point3D ad = AddPoint3DPoint3D(ab, bd);

            ad = Normalization_Point3D(ad);
            Point3DStd::Point3D ec = MulDoublePoint3D(sin(thetat), ad);
            Point3DStd::Point3D be = MulDoublePoint3D(-cos(thetat), n);
            return AddPoint3DPoint3D(be, ec);
        }
        //这个折射过程是tx从面元下表面进入上表面
        else {
            Point3DStd::Point3D bd = MulDoublePoint3D(-lenAB * cos(thetai), n);
            Point3DStd::Point3D ad = AddPoint3DPoint3D(ab, bd);

            ad = Normalization_Point3D(ad);
            Point3DStd::Point3D ec = MulDoublePoint3D(sin(thetat), ad);
            Point3DStd::Point3D be = MulDoublePoint3D(cos(thetat), n);
            return AddPoint3DPoint3D(be, ec);
        }
    }


    double GetAngle(
        const Point3DStd::Point3D& ab,
        const Point3DStd::Point3D& ac) {

        // 向量1
        double x1 = ab.x, y1 = ab.y, z1 = ab.z;
        // 向量2
        double x2 = ac.x, y2 = ac.y, z2 = ac.z;
        //System.out.println("x1/y1/z1=" + x1 + "/" + y1 + "/" + z1);
        //System.out.println("x2/y2/z2=" + x2 + "/" + y2 + "/" + z2);
        // 向量的点乘
        double vectorDot = x1 * x2 + y1 * y2 + z1 * z2;
        // 向量1的模
        double vectorMold1 = sqrt(pow(x1, 2) + pow(y1, 2) + pow(z1, 2));
        // 向量2的模
        double vectorMold2 = sqrt(pow(x2, 2) + pow(y2, 2) + pow(z2, 2));
        // 向量的夹角[0, PI]，当夹角为锐角时，cosθ>0；当夹角为钝角时,cosθ<0
        double cosAngle = vectorDot / (vectorMold1 * vectorMold2);
        double radian = acos(cosAngle);
        return radian;
        //return (180.0 / GlobalConstantStd::PI * radian);
    }


    void ChangeSenceMinPoint3D_x_noPadding(double x, Point3DStd::Point3D& res) {
        if (x < res.x) {
            res.x = x;
        }
    }
    void ChangeSenceMinPoint3D_y_noPadding(double y, Point3DStd::Point3D& res) {
        if (y < res.y) {
            res.y = y;
        }
    }
    void ChangeSenceMinPoint3D_z_noPadding(double z, Point3DStd::Point3D& res) {
        if (z < res.z) {
            res.z = z;
        }
    }

    void ChangeSenceMinPoint3D_point_noPadding(const Point3DStd::Point3D& p, Point3DStd::Point3D& res) {
        ChangeSenceMinPoint3D_x_noPadding(p.x - 0.001, res);
        ChangeSenceMinPoint3D_y_noPadding(p.y - 0.001, res);
        ChangeSenceMinPoint3D_z_noPadding(p.z - 0.001, res);
    }


    void ChangeSenceMaxPoint3D_x_noPadding(double x, Point3DStd::Point3D& res) {
        if (x > res.x) {
            res.x = x;
        }
    }
    void ChangeSenceMaxPoint3D_y_noPadding(double y, Point3DStd::Point3D& res) {
        if (y > res.y) {
            res.y = y;
        }
    }
    void ChangeSenceMaxPoint3D_z_noPadding(double z, Point3DStd::Point3D& res) {
        if (z > res.z) {
            res.z = z;
        }
    }


    void ChangeSenceMaxPoint3D_point_noPadding(const Point3DStd::Point3D& p, Point3DStd::Point3D& res) {
        ChangeSenceMaxPoint3D_x_noPadding(p.x + 0.001, res);
        ChangeSenceMaxPoint3D_y_noPadding(p.y + 0.001, res);
        ChangeSenceMaxPoint3D_z_noPadding(p.z + 0.001, res);
    }

    

    /// <summary>
    /// 计算镜像点,法相是任意的即可，
    /// </summary>
    /// <param name="point"></param>
    /// <param name="face"></param>
    /// <param name="eps"></param>
    /// <param name="res"></param>
    /// <returns></returns>
    bool GetMirrorPoint3D(const Point3DStd::Point3D& point, const Point3DStd::Point3D& triangle_p1,
        const Point3DStd::Point3D& n, Point3DStd::Point3D& res) {
        Point3DStd::Point3D curRes1 = Geometry3DOperateStd::CalculateShadowPoint3DOfPoint3DonPlane3D_plus_unsafe(point, triangle_p1, n);
        Point3DStd::Point3D curRes2 = Geometry3DOperateStd::SubPoint3DPoint3D(curRes1, point);
        if (Geometry3DOperateStd::Length_Point3D(curRes2) <= GlobalConstantStd::Eps) {
            return false;
        }

        //介质内部的反射运算不计算
        if (Geometry3DOperateStd::DotPoint3DPoint3D(curRes2, n) > 0) {
            return false;
        }

        res = Geometry3DOperateStd::AddPoint3DPoint3D(curRes1, curRes2);

        return true;
    }


}
