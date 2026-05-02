


#include"../Input.h"


namespace Geometry3DOperateStd {



    void DiscretizingTriangle3DPlus(
        const Point3DStd::Point3D& p1,
        const Point3DStd::Point3D& p2,
        const Point3DStd::Point3D& p3,
        double maxSideLength,
        std::vector<Point3DStd::Point3D>& res) {

        res.emplace_back(Center_Point3D_Point3D_Point3D(p1, p2, p3));


        bool type1 = GetDistancePoint3DPoint3D(p1, p2) > maxSideLength ? true : false;
        bool type2 = GetDistancePoint3DPoint3D(p1, p3) > maxSideLength ? true : false;
        bool type3 = GetDistancePoint3DPoint3D(p2, p3) > maxSideLength ? true : false;

        if (type1) {
            if (type2) {
                if (type3) {
                    Point3DStd::Point3D center1 = Center_Point3D_Point3D(p1, p2);
                    Point3DStd::Point3D center2 = Center_Point3D_Point3D(p1, p3);
                    Point3DStd::Point3D center3 = Center_Point3D_Point3D(p2, p3);
                    DiscretizingTriangle3DPlus(p1, center1, center2, maxSideLength, res);
                    DiscretizingTriangle3DPlus(center1, p2, center3, maxSideLength, res);
                    DiscretizingTriangle3DPlus(center1, center3, center2, maxSideLength, res);
                    DiscretizingTriangle3DPlus(center2, center3, p3, maxSideLength, res);
                }
                else {
                    Point3DStd::Point3D center1 = Center_Point3D_Point3D(p1, p2);
                    Point3DStd::Point3D center2 = Center_Point3D_Point3D(p1, p3);
                    DiscretizingTriangle3DPlus(p1, center1, center2, maxSideLength, res);
                    DiscretizingTriangle3DPlus(center1, p3, center2, maxSideLength, res);
                    DiscretizingTriangle3DPlus(center1, p2, p3, maxSideLength, res);
                }
            }
            else {
                if (type3) {
                    Point3DStd::Point3D center1 = Center_Point3D_Point3D(p1, p2);
                    Point3DStd::Point3D center3 = Center_Point3D_Point3D(p2, p3);
                    DiscretizingTriangle3DPlus(p1, center1, center3, maxSideLength, res);
                    DiscretizingTriangle3DPlus(center1, p2, center3, maxSideLength, res);
                    DiscretizingTriangle3DPlus(p1, center3, p3, maxSideLength, res);

                }
                else {
                    Point3DStd::Point3D center1 = Center_Point3D_Point3D(p1, p2);
                    DiscretizingTriangle3DPlus(p1, center1, p3, maxSideLength, res);
                    DiscretizingTriangle3DPlus(center1, p2, p3, maxSideLength, res);
                }
            }
        }
        else {
            if (type2) {
                if (type3) {
                    Point3DStd::Point3D center2 = Center_Point3D_Point3D(p1, p3);
                    Point3DStd::Point3D center3 = Center_Point3D_Point3D(p2, p3);
                    DiscretizingTriangle3DPlus(p3, center2, center3, maxSideLength, res);
                    DiscretizingTriangle3DPlus(center2, p1, center3, maxSideLength, res);
                    DiscretizingTriangle3DPlus(p1, p2, center3, maxSideLength, res);
                }
                else {
                    Point3DStd::Point3D center2 = Center_Point3D_Point3D(p1, p3);
                    DiscretizingTriangle3DPlus(p1, p2, center2, maxSideLength, res);
                    DiscretizingTriangle3DPlus(center2, p2, p3, maxSideLength, res);
                }
            }
            else {
                if (type3) {
                    Point3DStd::Point3D center3 = Center_Point3D_Point3D(p2, p3);
                    DiscretizingTriangle3DPlus(p1, center3, p3, maxSideLength, res);
                    DiscretizingTriangle3DPlus(p1, p2, center3, maxSideLength, res);
                }
                else {
                    res.emplace_back(Center_Point3D_Point3D_Point3D(p1, p2, p3));
                }
            }
        }

    }

    std::vector<Point3DStd::Point3D> DiscretizingTriangle3D(
        const Point3DStd::Point3D& p1,
        const Point3DStd::Point3D& p2,
        const Point3DStd::Point3D& p3,
        double maxSideLength) {
        std::vector<Point3DStd::Point3D> res;
        res.emplace_back(p1);
        res.emplace_back(p2);
        res.emplace_back(p3);

        bool type1 = GetDistancePoint3DPoint3D(p1, p2) > maxSideLength ? true : false;
        bool type2 = GetDistancePoint3DPoint3D(p1, p3) > maxSideLength ? true : false;
        bool type3 = GetDistancePoint3DPoint3D(p2, p3) > maxSideLength ? true : false;

        if (type1) {
            if (type2) {
                if (type3) {
                    Point3DStd::Point3D center1 = Center_Point3D_Point3D(p1, p2);
                    Point3DStd::Point3D center2 = Center_Point3D_Point3D(p1, p3);
                    Point3DStd::Point3D center3 = Center_Point3D_Point3D(p2, p3);
                    DiscretizingTriangle3DPlus(p1, center1, center2, maxSideLength, res);
                    DiscretizingTriangle3DPlus(center1, p2, center3, maxSideLength, res);
                    DiscretizingTriangle3DPlus(center1, center3, center2, maxSideLength, res);
                    DiscretizingTriangle3DPlus(center2, center3, p3, maxSideLength, res);
                }
                else {
                    Point3DStd::Point3D center1 = Center_Point3D_Point3D(p1, p2);
                    Point3DStd::Point3D center2 = Center_Point3D_Point3D(p1, p3);
                    DiscretizingTriangle3DPlus(p1, center1, center2, maxSideLength, res);
                    DiscretizingTriangle3DPlus(center1, p3, center2, maxSideLength, res);
                    DiscretizingTriangle3DPlus(center1, p2, p3, maxSideLength, res);
                }
            }
            else {
                if (type3) {
                    Point3DStd::Point3D center1 = Center_Point3D_Point3D(p1, p2);
                    Point3DStd::Point3D center3 = Center_Point3D_Point3D(p2, p3);
                    DiscretizingTriangle3DPlus(p1, center1, center3, maxSideLength, res);
                    DiscretizingTriangle3DPlus(center1, p2, center3, maxSideLength, res);
                    DiscretizingTriangle3DPlus(p1, center3, p3, maxSideLength, res);

                }
                else {
                    Point3DStd::Point3D center1 = Center_Point3D_Point3D(p1, p2);
                    DiscretizingTriangle3DPlus(p1, center1, p3, maxSideLength, res);
                    DiscretizingTriangle3DPlus(center1, p2, p3, maxSideLength, res);
                }
            }
        }
        else {
            if (type2) {
                if (type3) {
                    Point3DStd::Point3D center2 = Center_Point3D_Point3D(p1, p3);
                    Point3DStd::Point3D center3 = Center_Point3D_Point3D(p2, p3);
                    DiscretizingTriangle3DPlus(p3, center2, center3, maxSideLength, res);
                    DiscretizingTriangle3DPlus(center2, p1, center3, maxSideLength, res);
                    DiscretizingTriangle3DPlus(p1, p2, center3, maxSideLength, res);
                }
                else {
                    Point3DStd::Point3D center2 = Center_Point3D_Point3D(p1, p3);
                    DiscretizingTriangle3DPlus(p1, p2, center2, maxSideLength, res);
                    DiscretizingTriangle3DPlus(center2, p2, p3, maxSideLength, res);
                }
            }
            else {
                if (type3) {
                    Point3DStd::Point3D center3 = Center_Point3D_Point3D(p2, p3);
                    DiscretizingTriangle3DPlus(p1, center3, p3, maxSideLength, res);
                    DiscretizingTriangle3DPlus(p1, p2, center3, maxSideLength, res);
                }
                else {
                    res.emplace_back(Center_Point3D_Point3D_Point3D(p1, p2, p3));
                }
            }
        }

        return res;
    }

}