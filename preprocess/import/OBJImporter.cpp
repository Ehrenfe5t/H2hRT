// 文件目标：
// - 实现模块2 OBJ 导入器。
//
// 主要功能：
// - 读取 OBJ 文本；
// - 解析对象块、顶点、法向和三角面；
// - 为批次2生成 Scene 的原始几何区与对象记录。

#include "OBJImporter.h"

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

bool ParseFace(std::istringstream& lineStream, Face& face)
{
    std::string token0;
    std::string token1;
    std::string token2;
    if (!(lineStream >> token0 >> token1 >> token2))
    {
        return false;
    }

    auto parseVertexNormal = [](const std::string& token, int& vertexIndex, int& normalIndex) -> bool
    {
        const std::size_t slashPos = token.find("//");
        if (slashPos == std::string::npos)
        {
            return false;
        }

        vertexIndex = std::atoi(token.substr(0, slashPos).c_str()) - 1;
        normalIndex = std::atoi(token.substr(slashPos + 2U).c_str()) - 1;
        return true;
    };

    int normalIndex0 = -1;
    int normalIndex1 = -1;
    int normalIndex2 = -1;
    if (!parseVertexNormal(token0, face.vertex_index0, normalIndex0) ||
        !parseVertexNormal(token1, face.vertex_index1, normalIndex1) ||
        !parseVertexNormal(token2, face.vertex_index2, normalIndex2))
    {
        return false;
    }

    face.normal_index = normalIndex0;
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

            Face face;
            if (ParseFace(lineStream, face))
            {
                face.face_id = static_cast<int>(result.scene.faces.size());
                face.object_id = currentObjectId;
                face.object_name = currentObjectName;
                if (face.normal_index >= 0 && face.normal_index < static_cast<int>(result.scene.normals.size()))
                {
                    face.normal = result.scene.normals[face.normal_index];
                }
                else if (face.normal_index >= 0)
                {
                    sawOutOfRangeNormalIndex = true;
                    result.errors.push_back(RtError::Create(
                        ErrorCode::JsonParseError,
                        "Module2",
                        "OBJ face references an out-of-range normal index.",
                        filePath,
                        BuildLineLocationMessage(lineNumber),
                        false));
                    continue;
                }

                result.scene.faces.push_back(face);
                if (currentObjectId >= 0 && currentObjectId < static_cast<int>(result.scene.objects.size()))
                {
                    result.scene.objects[currentObjectId].face_ids.push_back(face.face_id);
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
                    "Current importer expects triangle faces in 'v//n' format. " + BuildLineLocationMessage(lineNumber),
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
