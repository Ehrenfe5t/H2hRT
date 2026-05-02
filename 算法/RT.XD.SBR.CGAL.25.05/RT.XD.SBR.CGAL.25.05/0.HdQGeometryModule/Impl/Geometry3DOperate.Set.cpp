

#include"../Input.h"

namespace Geometry3DOperateStd {


    /// <summary>
    /// 有没有空间位置重复的点,-1表示没有
    /// </summary>
    /// <param name="value"></param>
    /// <param name="list"></param>
    /// <returns></returns>
    int Set_Contain_Point3DInVectorPoint3D_Absolute(const Point3DStd::Point3D& value, const std::vector<Point3DStd::Point3D>& list) {
        for (int i = 0; i < list.size(); i++) {
            if (Equals_Point3D_Absolute(value, list[i])) {
                return i;
            }
        }
        return -1;
    }

    int Set_Contain_Point3DInVectorPoint3D(const Point3DStd::Point3D& value, const std::vector<Point3DStd::Point3D>& list) {
        for (int i = 0; i < list.size(); i++) {
            if (Equals_Point3D(value, list[i])) {
                return i;
            }
        }
        return -1;
    }

}