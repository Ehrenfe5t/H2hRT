
#include"DxQScenarioCorner3DIndex.h"

#include<vector>
namespace ScenarioCorner3DIndexStd {


    ScenarioCorner3DIndex::ScenarioCorner3DIndex()
    {
        this->ThetaDiv = 6;
        this->Radius = 0.05;

        this->P1Index = -1;
        this->P2Index = -1;
        this->P3Face0Index = -1;
        this->P3FaceNIndex = -1;
        this->Face0Index = -1;
        this->FaceNIndex = -1;
    }

    ScenarioCorner3DIndex::~ScenarioCorner3DIndex()
    {
    }

    bool ScenarioCorner3DIndex::ReBuild()
    {
        if (P1Index< P2Index) {
            return true;
        }
        else if (P1Index > P2Index) {
            int temp = P1Index;
            P1Index = P2Index;
            P2Index = temp;
            return true;
        }
        return false;
    }

    ScenarioCorner3DIndex::ScenarioCorner3DIndex(int thetaDiv, double radius, int p1Index, int p2Index, int p3Face0Index, int p3FaceNIndex, int face0Index, int faceNIndex)
    {
        this->ThetaDiv = thetaDiv;
        this->Radius = radius;
        //P1Index,P2Index,P3Face0Index,组成的三角形和Face0Index代表的三角形可能法相相反
        if (p1Index < p2Index)
        {
            this->P1Index = p1Index;
            this->P2Index = p2Index;
        }
        else
        {
            this->P1Index = p2Index;
            this->P2Index = p1Index;
        }
        this->P3Face0Index = p3Face0Index;
        this->P3FaceNIndex = p3FaceNIndex;
        this->Face0Index = face0Index;
        this->FaceNIndex = faceNIndex;
    }



    /// <summary>
    /// 从json字符串获取点
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void from_json(const nlohmann::json& j, ScenarioCorner3DIndex& obj) {
        {
            auto jsonStrConfig = j.at("Face0Index");
            if (!jsonStrConfig.is_null()) {
                jsonStrConfig.get_to(obj.Face0Index);
            }
        }
        {
            auto jsonStrConfig = j.at("FaceNIndex");
            if (!jsonStrConfig.is_null()) {
                jsonStrConfig.get_to(obj.FaceNIndex);
            }
        }
        {
            auto jsonStrConfig = j.at("P1Index");
            if (!jsonStrConfig.is_null()) {
                jsonStrConfig.get_to(obj.P1Index);
            }
        }
        {
            auto jsonStrConfig = j.at("P2Index");
            if (!jsonStrConfig.is_null()) {
                jsonStrConfig.get_to(obj.P2Index);
            }
        }
        {
            auto jsonStrConfig = j.at("P3Face0Index");
            if (!jsonStrConfig.is_null()) {
                jsonStrConfig.get_to(obj.P3Face0Index);
            }
        }
        {
            auto jsonStrConfig = j.at("P3FaceNIndex");
            if (!jsonStrConfig.is_null()) {
                jsonStrConfig.get_to(obj.P3FaceNIndex);
            }
        }
    }
    /// <summary>
    /// 将点对象转化为json字符串
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void to_json(nlohmann::json& j, const ScenarioCorner3DIndex& obj) {
        j["Face0Index"] = obj.Face0Index;
        j["FaceNIndex"] = obj.FaceNIndex;
        j["P1Index"] = obj.P1Index;
        j["P2Index"] = obj.P2Index;
        j["P3Face0Index"] = obj.P3Face0Index;
        j["P3FaceNIndex"] = obj.P3FaceNIndex;
        //j["Radius"] = obj.Radius;
    }

    int FindIndexOfScenarioCorner3DIndex(const ScenarioCorner3DIndexStd::ScenarioCorner3DIndex& obj, const std::vector<ScenarioCorner3DIndexStd::ScenarioCorner3DIndex>& list) {

        for (int i = 0; i < list.size(); ++i) {
            if (obj.P1Index == list[i].P1Index) {
                if (obj.P2Index == list[i].P2Index) {
                    return i;
                }
            }
        }
        return -1;
    }
}