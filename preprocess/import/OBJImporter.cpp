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

OBJImportResult ImportSceneFromOBJ(const std::string& filePath)
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
    while (std::getline(input, line))
    {
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
        }
        else if (tag == "vn")
        {
            Vec3 normal;
            if (ParseVector3(lineStream, normal))
            {
                result.scene.normals.push_back(normal);
            }
        }
        else if (tag == "f")
        {
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

                result.scene.faces.push_back(face);
                if (currentObjectId >= 0 && currentObjectId < static_cast<int>(result.scene.objects.size()))
                {
                    result.scene.objects[currentObjectId].face_ids.push_back(face.face_id);
                }
            }
        }
    }

    result.scene.meta.object_count = static_cast<int>(result.scene.objects.size());
    result.scene.meta.vertex_count = static_cast<int>(result.scene.vertices.size());
    result.scene.meta.normal_count = static_cast<int>(result.scene.normals.size());
    result.scene.meta.face_count = static_cast<int>(result.scene.faces.size());

    result.succeeded = !result.scene.faces.empty();
    if (!result.succeeded)
    {
        result.errors.push_back(RtError::Create(
            ErrorCode::JsonParseError,
            "Module2",
            "No valid triangle faces were parsed from the OBJ file.",
            filePath,
            "Check whether the input file is a triangle-face OBJ text.",
            true));
    }

    return result;
}

} // namespace rt
