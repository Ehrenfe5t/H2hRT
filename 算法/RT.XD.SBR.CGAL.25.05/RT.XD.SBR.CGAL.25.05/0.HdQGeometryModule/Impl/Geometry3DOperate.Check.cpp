


#include"../Input.h"


namespace Geometry3DOperateStd {


    /// <summary>
    /// 判断是否是标准化直线
    /// </summary>
    /// <param name="r"></param>
    /// <returns></returns>
    bool CheckLine3D(const Line3DStd::Line3D& line)
    {
        if (IsZeroAbs(1.0 - Length_Point3D(line.vec))) {
            return true;
        }
        return false;
    }




}