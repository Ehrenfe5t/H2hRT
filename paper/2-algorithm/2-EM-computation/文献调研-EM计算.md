# 电场计算理论依据与实现文献调研

> 调研日期：2026-06-01

---

## 1. Fresnel 反射与透射系数

### 1.1 理论基础

| 文献 | 年份 | 关键贡献 |
|------|------|---------|
| Jackson, J.D., "Classical Electrodynamics", 3rd ed., Ch. 7, Wiley, 1999. | 1999 | **电动力学权威教材**——Fresnel 方程的完整推导（TE/TM 极化） |
| Born, M. and Wolf, E., "Principles of Optics", 7th ed., Ch. 1.5, Cambridge University Press, 1999. | 1999 | Fresnel 公式在介质界面的光学推导 |

### 1.2 Fresnel TE/TM 公式

| 极化 | 反射系数 | 透射系数 |
|------|---------|---------|
| TE (s-polarized) | Γ_TE = (cosθ_i - √(ε_c - sin²θ_i)) / (cosθ_i + √(ε_c - sin²θ_i)) | T_TE = 2·cosθ_i / (cosθ_i + √(ε_c - sin²θ_i)) |
| TM (p-polarized) | Γ_TM = (ε_c·cosθ_i - √(ε_c - sin²θ_i)) / (ε_c·cosθ_i + √(ε_c - sin²θ_i)) | T_TM = 2·√ε_c·cosθ_i / (ε_c·cosθ_i + √(ε_c - sin²θ_i)) |

其中 ε_c = ε_r - jσ/(ωε₀) 为复介电常数。

### 1.3 复介电常数与损耗介质

| 文献 | 关键内容 |
|------|---------|
| ITU-R P.2040-1, "Effects of Building Materials and Structures on Radiowave Propagation above about 100 MHz", 2023. | **ITU 建筑材料电参数数据库**——ε_r(f) 和 σ(f) 的幂律模型 |
| Hippel, A.R., "Dielectric Materials and Applications", MIT Press, 1954. | 介质材料经典参考 |

### 1.4 Brewster 角与全内反射

| 文献 | 关键内容 |
|------|---------|
| Hecht, E., "Optics", 5th ed., Ch. 4, Pearson, 2017. | Brewster 角和 TIR 的标准解释 |
| 本实现：`SnellRefractV2`（Vec3.h）中的 TIR 检测 | `sin²θ_t >= 1.0` → 全内反射 |

---

## 2. Jones 矢量极化追踪

### 2.1 Jones Calculus 基础

| 文献 | 年份 | 关键贡献 |
|------|------|---------|
| Jones, R.C., "A New Calculus for the Treatment of Optical Systems", *JOSA*, 1941. | 1941 | **Jones 矢量开创论文**——7 篇系列论文建立完整数学框架 |
| Azzam, R.M.A. and Bashara, N.M., "Ellipsometry and Polarized Light", North-Holland, 1977. | 1977 | 椭圆偏振光与 Jones/Mueller 矩阵的标准参考 |
| Goldstein, D., "Polarized Light", 3rd ed., CRC Press, 2011. | 2011 | 偏振光教材——Stokes 参数、Jones 矢量、Poincaré 球 |

### 2.2 复矢量电场追踪

| 文献 | 关键内容 |
|------|---------|
| 本实现（主线B）： `ComplexVec3` 类型 + `electric_field_world` 态 | 三维复电场矢量沿路径传播 |
| 反射（`ApplyReflectionInteraction`）：TE/TM 投影 → Fresnel 系数 → 重构 | Γ_TE·E_TE, Γ_TM·E_TM → 世界重建 |
| 透射（`ApplyTransmissionInteraction`）：同反射流程 + 介质折射率更新 | T_TE·E_TE, T_TM·E_TM → n'=Re(√ε_c) |
| 绕射（`ApplyDiffractionInteraction`）：soft/hard 投影 → D_soft·E_soft, D_hard·E_hard | UTD 绕射系数 |

### 2.3 接收端极化匹配

| 文献 | 关键内容 |
|------|---------|
| Balanis, C.A., "Antenna Theory", 4th ed., Ch. 2.12—"Polarization", 2016. | 极化匹配因子 PLF = |ρ̂_tx · ρ̂_rx|² |
| 本实现：`FinalizeAtReceiver` v9 Stage D — 共轭匹配 | vr = E_inc · conj(h_rx) |

---

## 3. UTD 边缘绕射系数

### 3.1 UTD 系数公式

| 文献 | 关键内容 |
|------|---------|
| Kouyoumjian, R.G. and Pathak, P.H., "A Uniform Geometrical Theory of Diffraction", *Proc. IEEE*, 1974. | **标准 UTD 公式**——PEC 楔边的 soft/hard 绕射系数 |
| Luebbers, R.J., "Finite Conductivity Uniform GTD Versus Knife Edge Diffraction", *IEEE TAP*, 1984. | 有限电导率 UTD——损耗楔边的扩展 |
| 本实现：`ApplyDiffractionInteraction.cpp::ComputeUTD_D` | D_soft = pf · (T1+T2-T3-T4), D_hard = pf · (T1+T2+T3+T4) |

### 3.2 Fresnel 过渡函数

| 文献 | 关键内容 |
|------|---------|
| Abramowitz, M. and Stegun, I.A., "Handbook of Mathematical Functions", Ch. 7, Dover, 1964. | Fresnel 积分的标准数值参考 |
| 本实现：`FresnelTransitionNumerical`——8 点 Gauss-Legendre 积分 + 渐近展开 | F(x) = 2j√x·e^{jx} ∫_{√x}∞ e^{-jτ²}dτ |

### 3.3 Wedge 参数化

| 文献 | 关键内容 |
|------|---------|
| 本实现：`WedgeBuilder.cpp` | 外角 α = wedge_angle_deg, n = (2π-α)/π, 凸/凹分类 |

---

## 4. 自由空间传播与 Friis 公式

### 4.1 理论基础

| 文献 | 关键内容 |
|------|---------|
| Friis, H.T., "A Note on a Simple Transmission Formula", *Proc. IRE*, 1946. | **Friis 传输公式**——P_r/P_t = G_t·G_r·(λ/4πd)² |
| 本实现：`FinalizeAtReceiver`——fsplAmp = λ/(4πd) | `ApplyFreeSpaceSegment`——复传播因子 e^{-jk₀nd} |

### 4.2 介质衰减

| 文献 | 关键内容 |
|------|---------|
| 本实现：`FieldAccumulator`——`media_attenuation_np = Σ α_i·d_i` | α = k₀·Im(√ε_c)，透射后更新 current_attenuation |

---

## 5. 对本论文的启示

1. **Fresnel 方程的理论基础不可动摇**（Jackson 1999, Born & Wolf 1999），论文的 TE/TM 极化追踪有坚实理论支撑
2. **Jones 矢量追踪是本论文的核心算法创新**——相比现有 RT 工具的标量极化，复矢量追踪提供了更完整的极化信息
3. **UTD 系数公式可直接引用 K&P 1974**——本实现严格遵循该标准公式
4. **Fresnel 过渡函数的数值精度**——论文可讨论积分精度与渐近切换点选择的影响
