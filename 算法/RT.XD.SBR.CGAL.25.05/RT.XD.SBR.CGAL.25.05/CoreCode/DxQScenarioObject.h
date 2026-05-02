#pragma once

#include<vector>
#include"DxQScenarioTriangle3DIndex.h"
#include"DxQScenarioCorner3DIndex.h"

namespace ScenarioObjectStd {



    class ScenarioObject
    {
    public:

        /// <summary>
        /// 场景包围盒，最小点
        /// </summary>
        Point3DStd::Point3D scenarioMinPoint3D;
        /// <summary>
        /// 场景包围盒，最大点
        /// </summary>
        Point3DStd::Point3D scenarioMaxPoint3D;

        std::vector<Point3DStd::Point3D> scenarioPoint3D;
        std::vector<ScenarioTriangle3DIndexStd::ScenarioTriangle3DIndex> scenarioTriangle3DIndex;
        std::vector<ScenarioCorner3DIndexStd::ScenarioCorner3DIndex> scenarioCorner3DIndex;

        ScenarioObject();

        ScenarioObject(
            const std::vector<Point3DStd::Point3D>& scenarioPoint3D,
            const std::vector<ScenarioTriangle3DIndexStd::ScenarioTriangle3DIndex>& scenarioTriangle3DIndex,
            const std::vector<ScenarioCorner3DIndexStd::ScenarioCorner3DIndex>& scenarioCorner3DIndex);

        ~ScenarioObject();



    };



    /// <summary>
   /// 从json字符串获取点
   /// </summary>
   /// <param name="j"></param>
   /// <param name="p"></param>
    void from_json(const nlohmann::json& j, ScenarioObject& obj);

    /// <summary>
    /// 将点对象转化为json字符串
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void to_json(nlohmann::json& j, const ScenarioObject& obj);

    void CheckScenarioObject(ScenarioObjectStd::ScenarioObject& scenarioObject);
    void RebuildEdgeInformation(ScenarioObjectStd::ScenarioObject& scenarioObject);


    void ObtainScenarioBoundingBox(ScenarioObject& scenarioAccelerate);

    void ChangeScenarioMinMaxPoint3D(const Point3DStd::Point3D& p, ScenarioObject& scenarioAccelerate);
}




