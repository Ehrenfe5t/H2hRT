// v9 步骤1 自测验证: SnellRefractV2
// 使用方法: 在app/main.cpp中临时include此文件并调用 RunStep1SelfTest()
// 验证完成后删除include即可
//
// 推荐: 在 RtPipeline::Run 开始处插入:
//   #include "../test/step1_snell_selftest.h"
//   RunStep1SnellSelfTest();
// 运行一次验证输出后移除

#pragma once

#include "../core/common/math/Vec3.h"
#include <cstdio>
#include <cmath>

namespace rt {

inline void RunStep1SnellSelfTest() {
    std::printf("\n========== v9 Step1: SnellRefractV2 Self-Test ==========\n");

    int passed = 0, failed = 0;

    // ── Test 1: 空气→玻璃 30°入射 ──
    // 法线朝-Z, 入射方向从+Z侧30°斜入射: incident = (sin30, 0, cos30)
    {
        Vec3 incident = MakeVec3(0.5, 0.0, 0.8660254037844386);  // 30° from +Z
        Vec3 normal   = MakeVec3(0.0, 0.0, -1.0);                 // 法线指向-Z (朝向入射侧)
        double n1 = 1.0, n2 = 1.5;                                 // 空气→玻璃

        SnellResult sr = SnellRefractV2(incident, normal, n1, n2);

        bool ok = sr.valid && !sr.total_internal_reflection;
        ok = ok && (sr.residual < 1e-12);
        ok = ok && (std::fabs(sr.theta_i_rad - kPi / 6.0) < 1e-6); // θ_i ≈ 30° = π/6
        // 30°入射: sin(theta_t) = n1/n2 * sin(30°) = 1/1.5 * 0.5 = 0.3333
        // theta_t ≈ 19.47°
        double expectedThetaT_rad = std::asin(1.0 / 1.5 * 0.5);
        ok = ok && (std::fabs(sr.theta_t_rad - expectedThetaT_rad) < 1e-6);

        if (ok) {
            std::printf("  [PASS] Test1: Air→Glass 30°  theta_i=%.2f° theta_t=%.2f° residual=%.2e\n",
                sr.theta_i_rad * 180.0 / 3.141592653589793,
                sr.theta_t_rad * 180.0 / 3.141592653589793,
                sr.residual);
            passed++;
        } else {
            std::printf("  [FAIL] Test1: Air→Glass 30°  valid=%d tir=%d residual=%.2e theta_i=%.4f theta_t=%.4f (expected %.4f)\n",
                (int)sr.valid, (int)sr.total_internal_reflection, sr.residual,
                sr.theta_i_rad, sr.theta_t_rad, expectedThetaT_rad);
            failed++;
        }
    }

    // ── Test 2: 玻璃→空气 45° (超过临界角→TIR) ──
    // 临界角 = asin(1.0/1.5) ≈ 41.81°，45° > 临界角 → TIR
    {
        Vec3 incident = MakeVec3(0.0, 0.0, 1.0);
        // 法线朝-Z: 表示从玻璃内部打到玻璃-空气界面
        Vec3 normal = MakeVec3(0.0, 0.0, -1.0);
        double n1 = 1.5, n2 = 1.0;

        // 入射角45°需要让入射方向与-Z偏离45°
        // cos(45°) = 0.7071, 需要Dot(incident, normal) = -0.7071
        // normal = (0,0,-1), incident = (sin45, 0, -cos45) = (0.7071, 0, -0.7071)
        Vec3 inc45 = Normalize(MakeVec3(0.7071067811865476, 0.0, -0.7071067811865476));

        SnellResult sr = SnellRefractV2(inc45, normal, n1, n2);

        bool ok = sr.valid && sr.total_internal_reflection;
        ok = ok && (sr.theta_i_rad > 0.78 && sr.theta_i_rad < 0.79); // ≈45° = 0.7854 rad

        if (ok) {
            std::printf("  [PASS] Test2: Glass→Air 45°→TIR  theta_i=%.2f° tir=%d\n",
                sr.theta_i_rad * 180.0 / 3.141592653589793,
                (int)sr.total_internal_reflection);
            passed++;
        } else {
            std::printf("  [FAIL] Test2: Glass→Air 45°→TIR  valid=%d tir=%d theta_i=%.4f\n",
                (int)sr.valid, (int)sr.total_internal_reflection, sr.theta_i_rad);
            failed++;
        }
    }

    // ── Test 3: 法线翻转不变性 ──
    // 同Test1场景，但法线反方向（背面法线）→ 结果应完全一致
    {
        Vec3 incident = MakeVec3(0.5, 0.0, 0.8660254037844386); // 同Test1: 30° from +Z
        Vec3 normalFlipped = MakeVec3(0.0, 0.0, 1.0);            // 法线指向+Z (背面!)
        double n1 = 1.0, n2 = 1.5;

        SnellResult sr = SnellRefractV2(incident, normalFlipped, n1, n2);

        bool ok = sr.valid && !sr.total_internal_reflection;
        ok = ok && (sr.residual < 1e-12);
        ok = ok && (std::fabs(sr.theta_i_rad - kPi / 6.0) < 1e-6); // 30°
        double expectedThetaT = std::asin(1.0 / 1.5 * 0.5);
        ok = ok && (std::fabs(sr.theta_t_rad - expectedThetaT) < 1e-6);
        // 折射方向应与Test1一致 (出射方向应指向+Z侧，即进入玻璃)
        ok = ok && (sr.direction.z > 0.0); // 应继续向+Z传播

        if (ok) {
            std::printf("  [PASS] Test3: NormalFlipInvariant  theta_t=%.2f° dir.z=%.4f (should be >0)\n",
                sr.theta_t_rad * 180.0 / 3.141592653589793,
                sr.direction.z);
            passed++;
        } else {
            std::printf("  [FAIL] Test3: NormalFlipInvariant  valid=%d tir=%d residual=%.2e theta_t=%.4f (expected %.4f)\n",
                (int)sr.valid, (int)sr.total_internal_reflection, sr.residual,
                sr.theta_t_rad, expectedThetaT);
            failed++;
        }
    }

    // ── Test 4: cosI边界clamp (法线平行于入射方向 → cosI≈0) ──
    {
        Vec3 incident = MakeVec3(1.0, 0.0, 0.0);    // 掠入射
        Vec3 normal = MakeVec3(0.0, 0.0, -1.0);      // 法线垂直于入射方向
        double n1 = 1.0, n2 = 1.5;

        SnellResult sr = SnellRefractV2(incident, normal, n1, n2);

        // cosI ≈ 0 (掠入射), 应正常工作
        bool ok = sr.valid && !sr.total_internal_reflection;
        ok = ok && (sr.cos_i < 1e-10); // ≈0

        if (ok) {
            std::printf("  [PASS] Test4: GrazingIncidence  cos_i=%.2e (should be ≈0)\n", sr.cos_i);
            passed++;
        } else {
            std::printf("  [FAIL] Test4: GrazingIncidence  valid=%d cos_i=%.6f\n",
                (int)sr.valid, sr.cos_i);
            failed++;
        }
    }

    // ── Test 5: n1=n2 无折射 (直通) ──
    {
        Vec3 incident = Normalize(MakeVec3(0.5, 0.5, 0.7071));
        Vec3 normal = MakeVec3(0.0, 0.0, -1.0);
        double n1 = 1.3, n2 = 1.3;

        SnellResult sr = SnellRefractV2(incident, normal, n1, n2);

        // 方向应不变, theta_i == theta_t
        bool ok = sr.valid && !sr.total_internal_reflection;
        ok = ok && (std::fabs(sr.theta_i_rad - sr.theta_t_rad) < 1e-9);
        ok = ok && (std::fabs(sr.direction.x - incident.x) < 1e-9);
        ok = ok && (std::fabs(sr.direction.y - incident.y) < 1e-9);
        ok = ok && (std::fabs(sr.direction.z - incident.z) < 1e-9);

        if (ok) {
            std::printf("  [PASS] Test5: n1=n2 passthrough  direction unchanged\n");
            passed++;
        } else {
            std::printf("  [FAIL] Test5: n1=n2 passthrough  dirIn=(%.4f,%.4f,%.4f) dirOut=(%.4f,%.4f,%.4f)\n",
                incident.x, incident.y, incident.z,
                sr.direction.x, sr.direction.y, sr.direction.z);
            failed++;
        }
    }

    std::printf("========== Result: %d passed, %d failed ==========\n\n", passed, failed);
}

} // namespace rt
