
#include"LxQPlane3D.h"

namespace Plane3DStd {
   


    Plane3D::~Plane3D() {
    }

    Plane3D::Plane3D(const Point3DStd::Point3D& p, const Point3DStd::Point3D& n) {
        this->p.x = p.x;
        this->p.y = p.y;
        this->p.z = p.z;
        this->n.x = n.x;
        this->n.y = n.y;
        this->n.z = n.z;
    }
    Plane3D::Plane3D() {
        this->p.x = 0;
        this->p.y = 0;
        this->p.z = 0;
        this->n.x = 0;
        this->n.y = 0;
        this->n.z = 0;
    }
    Plane3D::Plane3D(const Plane3D& obj) {
        this->p.x = obj.p.x;
        this->p.y = obj.p.y;
        this->p.z = obj.p.z;
        this->n.x = obj.n.x;
        this->n.y = obj.n.y;
        this->n.z = obj.n.z;
    }
    bool Plane3D::operator<(const Plane3D& obj) const {
        return this->p < obj.p;
    }
}