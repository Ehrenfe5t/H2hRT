
#include"DxQScenarioDataInformation.h"
#include"HdQCornerAccelerateStructDatabase.h"

namespace ScenarioDataInformationStd {


    ScenarioDataInformation::ScenarioDataInformation()
    {
    }

    ScenarioDataInformation::~ScenarioDataInformation()
    {
    }

    /// <summary>
    /// 从json字符串获取点
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void from_json(const nlohmann::json& j, ScenarioDataInformation& obj) {
        {
            auto jsonStrConfig = j.at("scenarioObject");
            if (!jsonStrConfig.is_null()) {
                jsonStrConfig.get_to(obj.scenarioObject);
            }
        }
    }
    /// <summary>
    /// 将点对象转化为json字符串
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void to_json(nlohmann::json& j, const ScenarioDataInformation& obj) {
        j["scenarioObject"] = obj.scenarioObject;
    }


    bool InitScenarioDataInformationByScenarioObject(
        bool corner,
        const ScenarioObjectStd::ScenarioObject& scenarioObject,
        ScenarioDataInformation& scenarioDataInformation)
    {

        scenarioDataInformation.scenarioObject.scenarioMinPoint3D = scenarioObject.scenarioMinPoint3D;
        scenarioDataInformation.scenarioObject.scenarioMaxPoint3D = scenarioObject.scenarioMaxPoint3D;

        scenarioDataInformation.scenarioObject.scenarioPoint3D.clear();
        for (int i = 0; i < scenarioObject.scenarioPoint3D.size(); ++i) {
            scenarioDataInformation.scenarioObject.scenarioPoint3D.emplace_back(scenarioObject.scenarioPoint3D[i]);
        }

        scenarioDataInformation.scenarioObject.scenarioTriangle3DIndex.clear();
        for (int i = 0; i < scenarioObject.scenarioTriangle3DIndex.size(); ++i) {
            scenarioDataInformation.scenarioObject.scenarioTriangle3DIndex.emplace_back(scenarioObject.scenarioTriangle3DIndex[i]);
        }

        scenarioDataInformation.scenarioObject.scenarioCorner3DIndex.clear();
        for (int i = 0; i < scenarioObject.scenarioCorner3DIndex.size(); ++i) {
            scenarioDataInformation.scenarioObject.scenarioCorner3DIndex.emplace_back(scenarioObject.scenarioCorner3DIndex[i]);
        }

        if (scenarioObject.scenarioTriangle3DIndex.size() > 0) {
            if (!TriangleAccelerateStructDatabaseStd::InitTriangleAccelerateStructDatabaseByScenarioObject(
                scenarioObject, scenarioDataInformation.triangleAccelerateStructDatabase)) {
                return false;
            }

            if (corner) {
                if (scenarioObject.scenarioCorner3DIndex.size() > 0) {
                    if (!CornerAccelerateStructDatabaseStd::InitCornerAccelerateStructDatabaseByScenarioObject(
                        scenarioObject, scenarioDataInformation.cornerAccelerateStructDatabase)) {
                        return false;
                    }
                }
            }
        }

        return true;
    }
}