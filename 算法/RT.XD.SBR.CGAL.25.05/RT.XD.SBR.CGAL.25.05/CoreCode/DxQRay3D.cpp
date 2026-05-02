
#include"DxQRay3D.h"


namespace Ray3DStd {

    Ray3D::Ray3D(const Point3DStd::Point3D& o, const Point3DStd::Point3D& vec) {
        this->o.x = o.x;
        this->o.y = o.y;
        this->o.z = o.z;
        this->vec.x = vec.x;
        this->vec.y = vec.y;
        this->vec.z = vec.z;
    }

    Ray3D::Ray3D() {
    }

    Ray3D::Ray3D(const Ray3D& obj) {
        this->o.x = obj.o.x;
        this->o.y = obj.o.y;
        this->o.z = obj.o.z;
        this->vec.x = obj.vec.x;
        this->vec.y = obj.vec.y;
        this->vec.z = obj.vec.z;

    }
    Ray3D::~Ray3D() {
    }
}