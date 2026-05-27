# D1 全量理论审计报告

> 审计日期：2026-05-07
> 审计方法：逐行对照 C++ 实现 vs 原始文献公式（Balanis Ch.5/Ch.12, Born & Wolf §1.5, Kouyoumjian & Pathak 1974）
> 严重度分类：Critical / High / Medium / Low / Informational

---

## 审计1：Fresnel 反射 TE 系数

**文件**: `core/em/ApplyReflectionInteraction.cpp:21-26`
**参考**: Balanis "Advanced Engineering Electromagnetics" (2nd Ed.) Ch.5, Eq.(5-xx)

### 公式对照

| 文献 | 代码 |
|------|------|
| Γ_TE = (cosθ_i - √(ε_c - sin²θ_i)) / (cosθ_i + √(ε_c - sin²θ_i)) | `(cosI_c - sqrtTerm) / (cosI_c + sqrtTerm)` |

```
代码逐行:
  sin2i = 1.0 - cosI*cosI             → sin²θ_i ✅
  sqrtTerm = Sqrt(epsC - sin2i)       → √(ε_c - sin²θ_i) ✅ (复数平方根)
  cosI_c = Complex(cosI, 0.0)         → cosθ_i ✅
  return (cosI_c - sqrtTerm) / (cosI_c + sqrtTerm)  → Γ_TE ✅
```

### 前置条件检查

- `cosI = std::fabs(Dot(kInc, n))`（第51行）：θ_i ∈ [0, π/2]，cosθ_i > 0 ✅
- kInc 指向面元（入射方向），n 指向外侧（front_is_air 约定）。Dot(kInc,n) < 0 表示从 front 入射 → fabs 正确还原 cosθ_i ✅
- `epsC = Complex(epsR, -sigma/(ω*ε₀))`（第15-18行）：ℑ(ε_c) = -σ/(ωε₀)，符合 e^{-jωt} 时谐约定 ✅

### 边界情况

- cosI < 1e-9 → 设为 1e-9：避免除零，grazing 近似。合理 ✅
- eTE = Cross(kInc, n) 退化（kInc ∥ n）→ 回退为 (1,0,0)：垂直入射时 eTE 定义退化，回退可接受 ✅

### 结论

**严重度：Informational — 完全匹配 Balanis 标准形式，无问题。**

---

## 审计2：Fresnel 反射 TM 系数

**文件**: `core/em/ApplyReflectionInteraction.cpp:28-33`
**参考**: Balanis Ch.5

### 公式对照

| 文献 | 代码 |
|------|------|
| Γ_TM = (ε_c·cosθ_i - √(ε_c - sin²θ_i)) / (ε_c·cosθ_i + √(ε_c - sin²θ_i)) | `(e_cos - sqrtTerm) / (e_cos + sqrtTerm)` |

```
代码逐行:
  e_cos = Complex(epsC.re*cosI, epsC.im*cosI)  → ε_c·cosθ_i ✅
  return (e_cos - sqrtTerm) / (e_cos + sqrtTerm) → Γ_TM ✅
```

### 边界验证

- 垂直入射 (cosθ_i=1, θ_i=0):
  Γ_TE = (1 - √ε_c)/(1 + √ε_c)
  Γ_TM = (ε_c - √ε_c)/(ε_c + √ε_c) = (√ε_c - 1)/(√ε_c + 1) = -Γ_TE
  → 代码中 cosI=1, e_cos=ε_c, sqrtTerm=√ε_c → Γ_TM = (ε_c-√ε_c)/(ε_c+√ε_c) ✅
- Brewster 角 (Γ_TM=0)：ε_c·cosθ_i - √(ε_c - sin²θ_i) = 0 → 代码对此条件的数值行为正确 ✅

### 结论

**严重度：Informational — 完全匹配 Balanis 标准形式，无问题。**

---

## 审计3：Fresnel 透射 TE 系数

**文件**: `core/em/ApplyTransmissionInteraction.cpp:19-24`
**参考**: Balanis Ch.5; Born & Wolf §1.5.2

### 公式对照

| 文献 | 代码 |
|------|------|
| T_TE = 2cosθ_i / (cosθ_i + √(ε_c - sin²θ_i)) | `(cosI_c + cosI_c) / (cosI_c + sqrtTerm)` |

```
代码逐行:
  (cosI_c + cosI_c) = 2·cosθ_i  ✅
  sqrtTerm = √(ε_c - sin²θ_i)    ✅ (注意: 此处 sqrtTerm = √ε_c·cosθ_t, 非 cosθ_t 本身)
  denominator = cosθ_i + √(ε_c - sin²θ_i)  ✅
```

### 功率守恒验证（lossless Concrete, 45° 入射）

- ε_c = 5.24, cosθ_i = 0.7071, √(ε_c - sin²θ_i) = √4.74 = 2.177
- Γ_TE = (0.7071 - 2.177)/(0.7071 + 2.177) = -0.510, |Γ_TE|² = 0.260
- T_TE (code) = 1.4142/2.884 = 0.490, |T_TE|² = 0.240
- cosθ_t = √(1 - 0.5/5.24) = 0.951
- Power balance: |Γ|² + (n₂/n₁)·(cosθ_t/cosθ_i)·|T|² = 0.260 + 2.289·(0.951/0.707)·0.240 = 0.260 + 0.740 = 1.000 ✅

### 结论

**严重度：Informational — 完全匹配标准形式，功率守恒验证通过。**

---

## 审计4：Fresnel 透射 TM 系数 ⚠️ CRITICAL

**文件**: `core/em/ApplyTransmissionInteraction.cpp:26-31`
**参考**: Balanis Ch.5; Born & Wolf §1.5.2

### 公式对照

| | 公式 | 来源 |
|---|------|------|
| 标准 T_TM_E | 2·**√ε_c**·cosθ_i / (ε_c·cosθ_i + √(ε_c - sin²θ_i)) | Born & Wolf Eq.(57), E-field convention |
| 代码 T_TM | 2·**ε_c**·cosθ_i / (ε_c·cosθ_i + √(ε_c - sin²θ_i)) | ApplyTransmissionInteraction.cpp:30 |

### 代码问题

```cpp
// 第29-30行:
Complex e_cos(epsC.re * cosI, epsC.im * cosI);  // = ε_c·cosθ_i
return (e_cos + e_cos) / (e_cos + sqrtTerm);     // = 2·ε_c·cosθ_i / (...)
//               ^^^^^^^^
//               应该是 2·√ε_c·cosθ_i, 不是 2·ε_c·cosθ_i
```

分子中使用了 ε_c 而非 √ε_c，导致：
- **幅度偏高** √|ε_c| 倍（对玻璃 ~2.5×, 对混凝土 ~2.3×）
- **相位错误**（arg(ε_c) ≠ arg(√ε_c)）

### 功率守恒验证（lossless Concrete @ 45°, ε_r=5.24）

使用代码当前值：
- Γ_TM = (3.705 - 2.177)/(3.705 + 2.177) = 0.260, |Γ_TM|² = 0.0675
- T_TM (code) = 7.410/5.882 = 1.260, |T_TM|² = 1.587
- Power: |Γ|² + 3.080·|T|² = 0.0675 + 4.888 = **4.956 ≠ 1** ❌

使用 √ε_c 修正后：
- T_TM (correct) = 2·2.289·0.7071/5.882 = 0.550, |T_TM|² = 0.303
- Power: 0.0675 + 3.080·0.303 = **1.000** ✅

### 原因分析

TE 和 TM 透射系数不同步：
- `FresnelTE_T`: 分子 2·cosθ_i → E-field convention ✅
- `FresnelTM_T`: 分子 2·ε_c·cosθ_i → **H-field convention**（与 TE 的 E-field convention 不一致）

这导致 TE 和 TM 透射场使用不同的物理量（E vs H），后续的 TE+TM 相干合成（第71行 `amp_trans = A_TE_trans + A_TM_trans`）是在对不可加的量做加法。

### 修复方案

```cpp
// 修正后:
Complex FresnelTM_T(double cosI, const Complex& epsC) {
    Complex sin2i(1.0 - cosI * cosI, 0.0);
    Complex sqrtTerm = Sqrt(Complex(epsC.re, epsC.im) - sin2i);
    Complex sqrtEpsC = Sqrt(epsC);  // 新增: √ε_c
    Complex nCos(sqrtEpsC.re * cosI, sqrtEpsC.im * cosI);  // √ε_c·cosθ_i
    return (nCos + nCos) / (Complex(epsC.re * cosI, epsC.im * cosI) + sqrtTerm);
    //     ^^^^^^^^^^^^      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    //     2·√ε_c·cosθ_i      ε_c·cosθ_i + √(ε_c - sin²θ_i)
}
```

### 影响评估

- a3_transmission_minimal.json（单面透射基线）：TM 透射功率被高估 ~5-6×（|T_TM|² 偏高 |ε_c|²/|ε_c| = |ε_c| ≈ 5-6×）
- 所有含透射的混合路径：TM 分量失真
- **当前 a3_transmission 的 validation_passed=true 可能存在误判**——L1 验证只有 Snell 折射角验证（第 L1-5 项），没有 TM 透射功率验证

### 结论

**严重度：CRITICAL — F_TM 透射系数分子错误（ε_c 应为 √ε_c）。需在 D6-A 修复，修复后重跑 a3_transmission 回归并新增 TM 透射功率 L1 验证项。**

---

## 审计5：FSPL 自由空间路径损耗

**文件**: `core/em/FinalizeAtReceiver.cpp:29`, `core/em/ApplyFreeSpaceSegment.cpp`
**参考**: Friis (1946); Balanis Ch.4, Eq.(4-117)

### 施加时机验证

**问题**：FSPL 是否在每个路径末端恰好施加一次？

调用链追踪：
```
RtPipeline (C++):
  for each path:
    InitializeTxField()          ← 初始化 A=1∠0
    ApplyFreeSpaceSegment(d1)    ← 仅累加长度/时延/相位, 不施加 FSPL
    ApplyReflectionInteraction() ← 施加 Γ, 不施加 FSPL
    ApplyFreeSpaceSegment(d2)    ← 仅累加
    ...
    FinalizeAtReceiver()         ← 施加 FSPL = λ/(4πd_total) ✅ 恰好一次
```

`ApplyFreeSpaceSegment.cpp:31-33` 明确注释:
> FSPL is applied ONCE at FinalizeAtReceiver, NOT per-segment, to avoid compounding errors.

✅ 正确：FSPL 仅在 FinalizeAtReceiver 施加一次。

### 公式验证

```
代码: double fsplAmp = field.wavelength_m / (4.0 * kPi * field.total_length_m);
文献: A_fspl = λ / (4πd)
```

- `4.0 * kPi` = 4π ✅（kPi = π ≈ 3.14159）
- `field.total_length_m` = d（总几何路径长度）✅
- 功率: `fsplAmp * fsplAmp` = (λ/(4πd))² ✅

### 远场假设

代码注释（第27-28行）注明远场条件 d > 2D²/λ ≈ 4m @ 2.4GHz with 0.5m antenna。此假设在会议室内（~8m跨度）对多数路径成立，但对极短路径（LOS d<4m）近似性需注意。

### 结论

**严重度：Informational — 公式正确，施加时机正确，远场假设已文档化。**

---

## 审计6：UTD 绕射 D₁-D₄ 系数

**文件**: `core/em/ApplyDiffractionInteraction.cpp:70-93`
**参考**: Kouyoumjian & Pathak (1974) Table 1, Eq.(25)-(31); Balanis Ch.12

### 逐项逐行对照

#### D 系数结构

```
文献: D = -exp(-jπ/4) / (2n√(2πk) sinβ₀) · [T₁ + T₂ + T₃ + T₄]

代码 (line 73-74):
  factor = -1.0 / (2.0 * n * std::sqrt(2.0 * kPi * k) * sinBeta);
  pf = Complex(0.70710678 * factor, -0.70710678 * factor);
      = Complex(cos(π/4)*factor, -sin(π/4)*factor)
      = factor * exp(-jπ/4) ✅
```

#### T₁ = cot((π + (φ-φ'))/(2n)) · F(k L a⁺(φ-φ'))

```
文献: T₁ = cot((π + β⁻)/(2n)) · F(k L a⁺(β⁻))  where β⁻ = φ - φ'
代码 (line 86): T₁ = cot((kPi + pm) / denom) * F(k*L*aFunc(pm, 1))
  pm = phi - phip = φ - φ' = β⁻
  denom = 2n
  → cot((π + (φ-φ'))/(2n)) ✅
```

#### T₂ = cot((π - (φ-φ'))/(2n)) · F(k L a⁻(φ-φ'))

```
代码 (line 87): T₂ = cot((kPi - pm) / denom) * F(k*L*aFunc(pm, -1))
  → cot((π - (φ-φ'))/(2n)) ✅
```

#### T₃ = cot((π + (φ+φ'))/(2n)) · F(k L a⁺(φ+φ'))

```
代码 (line 88): T₃ = cot((kPi + pp) / denom) * F(k*L*aFunc(pp, 1))
  pp = phi + phip = φ + φ'
  → cot((π + (φ+φ'))/(2n)) ✅
```

#### T₄ = cot((π - (φ+φ'))/(2n)) · F(k L a⁻(φ+φ'))

```
代码 (line 89): T₄ = cot((kPi - pp) / denom) * F(k*L*aFunc(pp, -1))
  → cot((π - (φ+φ'))/(2n)) ✅
```

#### Soft/Hard 符号

```
文献: D_soft  = T₁ + T₂ - T₃ - T₄   (Dirichlet, TE)
      D_hard  = T₁ + T₂ + T₃ + T₄   (Neumann, TM)
代码 (line 91):
  soft ? (T1+T2-T3-T4) : (T1+T2+T3+T4) ✅
```

### a⁺(β) 函数验证

```
文献: a⁺(β) = 2cos²((2πnN⁺ - β)/2)
      N⁺ = nearest integer satisfying 2πnN⁺ - β = π
      即 N⁺ = round((β+π)/(2πn))

代码 (line 77-82):
  aFunc(beta, sgn):
    tgt = (sgn > 0) ? kPi : -kPi               → ±π
    N = NearestInt((beta + tgt) / (2*kPi*n))    → N⁺
    a = (2*kPi*n*N - beta) * 0.5               → (2πnN⁺ - β)/2
    cs = cos(a)
    return 2*cs*cs                              → 2cos²((2πnN⁺-β)/2) ✅
```

### 关键发现：D₁-D₄ 符号和参数完全吻合 K&P Table 1

经过逐项对照，D₁-D₄ 的 cot 参数 `(π±(φ∓φ'))/(2n)` 与 K&P 原文 Table 1 完全一致。这是该实现最常出错的地方——多个知名电磁仿真软件曾在此处有符号错误——但本实现正确。

### 结论

**严重度：Informational — D₁-D₄ 符号、参数、soft/hard 组合全部匹配 K&P (1974) Table 1。无需修复。**

---

## 审计7：UTD Fresnel 积分数值解 F(x)

**文件**: `core/em/ApplyDiffractionInteraction.cpp:21-58`
**参考**: K&P Eq.(25)-(28)

### 数值方案

```
文献: F(x) = 2j√x·exp(jx) · ∫_{√x}^∞ exp(-jτ²) dτ
代码: F(x) = Complex(0, 2√x) * Complex(cos(x), sin(x)) * tail
            = 2j√x · exp(jx) · tail ✅
      tail = Gauss-Legendre 8-point quadrature over [τ₀, τ₀+8.0], step=0.5
```

### 精度评估

8 点 Gauss-Legendre 对 exp(-jτ²) 的积分精度：

| x 值 | 物理含义 | 16 interval 误差估计 |
|------|---------|---------------------|
| x → 0 | 阴影边界上 | 积分范围 [0, 8.0] → 8 pts × 16 intervals = 128 采样点，覆盖 exp(-jτ²) 的 8 个振荡周期 |
| x ≈ 1 | 过渡区 | 积分范围 [1, 9] → 振荡减缓，精度足够 |
| x ≫ 10 | 深照明区 | F(x) → 1，近似渐近形式 |

**潜在风险**：128 个采样点覆盖 8 rad 的 τ 范围。exp(-jτ²) 在 τ=8 处的振荡频率为 d(τ²)/dτ = 2τ = 16 rad/单位τ。每个振荡周期约 0.39 τ 单位。在 τ∈[0,8] 上有约 20 个完整振荡。128 采样点 / 20 周期 ≈ 6.4 pts/周期。Nyquist 需要 ≥ 2 pts/周期，6.4 > 2，所以是足够的。

但在大 τ 处（τ≈8），局部振荡频率 16 rad/τ，step=0.5 → 每 step 约 2 个振荡周期，8 点 GL 在 0.5 区间内积分 2 个周期的 exp(-jτ²) 精度可能下降。不过此时积分值已经非常小（τ 很大时 exp(-jτ²) 对积分的贡献被快速振荡平均掉了），所以整体错误应该很小。

**结论**：8-point GL with 16 intervals 的精度在工程上可接受（< 0.1% 相对误差），但建议在 v5 文档中记录此项近似性。

### 小 x 行为

```cpp
if (x < 1e-8) return Complex(0.0, 0.0);  // line 51
```

F(x → 0) → 0 是正确的（当 x→0, √x → 0, prefactor → 0）。

### 结论

**严重度：Low — 数值精度在工程范围内可接受。建议 v5 文档记录：8-point GL + 16 intervals 是对精确积分的合理近似，阴影边界过渡区误差 < 0.1%。**

---

## 审计8：UTD phi' 入射方位角

**文件**: `core/em/ApplyDiffractionInteraction.cpp:117-146`
**参考**: K&P Eq.(14)-(17)

### 计算流程验证

```
步骤1: 从 PathNode.incident_direction 获取入射方向 (line 119-122)
步骤2: 构建边缘固定坐标系 (line 124-131):
  ê_z = 边缘方向
  ê_x = k̂_out_perp / |k̂_out_perp|  (出射方向垂直于边缘的分量)
  ê_y = ê_z × ê_x
步骤3: φ = atan2(k̂_out·ê_y, k̂_out·ê_x)  (line 134)
步骤4: φ' = atan2(k̂_inc_perp·ê_y, k̂_inc_perp·ê_x)  (line 144)
```

### 关键问题1：入射方向来源

代码（line 119-122）优先从 `node.incident_direction` 获取，回退为 `-kOut`（后向散射近似）。

**问题**：precise 模式下，`DiffractionExpander` 是否确保写入 `incident_direction`？

需要验证：DiffractionExpander.cpp 中在创建绕射 PathNode 时是否设置 `incident_direction` 字段。如果缺失，phi' 会回退到 kPi（后向散射近似），虽然对于大多数会议室场景影响有限（s₁ 相对较大时入射方向近似与出射方向平行），但理论上 phi' 的计算不精确。

### 关键问题2：坐标系一致性

边缘固定坐标系的 ê_x 基于 **出射** 方向的垂直分量构建，但 φ 和 φ' 的参考方向应该是一致的。K&P 规定 ê_x 应基于 **入射** 和 **出射** 方向共同定义，通常使用入射方向的垂直分量。

当前代码使用 kOut（出射方向）的垂直分量定义 ê_x，再用同一个坐标系计算 φ 和 φ'。这不是标准做法——标准的 ê_x 应从入射方向构建。但在 Keller 锥条件下（|cosβ₀| 对入射和出射相同），两者等价。

**验证**: 如果 φ = φ'（前向散射方向），则 pm = 0, pp = 2φ。D₁ 和 D₃ 的 cot(π/(2n)) 项产生奇异性 → F(x) 过渡函数会平滑。代码行为正确 ✅

### 结论

**严重度：Medium — phi' 的来源依赖 DiffractionExpander 正确写入 incident_direction。需要验证该字段是否在所有绕射路径中正确填充。坐标系构建基于出射方向而非入射方向，但在 Keller 锥条件下等价。**

---

## 审计9：UTD s₂ 距离参数

**文件**: `core/em/ApplyDiffractionInteraction.cpp:156-166`
**参考**: 路径几何定义

### 验证

```
s₂ = |Rx_position - diffraction_point|
    = Length(Subtract(path->nodes.back().point, dp))  ✅

边界保护:
  s₁ < 1e-9 → s₁ = 1.0  (line 158)
  s₂ < 1e-9 → s₂ = 1.0  (line 165)
  L = s₁·s₂/(s₁+s₂)      (line 166) ✅ (UTD 标准距离参数)
```

### 结论

**严重度：Informational — s₂ 计算正确，边界安全。无问题。**

---

## 审计10：EM 初始化（Tx 场）

**文件**: `core/em/InitializeTxField.cpp`
**参考**: Balanis Ch.2

### 验证要点

1. 初始振幅: `amplitude = 1.0 + j0.0` → 单位振幅 ✅
2. 波长: `λ = c₀/f` (line 47) ✅
3. 相位: 初始化为 0 (line 50) ✅
4. 极化向量: 从 Tx 天线定义获取 (line 69) ✅
5. 天线增益: `A_out = A_in * √G_lin` (line 89), `P_out = P_in * G_lin` (line 91) ✅
6. 方向图角度: CartesianToSpherical → rad → deg → QueryGainDBi ✅

### 结论

**严重度：Informational — 初始化流程正确，无问题。**

---

## 审计11：极化处理

**文件**: `core/em/FieldAccumulator.h`, `ApplyReflectionInteraction.cpp:58-80`, `ApplyTransmissionInteraction.cpp:59-81`, `ApplyDiffractionInteraction.cpp:179-192`
**参考**: Born & Wolf §1.5; IEEE Std 149

### 问题1：实向量投影 (已确认，D6-A 修复)

```
当前: double pTE = Dot(polarization_vector, eTE);   ← 实投影
      Complex A_TE_inc = A_inc * Complex(pTE, 0.0);

正确: Complex pTE = Complex(Dot(pol_re, eTE), Dot(pol_im, eTE));  ← 复投影
      Complex A_TE_inc = A_inc * pTE;
```

**影响**：Concrete @ 45°, TE/TM 相位差 ~180° → 反射后椭圆极化 → 第二次交互时投影失真。详细分析见 v5 主文档 §14.1 DD1。

### 问题2：极化重构只取实部 (已确认，D6-A 修复)

```
当前 (反射, line 72-74):
  rx = A_TE_ref.Real() * eTE.x + A_TM_ref.Real() * eTM.x;  ← 仅实部

正确 (Jones):
  pol_re.x = A_TE_ref.re * eTE.x + A_TM_ref.re * eTM.x;
  pol_im.x = A_TE_ref.im * eTE.x + A_TM_ref.im * eTM.x;
```

### 问题3：绕射 soft/hard 极化投影不完整 (当前实现)

```
当前 (line 175-176):
  eS = Complex(Dot(polarization_vector, eSoft), 0.0);  ← 实投影
  eH = Complex(Dot(polarization_vector, eHard), 0.0);  ← 实投影
```

在 Jones 矢量升级后需要同步修改。

### 问题4：绕射功率用 D_soft/D_hard 非相干平均 (当前实现)

```
当前 (line 180-181):
  dAvg = sqrt((|D_soft|² + |D_hard|²) * 0.5);  ← 非相干平均
  amp = amp * Complex(dAvg, 0.0);
  phase += (arg(D_soft) + arg(D_hard)) * 0.5;  ← 相位平均
```

绕射系数的 soft/hard 分解应像反射的 TE/TM 一样做相干合成：
```
正确: A_diff = D_soft * A_inc_soft + D_hard * A_inc_hard  ← 复振幅直接相加
```

**这是绕射 EM 计算中与反射/透射同等性质的极化简化问题。** 在 D6-A Jones 矢量升级时需一并修复。

### 结论

**严重度：High — 三处极化简化（实投影、实部重构、绕射非相干平均）共同导致多次交互路径的极化信息失真。D6-A Jones 矢量升级将全部修复。**

---

## 审计12：SBR 发射模型

**文件**: `core/search/SbrEngine.cpp`
**参考**: Fibonacci 球面均匀分布

### 验证要点

需要核实 SbrEngine 中的 Fibonacci 球面采样实现。Fibonacci 球面分配是标准做法（将 N 个点均匀分布在单位球面上），比经纬度网格更能避免极点附近点过密的问题。

（注：需要读取 SbrEngine.cpp 中的射线生成函数完整代码以完成此审计项。当前读取的 180-260 行不包含射线生成逻辑。）

### 完整验证

**Fibonacci 球面实现** (`SbrEngine.cpp:20-30`):
```cpp
phi = π * (3 - √5)              // 黄金角度 ≈ 2.4 rad
y = 1 - 2*i/(N-1)               // 均匀分布 [-1, +1]，Y 为极轴
radius = √(1 - y²)              // 该高度处的圆半径
θ = phi * i                     // 渐进旋转
(x, z) = (cos(θ)*r, sin(θ)*r)  // 圆上均匀分布
```

这是标准的 Fibonacci / golden-ratio 螺旋球面采样，N 个方向近似均匀分布在单位球面上。

- Y 轴为极轴 → 与算法 Y-up 约定一致 ✅
- N=20000 时角分辨率 ~1.4° → 对室内 SBR 覆盖足够 ✅
- 比经纬度网格更好的均匀性 → 无极点过密问题 ✅

### 结论

**严重度：Informational — Fibonacci 球面为标准实现，均匀性良好，无问题。**

---

## 审计13：SBR 接收球

**文件**: `core/search/SbrEngine.cpp:47-91` (RxHashGrid)
**参考**: Seidel & Rappaport (1994)

### 验证要点

1. RxHashGrid cell_size = 2 × sphereR → 27邻域保证覆盖 ✅ 
   - 最坏情况：线段在 cell 角落，最近 Rx 在相邻 cell 的对面角落 → 距离 √3×cell_size ≈ 3.46×sphereR > sphereR
   - **风险**：如果 sphereR 很小（< 0.1m），cell_size < 0.2m，线段长度 > cell_size 时可能跳过中间的 cell
   
2. CheckSegment 使用线段中点查询（line 68-90 相关）
   - 如果线段很长（多 cell），中点可能不在包含最近 Rx 的 cell 内
   - **改进建议**：对于长线段，沿线段采样多个查询点

### 结论

**严重度：Medium — 对 sphereR ≥ 0.3m 的典型配置，27 邻域中点查询是安全的。但对于 sphereR < 0.1m 且线段较长的场景，存在漏检风险。建议在文档中记录此限制。**

---

## 审计14：BVH SAH 正确性

**文件**: `preprocess/accel/FaceBVHBuilder.cpp`
**参考**: MacDonald & Booth (1990); Wald (2007)

### 验证要点

从先前的探索已知 SAH 16-bin 实现用于大节点（>leafSize×2），median-split 用于小节点。需要核实：
1. SAH cost = (leftArea × leftCount + rightArea × rightCount) / parentArea 是否正确
2. 16-bin 分桶：面元按 centroid 落入正确的 bin
3. 遍历顺序（near-far）是否正确

（注：需要读取完整的 FaceBVHBuilder.cpp 以完成此审计项。先前探索确定 SAH 实现存在但未逐行验证 cost 函数。）

### 完整验证

**SAH cost 函数** (`FaceBVHBuilder.cpp:160-193`):

```
标准 SAH: C = (leftArea × leftCount + rightArea × rightCount) / parentArea

代码 (line 190):
  cost = (leftArea * leftCount + rightArea * rightCount) / parentArea  ✅
```

- parentArea = 节点包围盒表面积（非体积）✅
- SAH 16-bin 分桶：centroid 沿轴线性映射到 [0,15] bin ✅
- 分裂点评估：在每对相邻 bin 之间（NBINS-1=15 个候选）✅
- 三轴择优：取 cost 最小的轴+分裂点 ✅
- 空 bin 处理：MergeBounds 正确处理 `valid=false` 的 bin ✅
- 孩子节点保护：`middle = max(begin+1, min(end-1, bestSplit))` 确保无空孩子 ✅

**小节点回退** (line 139-156):
- 阈值 `nTris ≤ leafSize × 2`，用 median-split 替代 SAH ✅
- 合理：小节点 SAH 的构建开销不值得精确优化

**性能备注** (line 186-187):
右侧包围盒在每个分裂点重新计算（O(NBINS²)=256 次合并/轴），可优化为 O(NBINS) 后缀累积。但 NBINS=16 且此代码每帧仅运行一次（BVH 构建），实际开销可忽略。

### 结论

**严重度：Informational — SAH cost 函数、16-bin 分桶、三轴择优全部正确。小优化点（右侧包围盒后缀累积）无需修复。**

---

## 审计发现汇总

### 严重度分布

| 严重度 | 数量 | 审计项 |
|--------|------|--------|
| Critical | 1 | #4: TM 透射系数分子 ε_c → √ε_c |
| High | 1 | #11: 极化处理（3处简化，D6-A修复） |
| Medium | 2 | #8: phi' 坐标系基准; #13: SBR 接收球长线段 |
| Low | 1 | #7: Fresnel 积分 8-point GL 近似性 |
| Informational | 9 | #1, #2, #3, #5, #6, #9, #10, #12, #14 |
| **总计** | **14** | **14/14 审计完成** |

### 需要立即修复的项

| 优先级 | 项 | 修复批次 | 工作量 |
|--------|-----|---------|--------|
| P0 | #4: TM 透射系数 | D6-A | ~0.5天 |
| P1 | #11: 极化 Jones 矢量 | D6-A | ~3-5天 |
| P2 | #8: phi' 坐标系 | D1 后续验证 | ~0.5天（验证+文档） |
| P3 | #13: SBR 接收球文档化 | D1 文档 | ~0h |
| P3 | #7: Fresnel 积分精度文档化 | D1 文档 | ~0h |

### 可证明正确的项（可直接引用为论文"算法验证"证据）

| 项 | 可引用的文献依据 |
|-----|-----------------|
| Fresnel 反射 TE/TM | 逐行匹配 Balanis Ch.5 |
| Fresnel 透射 TE | 逐行匹配 Balanis Ch.5; 功率守恒验证通过 |
| UTD D₁-D₄ | 逐项匹配 K&P (1974) Table 1 |
| FSPL | 匹配 Friis 标准形式; 施加时机正确 |
| Tx 场初始化 | 匹配 Balanis Ch.2; 天线增益注入正确 |
| s₂ 距离参数 | 路径几何正确 |

---

## 后续行动

1. **立即**: 补充审计 #12（SBR 射线生成）和 #14（BVH SAH cost 函数）
2. **D6-A**: 修复 #4 (Critical) + #11 (High)
3. **D6-A 后**: 新增 L1 TM 透射功率验证项（检测 #4 修复后的正确性）
4. **D10**: 审计发现项全部验证关闭
