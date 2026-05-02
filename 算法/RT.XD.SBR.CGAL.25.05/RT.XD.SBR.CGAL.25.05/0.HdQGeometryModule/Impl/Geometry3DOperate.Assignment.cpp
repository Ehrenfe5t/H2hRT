


#include"../Input.h"


namespace Geometry3DOperateStd {


    void AssignmentPoint3DPoint3D(Point3DStd::Point3D& a, const Point3DStd::Point3D& b) {
        a.x = b.x;
        a.y = b.y;
        a.z = b.z;
    }

    void AssignmentRay3DRay3D(Ray3DStd::Ray3D& r1, const Ray3DStd::Ray3D& r2) {
        AssignmentPoint3DPoint3D(r1.o, r2.o);
        AssignmentPoint3DPoint3D(r1.vec, r2.vec);
    }

    void AssignmentTriangle3DTriangle3D(Triangle3DStd::Triangle3D& t1, const Triangle3DStd::Triangle3D& t2) {
        AssignmentPoint3DPoint3D(t1.p1, t2.p1);
        AssignmentPoint3DPoint3D(t1.p2, t2.p2);
        AssignmentPoint3DPoint3D(t1.p3, t2.p3);
    }
}