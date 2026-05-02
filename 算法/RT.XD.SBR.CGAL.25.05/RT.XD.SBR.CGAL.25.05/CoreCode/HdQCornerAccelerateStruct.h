#pragma once

#include"LxQPoint3D.h"

namespace CornerAccelerateStructStd {

    class CornerAccelerateStruct
    {
    public:

        int triangle_1_index;
        int triangle_2_index;

        Point3DStd::Point3D start_location;
        Point3DStd::Point3D end_location;
        CornerAccelerateStruct();
        ~CornerAccelerateStruct();

    private:

    };



}