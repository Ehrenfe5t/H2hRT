

#include"DxQScenarioTriangle3DIndex.h"
#include"QzQGlobalConstant.h"

namespace ScenarioTriangle3DIndexStd {
    bool ScenarioTriangle3DIndex::ReBuild()
    {
        //p1编号最小
        if (TriangleP1Index < TriangleP2Index && TriangleP1Index < TriangleP3Index && TriangleP2Index != TriangleP3Index)
        {
            return true;
        }
        //p2编号最小
        else if (TriangleP2Index < TriangleP1Index && TriangleP2Index < TriangleP3Index && TriangleP1Index != TriangleP3Index)
        {
            int temp = TriangleP1Index;
            this->TriangleP1Index = TriangleP2Index;
            this->TriangleP2Index = TriangleP3Index;
            this->TriangleP3Index = temp;
            return true;
        }
        //p3编号最小
        else if (TriangleP3Index < TriangleP1Index && TriangleP3Index < TriangleP2Index && TriangleP1Index != TriangleP2Index)
        {
            int temp1 = TriangleP1Index;
            int temp2 = TriangleP2Index;
            this->TriangleP1Index = TriangleP3Index;
            this->TriangleP2Index = temp1;
            this->TriangleP3Index = temp2;
            return true;
        }

        return false;
    }
    ScenarioTriangle3DIndex::~ScenarioTriangle3DIndex() {

    }
    ScenarioTriangle3DIndex::ScenarioTriangle3DIndex()
    {
        this->UpTypeNumber = GlobalConstantStd::GetAirSubstanceType();
        this->DownTypeNumber = GlobalConstantStd::GetAirSubstanceType();
        this->Roughness = 0.0;
        this->TriangleP1Index = -1;
        this->TriangleP2Index = -1;
        this->TriangleP3Index = -1;
    }

    ScenarioTriangle3DIndex::ScenarioTriangle3DIndex(int upTypeNumber, int downTypeNumber, double roughness,
        int triangleP1Index, int triangleP2Index, int triangleP3Index, const Point3DStd::Point3D& n)
    {
        this->UpTypeNumber = upTypeNumber;
        this->DownTypeNumber = downTypeNumber;
        this->Roughness = roughness;
        this->n = n;

        //p1编号最小
        if (triangleP1Index < triangleP2Index && triangleP1Index < triangleP3Index)
        {
            this->TriangleP1Index = triangleP1Index;
            this->TriangleP2Index = triangleP2Index;
            this->TriangleP3Index = triangleP3Index;
        }
        //p2编号最小
        else if (triangleP2Index < triangleP1Index && triangleP2Index < triangleP3Index)
        {
            this->TriangleP1Index = triangleP2Index;
            this->TriangleP2Index = triangleP3Index;
            this->TriangleP3Index = triangleP1Index;
        }
        //p3编号最小
        else
        {
            this->TriangleP1Index = triangleP3Index;
            this->TriangleP2Index = triangleP1Index;
            this->TriangleP3Index = triangleP2Index;
        }

    }


    void CopyVectorScenarioTriangle3DIndex(const std::vector<ScenarioTriangle3DIndex>& obj, std::vector<ScenarioTriangle3DIndex>& res) {

        res.clear();
        res.resize(obj.size());

        for (int loop = 0; loop < obj.size(); ++loop) {
            res[loop] = obj[loop];
        }

    }


    /// <summary>
    /// 从json字符串获取点
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void from_json(const nlohmann::json& j, ScenarioTriangle3DIndex& obj) {
        j.at("TriangleP1Index").get_to(obj.TriangleP1Index);
        j.at("TriangleP2Index").get_to(obj.TriangleP2Index);
        j.at("TriangleP3Index").get_to(obj.TriangleP3Index);
        j.at("UpTypeNumber").get_to(obj.UpTypeNumber);
        j.at("DownTypeNumber").get_to(obj.DownTypeNumber);
        j.at("Roughness").get_to(obj.Roughness);
        j.at("N").get_to(obj.n);
    }
    /// <summary>
    /// 将点对象转化为json字符串
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void to_json(nlohmann::json& j, const ScenarioTriangle3DIndex& obj) {
        j = nlohmann::json{
        {"TriangleP1Index",obj.TriangleP1Index },
        {"TriangleP2Index",obj.TriangleP2Index },
        {"TriangleP3Index",obj.TriangleP3Index},
        {"UpTypeNumber",obj.UpTypeNumber},
        {"DownTypeNumber",obj.DownTypeNumber},
        {"Roughness",obj.Roughness},
        {"N",obj.n}
        };
    }
}