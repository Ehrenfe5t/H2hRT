
#include"LxQLineSegment3D.h"

namespace LineSegment3DStd {

    LineSegment3D::LineSegment3D() {
        this->start.x = 0.0;
        this->start.y = 0.0;
        this->start.z = 0.0;
        this->end.x = 0.0;
        this->end.y = 0.0;
        this->end.z = 0.0;
    }
    LineSegment3D::LineSegment3D(const Point3DStd::Point3D& start, const Point3DStd::Point3D& end) {
        this->start.x = start.x;
        this->start.y = start.y;
        this->start.z = start.z;
        this->end.x = end.x;
        this->end.y = end.y;
        this->end.z = end.z;
    }
    LineSegment3D::LineSegment3D(const LineSegment3D& obj) {
        this->start.x = obj.start.x;
        this->start.y = obj.start.y;
        this->start.z = obj.start.z;
        this->end.x = obj.end.x;
        this->end.y = obj.end.y;
        this->end.z = obj.end.z;
    }
    LineSegment3D::~LineSegment3D() {
    }

}

