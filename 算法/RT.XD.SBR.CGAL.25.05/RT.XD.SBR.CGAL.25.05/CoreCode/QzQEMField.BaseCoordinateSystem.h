#pragma once

#include"HdQBaseCoordinateSystem.h"

namespace EMFieldBaseCoordinateSystemStd{


    BaseCoordinateSystemStd::BaseCoordinateSystem EMField_BaseCoordinateSystem_thetai0(
        double dot_r_n,
        const Point3DStd::Point3D& n,
        const Point3DStd::Point3D& y,
        const Point3DStd::Point3D& cur_point);


    BaseCoordinateSystemStd::BaseCoordinateSystem EMField_BaseCoordinateSystem(
        double dot_r_n,
        const Point3DStd::Point3D& n,
        const Point3DStd::Point3D& r,
        const Point3DStd::Point3D& cur_point);

}