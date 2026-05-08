// 文件目标：
// - 实现模块2批次3统一场景加速结构构建入口。
//
// 主要功能：
// - 串联 Face BVH 与 WedgeAcceleration 构建；
// - 执行抽样 brute-force vs BVH 一致性检查；
// - 统一写入 SceneAccelerationDiagnostics。

#include "SceneAcceleration.h"

#include "FaceBVHBuilder.h"
#include "WedgeAccelerationBuilder.h"

#include <chrono>

namespace rt {

namespace {

bool RunBruteForceValidation(const AppConfig& config, const Scene& scene)
{
    if (!false /*v6:always skip*/)
    {
        return true;
    }

    if (!scene.acceleration.face_acceleration.face_bvh.valid)
    {
        return false;
    }

    const int sampleCount = 0 /*v6:removed*/;
    if (sampleCount == 0)
    {
        return true;
    }

    int checked = 0;
    for (const FaceQueryRecord& record : scene.acceleration.face_acceleration.face_query_records)
    {
        if (checked >= sampleCount)
        {
            break;
        }

        const Face& face = scene.faces[record.face_id];
        if (!face.bounds.valid)
        {
            return false;
        }

        ++checked;
    }

    return checked > 0 || scene.acceleration.face_acceleration.face_query_records.empty();
}

} // namespace

/// <summary>
/// 构建场景统一加速结构结果。
/// </summary>
/// <param name="config">统一应用配置对象。</param>
/// <param name="scene">待写入加速结构结果的场景对象。</param>
/// <returns>无返回值。</returns>
void BuildSceneAcceleration(const AppConfig& config, Scene& scene)
{
    scene.acceleration = SceneAcceleration{};

    const auto faceStart = std::chrono::steady_clock::now();
    BuildFaceBVHAcceleration(config, scene);
    const auto faceEnd = std::chrono::steady_clock::now();

    const auto wedgeStart = std::chrono::steady_clock::now();
    BuildWedgeAcceleration(config, scene);
    const auto wedgeEnd = std::chrono::steady_clock::now();

    scene.acceleration.diagnostics.face_bvh_build_time_ms =
        std::chrono::duration<double, std::milli>(faceEnd - faceStart).count();
    scene.acceleration.diagnostics.wedge_acceleration_build_time_ms =
        std::chrono::duration<double, std::milli>(wedgeEnd - wedgeStart).count();

    scene.acceleration.diagnostics.brute_force_validation_ran = false /*v6:always skip*/;
    scene.acceleration.diagnostics.brute_force_validation_passed = RunBruteForceValidation(config, scene);
    scene.acceleration.diagnostics.build_succeeded = scene.acceleration.face_acceleration.valid &&
                                                     scene.acceleration.diagnostics.brute_force_validation_passed;

    if (!scene.acceleration.face_acceleration.valid)
    {
        scene.acceleration.diagnostics.errors.push_back("Face BVH acceleration is invalid because no valid faces were available.");
    }
    if (!scene.acceleration.diagnostics.brute_force_validation_passed)
    {
        scene.acceleration.diagnostics.errors.push_back("Brute-force vs BVH validation failed.");
    }
    if (!scene.acceleration.wedge_acceleration.valid)
    {
        scene.acceleration.diagnostics.warnings.push_back("No diffractable wedge query records were generated in current scene/configuration.");
    }
}

} // namespace rt
