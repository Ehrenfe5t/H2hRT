

#include"../Input.h"


namespace Geometry3DOperateStd {

    /// <summary>
     /// 创建一个点坐标
     /// </summary>
     /// <param name="x"></param>
     /// <param name="y"></param>
     /// <param name="z"></param>
     /// <returns></returns>
    Point3DStd::Point3D CreatePoint3D(double x, double y, double z) {
        Point3DStd::Point3D res;
        res.x = x;
        res.y = y;
        res.z = z;
        return res;
    }


    BoundingBox3DStd::BoundingBox3D CreateBoundingBox3DBy3Point3D(const Point3DStd::Point3D& a, const Point3DStd::Point3D& b, const Point3DStd::Point3D& c) {
        BoundingBox3DStd::BoundingBox3D box(a, a);

        ChangeSenceMaxPoint3D_point_noPadding(b, box.max);
        ChangeSenceMaxPoint3D_point_noPadding(c, box.max);

        ChangeSenceMinPoint3D_point_noPadding(b, box.min);
        ChangeSenceMinPoint3D_point_noPadding(c, box.min);
        return box;
    }

    Ball3DStd::Ball3D CreateBall3DByListPoint3D(const std::vector<Point3DStd::Point3D>& list) {
        if (list.size() < 1) {
            return Ball3DStd::Ball3D();
        }
        Point3DStd::Point3D center(0.0, 0.0, 0.0);
        for (int i = 0; i < list.size(); ++i) {
            center = AddPoint3DPoint3D(center, list[i]);
        }
        center = MulDoublePoint3D(1.0 / list.size(), center);

        double radius = 0.0;
        for (int i = 0; i < list.size(); ++i) {
            double dis = GetDistancePoint3DPoint3D(center, list[i]);
            if (dis > radius) {
                radius = dis + 0.001;
            }
        }

        return Ball3DStd::Ball3D(radius, center);
    }

    Ball3DStd::Ball3D CreateBall3DBy3Point3D(const Point3DStd::Point3D& a, const Point3DStd::Point3D& b, const Point3DStd::Point3D& c) {
        std::vector<Point3DStd::Point3D> list;
        list.emplace_back(a);
        list.emplace_back(b);
        list.emplace_back(c);
        return CreateBall3DByListPoint3D(list);
    }


    bool CreateLine3DByTwoPoint3D_safe(const Point3DStd::Point3D& start, const Point3DStd::Point3D& end, Line3DStd::Line3D& res) {
        Point3DStd::Point3D vec = SubPoint3DPoint3D(end, start);
        double len = Length_Point3D(vec);
        if (MathOperateStd::OneNumberIsZeroByEps(len) == 1)
        {
            return false;
        }
        AssignmentPoint3DPoint3D(res.vec, MulDoublePoint3D(1.0 / len, vec));
        AssignmentPoint3DPoint3D(res.p, start);
        return true;
    }

    Line3DStd::Line3D CreateLine3DByTwoPoint3D_unsafe(const Point3DStd::Point3D& start, const Point3DStd::Point3D& end) {
        Point3DStd::Point3D vec = SubPoint3DPoint3D(end, start);
        double len = Length_Point3D(vec);
        return Line3DStd::Line3D(start, MulDoublePoint3D(1.0 / len, vec));
    }


    Line3DStd::Line3D CreateLine3D_unsafe(const Point3DStd::Point3D& o, const Point3DStd::Point3D& vec) {
        return Line3DStd::Line3D(o, vec);
    }

}