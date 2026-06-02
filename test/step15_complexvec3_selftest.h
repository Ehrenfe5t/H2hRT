// v9 步骤15 (B1-a): ComplexVec3 自测验证
// 使用方法: 在 app/main.cpp 中临时 include 并调用 RunStep15SelfTest()
// 验证完成后删除 include 和调用

#pragma once

#include "../core/common/math/ComplexVec3.h"
#include <cstdio>
#include <cmath>

namespace rt {

inline void RunStep15SelfTest() {
    std::printf("\n========== v9 Step15 (B1-a): ComplexVec3 Self-Test ==========\n");

    int passed = 0, failed = 0;
    const double eps = 1e-14;

    // ── Test 1: 加减恒等式 (a+b)-b == a ──
    {
        ComplexVec3 a(Complex(1.5, 0.3), Complex(-2.1, 0.7), Complex(0.0, 1.0));
        ComplexVec3 b(Complex(0.5, -0.2), Complex(1.0, 0.0), Complex(-0.3, 0.8));
        ComplexVec3 diff = Subtract(Add(a, b), b);
        bool ok = (std::fabs(diff.x.re - a.x.re) < eps) &&
                  (std::fabs(diff.x.im - a.x.im) < eps) &&
                  (std::fabs(diff.y.re - a.y.re) < eps) &&
                  (std::fabs(diff.y.im - a.y.im) < eps) &&
                  (std::fabs(diff.z.re - a.z.re) < eps) &&
                  (std::fabs(diff.z.im - a.z.im) < eps);
        if (ok) { std::printf("  [PASS] Test1: (a+b)-b == a\n"); passed++; }
        else    { std::printf("  [FAIL] Test1: (a+b)-b != a\n"); failed++; }
    }

    // ── Test 2: 正交实基投影+重构 ──
    // 构造TE基和TM基, E在基平面内 → 投影再重构应精确还原
    {
        Vec3 eTE   = MakeVec3(0.0, 1.0, 0.0);  // Y轴: TE基
        Vec3 eTM   = MakeVec3(1.0, 0.0, 0.0);  // X轴: TM基
        // E在XY平面内 (Z=0), 确保在基张成空间内
        ComplexVec3 E(Complex(2.0, 0.5), Complex(-1.0, 0.3), Complex(0.0, 0.0));

        Complex cTE = ComplexDot(E, eTE);  // 投影到TE → E.y = -1.0+0.3i
        Complex cTM = ComplexDot(E, eTM);  // 投影到TM → E.x = 2.0+0.5i
        ComplexVec3 E_recon = ReconstructFromBasis(cTE, eTE, cTM, eTM);

        ComplexVec3 d = Subtract(E, E_recon);
        bool ok = (Norm(d) < eps);
        if (ok) { std::printf("  [PASS] Test2: project+reconstruct TE/TM basis\n"); passed++; }
        else    { std::printf("  [FAIL] Test2: reconstruction error norm=%.2e\n", Norm(d)); failed++; }
    }

    // ── Test 3: 正交基复内积为零 ──
    // eTE = (0,1,0), eTM = (1,0,0) — 正交
    // 任意复矢量投影到两个正交基上的分量做复内积应为零
    {
        ComplexVec3 E(Complex(1.2, -0.8), Complex(0.5, 1.5), Complex(-2.0, 0.1));
        Vec3 eTE = MakeVec3(0.0, 1.0, 0.0);
        Vec3 eTM = MakeVec3(1.0, 0.0, 0.0);

        Complex cTE = ComplexDot(E, eTE);
        Complex cTM = ComplexDot(E, eTM);

        // 构造两个纯极化复矢量: E_TE_only = cTE * eTE, E_TM_only = cTM * eTM
        ComplexVec3 E_TE = ScaleComplexVec3(eTE, cTE);
        ComplexVec3 E_TM = ScaleComplexVec3(eTM, cTM);

        // 正交极化的复点积应为零
        Complex dotCheck = ComplexDot(E_TE, E_TM);
        bool ok = (dotCheck.Norm() < eps);
        if (ok) { std::printf("  [PASS] Test3: orthogonal polarization dot = 0\n"); passed++; }
        else    { std::printf("  [FAIL] Test3: dot Norm=%.2e\n", dotCheck.Norm()); failed++; }
    }

    // ── Test 4: 归一化 — 归一化后模=1, 方向(相位比)不变 ──
    {
        ComplexVec3 E(Complex(3.0, 4.0), Complex(0.0, 5.0), Complex(-6.0, 8.0));
        ComplexVec3 E_norm = Normalize(E);
        bool ok = (std::fabs(Norm(E_norm) - 1.0) < eps);
        // 归一化后的方向应与原方向一致: E_norm 应等于 E/|E|
        double n = Norm(E);
        ComplexVec3 expected = Scale(E, 1.0 / n);
        ComplexVec3 d = Subtract(E_norm, expected);
        ok = ok && (Norm(d) < eps);
        if (ok) { std::printf("  [PASS] Test4: normalize |E|=1, direction preserved\n"); passed++; }
        else    { std::printf("  [FAIL] Test4: norm=%.15f, dir error=%.2e\n", Norm(E_norm), Norm(d)); failed++; }
    }

    // ── Test 5: ScaleComplexVec3 一致性 ──
    // ScaleComplexVec3(v, c) ≡ ComplexVec3(v.x*c, v.y*c, v.z*c)
    {
        Vec3 v = MakeVec3(0.6, 0.8, 0.0); // 任意方向
        Complex c(2.0, 1.0);
        ComplexVec3 scaled = ScaleComplexVec3(v, c);
        ComplexVec3 manual(
            Complex(v.x * c.re - 0.0 * c.im, v.x * c.im + 0.0 * c.re), // v.x * c
            Complex(v.y * c.re, v.y * c.im),
            Complex(0.0, 0.0));
        // v.x * c = 0.6*(2+i) = 1.2+0.6i
        bool ok = (std::fabs(scaled.x.re - 1.2) < eps) &&
                  (std::fabs(scaled.x.im - 0.6) < eps) &&
                  (std::fabs(scaled.y.re - 1.6) < eps) &&  // 0.8*2
                  (std::fabs(scaled.y.im - 0.8) < eps);    // 0.8*1
        if (ok) { std::printf("  [PASS] Test5: ScaleComplexVec3 consistency\n"); passed++; }
        else    { std::printf("  [FAIL] Test5: scaled=(%.4f%+.4fi, %.4f%+.4fi)\n",
                              scaled.x.re, scaled.x.im, scaled.y.re, scaled.y.im); failed++; }
    }

    // ── Test 6: 零场归一化不崩溃 ──
    {
        ComplexVec3 zero;
        ComplexVec3 nz = Normalize(zero);
        bool ok = (Norm(nz) < eps);
        if (ok) { std::printf("  [PASS] Test6: zero field normalize → zero (no crash)\n"); passed++; }
        else    { std::printf("  [FAIL] Test6: zero norm result norm=%.2e\n", Norm(nz)); failed++; }
    }

    std::printf("========== Result: %d passed, %d failed ==========\n\n", passed, failed);
}

} // namespace rt
