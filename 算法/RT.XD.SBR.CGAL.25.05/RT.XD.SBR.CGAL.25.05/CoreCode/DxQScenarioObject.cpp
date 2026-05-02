
#include"DxQScenarioObject.h"
#include"QzQGeometry3DOperate.Location.h"
#include"QzQGeometry3DOperate.Point3D.h"
#include"QzQGeometry3DOperate.CoordinateSystem.h"
#include"QzQGeometry3DOperate.Normalization.h"

#include"QzQGlobalConstant.h"
#include"LxQProjectDependencies.h"
#include"DxQVectorOperate.h"

namespace ScenarioObjectStd {


    ScenarioObject::ScenarioObject()
    {

    }

    ScenarioObject::ScenarioObject(const std::vector<Point3DStd::Point3D>& scenarioPoint3D,
        const std::vector<ScenarioTriangle3DIndexStd::ScenarioTriangle3DIndex>& scenarioTriangle3DIndex,
        const std::vector<ScenarioCorner3DIndexStd::ScenarioCorner3DIndex>& scenarioCorner3DIndex)
    {
        this->scenarioPoint3D = scenarioPoint3D;
        this->scenarioTriangle3DIndex = scenarioTriangle3DIndex;
        this->scenarioCorner3DIndex = scenarioCorner3DIndex;
    }

    ScenarioObject::~ScenarioObject()
    {

    }


    /// <summary>
   /// 从json字符串获取点
   /// </summary>
   /// <param name="j"></param>
   /// <param name="p"></param>
    void from_json(const nlohmann::json& j, ScenarioObject& obj) {
        {
            auto jsonStrConfig = j.at("scenarioMinPoint3D");
            if (!jsonStrConfig.is_null()) {
                jsonStrConfig.get_to(obj.scenarioMinPoint3D);
            }
        }
        {
            auto jsonStrConfig = j.at("scenarioMaxPoint3D");
            if (!jsonStrConfig.is_null()) {
                jsonStrConfig.get_to(obj.scenarioMaxPoint3D);
            }
        }
        {
            auto jsonStrConfig = j.at("scenarioPoint3D");
            if (!jsonStrConfig.is_null()) {
                jsonStrConfig.get_to(obj.scenarioPoint3D);
            }
        }
        {
            auto jsonStrConfig = j.at("scenarioTriangle3DIndex");
            if (!jsonStrConfig.is_null()) {
                jsonStrConfig.get_to(obj.scenarioTriangle3DIndex);
            }
        }
        {
            auto jsonStrConfig = j.at("scenarioCorner3DIndex");
            if (!jsonStrConfig.is_null()) {
                jsonStrConfig.get_to(obj.scenarioCorner3DIndex);
            }
        }
    }
    /// <summary>
    /// 将点对象转化为json字符串
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void to_json(nlohmann::json& j, const ScenarioObject& obj) {
        j["scenarioMinPoint3D"] = obj.scenarioMinPoint3D;
        j["scenarioMaxPoint3D"] = obj.scenarioMaxPoint3D;
        j["scenarioPoint3D"] = obj.scenarioPoint3D;
        j["scenarioTriangle3DIndex"] = obj.scenarioTriangle3DIndex;
        j["scenarioCorner3DIndex"] = obj.scenarioCorner3DIndex;
    }

    bool ABandCDcollinear_99(
        const Point3DStd::Point3D& a,
        const Point3DStd::Point3D& lineSegment1start,
        const Point3DStd::Point3D& lineSegment1end,
        const Point3DStd::Point3D& b,
        const Point3DStd::Point3D& lineSegment2start,
        const Point3DStd::Point3D& lineSegment2end)
    {
        if (Geometry3DOperateStd::Location_Point3DonLineSegment3D_plus(a, lineSegment1start, lineSegment1end))
        {
            if (Geometry3DOperateStd::Location_Point3DonLineSegment3D_plus(b, lineSegment2start, lineSegment2end))
            {
                return true;
            }
        }
        return false;
    }

    bool CurRes(ScenarioCorner3DIndexStd::ScenarioCorner3DIndex& obj, int startIndex, int endIndex, int triangle1P1Index, int triangle2P1Index, int face0Index, int faceNIndex) {
        if (startIndex == endIndex) {
            return false;
        }
        obj.P1Index = startIndex;
        obj.P2Index = endIndex;
        obj.Face0Index = face0Index;
        obj.FaceNIndex = faceNIndex;
        obj.P3Face0Index = triangle1P1Index;
        obj.P3FaceNIndex = triangle2P1Index;

        return true;
    }

    bool abcdef(
        const Point3DStd::Point3D& A, int Aindex,
        const Point3DStd::Point3D& B, int Bindex,
        const Point3DStd::Point3D& C, int Cindex1,
        const Point3DStd::Point3D& D, int Dindex,
        const Point3DStd::Point3D& E, int Eindex,
        const Point3DStd::Point3D& F, int Findex1,
        int triangle1P1Index, int triangle2P1Index, int face0Index, int faceNIndex,
        ScenarioCorner3DIndexStd::ScenarioCorner3DIndex& obj)
    {
        //AB是边
        if (ABandCDcollinear_99(A, D, E, B, D, E))
        {
            int startIndex = Aindex;
            int endIndex = Bindex;
            return CurRes(obj, startIndex, endIndex, triangle1P1Index, triangle2P1Index, face0Index, faceNIndex);

        }

        //AD是边
        if (ABandCDcollinear_99(A, D, E, D, A, B))
        {
            int startIndex = Aindex;
            int endIndex = Dindex;
            return CurRes(obj, startIndex, endIndex, triangle1P1Index, triangle2P1Index, face0Index, faceNIndex);
        }
        //AE是边
        if (ABandCDcollinear_99(A, D, E, E, A, B))
        {
            int startIndex = Aindex;
            int endIndex = Eindex;
            return CurRes(obj, startIndex, endIndex, triangle1P1Index, triangle2P1Index, face0Index, faceNIndex);
        }

        //BD是边
        if (ABandCDcollinear_99(B, D, E, D, A, B))
        {
            int startIndex = Bindex;
            int endIndex = Dindex;
            return CurRes(obj, startIndex, endIndex, triangle1P1Index, triangle2P1Index, face0Index, faceNIndex);
        }

        //BE是边
        if (ABandCDcollinear_99(B, D, E, E, A, B))
        {
            int startIndex = Bindex;
            int endIndex = Eindex;
            return CurRes(obj, startIndex, endIndex, triangle1P1Index, triangle2P1Index, face0Index, faceNIndex);
        }

        //DE是边
        if (ABandCDcollinear_99(D, A, B, E, A, B))
        {
            int startIndex = Dindex;
            int endIndex = Eindex;
            return CurRes(obj, startIndex, endIndex, triangle1P1Index, triangle2P1Index, face0Index, faceNIndex);
        }


        return false;
    }

    bool RebuildScenarioCorner3DIndexPlus(
        const std::vector<Point3DStd::Point3D>& scenarioPoint3D,
        int face0Index,
        const ScenarioTriangle3DIndexStd::ScenarioTriangle3DIndex& triangleIndexObject1,
        int faceNIndex,
        const ScenarioTriangle3DIndexStd::ScenarioTriangle3DIndex& triangleIndexObject2,
        ScenarioCorner3DIndexStd::ScenarioCorner3DIndex& obj) {

        //弃用，使用phiE进行筛选更合理
        //double angle = Geometry3DOperateStd::GetAngle(triangleIndexObject1.n, triangleIndexObject2.n) * 180.0 / GlobalConstantStd::Pi;
        //if (angle < 5 || angle > 175)
        //{
        //    return false;
        //}

        //ABC，a<b,a<c，b和c的大小无法确;，d<e,d<f，e和f的大小无法确定，
        int Aindex = triangleIndexObject1.TriangleP1Index;
        int Bindex = triangleIndexObject1.TriangleP2Index;
        int Cindex = triangleIndexObject1.TriangleP3Index;
        int Dindex = triangleIndexObject2.TriangleP1Index;
        int Eindex = triangleIndexObject2.TriangleP2Index;
        int Findex = triangleIndexObject2.TriangleP3Index;

        auto A = scenarioPoint3D[Aindex];
        auto B = scenarioPoint3D[Bindex];
        auto C = scenarioPoint3D[Cindex];
        auto D = scenarioPoint3D[Dindex];
        auto E = scenarioPoint3D[Eindex];
        auto F = scenarioPoint3D[Findex];

        //AB与DE共线
        if (Geometry3DOperateStd::Location_Coincident_LineSegment3D_LineSegment3D_plus_unsafe(A, B, D, E))
        {
            int triangle1P1Index = Cindex;
            int triangle2P1Index = Findex;
            return abcdef(A, Aindex, B, Bindex, C, Cindex, D, Dindex, E, Eindex, F, Findex, triangle1P1Index, triangle2P1Index, face0Index, faceNIndex, obj);
        }
        //AB与EF共线
        if (Geometry3DOperateStd::Location_Coincident_LineSegment3D_LineSegment3D_plus_unsafe(A, B, E, F))
        {
            int triangle1P1Index = Cindex;
            int triangle2P1Index = Dindex;
            return abcdef(A, Aindex, B, Bindex, C, Cindex, E, Eindex, F, Findex, D, Dindex, triangle1P1Index, triangle2P1Index, face0Index, faceNIndex, obj);
        }
        //AB与FD共线
        if (Geometry3DOperateStd::Location_Coincident_LineSegment3D_LineSegment3D_plus_unsafe(A, B, F, D))
        {
            int triangle1P1Index = Cindex;
            int triangle2P1Index = Eindex;
            return abcdef(A, Aindex, B, Bindex, C, Cindex, F, Findex, D, Dindex, E, Eindex, triangle1P1Index, triangle2P1Index, face0Index, faceNIndex, obj);
        }

        //BC与DE共线
        if (Geometry3DOperateStd::Location_Coincident_LineSegment3D_LineSegment3D_plus_unsafe(B, C, D, E))
        {
            int triangle1P1Index = Aindex;
            int triangle2P1Index = Findex;
            return abcdef(B, Bindex, C, Cindex, A, Aindex, D, Dindex, E, Eindex, F, Findex, triangle1P1Index, triangle2P1Index, face0Index, faceNIndex, obj);
        }
        //BC与EF共线
        if (Geometry3DOperateStd::Location_Coincident_LineSegment3D_LineSegment3D_plus_unsafe(B, C, E, F))
        {
            int triangle1P1Index = Aindex;
            int triangle2P1Index = Dindex;
            return abcdef(B, Bindex, C, Cindex, A, Aindex, E, Eindex, F, Findex, D, Dindex, triangle1P1Index, triangle2P1Index, face0Index, faceNIndex, obj);
        }
        //BC与FD共线
        if (Geometry3DOperateStd::Location_Coincident_LineSegment3D_LineSegment3D_plus_unsafe(B, C, F, D))
        {
            int triangle1P1Index = Aindex;
            int triangle2P1Index = Eindex;
            return abcdef(B, Bindex, C, Cindex, A, Aindex, F, Findex, D, Dindex, E, Eindex, triangle1P1Index, triangle2P1Index, face0Index, faceNIndex, obj);
        }

        //CA与DE共线
        if (Geometry3DOperateStd::Location_Coincident_LineSegment3D_LineSegment3D_plus_unsafe(C, A, D, E))
        {
            int triangle1P1Index = Bindex;
            int triangle2P1Index = Findex;
            return abcdef(C, Cindex, A, Aindex, B, Bindex, D, Dindex, E, Eindex, F, Findex, triangle1P1Index, triangle2P1Index, face0Index, faceNIndex, obj);
        }
        //CA与EF共线
        if (Geometry3DOperateStd::Location_Coincident_LineSegment3D_LineSegment3D_plus_unsafe(C, A, E, F))
        {
            int triangle1P1Index = Bindex;
            int triangle2P1Index = Dindex;
            return abcdef(C, Cindex, A, Aindex, B, Bindex, E, Eindex, F, Findex, D, Dindex, triangle1P1Index, triangle2P1Index, face0Index, faceNIndex, obj);
        }
        //CA与FD共线
        if (Geometry3DOperateStd::Location_Coincident_LineSegment3D_LineSegment3D_plus_unsafe(C, A, F, D))
        {
            int triangle1P1Index = Bindex;
            int triangle2P1Index = Eindex;
            return abcdef(C, Cindex, A, Aindex, B, Bindex, F, Findex, D, Dindex, E, Eindex, triangle1P1Index, triangle2P1Index, face0Index, faceNIndex, obj);
        }

        return false;
    }

    void CheckScenarioObject(ScenarioObjectStd::ScenarioObject& scenarioObject) {
        {

            std::vector<Point3DStd::Point3D> points;
            std::vector<ScenarioTriangle3DIndexStd::ScenarioTriangle3DIndex> triangles;
            Point3DStd::Point3D yiqi;

            std::vector<int> del_Index;
            for (int i = 0; i < scenarioObject.scenarioTriangle3DIndex.size(); ++i) {
                auto old = scenarioObject.scenarioTriangle3DIndex[i];
                if (old.ReBuild()) {
                    auto p1 = scenarioObject.scenarioPoint3D[old.TriangleP1Index];
                    auto p2 = scenarioObject.scenarioPoint3D[old.TriangleP2Index];
                    auto p3 = scenarioObject.scenarioPoint3D[old.TriangleP3Index];

                    Point3DStd::Point3D n1 = Geometry3DOperateStd::SubPoint3DPoint3D(p1, p2);
                    Point3DStd::Point3D n2 = Geometry3DOperateStd::SubPoint3DPoint3D(p1, p3);
                    Point3DStd::Point3D temp111 = Geometry3DOperateStd::CrossPoint3DPoint3D(n1, n2);
                    if (!Geometry3DOperateStd::Normalization_Point3D_safe(temp111, yiqi)) {
                        del_Index.emplace_back(i);
                        continue;
                    }

                    int index1 = Geometry3DOperateStd::AddPointAndReturnIndex(p1, points);
                    int index2 = Geometry3DOperateStd::AddPointAndReturnIndex(p2, points);
                    int index3 = Geometry3DOperateStd::AddPointAndReturnIndex(p3, points);
                    ScenarioTriangle3DIndexStd::ScenarioTriangle3DIndex triangle(
                        old.UpTypeNumber, old.DownTypeNumber, old.Roughness, index1, index2, index3, old.n);
                    if (triangle.ReBuild()) {
                        triangles.emplace_back(triangle);
                    }
                    else {
                        del_Index.emplace_back(i);
                    }
                }
                else {
                    del_Index.emplace_back(i);
                }
            }

            for (int i = (int)scenarioObject.scenarioCorner3DIndex.size() - 1;i>=0; --i) {
                int triangle1_index = scenarioObject.scenarioCorner3DIndex[i].Face0Index;
                if (VectorOperateStd::ContainIntInVectorInt(triangle1_index, del_Index) > -1) {
                    scenarioObject.scenarioCorner3DIndex.erase(scenarioObject.scenarioCorner3DIndex.begin() + i);
                }
                else {
                    int triangle2_index = scenarioObject.scenarioCorner3DIndex[i].FaceNIndex;
                    if (VectorOperateStd::ContainIntInVectorInt(triangle2_index, del_Index) > -1) {
                        scenarioObject.scenarioCorner3DIndex.erase(scenarioObject.scenarioCorner3DIndex.begin() + i);
                    }
                }
            }

            //std::cout << del_Index.size() << std::endl;

            scenarioObject.scenarioPoint3D.clear();
            scenarioObject.scenarioTriangle3DIndex.clear();
            for (int i = 0; i < points.size(); ++i) {
                scenarioObject.scenarioPoint3D.emplace_back(points[i]);
            }
            for (int i = 0; i < triangles.size(); ++i) {
                scenarioObject.scenarioTriangle3DIndex.emplace_back(triangles[i]);
            }

        }
    }

    void RebuildEdgeInformation(ScenarioObjectStd::ScenarioObject& scenarioObject) {

        ProjectDependenciesStd::DisplayPromptOrReason("重构边开始", false, __FILE__, __LINE__);

        //对点和三角形进行重构
        CheckScenarioObject(scenarioObject);

        scenarioObject.scenarioCorner3DIndex.clear();
        //key表示点的索引，std::vector<int>表示映射的三角形的编号
        std::map<int, std::vector<int>> triangle_index_map;
        for (int i = 0; i < (int)scenarioObject.scenarioTriangle3DIndex.size(); ++i) {
            auto old = scenarioObject.scenarioTriangle3DIndex[i];
            if (!old.ReBuild()) {
                continue;
            }
            triangle_index_map[old.TriangleP1Index].emplace_back(i);
            triangle_index_map[old.TriangleP2Index].emplace_back(i);
            triangle_index_map[old.TriangleP3Index].emplace_back(i);
        }

        for (int i = 0; i < (int)scenarioObject.scenarioTriangle3DIndex.size(); ++i) {
            auto old = scenarioObject.scenarioTriangle3DIndex[i];

            if (i % 500 == 199) {
                std::ostringstream oss;
                oss << "进展：" << 100.0 * (i + 0.01) / scenarioObject.scenarioTriangle3DIndex.size() << " %";
                ProjectDependenciesStd::DisplayPromptOrReason(oss.str(), false, __FILE__, __LINE__);
            }
            if (!old.ReBuild()) {
                continue;
            }

            std::vector<int> triangle_Index;
            for (int loop11 = 0; loop11 < triangle_index_map[old.TriangleP1Index].size(); ++loop11) {
                if (i == triangle_index_map[old.TriangleP1Index][loop11]) {
                    continue;
                }
                if (-1 == VectorOperateStd::ContainIntInVectorInt(triangle_index_map[old.TriangleP1Index][loop11], triangle_Index)) {
                    triangle_Index.emplace_back(triangle_index_map[old.TriangleP1Index][loop11]);
                }
            }

            for (int loop12 = 0; loop12 < triangle_index_map[old.TriangleP2Index].size(); ++loop12) {
                if (i == triangle_index_map[old.TriangleP2Index][loop12]) {
                    continue;
                }
                if (-1 == VectorOperateStd::ContainIntInVectorInt(triangle_index_map[old.TriangleP2Index][loop12], triangle_Index)) {
                    triangle_Index.emplace_back(triangle_index_map[old.TriangleP2Index][loop12]);
                }
            }
            for (int loop13 = 0; loop13 < triangle_index_map[old.TriangleP3Index].size(); ++loop13) {
                if (i == triangle_index_map[old.TriangleP3Index][loop13]) {
                    continue;
                }
                if (-1 == VectorOperateStd::ContainIntInVectorInt(triangle_index_map[old.TriangleP3Index][loop13], triangle_Index)) {
                    triangle_Index.emplace_back(triangle_index_map[old.TriangleP3Index][loop13]);
                }
            }


            for (int loop = 0; loop < triangle_Index.size(); ++loop) {
                int j = triangle_Index[loop];
                if (!scenarioObject.scenarioTriangle3DIndex[j].ReBuild()) {
                    continue;
                }
                ScenarioCorner3DIndexStd::ScenarioCorner3DIndex obj;
                if (RebuildScenarioCorner3DIndexPlus(scenarioObject.scenarioPoint3D, i, scenarioObject.scenarioTriangle3DIndex[i], j, scenarioObject.scenarioTriangle3DIndex[j], obj)) {
                    if (-1 == FindIndexOfScenarioCorner3DIndex(obj, scenarioObject.scenarioCorner3DIndex))
                    {
                        if (obj.ReBuild()) {
                            bool flag = true;
                            for (int looppp = 0; looppp < scenarioObject.scenarioCorner3DIndex.size(); ++looppp) {
                                if (obj.P1Index == scenarioObject.scenarioCorner3DIndex[looppp].P1Index) {
                                    if (obj.P2Index == scenarioObject.scenarioCorner3DIndex[looppp].P2Index) {
                                        flag = false;
                                        break;
                                    }
                                }
                            }
                            if (flag) {
                                //对非法边进行筛选，
                                //phiE小于180.5的进行删除
                                Point3DStd::Point3D start = scenarioObject.scenarioPoint3D[obj.P1Index];
                                Point3DStd::Point3D end = scenarioObject.scenarioPoint3D[obj.P2Index];
                                Point3DStd::Point3D triangle_0_p3 = scenarioObject.scenarioPoint3D[obj.P3Face0Index];
                                Point3DStd::Point3D triangle_n_p3 = scenarioObject.scenarioPoint3D[obj.P3FaceNIndex];
                                Point3DStd::Point3D face0_n = scenarioObject.scenarioTriangle3DIndex[obj.Face0Index].n;

                                if (!Geometry3DOperateStd::CanBuildDiffractionCoordinateSystemByCorner(
                                    start, end, triangle_0_p3, triangle_n_p3, face0_n)) {
                                    continue;
                                }

                                scenarioObject.scenarioCorner3DIndex.emplace_back(obj);
                            }

                        }
                    }
                }
            }


        }

        //std::cout << std::endl << scenarioObject.scenarioCorner3DIndex.size() << std::endl;

        ProjectDependenciesStd::DisplayPromptOrReason("重构边结束", false, __FILE__, __LINE__);
    }

    void ObtainScenarioBoundingBox(ScenarioObject& scenarioAccelerate) {

        scenarioAccelerate.scenarioMaxPoint3D.x = -GlobalConstantStd::BoundingBoxLength;
        scenarioAccelerate.scenarioMaxPoint3D.y = -GlobalConstantStd::BoundingBoxLength;
        scenarioAccelerate.scenarioMaxPoint3D.z = -GlobalConstantStd::BoundingBoxLength;
        scenarioAccelerate.scenarioMinPoint3D.x = GlobalConstantStd::BoundingBoxLength;
        scenarioAccelerate.scenarioMinPoint3D.y = GlobalConstantStd::BoundingBoxLength;
        scenarioAccelerate.scenarioMinPoint3D.z = GlobalConstantStd::BoundingBoxLength;
        for (int i = 0; i < scenarioAccelerate.scenarioPoint3D.size(); ++i) {
            Geometry3DOperateStd::ChangeSenceMinPoint3D_point(scenarioAccelerate.scenarioPoint3D[i], scenarioAccelerate.scenarioMinPoint3D);
            Geometry3DOperateStd::ChangeSenceMaxPoint3D_point(scenarioAccelerate.scenarioPoint3D[i], scenarioAccelerate.scenarioMaxPoint3D);
        }
    }

    void ChangeScenarioMinMaxPoint3D(const Point3DStd::Point3D& p, ScenarioObject& scenarioAccelerate) {

        Geometry3DOperateStd::ChangeSenceMinPoint3D_point(p, scenarioAccelerate.scenarioMinPoint3D);
        Geometry3DOperateStd::ChangeSenceMaxPoint3D_point(p, scenarioAccelerate.scenarioMaxPoint3D);

    }
}