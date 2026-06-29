// 文件目标：
// - 实现模块2 OBJ 导入器。
//
// 主要功能：
// - 读取 OBJ 文本；
// - 解析对象块、顶点、法向和三角面；
// - 为批次2生成 Scene 的原始几何区与对象记录。

#include "OBJImporter.h"

#include "../../core/common/math/Vec3.h"

#include <cstdlib>
#include <fstream>
#include <sstream>

namespace rt {

namespace {

std::string BuildLineLocationMessage(int lineNumber)
{
    std::ostringstream stream;
    stream << "OBJ parse error near line " << lineNumber << ".";
    return stream.str();
}

bool ParseVector3(std::istringstream& lineStream, Vec3& vector)
{
    return static_cast<bool>(lineStream >> vector.x >> vector.y >> vector.z);
}

/// <summary>
/// v9 step23: 增强OBJ面解析 — 支持 v, v//n, v/t, v/t/n 四种格式 + 负索引 + 四边形fan triangulation
/// </summary>
/// <param name="tokens">已split的面token列表 (不含'f'前缀).</param>
/// <param name="outFaces">输出的三角面列表 (四边形拆为2个).</param>
/// <param name="totalVertices">网格总顶点数 (用于解析负索引).</param>
/// <param name="totalNormals">网格总法向量数.</param>
/// <returns>解析成功返回true.</returns>
bool ParseFaceTokens(const std::vector<std::string>& tokens,
                     std::vector<Face>& outFaces,
                     int totalVertices, int totalNormals)
{
    if (tokens.size() < 3) return false;

    // 解析单个顶点标记 "v", "v//n", "v/t", "v/t/n"
    auto parseToken = [&](const std::string& token, int& vi, int& ni) -> bool
    {
        vi = -1; ni = -1;
        if (token.empty()) return false;

        // 计数斜杠
        size_t s1 = token.find('/');
        if (s1 == std::string::npos) {
            // 格式: "v" — 仅顶点
            vi = std::atoi(token.c_str());
        } else {
            vi = std::atoi(token.substr(0, s1).c_str());
            size_t s2 = token.find('/', s1 + 1);
            if (s2 == s1 + 1) {
                // 格式: "v//n" — 顶点+法线 (跳过纹理坐标)
                ni = std::atoi(token.substr(s2 + 1).c_str());
            } else if (s2 == std::string::npos) {
                // 格式: "v/t" — 顶点+纹理 (忽略纹理)
                // ni stays -1
            } else {
                // 格式: "v/t/n" — 顶点+纹理+法线
                ni = std::atoi(token.substr(s2 + 1).c_str());
            }
        }

        // 负索引转换: -1 = 最后一个, -2 = 倒数第二个...
        if (vi < 0) vi = totalVertices + vi;  // vi is negative, so this adds
        else vi = vi - 1;                      // 1-based → 0-based
        if (ni < 0 && ni != -1) ni = totalNormals + ni;
        else if (ni > 0) ni = ni - 1;

        // Bounds check: 防止vector subscript out of range
        if (vi < 0 || vi >= totalVertices) return false;
        if (ni >= 0 && ni >= totalNormals) ni = -1;  // invalid normal → ignore
        return true;
    };

    // 解析3个顶点索引 (三角面或四边形的前3个)
    auto buildFace = [&](int v0, int v1, int v2, int n0, int n1, int n2) -> Face
    {
        Face f;
        f.vertex_index0 = v0;
        f.vertex_index1 = v1;
        f.vertex_index2 = v2;
        f.normal_index = (n0 >= 0) ? n0 : -1;
        return f;
    };

    int v[4] = {-1, -1, -1, -1}, n[4] = {-1, -1, -1, -1};

    // 解析前3个顶点 (至少需要3个)
    for (int i = 0; i < 3; ++i) {
        if (!parseToken(tokens[i], v[i], n[i])) return false;
    }
    outFaces.push_back(buildFace(v[0], v[1], v[2], n[0], n[1], n[2]));

    // 四边形: 第4个顶点 → fan triangulation (v0, v2, v3)
    if (tokens.size() >= 4) {
        if (!parseToken(tokens[3], v[3], n[3])) return false;
        outFaces.push_back(buildFace(v[0], v[2], v[3], n[0], n[2], n[3]));
    }

    // 五边形及以上: fan triangulation (v0, vi-1, vi) for i=3..n-1
    for (size_t i = 4; i < tokens.size(); ++i) {
        int vi, ni;
        if (!parseToken(tokens[i], vi, ni)) return false;
        outFaces.push_back(buildFace(v[0], v[i-2], vi, n[0], n[i-2], ni));
    }

    return true;
}

} // namespace

OBJImportResult ImportSceneFromOBJ(const std::string& filePath,
                                   const std::string& coordinateTransform)
{
    OBJImportResult result;
    result.scene.meta.source_file_path = filePath;
    result.scene.meta.source_format = "obj";

    std::ifstream input(filePath.c_str(), std::ios::in | std::ios::binary);
    if (!input.is_open())
    {
        result.errors.push_back(RtError::Create(
            ErrorCode::FileNotFound,
            "Module2",
            "Unable to open OBJ scene file.",
            filePath,
            "Check scene_import.source_file.",
            true));
        return result;
    }

    int currentObjectId = -1;
    std::string currentObjectName;
    std::string line;
    int lineNumber = 0;
    bool sawUnsupportedFaceFormat = false;
    bool sawMalformedVertex = false;
    bool sawMalformedNormal = false;
    bool sawMalformedFace = false;
    bool sawOutOfRangeNormalIndex = false;

    while (std::getline(input, line))
    {
        ++lineNumber;
        if (line.empty() || line[0] == '#')
        {
            continue;
        }

        std::istringstream lineStream(line);
        std::string tag;
        lineStream >> tag;

        if (tag == "o")
        {
            lineStream >> currentObjectName;
            if (currentObjectName.empty())
            {
                currentObjectName = "unnamed_object_" + std::to_string(static_cast<int>(result.scene.objects.size()));
            }
            currentObjectId = static_cast<int>(result.scene.objects.size());

            SceneObjectRecord objectRecord;
            objectRecord.object_id = currentObjectId;
            objectRecord.object_name = currentObjectName;
            result.scene.objects.push_back(objectRecord);
        }
        else if (tag == "v")
        {
            Vec3 vertex;
            if (ParseVector3(lineStream, vertex))
            {
                result.scene.vertices.push_back(vertex);
            }
            else
            {
                sawMalformedVertex = true;
                result.errors.push_back(RtError::Create(
                    ErrorCode::JsonParseError,
                    "Module2",
                    "Malformed OBJ vertex record.",
                    filePath,
                    BuildLineLocationMessage(lineNumber),
                    false));
            }
        }
        else if (tag == "vn")
        {
            Vec3 normal;
            if (ParseVector3(lineStream, normal))
            {
                result.scene.normals.push_back(normal);
            }
            else
            {
                sawMalformedNormal = true;
                result.errors.push_back(RtError::Create(
                    ErrorCode::JsonParseError,
                    "Module2",
                    "Malformed OBJ normal record.",
                    filePath,
                    BuildLineLocationMessage(lineNumber),
                    false));
            }
        }
        else if (tag == "f")
        {
            if (currentObjectId < 0)
            {
                currentObjectName = "default_object_0";
                currentObjectId = static_cast<int>(result.scene.objects.size());

                SceneObjectRecord objectRecord;
                objectRecord.object_id = currentObjectId;
                objectRecord.object_name = currentObjectName;
                result.scene.objects.push_back(objectRecord);
            }

            // v9 step23: 支持 v/vt/vn, v//vn, v/vt 格式 + 四边形fan triangulation
            std::vector<std::string> faceTokens;
            std::string token;
            while (lineStream >> token) faceTokens.push_back(token);

            std::vector<Face> parsedFaces;
            if (!faceTokens.empty() && ParseFaceTokens(faceTokens, parsedFaces,
                static_cast<int>(result.scene.vertices.size()),
                static_cast<int>(result.scene.normals.size())))
            {
                int totalVerts = static_cast<int>(result.scene.vertices.size());
                for (auto& face : parsedFaces) {
                    // 顶点索引越界保护 — 拒绝非法面
                    if (face.vertex_index0 < 0 || face.vertex_index0 >= totalVerts ||
                        face.vertex_index1 < 0 || face.vertex_index1 >= totalVerts ||
                        face.vertex_index2 < 0 || face.vertex_index2 >= totalVerts) {
                        sawMalformedFace = true;
                        continue;
                    }
                    face.face_id = static_cast<int>(result.scene.faces.size());
                    face.object_id = currentObjectId;
                    face.object_name = currentObjectName;
                    Vec3 authoredNormal;
                    bool hasAuthoredNormal = false;
                    if (face.normal_index >= 0 && face.normal_index < static_cast<int>(result.scene.normals.size()))
                    {
                        authoredNormal = Normalize(result.scene.normals[face.normal_index]);
                        hasAuthoredNormal = Length(authoredNormal) > 0.5;
                    }
                    else if (face.normal_index >= 0)
                    {
                        sawOutOfRangeNormalIndex = true;
                    }
                    // Propagation geometry must use the triangle plane normal,
                    // never an interpolated/smoothed OBJ shading normal. The
                    // authored normal is used only to orient the geometric
                    // normal toward the scene-defined outward side.
                    const Vec3& v0 = result.scene.vertices[face.vertex_index0];
                    const Vec3& v1 = result.scene.vertices[face.vertex_index1];
                    const Vec3& v2 = result.scene.vertices[face.vertex_index2];
                    Vec3 geometricNormal = Normalize(Cross(Subtract(v1, v0), Subtract(v2, v0)));
                    if (hasAuthoredNormal && Dot(geometricNormal, authoredNormal) < 0.0)
                        geometricNormal = Scale(geometricNormal, -1.0);
                    face.normal = geometricNormal;

                    result.scene.faces.push_back(face);
                    if (currentObjectId >= 0 && currentObjectId < static_cast<int>(result.scene.objects.size()))
                    {
                        result.scene.objects[currentObjectId].face_ids.push_back(face.face_id);
                    }
                }
            }
            else
            {
                sawMalformedFace = true;
                sawUnsupportedFaceFormat = true;
                result.errors.push_back(RtError::Create(
                    ErrorCode::JsonParseError,
                    "Module2",
                    "Unsupported or malformed OBJ face record.",
                    filePath,
                    "Expects 3+ vertex indices. " + BuildLineLocationMessage(lineNumber),
                    false));
            }
        }
    }

    result.scene.meta.object_count = static_cast<int>(result.scene.objects.size());
    result.scene.meta.vertex_count = static_cast<int>(result.scene.vertices.size());
    result.scene.meta.normal_count = static_cast<int>(result.scene.normals.size());
    result.scene.meta.face_count = static_cast<int>(result.scene.faces.size());

    result.succeeded = !result.scene.faces.empty() &&
                       !sawMalformedVertex &&
                       !sawMalformedNormal &&
                       !sawMalformedFace &&
                       !sawOutOfRangeNormalIndex;
    if (!result.succeeded)
    {
        if (result.scene.faces.empty())
        {
            result.errors.push_back(RtError::Create(
                ErrorCode::JsonParseError,
                "Module2",
                "No valid triangle faces were parsed from the OBJ file.",
                filePath,
                sawUnsupportedFaceFormat
                    ? "The file may contain unsupported face syntax; current importer expects triangle faces in 'v//n' format."
                    : "Check whether the input file is a triangle-face OBJ text.",
                true));
        }
    }

    // v5 D6-A: OBJ坐标变换 (Blender Z-up → 算法 Y-up)
    if (coordinateTransform == "blender_z_up_to_y_up") {
        for (auto& v : result.scene.vertices) {
            double tmp = v.y;
            v.y = v.z;  // Blender Z → 算法 Y (up)
            v.z = tmp;  // Blender Y → 算法 Z
        }
        for (auto& face : result.scene.faces) {
            double tmp = face.normal.y;
            face.normal.y = face.normal.z;
            face.normal.z = tmp;
            // 面元centroid也需变换 (在BuildFaceBVHAcceleration中重新计算, 此处可跳过)
        }
        for (auto& edge : result.scene.edges) {
            double tmp = edge.midpoint.y;
            edge.midpoint.y = edge.midpoint.z;
            edge.midpoint.z = tmp;
            tmp = edge.start.y; edge.start.y = edge.start.z; edge.start.z = tmp;
            tmp = edge.end.y; edge.end.y = edge.end.z; edge.end.z = tmp;
        }
    }

    return result;
}

} // namespace rt
