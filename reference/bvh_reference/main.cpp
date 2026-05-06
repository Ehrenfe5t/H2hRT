#define _CRT_SECURE_NO_WARNINGS

#include "Core.h"
#include <filesystem>
#include <omp.h>  // OpenMP并行支持
#include <iomanip>

namespace fs = std::filesystem;

int main() {
    // ===================== 目录管理 =====================
    const std::string cacheRootDir = "Cache";
    const std::string modelFilePath = "obj\\room.obj";
    const fs::path modelFile(modelFilePath);
    const std::string sceneName = modelFile.stem().string();
    const std::string sceneCacheDir = cacheRootDir + "\\" + sceneName;

    // 递归创建场景缓存目录
    if (!CreateDirectoryRecursive(sceneCacheDir)) {
        std::cerr << "错误：创建场景缓存目录失败，程序退出" << std::endl;
        return -1;
    }

    // ===================== 日志初始化 =====================
    const std::string logFilePath = sceneCacheDir + "\\log.txt";
    std::ofstream logFile(logFilePath);
    if (!logFile.is_open()) {
        std::cerr << "错误：无法打开日志文件 " << logFilePath << std::endl;
        return -1;
    }

    // 保存控制台原始缓冲区
    std::streambuf* originalCoutBuf = std::cout.rdbuf();
    std::streambuf* originalCerrBuf = std::cerr.rdbuf();
    // 重定向cout/cerr到日志文件
    std::cout.rdbuf(logFile.rdbuf());
    std::cerr.rdbuf(logFile.rdbuf());

    // ===================== 3. 配置参数与模型加载 =====================
    // 射线参数（与模型坐标系保持一致，互换y/z轴）
    Point3D rayOrigin(-1, 1, 1000);  // 适配OBJ加载时的y/z互换
    Point3D rayDir(-1.0, 3.0, 0.5);   // 方向向量同步互换

    std::cout << "rayOrigin" << std::endl;
    std::cout << "  x：" << rayOrigin.x << "  y：" << rayOrigin.y << "  z：" << rayOrigin.z << std::endl;
    std::cout << "rayDir" << std::endl;
    std::cout << "  未归一化" << std::endl;
    std::cout << "  x：" << rayDir.x << "  y：" << rayDir.y << "  z：" << rayDir.z << std::endl;
    rayDir = GeometryUtils::Normalize(rayDir);
    
    // ===================== 射线参数输出 =====================


    // 预处理射线方向逆向量
    Point3D rayDirInv(
        rayDir.x == 0 ? (rayOrigin.x > 0 ? kRayMaxDistance : -kRayMaxDistance) : 1.0 / rayDir.x,
        rayDir.y == 0 ? (rayOrigin.y > 0 ? kRayMaxDistance : -kRayMaxDistance) : 1.0 / rayDir.y,
        rayDir.z == 0 ? (rayOrigin.z > 0 ? kRayMaxDistance : -kRayMaxDistance) : 1.0 / rayDir.z
    );

    // 加载模型
    Scenario3D scene;
    bool loadSuccess = false;
    const std::string modelExt = modelFile.extension().string();

    std::cout.rdbuf(originalCoutBuf);  // 切换到控制台显示进度条
    std::cout << "============================================" << std::endl;
    std::cout << "            模型加载中...                    " << std::endl;
    std::cout.rdbuf(logFile.rdbuf());  // 切回日志文件

    if (modelExt == ".obj" || modelExt == ".OBJ") {
        // 统计OBJ文件行数，作为进度条总步数（修复：CountFileLines函数声明与调用一致）
        int objTotalLines = CountFileLines(modelFilePath);
        ProgressBar loadProgress(objTotalLines, "加载场景");

        std::cout.rdbuf(originalCoutBuf);  // 控制台显示进度条
        loadSuccess = ModelImporter::LoadOBJ(modelFilePath, scene, &loadProgress);
        std::cout.rdbuf(logFile.rdbuf());  // 切回日志文件
    }
    else {
        std::cerr << "错误：不支持的模型格式 " << modelExt << "，仅支持OBJ格式" << std::endl;
    }

    if (!loadSuccess) {
        std::cerr << "模型加载失败，程序退出" << std::endl;
        // 恢复控制台并清理资源
        std::cout.rdbuf(originalCoutBuf);
        std::cerr.rdbuf(originalCerrBuf);
        logFile.close();
        std::cout << "错误：模型加载失败！" << std::endl;
        return -1;
    }

    // 输出模型加载信息（控制台+日志）
    std::cout.rdbuf(originalCoutBuf);
    std::cout << "============================================" << std::endl;
    std::cout << "            模型加载成功                    " << std::endl;
    std::cout << "  模型文件：" << modelFilePath << std::endl;
    std::cout << "  - 顶点数量：" << scene.points.size() << std::endl;
    std::cout << "  - 三角形数量：" << scene.triangles.size() << std::endl;
    std::cout.rdbuf(logFile.rdbuf());
    std::cout << "============================================" << std::endl;
    std::cout << "            模型加载成功                    " << std::endl;
    std::cout << "  模型文件：" << modelFilePath << std::endl;
    std::cout << "  - 顶点数量：" << scene.points.size() << std::endl;
    std::cout << "  - 三角形数量：" << scene.triangles.size() << std::endl;

    // ===================== 4. BVH构建/加载（带进度条） =====================
    const std::string objFileMTime = GetFileLastModifyTime(modelFilePath);  // 修复：函数声明与调用一致
    const std::string aabbCachePath = sceneCacheDir + "\\aabb_bvh_cache.pb";
    const std::string sphereCachePath = sceneCacheDir + "\\sphere_bvh_cache.pb";

    std::unique_ptr<BVHNode<AABB>> aabbBVH;
    std::unique_ptr<BVHNode<BoundingSphere>> sphereBVH;
    double aabbInitTime = 0.0, sphereInitTime = 0.0;
    bool isAABBCacheValid = false, isSphereCacheValid = false;

    // 设置OpenMP线程数（避免超核心并行开销，修复：kMaxThreadCount已在Core.h声明）
    omp_set_num_threads(std::min(kMaxThreadCount, static_cast<int>(std::thread::hardware_concurrency())));

    // 切换到控制台显示BVH进度
    std::cout.rdbuf(originalCoutBuf);
    std::cout << "\n============================================" << std::endl;
    std::cout << "            BVH树构建/加载中...                  " << std::endl;
    std::cout.rdbuf(logFile.rdbuf());

    // 加载AABB-BVH（带进度条，修复：LoadBVHCached模板函数调用正确）
    std::cout.rdbuf(originalCoutBuf);
    aabbBVH = BVHAccelerator::LoadBVHCached<AABB>(
        aabbCachePath, modelFilePath, objFileMTime,
        static_cast<int>(scene.points.size()), static_cast<int>(scene.triangles.size()),
        isAABBCacheValid, aabbInitTime
    );
    if (!isAABBCacheValid) {
        // 缓存无效，重新构建（带进度条）
        ProgressBar aabbBuildProgress(static_cast<int>(scene.triangles.size()), "构建AABB-BVH");
        const TimePoint aabbInitStart = GetCurrentTimeMy();  // 修复：TimePoint已在Core.h定义
        aabbBVH = BVHAccelerator::BuildAABBBVH(scene, [&](int steps) {
            aabbBuildProgress.Update(steps);
            });
        const TimePoint aabbInitEnd = GetCurrentTimeMy();
        aabbInitTime = CalculateElapsedMs(aabbInitStart, aabbInitEnd);  // 修复：函数声明与调用一致
        aabbBuildProgress.Finish();

        // 保存新缓存（修复：SaveBVHCached参数正确）
        if (!objFileMTime.empty()) {
            BVHAccelerator::SaveBVHCached<AABB>(
                aabbCachePath, aabbBVH, modelFilePath, objFileMTime,
                static_cast<int>(scene.points.size()), static_cast<int>(scene.triangles.size()),
                "AABB-BVH(SAH-Optimized(Bucket))", aabbInitTime
            );
        }
    }
    else {
        // 缓存有效，直接显示加载完成
        std::cout << "构建AABB-BVH [##################################] 100% (加载缓存)" << std::endl;
    }
    std::cout.rdbuf(logFile.rdbuf());

    // 加载Sphere-BVH（带进度条，修复：LoadBVHCached模板函数调用正确）
    std::cout.rdbuf(originalCoutBuf);
    sphereBVH = BVHAccelerator::LoadBVHCached<BoundingSphere>(
        sphereCachePath, modelFilePath, objFileMTime,
        static_cast<int>(scene.points.size()), static_cast<int>(scene.triangles.size()),
        isSphereCacheValid, sphereInitTime
    );
    if (!isSphereCacheValid) {
        // 缓存无效，重新构建（带进度条）
        ProgressBar sphereBuildProgress(static_cast<int>(scene.triangles.size()), "构建Sphere-BVH");
        const TimePoint sphereInitStart = GetCurrentTimeMy();
        sphereBVH = BVHAccelerator::BuildSphereBVH(scene, [&](int steps) {
            sphereBuildProgress.Update(steps);
            });
        const TimePoint sphereInitEnd = GetCurrentTimeMy();
        sphereInitTime = CalculateElapsedMs(sphereInitStart, sphereInitEnd);
        sphereBuildProgress.Finish();

        // 保存新缓存
        if (!objFileMTime.empty()) {
            BVHAccelerator::SaveBVHCached<BoundingSphere>(
                sphereCachePath, sphereBVH, modelFilePath, objFileMTime,
                static_cast<int>(scene.points.size()), static_cast<int>(scene.triangles.size()),
                "Sphere-BVH(SAH-Optimized(Bucket))", sphereInitTime
            );
        }
    }
    else {
        // 缓存有效，直接显示加载完成
        std::cout << "构建Sphere-BVH [##################################] 100% (加载缓存)" << std::endl;
    }
    std::cout.rdbuf(logFile.rdbuf());

    // 输出BVH构建/加载结果
    std::cout.rdbuf(originalCoutBuf);
    std::cout << "\n============================================" << std::endl;
    std::cout << "  AABB-BVH" << (isAABBCacheValid ? "加载" : "构建") << "完成！耗时：" << std::fixed << std::setprecision(3) << aabbInitTime << "ms" << std::endl;
    std::cout << "  Sphere-BVH" << (isSphereCacheValid ? "加载" : "构建") << "完成！耗时：" << std::fixed << std::setprecision(3) << sphereInitTime << "ms" << std::endl;
    std::cout.rdbuf(logFile.rdbuf());

    // ===================== 5. 射线碰撞检测（带进度条） =====================
    std::cout.rdbuf(originalCoutBuf);
    std::cout << "\n============================================" << std::endl;
    std::cout << "          射线碰撞检测中...                 " << std::endl;
    std::cout.rdbuf(logFile.rdbuf());

    // AABB-BVH检测（带进度条，修复：RayIntersectAABB参数正确）
    std::cout.rdbuf(originalCoutBuf);
    ProgressBar aabbDetectProgress(static_cast<int>(scene.triangles.size()), "AABB-BVH检测");
    const TimePoint aabbDetectStart = GetCurrentTimeMy();
    const RayIntersectResult aabbResult = BVHAccelerator::RayIntersectAABB(
        scene, aabbBVH.get(), rayOrigin, rayDir, [&](int steps) {
            aabbDetectProgress.Update(steps);
        }
    );
    const TimePoint aabbDetectEnd = GetCurrentTimeMy();
    const double aabbDetectTime = CalculateElapsedMs(aabbDetectStart, aabbDetectEnd);
    aabbDetectProgress.Finish();

    // Sphere-BVH检测（带进度条，修复：RayIntersectSphere参数正确）
    ProgressBar sphereDetectProgress(static_cast<int>(scene.triangles.size()), "Sphere-BVH检测");
    const TimePoint sphereDetectStart = GetCurrentTimeMy();
    const RayIntersectResult sphereResult = BVHAccelerator::RayIntersectSphere(
        scene, sphereBVH.get(), rayOrigin, rayDir, [&](int steps) {
            sphereDetectProgress.Update(steps);
        }
    );
    const TimePoint sphereDetectEnd = GetCurrentTimeMy();
    const double sphereDetectTime = CalculateElapsedMs(sphereDetectStart, sphereDetectEnd);
    sphereDetectProgress.Finish();
    std::cout.rdbuf(logFile.rdbuf());

    // 暴力精确检测（新增：开关控制，默认开启，修复：RayIntersectBruteForce调用正确）
    const bool useBruteForce = true;
    RayIntersectResult bruteResult;
    double bruteDetectTime = 0.0;

    if (useBruteForce) {
        std::cout.rdbuf(originalCoutBuf);
        std::cout << "\n============================================" << std::endl;
        std::cout << "        暴力精确碰撞检测中...                 " << std::endl;
        std::cout.rdbuf(logFile.rdbuf());

        std::cout.rdbuf(originalCoutBuf);
        ProgressBar bruteDetectProgress(static_cast<int>(scene.triangles.size()), "暴力检测");
        const TimePoint bruteDetectStart = GetCurrentTimeMy();
        bruteResult = BVHAccelerator::RayIntersectBruteForce(
            scene, rayOrigin, rayDir, [&](int steps) {
                bruteDetectProgress.Update(steps);
            }
        );
        const TimePoint bruteDetectEnd = GetCurrentTimeMy();
        bruteDetectTime = CalculateElapsedMs(bruteDetectStart, bruteDetectEnd);
        bruteDetectProgress.Finish();
        std::cout.rdbuf(logFile.rdbuf());

        // 输出暴力检测结果
        std::cout.rdbuf(originalCoutBuf);
        std::cout << "\n============================================" << std::endl;
        std::cout << "  暴力精确检测完成！耗时：" << std::fixed << std::setprecision(3) << bruteDetectTime << "ms" << std::endl;
        std::cout.rdbuf(logFile.rdbuf());
    }

    // 输出BVH检测结果
    std::cout.rdbuf(originalCoutBuf);
    std::cout << "\n============================================" << std::endl;
    std::cout << "  AABB-BVH检测完成！耗时：" << std::fixed << std::setprecision(3) << aabbDetectTime << "ms" << std::endl;
    std::cout << "  Sphere-BVH检测完成！耗时：" << std::fixed << std::setprecision(3) << sphereDetectTime << "ms" << std::endl;
    std::cout.rdbuf(logFile.rdbuf());

    // ===================== 6. 差异日志与Protobuf导出 =====================
    // 统计BVH节点数
    const int aabbNodeCount = BVHAccelerator::CountBVHNodes(aabbBVH);
    const int sphereNodeCount = BVHAccelerator::CountBVHNodes(sphereBVH);
    const int nodeCountDiff = aabbNodeCount - sphereNodeCount;
    const double initTimeDiff = aabbInitTime - sphereInitTime;
    const double detectTimeDiff = aabbDetectTime - sphereDetectTime;

    // 输出差异对比日志
    std::cout << "\n============================================" << std::endl;
    std::cout << "            AABB-BVH vs Sphere-BVH 差异对比             " << std::endl;
    std::cout << "============================================" << std::endl;
    std::cout << "【构建耗时差异】" << std::endl;
    std::cout << "  AABB-BVH " << (isAABBCacheValid ? "(复用)" : "") << "耗时：" << std::fixed << std::setprecision(3) << aabbInitTime << "ms" << std::endl;
    std::cout << "  Sphere-BVH " << (isSphereCacheValid ? "(复用)" : "") << "耗时：" << std::fixed << std::setprecision(3) << sphereInitTime << "ms" << std::endl;
    std::cout << "  差异（AABB - Sphere）：" << std::fixed << std::setprecision(3) << initTimeDiff << "ms" << std::endl;
    std::cout << "  结论：" << (initTimeDiff > 0 ? "Sphere-BVH构建更快" : "AABB-BVH构建更快") << std::endl;

    std::cout << "\n【检测耗时差异】" << std::endl;
    std::cout << "  AABB-BVH 耗时：" << std::fixed << std::setprecision(3) << aabbDetectTime << "ms" << std::endl;
    std::cout << "  Sphere-BVH 耗时：" << std::fixed << std::setprecision(3) << sphereDetectTime << "ms" << std::endl;
    std::cout << "  差异（AABB - Sphere）：" << std::fixed << std::setprecision(3) << detectTimeDiff << "ms" << std::endl;
    std::cout << "  结论：" << (detectTimeDiff > 0 ? "Sphere-BVH检测更快" : "AABB-BVH检测更快") << std::endl;

    // 碰撞结果详情（含BVH与暴力检测对比）
    std::cout << "\n【碰撞结果详情】" << std::endl;
    // AABB-BVH 结果
    std::cout << "  AABB-BVH 碰撞检测结果：" << std::endl;
    if (aabbResult.hasAnyHit) {
        const auto& closestAABBHit = aabbResult.GetClosestHit();
        std::cout << "    - 碰撞状态：检测到碰撞" << std::endl;
        std::cout << "    - 总碰撞点数量：" << aabbResult.allHits.size() << " 个" << std::endl;
        std::cout << "    - 最近碰撞点距离：" << std::fixed << std::setprecision(6) << closestAABBHit.distance << " m" << std::endl;
        std::cout << "    - 最近碰撞三角形索引：" << closestAABBHit.triangleIndex << std::endl;
        std::cout << "    - 最近碰撞点坐标：(" << closestAABBHit.hitPoint.x << ", " << closestAABBHit.hitPoint.y << ", " << closestAABBHit.hitPoint.z << ")" << std::endl;
    }
    else {
        std::cout << "    - 碰撞状态：未检测到任何碰撞" << std::endl;
    }
    // Sphere-BVH 结果
    std::cout << "  Sphere-BVH 碰撞检测结果：" << std::endl;
    if (sphereResult.hasAnyHit) {
        const auto& closestSphereHit = sphereResult.GetClosestHit();
        std::cout << "    - 碰撞状态：检测到碰撞" << std::endl;
        std::cout << "    - 总碰撞点数量：" << sphereResult.allHits.size() << " 个" << std::endl;
        std::cout << "    - 最近碰撞点距离：" << std::fixed << std::setprecision(6) << closestSphereHit.distance << " m" << std::endl;
        std::cout << "    - 最近碰撞三角形索引：" << closestSphereHit.triangleIndex << std::endl;
        std::cout << "    - 最近碰撞点坐标：(" << closestSphereHit.hitPoint.x << ", " << closestSphereHit.hitPoint.y << ", " << closestSphereHit.hitPoint.z << ")" << std::endl;
    }
    else {
        std::cout << "    - 碰撞状态：未检测到任何碰撞" << std::endl;
    }
    // 暴力检测结果（开关开启时显示）
    if (useBruteForce) {
        std::cout << "  暴力精确检测结果：" << std::endl;
        if (bruteResult.hasAnyHit) {
            const auto& closestBruteHit = bruteResult.GetClosestHit();
            std::cout << "    - 碰撞状态：检测到碰撞" << std::endl;
            std::cout << "    - 总碰撞点数量：" << bruteResult.allHits.size() << " 个" << std::endl;
            std::cout << "    - 最近碰撞点距离：" << std::fixed << std::setprecision(6) << closestBruteHit.distance << " m" << std::endl;
            std::cout << "    - 最近碰撞三角形索引：" << closestBruteHit.triangleIndex << std::endl;
            std::cout << "    - 最近碰撞点坐标：(" << closestBruteHit.hitPoint.x << ", " << closestBruteHit.hitPoint.y << ", " << closestBruteHit.hitPoint.z << ")" << std::endl;
        }
        else {
            std::cout << "    - 碰撞状态：未检测到任何碰撞" << std::endl;
        }
    }

    // BVH vs 暴力检测 一致性校验（开关开启时）
    if (useBruteForce) {
        std::cout << "\n============================================" << std::endl;
        std::cout << "            BVH  vs  暴力检测 结果一致性校验             " << std::endl;
        std::cout << "============================================" << std::endl;
        std::cout << "暴力检测耗时：" << std::fixed << std::setprecision(3) << bruteDetectTime * 100 << "ms" << std::endl;
        // AABB-BVH 与暴力检测对比
        std::cout << "【AABB-BVH vs 暴力检测】" << std::endl;
        bool aabbBruteConsistent = true;
        if (aabbResult.hasAnyHit != bruteResult.hasAnyHit) {
            aabbBruteConsistent = false;
            std::cout << "   碰撞状态不一致：AABB-BVH" << (aabbResult.hasAnyHit ? "检测到" : "未检测到")
                << "碰撞，暴力检测" << (bruteResult.hasAnyHit ? "检测到" : "未检测到") << "碰撞" << std::endl;
        }
        else if (aabbResult.hasAnyHit) {
            const auto& aabbClosest = aabbResult.GetClosestHit();
            const auto& bruteClosest = bruteResult.GetClosestHit();
            double distDiff = fabs(aabbClosest.distance - bruteClosest.distance);
            bool distConsistent = (distDiff < 1e-3);  // 误差≤1mm视为一致
            bool triConsistent = (aabbClosest.triangleIndex == bruteClosest.triangleIndex);
            std::cout << "  最近距离误差：" << std::fixed << std::setprecision(6) << distDiff << "m" << std::endl;
            std::cout << "  距离一致性：" << (distConsistent ? " 一致" : " 不一致") << std::endl;
            std::cout << "  三角形索引一致性：" << (triConsistent ? " 一致" : " 不一致") << std::endl;
            if (!triConsistent) {
                std::cout << "    - AABB-BVH：三角形索引=" << aabbClosest.triangleIndex << std::endl;
                std::cout << "    - 暴力检测：三角形索引=" << bruteClosest.triangleIndex << std::endl;
            }
            aabbBruteConsistent = distConsistent && triConsistent;
        }
        else {
            std::cout << "   均未检测到碰撞，一致性通过" << std::endl;
        }

        // Sphere-BVH 与暴力检测对比
        std::cout << "\n【Sphere-BVH vs 暴力检测】" << std::endl;
        bool sphereBruteConsistent = true;
        if (sphereResult.hasAnyHit != bruteResult.hasAnyHit) {
            sphereBruteConsistent = false;
            std::cout << "   碰撞状态不一致：Sphere-BVH" << (sphereResult.hasAnyHit ? "检测到" : "未检测到")
                << "碰撞，暴力检测" << (bruteResult.hasAnyHit ? "检测到" : "未检测到") << "碰撞" << std::endl;
        }
        else if (sphereResult.hasAnyHit) {
            const auto& sphereClosest = sphereResult.GetClosestHit();
            const auto& bruteClosest = bruteResult.GetClosestHit();
            double distDiff = fabs(sphereClosest.distance - bruteClosest.distance);
            bool distConsistent = (distDiff < 1e-3);
            bool triConsistent = (sphereClosest.triangleIndex == bruteClosest.triangleIndex);
            std::cout << "  最近距离误差：" << std::fixed << std::setprecision(6) << distDiff << "m" << std::endl;
            std::cout << "  距离一致性：" << (distConsistent ? " 一致" : " 不一致") << std::endl;
            std::cout << "  三角形索引一致性：" << (triConsistent ? " 一致" : " 不一致") << std::endl;
            if (!triConsistent) {
                std::cout << "    - Sphere-BVH：三角形索引=" << sphereClosest.triangleIndex << std::endl;
                std::cout << "    - 暴力检测：三角形索引=" << bruteClosest.triangleIndex << std::endl;
            }
            sphereBruteConsistent = distConsistent && triConsistent;
        }
        else {
            std::cout << "   均未检测到碰撞，一致性通过" << std::endl;
        }

        std::cout << "\n【整体结论】" << std::endl;
        if (aabbBruteConsistent && sphereBruteConsistent) {
            std::cout << "   两种BVH碰撞检测结果均与暴力精确检测一致，精度验证通过" << std::endl;
        }
        else if (aabbBruteConsistent) {
            std::cout << "   仅AABB-BVH与暴力检测一致，Sphere-BVH存在精度问题" << std::endl;
        }
        else if (sphereBruteConsistent) {
            std::cout << "   仅Sphere-BVH与暴力检测一致，AABB-BVH存在精度问题" << std::endl;
        }
        else {
            std::cout << "   两种BVH均与暴力检测不一致，需检查BVH构建或碰撞检测逻辑" << std::endl;
        }
        std::cout << "============================================" << std::endl;
    }

    std::cout << "\n【节点数差异】" << std::endl;
    std::cout << "  AABB-BVH 节点总数：" << aabbNodeCount << std::endl;
    std::cout << "  Sphere-BVH 节点总数：" << sphereNodeCount << std::endl;
    std::cout << "  差异（AABB - Sphere）：" << nodeCountDiff << "个" << std::endl;
    std::cout << "============================================" << std::endl;

    // Protobuf导出（修复：ExportBVHToProtobuf参数正确，与BVHData.proto匹配）
    std::cout.rdbuf(originalCoutBuf);
    std::cout << "\n============================================" << std::endl;
    std::cout << "          结果导出中...                     " << std::endl;
    Visualizer::ExportRayAndHitToProtobuf(sceneCacheDir + "\\ray_hit_data.pb", rayOrigin, rayDir, aabbResult);
    BVHAccelerator::ExportBVHToProtobuf(sceneCacheDir + "\\aabb_bvh_structure.pb", aabbBVH.get(), "AABB-BVH(SAH-Optimized(Bucket))");
    BVHAccelerator::ExportBVHToProtobuf(sceneCacheDir + "\\sphere_bvh_structure.pb", sphereBVH.get(), "Sphere-BVH(SAH-Optimized(Bucket))");
    // 导出暴力检测结果（开关开启时）
    if (useBruteForce) {
        Visualizer::ExportRayAndHitToProtobuf(sceneCacheDir + "\\brute_ray_hit_data.pb", rayOrigin, rayDir, bruteResult);
    }
    std::cout.rdbuf(logFile.rdbuf());

    // ===================== 7. 最终结果输出与资源清理 =====================
    std::cout.rdbuf(originalCoutBuf);
    std::cout << "============================================" << std::endl;
    std::cout << "            程序执行完成！                  " << std::endl;
    std::cout << "============================================" << std::endl;
    std::cout << "  输出文件清单（场景缓存目录：" << sceneCacheDir << "）：" << std::endl;
    std::cout << "  1. 详细日志：log.txt" << std::endl;
    std::cout << "  2. AABB-BVH射线碰撞数据：ray_hit_data.pb" << std::endl;
    std::cout << "  3. AABB-BVH结构：aabb_bvh_structure.pb" << std::endl;
    std::cout << "  4. Sphere-BVH结构：sphere_bvh_structure.pb" << std::endl;
    std::cout << "  5. AABB-BVH缓存：aabb_bvh_cache.pb" << (isAABBCacheValid ? "（已复用）" : "（新生成）") << std::endl;
    std::cout << "  6. Sphere-BVH缓存：sphere_bvh_cache.pb" << (isSphereCacheValid ? "（已复用）" : "（新生成）") << std::endl;
    if (useBruteForce) {
        std::cout << "  7. 暴力检测射线碰撞数据：brute_ray_hit_data.pb" << std::endl;
    }

    // 恢复控制台并关闭日志文件
    std::cout.rdbuf(originalCoutBuf);
    std::cerr.rdbuf(originalCerrBuf);
    logFile.close();

    return 0;
}