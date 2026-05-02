#pragma once

#include"QzQJson.hpp"
#include"LxQPoint3D.h"
#include<vector>

namespace ScenarioTriangle3DIndexStd {

    class ScenarioTriangle3DIndex
    {
    public:
        int UpTypeNumber;
        int DownTypeNumber;
        int TriangleP1Index;
        int TriangleP2Index;
        int TriangleP3Index;
        double Roughness;
        Point3DStd::Point3D n;
        ScenarioTriangle3DIndex();

        ScenarioTriangle3DIndex(int upTypeNumber, int downTypeNumber, double roughness,
            int triangleP1Index, int triangleP2Index, int triangleP3Index, const Point3DStd::Point3D& n);

        bool ReBuild();
        ~ScenarioTriangle3DIndex();
    };

    void CopyVectorScenarioTriangle3DIndex(const std::vector<ScenarioTriangle3DIndex>& obj, std::vector<ScenarioTriangle3DIndex>& res);


    /// <summary>
    /// 从json字符串获取点
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void from_json(const nlohmann::json& j, ScenarioTriangle3DIndex& obj);

    /// <summary>
    /// 将点对象转化为json字符串
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void to_json(nlohmann::json& j, const ScenarioTriangle3DIndex& obj);
}