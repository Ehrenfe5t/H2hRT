#pragma once




#include<vector>

namespace Point3FStd {
    /// <summary>
    /// 三维点
    /// </summary>
    class Point3F {
    public:
        /// <summary>
        /// 点的x坐标
        /// </summary>
        float x;
        /// <summary>
        /// 点的y坐标
        /// </summary>
        float y;
        /// <summary>
        /// 点的z坐标
        /// </summary>
        float z;

        Point3F();

        Point3F(float x, float y, float z);

        Point3F(const Point3F& p);

        ~Point3F();

        bool operator<(const Point3F& obj) const;
    };

    void CopyVectorPoint3F(const std::vector<Point3F>& obj, std::vector<Point3F>& res);
}