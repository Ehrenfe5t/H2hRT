



#include"../Input.h"

namespace Geometry3DOperateStd {

    /// <summary>
    /// 计算向量长度
    /// </summary>
    /// <param name="p"></param>
    /// <returns></returns>
    double Length_Point3D(const Point3DStd::Point3D& p)
    {
        return sqrt(DotPoint3DPoint3D(p, p));
    }
}

