#pragma once

#include"QzQJson.hpp"
#include<vector>
namespace ScenarioCorner3DIndexStd {

    class ScenarioCorner3DIndex
    {
    public:
        int ThetaDiv;
        int P1Index;
        int P2Index;

        /// <summary>
        /// P3Face0Index和公共线段构成三角面元1
        /// </summary>
        int P3Face0Index;

        /// <summary>
        /// P3FaceNIndex和公共线段构成三角面元2
        /// </summary>
        int P3FaceNIndex;

        int Face0Index;
        int FaceNIndex;
        double Radius;

        ScenarioCorner3DIndex();

        ~ScenarioCorner3DIndex();

        bool ReBuild();
        ScenarioCorner3DIndex(int thetaDiv, double radius, int p1Index, int p2Index, 
            int p3Face0Index, int p3FaceNIndex, int face0Index, int faceNIndex);
    };
    /// <summary>
    /// 从json字符串获取点
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void from_json(const nlohmann::json& j, ScenarioCorner3DIndex& obj);

    /// <summary>
    /// 将点对象转化为json字符串
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void to_json(nlohmann::json& j, const ScenarioCorner3DIndex& obj);

    int FindIndexOfScenarioCorner3DIndex(const ScenarioCorner3DIndexStd::ScenarioCorner3DIndex& obj,
        const std::vector<ScenarioCorner3DIndexStd::ScenarioCorner3DIndex>& list);
}