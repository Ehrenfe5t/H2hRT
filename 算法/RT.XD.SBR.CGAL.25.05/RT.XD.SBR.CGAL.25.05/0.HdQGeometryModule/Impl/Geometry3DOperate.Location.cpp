

#include"../Input.h"

namespace Geometry3DOperateStd {
    
    /// <summary>
    /// b是否在ac之间
    /// </summary>
    /// <param name="b"></param>
    /// <param name="a"></param>
    /// <param name="c"></param>
    /// <param name="distaceAB"></param>
    /// <param name="distancBC"></param>
    /// <returns></returns>
    bool Location_Point3DonLineSegment3D_plus_plus(const Point3DStd::Point3D& b, const Point3DStd::Point3D& a,const Point3DStd::Point3D& c,double& distaceAB,double& distancBC) {
        distaceAB = GetDistancePoint3DPoint3D(a, b);
        double ac = GetDistancePoint3DPoint3D(a, c);
        distancBC = GetDistancePoint3DPoint3D(b, c);
        double temp = distaceAB + distancBC - ac;
        if (abs(temp) <= GlobalConstantStd::Eps) {
            return true;
        }
        return false;
    }

    /// <summary>
    /// b是否在ac之间
    /// </summary>
    /// <param name="a"></param>
    /// <param name="a"></param>
    /// <param name="a"></param>
    /// <param name="eps"></param>
    bool Location_Point3DonLineSegment3D_plus(const Point3DStd::Point3D& b, const Point3DStd::Point3D& a, const Point3DStd::Point3D& c) {
        double ab = GlobalConstantStd::BoundingBoxLength;
        double bc = GlobalConstantStd::BoundingBoxLength;
        return Location_Point3DonLineSegment3D_plus_plus(b, a, c, ab, bc);
    }


    bool Location_Point3DonLineSegment3D_plus_plus_2(const Point3DStd::Point3D& b, const Point3DStd::Point3D& a, const Point3DStd::Point3D& c, double eps,double& distaceAB, double& distancBC) {
        distaceAB = GetDistancePoint3DPoint3D(a, b);
        double ac = GetDistancePoint3DPoint3D(a, c);
        distancBC = GetDistancePoint3DPoint3D(b, c);
        double temp = distaceAB + distancBC - ac;
        if (abs(temp) <= eps) {
            return true;
        }
        return false;
    }
    bool Location_Point3DonLineSegment3D_plus_2(const Point3DStd::Point3D& b, const Point3DStd::Point3D& a, const Point3DStd::Point3D& c,double eps) {
        double ab = GlobalConstantStd::BoundingBoxLength;
        double bc = GlobalConstantStd::BoundingBoxLength;
        return Location_Point3DonLineSegment3D_plus_plus_2(b, a, c,eps, ab, bc);
    }

    /// <summary>
    /// -2,标准在左边，-1表示start点，0表示在线段上，但是不包含start和end，1表示在end，2表示在右边
    /// </summary>
    /// <param name="b"></param>
    /// <param name="start"></param>
    /// <param name="end"></param>
    /// <returns></returns>
    char Location_Point3DOfShadowonLineSegment3D_plus(const Point3DStd::Point3D& b, const Point3DStd::Point3D& start, const Point3DStd::Point3D& end) {
        
        Point3DStd::Point3D vec = SubPoint3DPoint3D(end, start);
        double len = Length_Point3D(vec);
        vec = MulDoublePoint3D(1.0 / len, vec);
        Point3DStd::Point3D shadow = CalculateShadowPoint3DOfPoint3DonLine3D_plus_unsafe(b, start, vec);
        if (Equals_Point3D(shadow, start)) {
            return -1;
        }
        if (Equals_Point3D(shadow, end)) {
            return 1;
        }
        auto ab = SubPoint3DPoint3D(shadow, start);
        double dot = DotPoint3DPoint3D(ab, vec);
        if (IsZeroAbs(dot)) {
            return 0;
        }
        else if (dot < 0.0) {
            return -2;
        }
        double len_ab = Length_Point3D(ab);
        if (len_ab>len) {
            return 2;
        }
        return 0;
    }


    bool Location_Point3DonLineSegment3D(const Point3DStd::Point3D& p, const LineSegment3DStd::LineSegment3D& seg) {
        return Location_Point3DonLineSegment3D_plus(p, seg.start, seg.end);
    }

    /// <summary>
    /// 点是否在直线上
    /// </summary>
    /// <param name="p"></param>
    /// <param name="lineO"></param>
    /// <param name="lineVec"></param>
    /// <returns></returns>
    bool Location_Point3DonLine3D_plus_safe(const Point3DStd::Point3D& p, const Point3DStd::Point3D& lineO, const Point3DStd::Point3D& lineVec) {
        
        double distance = GlobalConstantStd::BoundingBoxLength;
        if (GetDistancePoint3DLine3D_plus_safe(p, lineO, lineVec, distance))
        {
            if (distance<=GlobalConstantStd::Eps) {
                return true;
            }
        }
        return false;
    }

    /// <summary>
    /// 点是否在直线上
    /// </summary>
    /// <param name="p"></param>
    /// <param name="lineO"></param>
    /// <param name="lineVec"></param>
    /// <returns></returns>
    bool Location_Point3DonLine3D_plus_unsafe(const Point3DStd::Point3D& p, const Point3DStd::Point3D& lineO, const Point3DStd::Point3D& lineVec) {

        double distance = GetDistancePoint3DLine3D_plus_unsafe(p, lineO, lineVec);
        if (distance <= GlobalConstantStd::Eps) {
            return true;
        }
        return false;
    }

    /// <summary>
    /// 点是否在直线上
    /// </summary>
    /// <param name="p"></param>
    /// <param name="line"></param>
    /// <returns></returns>
    bool Location_Point3DonLine3D_safe(const Point3DStd::Point3D& p, const Line3DStd::Line3D& line) {
        return Location_Point3DonLine3D_plus_safe(p, line.p, line.vec);
    }

    /// <summary>
    /// 点是否在直线上
    /// </summary>
    /// <param name="p"></param>
    /// <param name="line"></param>
    /// <returns></returns>
    bool Location_Point3DonLine3D_unsafe(const Point3DStd::Point3D& p, const Line3DStd::Line3D& line) {
        return Location_Point3DonLine3D_plus_unsafe(p, line.p, line.vec);
    }

    /// <summary>
    /// 点是否在平面上
    /// </summary>
    /// <param name="p"></param>
    /// <param name="planeO"></param>
    /// <param name="planeN"></param>
    /// <returns></returns>
    bool Location_Point3DonPlane3D_plus_unsafe(const Point3DStd::Point3D& p, const Point3DStd::Point3D& planeO, const Point3DStd::Point3D& planeN) {
        double distance = GetDistancePoint3DPlane3D_plus_unsafe(p, planeO, planeN);
        if (distance <= GlobalConstantStd::Eps) {
            return true;
        }
        return false;
    }

    char Location_Point3D_Plane3D_UpDown_plus(const Point3DStd::Point3D& p, const Point3DStd::Point3D& planeO, const Point3DStd::Point3D& planeN) {
        Point3DStd::Point3D vec = SubPoint3DPoint3D(planeO, p);
        double dot = DotPoint3DPoint3D(planeN, vec);
        if (IsZeroAbs(dot)) {
            return 0;
        }
        if (dot > 0.0) {
            return -1;
        }
        return 1;
    }

    /// <summary>
    /// 点是否在平面上
    /// </summary>
    /// <param name="p"></param>
    /// <param name="planeO"></param>
    /// <param name="planeN"></param>
    /// <returns></returns>
    bool Location_Point3DonPlane3D_plus_safe(const Point3DStd::Point3D& p, const Point3DStd::Point3D& planeO, const Point3DStd::Point3D& planeN) {
        if (IsZeroAbs(1.0 - Length_Point3D(planeN))) {
            return Location_Point3DonPlane3D_plus_unsafe(p, planeO, planeN);
        }
        //面不存在
        return false;
    }

    /// <summary>
    /// 点是否在平面上
    /// </summary>
    /// <param name="p"></param>
    /// <param name="plane"></param>
    /// <returns></returns>
    bool Location_Point3DonPlane3D_unsafe(const Point3DStd::Point3D& p, const Plane3DStd::Plane3D& plane) {
        return Location_Point3DonPlane3D_plus_unsafe(p, plane.p, plane.n);
    }



    /// <summary>
    /// 点是否在平面上
    /// </summary>
    /// <param name="p"></param>
    /// <param name="plane"></param>
    /// <returns></returns>
    bool Location_Point3DonPlane3D_safe(const Point3DStd::Point3D& p, const Plane3DStd::Plane3D& plane) {
        return Location_Point3DonPlane3D_plus_safe(p, plane.p, plane.n);
    }

    /// <summary>
    /// 两个向量是否平行或者共线
    /// </summary>
    /// <param name="line1O"></param>
    /// <param name="line1Vec"></param>
    /// <param name="line2O"></param>
    /// <param name="line2Vec"></param>
    /// <returns></returns>
    bool Location_ParallelOrCoincident_Point3D_Point3D_plus_unsafe(
        const Point3DStd::Point3D& line1Vec,
        const Point3DStd::Point3D& line2Vec) {
        return Equals_Point3D_N(line1Vec, line2Vec);
    }

    /// <summary>
    /// 平行或者共线
    /// </summary>
    /// <param name="line1O"></param>
    /// <param name="line1Vec"></param>
    /// <param name="line2O"></param>
    /// <param name="line2Vec"></param>
    /// <returns></returns>
    bool Location_ParallelOrCoincident_Line3D_Line3D_plus_unsafe(
        const Point3DStd::Point3D& line1O,
        const Point3DStd::Point3D& line1Vec,
        const Point3DStd::Point3D& line2O,
        const Point3DStd::Point3D& line2Vec) {
        return Location_ParallelOrCoincident_Point3D_Point3D_plus_unsafe(line1Vec, line2Vec);
    }

    /// <summary>
    /// 平行或者共线
    /// </summary>
    /// <param name="line1"></param>
    /// <param name="line2"></param>
    /// <returns></returns>
    bool Location_ParallelOrCoincident_Line3D_Line3D_unsafe(
        const Line3DStd::Line3D& line1,
        const Line3DStd::Line3D& line2) {
        return Location_ParallelOrCoincident_Point3D_Point3D_plus_unsafe(line1.vec, line2.vec);
    }

    /// <summary>
    /// 共线
    /// </summary>
    /// <param name="line1O"></param>
    /// <param name="line1Vec"></param>
    /// <param name="line2O"></param>
    /// <param name="line2Vec"></param>
    /// <returns></returns>
    bool Location_Coincident_Line3D_Line3D_plus_unsafe(
        const Point3DStd::Point3D& line1O,
        const Point3DStd::Point3D& line1Vec,
        const Point3DStd::Point3D& line2O,
        const Point3DStd::Point3D& line2Vec) {
        if (Location_ParallelOrCoincident_Line3D_Line3D_plus_unsafe(line1O, line1Vec, line2O, line2Vec)) {
            Point3DStd::Point3D vec = SubPoint3DPoint3D(line2O, line1O);
            Point3DStd::Point3D cross = CrossPoint3DPoint3D(line1Vec, vec);
            if (IsZero(Length_Point3D(cross))) {
                return true;
            }
        }
        return false;
    }

    /// <summary>
    /// 共线
    /// </summary>
    /// <param name="line1"></param>
    /// <param name="line2"></param>
    /// <returns></returns>
    bool Location_Coincident_Line3D_Line3D_unsafe(
        const Line3DStd::Line3D& line1,
        const Line3DStd::Line3D& line2) {
        return Location_Coincident_Line3D_Line3D_plus_unsafe(line1.p, line1.vec, line2.p, line2.vec);
    }

    /// <summary>
    /// 平行或者共线
    /// </summary>
    /// <param name="seg1Start"></param>
    /// <param name="seg1End"></param>
    /// <param name="seg2Start"></param>
    /// <param name="seg2End"></param>
    /// <returns></returns>
    bool Location_ParallelOrCoincident_LineSegment3D_LineSegment3D_plus_unsafe(
        const Point3DStd::Point3D& seg1Start,
        const Point3DStd::Point3D& seg1End,
        const Point3DStd::Point3D& seg2Start,
        const Point3DStd::Point3D& seg2End) {

        auto vec1 = SubPoint3DPoint3D(seg1End, seg1Start);
        auto vec2 = SubPoint3DPoint3D(seg2End, seg2Start);
        vec1 = Normalization_Point3D(vec1);
        vec2 = Normalization_Point3D(vec2);
        return Equals_Point3D_N(vec1, vec2);
    }

    /// <summary>
    /// 平行或者共线
    /// </summary>
    /// <param name="seg1"></param>
    /// <param name="seg2"></param>
    /// <returns></returns>
    bool Location_ParallelOrCoincident_LineSegment3D_LineSegment3D_unsafe(
        const LineSegment3DStd::LineSegment3D& seg1,
        const LineSegment3DStd::LineSegment3D& seg2) {
        return Location_ParallelOrCoincident_LineSegment3D_LineSegment3D_plus_unsafe(seg1.start, seg1.end, seg2.start, seg2.end);
    }

    /// <summary>
    /// 共线
    /// </summary>
    /// <param name="seg1Start"></param>
    /// <param name="seg1End"></param>
    /// <param name="seg2Start"></param>
    /// <param name="seg2End"></param>
    /// <returns></returns>
    bool Location_Coincident_LineSegment3D_LineSegment3D_plus_unsafe(
        const Point3DStd::Point3D& seg1Start,
        const Point3DStd::Point3D& seg1End,
        const Point3DStd::Point3D& seg2Start,
        const Point3DStd::Point3D& seg2End) {

        auto vec1 = SubPoint3DPoint3D(seg1End, seg1Start);
        auto vec2 = SubPoint3DPoint3D(seg2End, seg2Start);
        vec1 = Normalization_Point3D(vec1);
        vec2 = Normalization_Point3D(vec2);

        if (Equals_Point3D_N(vec1, vec2)) {
            Point3DStd::Point3D vec = SubPoint3DPoint3D(seg2Start, seg1Start);
            Point3DStd::Point3D cross = CrossPoint3DPoint3D(vec1, vec);
            if (IsZero(Length_Point3D(cross))) {
                return true;
            }
        }
        return false;
    }

    /// <summary>
    /// 共线
    /// </summary>
    /// <param name="seg1"></param>
    /// <param name="seg2"></param>
    /// <returns></returns>
    bool Location_Coincident_LineSegment3D_LineSegment3D_unsafe(
        const LineSegment3DStd::LineSegment3D& seg1,
        const LineSegment3DStd::LineSegment3D& seg2) {
        return Location_Coincident_LineSegment3D_LineSegment3D_plus_unsafe(seg1.start, seg1.end, seg2.start, seg2.end);
    }

    /// <summary>
    /// 点是否在射线上，此时已知点和射线在同一个直线上
    /// </summary>
    /// <param name="p"></param>
    /// <param name="rayO"></param>
    /// <param name="rayVec"></param>
    /// <returns></returns>
    bool Location_Point3DonRay3D_SameLine3D_plus_safe(const Point3DStd::Point3D& p, const Point3DStd::Point3D& rayO, const Point3DStd::Point3D& rayVec) {
        Point3DStd::Point3D op = Geometry3DOperateStd::SubPoint3DPoint3D(p, rayO);
        if (Geometry3DOperateStd::DotPoint3DPoint3D(op, rayVec) > -GlobalConstantStd::Eps) {
            return true;
        }
        return false;
    }

    /// <summary>
    /// 点是否在射线上，此时已知点和射线在同一个直线上
    /// </summary>
    /// <param name="p"></param>
    /// <param name="ray"></param>
    /// <returns></returns>
    bool Location_Point3DonRay3D_SameLine3D_safe(const Point3DStd::Point3D& p, const Ray3DStd::Ray3D& ray) {
        return Location_Point3DonRay3D_SameLine3D_plus_safe(p,ray.o,ray.vec);
    }


    bool Location_Point3DOfShadowonLineSegment3D_SameLine3D_plus(const Point3DStd::Point3D& b, const Point3DStd::Point3D& a, const Point3DStd::Point3D& c) {

        return Location_Point3DonLineSegment3D_plus(b, a, c);
    }

    /// <summary>
    /// -1表示在三角形之外，0表示在三角形的边上，1表示在三角形内部
    /// </summary>
    /// <param name="p"></param>
    /// <param name="t1"></param>
    /// <param name="t2"></param>
    /// <param name="t3"></param>
    /// <returns></returns>
    int Location_Point3DonTriangle3D_SamePlane3D_unsafe(const Point3DStd::Point3D& p, const Point3DStd::Point3D& t1, const Point3DStd::Point3D& t2, const Point3DStd::Point3D& t3) {

        if (Location_Point3DonLineSegment3D_plus(p, t1, t2)) {
            return 0;
        }
        if (Location_Point3DonLineSegment3D_plus(p, t2, t3)) {
            return 0;
        }
        if (Location_Point3DonLineSegment3D_plus(p, t3, t1)) {
            return 0;
        }

        Point3DStd::Point3D t1t2 = Geometry3DOperateStd::SubPoint3DPoint3D(t2, t1);
        Point3DStd::Point3D t1p = Geometry3DOperateStd::SubPoint3DPoint3D(p, t1);

        Point3DStd::Point3D cross1 = Geometry3DOperateStd::CrossPoint3DPoint3D(t1t2, t1p);

        Point3DStd::Point3D t2t3 = Geometry3DOperateStd::SubPoint3DPoint3D(t3, t2);
        Point3DStd::Point3D t2p = Geometry3DOperateStd::SubPoint3DPoint3D(p, t2);

        Point3DStd::Point3D cross2 = Geometry3DOperateStd::CrossPoint3DPoint3D(t2t3, t2p);

        if (Geometry3DOperateStd::DotPoint3DPoint3D(cross1, cross2) < GlobalConstantStd::Eps) {
            return -1;
        }

        Point3DStd::Point3D t3t1 = Geometry3DOperateStd::SubPoint3DPoint3D(t1, t3);
        Point3DStd::Point3D t3p = Geometry3DOperateStd::SubPoint3DPoint3D(p, t3);

        Point3DStd::Point3D cross3 = Geometry3DOperateStd::CrossPoint3DPoint3D(t3t1, t3p);

        if (Geometry3DOperateStd::DotPoint3DPoint3D(cross1, cross3) < GlobalConstantStd::Eps) {
            return -1;
        }
        return 1;
    }

}