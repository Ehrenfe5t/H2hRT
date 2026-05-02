
#include"LxQLine3D.h"



namespace Line3DStd {


    Line3D::Line3D() {
        this->p.x = 0;
        this->p.y = 0;
        this->p.z = 0;
        this->vec.x = 0;
        this->vec.y = 0;
        this->vec.z = 0;
    }
    Line3D::Line3D(const Point3DStd::Point3D& p, const Point3DStd::Point3D& vec) {
        this->p.x = p.x;
        this->p.y = p.y;
        this->p.z = p.z;
        this->vec.x = vec.x;
        this->vec.y = vec.y;
        this->vec.z = vec.z;
    }
    Line3D::Line3D(const Line3D& obj) {
        this->p.x = obj.p.x;
        this->p.y = obj.p.y;
        this->p.z = obj.p.z;
        this->vec.x = obj.vec.x;
        this->vec.y = obj.vec.y;
        this->vec.z = obj.vec.z;
    }
    Line3D::~Line3D() {

    }

}

