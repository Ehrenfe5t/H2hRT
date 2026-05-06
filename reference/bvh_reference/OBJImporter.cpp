#define _CRT_SECURE_NO_WARNINGS

#include "Core.h"
#include <sstream>
#include <unordered_map>
#include <algorithm>

namespace ModelImporter {

    // 多边形三角化（与Core.h声明完全一致）
    void TriangulatePolygon(const std::vector<int>& indices, std::vector<std::tuple<int, int, int>>& triangles) {
        if (indices.size() < 3) return;

        // 扇面三角化：以第一个顶点为中心拆分
        for (size_t i = 1; i < indices.size() - 1; ++i) {
            triangles.emplace_back(indices[0], indices[i], indices[i + 1]);
        }
    }

    // OBJ文件加载（修复所有编译错误）
    bool LoadOBJ(const std::string& filePath, Scenario3D& scene, ProgressBar* progressBar) {
        scene.points.clear();
        scene.triangles.clear();

        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "错误：无法打开OBJ文件 " << filePath << std::endl;
            return false;
        }

        // 兼容C++17及以下：替换starts_with为substr判断（避免越界）
        int estimatedTotalSteps = 0;
        std::string line;
        while (std::getline(file, line)) {
            bool isVertex = (line.size() >= 2) && (line.substr(0, 2) == "v ");
            bool isFace = (line.size() >= 2) && (line.substr(0, 2) == "f ");

            if (isVertex) estimatedTotalSteps += 1;
            else if (isFace) estimatedTotalSteps += 3;
        }

        // 重置文件指针到开头
        file.clear();
        file.seekg(0, std::ios::beg);

        int processedSteps = 0;
        if (progressBar) progressBar->Update(0);

        std::vector<Point3D> normals;

        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string token;
            iss >> token;

            // 顶点（v x y z）：互换y/z轴，与模型坐标系一致
            if (token == "v") {
                double x, y, z;
                iss >> x >> y >> z;
                scene.points.emplace_back(x, z, y); // 核心修正：y↔z互换
                processedSteps += 1;
            }
            // 顶点法向量（vn nx ny nz）：同步互换y/z轴
            else if (token == "vn") {
                double nx, ny, nz;
                iss >> nx >> ny >> nz;
                normals.emplace_back(nx, nz, ny); // 核心修正：y↔z互换
            }
            // 面（f v1[/vt1[/vn1]] ...）：支持多种索引格式
            else if (token == "f") {
                std::vector<int> vertexIndices;
                std::string faceToken;

                while (iss >> faceToken) {
                    std::vector<std::string> parts;
                    std::stringstream ss(faceToken);
                    std::string part;

                    // 拆分索引（支持"v"、"v/vt"、"v/vt/vn"、"v//vn"）
                    while (std::getline(ss, part, '/')) {
                        parts.push_back(part);
                    }

                    if (parts.empty() || parts[0].empty()) continue;

                    // OBJ索引从1开始，转换为0-based
                    int vIdx = std::stoi(parts[0]) - 1;
                    vertexIndices.push_back(vIdx);
                }

                // 多边形三角化
                std::vector<std::tuple<int, int, int>> triIndices;
                TriangulatePolygon(vertexIndices, triIndices);

                // 逐个添加三角形到场景
                for (const auto& tri : triIndices) {
                    int p1 = std::get<0>(tri);
                    int p2 = std::get<1>(tri);
                    int p3 = std::get<2>(tri);
                    Point3D normal;

                    // 优先使用OBJ定义的法向量
                    if (!normals.empty() && p1 < static_cast<int>(normals.size())) {
                        normal = normals[p1];
                    }
                    // 无定义时，通过顶点叉乘计算
                    else {
                        const Point3D& p1p = scene.points[p1];
                        const Point3D& p2p = scene.points[p2];
                        const Point3D& p3p = scene.points[p3];

                        Point3D e1 = p2p - p1p;
                        Point3D e2 = p3p - p1p;
                        normal = GeometryUtils::Cross(e1, e2);

                        // 避免零法向量
                        if (normal.Length() > kEps) {
                            normal = normal.Normalize();
                        }
                        else {
                            normal = Point3D(0.0, 1.0, 0.0); //  fallback：默认向上
                        }
                    }

                    // 添加到场景（匹配Triangle3D构造函数）
                    scene.triangles.emplace_back(p1, p2, p3, normal);
                }

                processedSteps += 3;
            }

            // 进度条更新：每100步更新一次，减少IO开销
            if (progressBar && processedSteps % 100 == 0) {
                progressBar->Update(100);
            }
        }

        // 完成进度条（确保进度100%）
        if (progressBar) {
            int remainingSteps = estimatedTotalSteps - processedSteps;
            if (remainingSteps > 0) {
                progressBar->Update(remainingSteps);
            }
            progressBar->Finish();
        }

        file.close();

        // 输出加载信息（与整体日志格式一致）
        std::cout << "成功加载OBJ文件：" << filePath << std::endl;
        std::cout << "  - 顶点数：" << scene.points.size() << std::endl;
        std::cout << "  - 三角形数：" << scene.triangles.size() << std::endl;
        std::cout << "  - 法向量数：" << normals.size() << std::endl;

        return true;
    }

} // namespace ModelImporter