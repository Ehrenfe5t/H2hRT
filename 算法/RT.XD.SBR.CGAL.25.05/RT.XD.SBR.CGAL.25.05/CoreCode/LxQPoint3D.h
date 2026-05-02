#pragma once

#include"QzQJson.hpp"
#include<vector>

namespace Point3DStd {
    /// <summary>
    /// 三维点
    /// </summary>
    class Point3D {
    public:
        /// <summary>
        /// 点的x坐标
        /// </summary>
        double x;
        /// <summary>
        /// 点的y坐标
        /// </summary>
        double y;
        /// <summary>
        /// 点的z坐标
        /// </summary>
        double z;

        Point3D();

        Point3D(double x, double y, double z);

        Point3D(const Point3D& p);

        ~Point3D();

        bool operator<(const Point3D& obj) const;
        const double& operator[](int axis) const;

        // 加法运算符重载
        Point3D operator+(const Point3D& p) const;

        // 减法运算符重载
        Point3D operator-(const Point3D& p) const;

        // 标量乘法运算符重载（向量 * 标量）
        Point3D operator*(double s) const;

        // 标量除法运算符重载
        Point3D operator/(double s) const;

        double distance(const Point3D& p) const;

    };

    void CopyVectorPoint3D(const std::vector<Point3D>& obj, std::vector<Point3D>& res);


    /// <summary>
    /// 从json字符串获取点
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void from_json(const nlohmann::json& j, Point3DStd::Point3D& p);

    /// <summary>
    /// 将点对象转化为json字符串
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void to_json(nlohmann::json& j, const Point3DStd::Point3D& p);

}