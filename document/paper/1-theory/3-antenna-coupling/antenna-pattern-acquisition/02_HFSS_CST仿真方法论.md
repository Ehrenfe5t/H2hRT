# 02 — HFSS/CST 天线全波仿真方法论

> 目的：建立三种天线的 HFSS 全波仿真标准流程，导出 H2hRT 兼容的方向图 CSV  
> 软件：Ansys HFSS (版本 ≥ 2023 R1) 或 CST Studio Suite (版本 ≥ 2023)  
> 关键参考：HFSS 在线帮助文档、Balanis Ch.8 (Integral Equations & MoM/FEM)

---

## 2.1 仿真总流程

```
几何建模 → 材料/边界条件 → 激励端口 → 求解设置 → 网格收敛验证 → 远场设置 → 数据导出
```

每一阶段分别针对三种天线展开。

---

## 2.2 半波偶极子仿真（λ/2 @ 2.4 GHz）

### 2.2.1 几何建模

| 参数 | 值 |
|---|---|
| 频率 | 2.4 GHz → λ ≈ 125 mm |
| 偶极子臂长 | λ/4 = 31.25 mm（每臂） |
| 导线半径 | 1 mm（细线近似，r ≪ λ） |
| 馈电间隙 | 1 mm（两臂间距） |
| 材料 | PEC（理想导体） |

**HFSS 操作**：
1. Create → Cylinder（半径 1 mm, 长度 31.25 mm, 沿 Z 轴）
2. Duplicate → Mirror（关于 XY 平面镜像）
3. 两臂间距 1 mm（间隙位于 Z=0）

### 2.2.2 激励

- **Lumped Port**：100Ω 参考阻抗（偶极子谐振阻抗 ≈ 73 + j0 Ω，选择 100Ω 作为近似匹配，也可选 73Ω）
- 端口面连接两臂间隙处

### 2.2.3 边界条件

- **Radiation Boundary**（辐射边界）：球面或圆柱面，距天线 ≥ λ/4（~31 mm）
- 或使用 **PML**（Perfectly Matched Layer，完美匹配层）：更精确但求解更慢

### 2.2.4 求解设置

- **Solution Type**：Driven Modal（模式驱动）或 Driven Terminal
- **求解频率**：2.4 GHz
- **自适应网格**：Maximum Delta S = 0.02（默认），Maximum Number of Passes = 15

### 2.2.5 网格收敛验证

**关键步骤**：连续两次自适应迭代的 |ΔS| < 0.005 且远场增益变化 < 0.1 dB，方可认为收敛。若不收敛：
1. 增加 Maximum Passes → 20
2. 手动加密馈电区域网格（Length-based mesh, ~λ/20）

---

## 2.3 双锥天线仿真（SZ-2001800/P, 2–18 GHz）

### 2.3.1 几何建模

SZ-2001800/P 的具体几何参数需要从制造商数据表或实际测量获取。**如果无法获取精确尺寸**，使用以下典型参数作为初始模型：

| 参数 | 典型值（待确认） |
|---|---|
| 锥角（半角）θ₀ | 30°–45°（对应阻抗 50–100Ω） |
| 锥高 | ~100 mm |
| 终端结构 | 球状（直径 ~8–12 mm）或渐变过渡 |
| 馈电间隙 | 1–2 mm |
| N 型连接器 | 含介质支撑（PTFE, εr=2.1） |

### 2.3.2 宽频仿真策略

双锥覆盖 2–18 GHz（9:1 带宽），需要宽带仿真策略：

**方案 A：频域离散扫描**（HFSS Discrete Sweep）
- 选择 8–12 个频点：2, 3, 4, 6, 8, 10, 12, 15, 18 GHz
- 每个频点独立求解 → 总时间长但精度高

**方案 B：宽带插值扫描**（HFSS Interpolating Sweep）
- 求解频点：2, 6, 12, 18 GHz（4 个）
- 中间频点通过插值获得 → 速度快但精度略低

**推荐**：对论文使用的每个频点做 Discrete Sweep（精度优先）。

### 2.3.3 边界条件

- **Radiation Boundary**：圆柱面，距天线 ≥ λ_max/4（λ_max @ 2 GHz = 150 mm → 距离 ≥ 38 mm）
- 推荐使用 **PML**（尤其在低频端，辐射边界距离不足时）

### 2.3.4 特别注意

1. **馈电结构建模**：N 型连接器的内部结构（中心针 + PTFE 介质）对 15–18 GHz 阻抗有显著影响，必须建模
2. **网格收敛**：每个频点独立检查 |ΔS| < 0.005
3. **端部效应**：有限长锥的端部电流反射是方向图波纹的物理来源，仿真应捕捉此效应

---

## 2.4 角锥喇叭仿真（LB-75-10-C-NF, 10–15 GHz）

### 2.4.1 几何建模

| 参数 | 来源 |
|---|---|
| WR-75 波导截面 | a=19.05 mm, b=9.525 mm（标准） |
| 波导段长度 | 15–20 mm（确保 TE₁₀ 模充分建立） |
| 喇叭张角 | 从数据表或实测获取 |
| 口径面尺寸 A×B | 从数据表获取（典型 ~80×60 mm） |
| 材料 | 铝（Al 6061，σ=3.8E7 S/m）或 PEC 近似 |

**若缺乏精确几何数据**：可对喇叭照片进行比例估算，或使用 WR-75 标准增益喇叭的典型设计参数。

### 2.4.2 激励

- **Wave Port**：位于 WR-75 波导截面（19.05 × 9.525 mm）
- 模式数：1（仅 TE₁₀）
- 积分线：从波导宽边中心到对面（定义极化方向）
- De-embedding：0 mm（端口就在波导起始处）

### 2.4.3 边界条件

- **Radiation Boundary**：盒子距口径面 ≥ λ/2（λ @ 12.5 GHz ≈ 24 mm → 距离 ≥ 12 mm）
- 或 **PML** 边界以提高远旁瓣精度

### 2.4.4 求解设置

- 求解频率：12.5 GHz（频段中心）
- 宽带扫描：10–15 GHz，步长 0.5 GHz（11 个频点）

---

## 2.5 远场设置与方向图导出

### 2.5.1 Infinite Sphere 设置（HFSS）

HFSS 在求解区域内计算远场，将结果投影到无限大球面上。

**设置步骤**：
1. HFSS → Radiation → Insert Far Field Setup → Infinite Sphere
2. **θ 范围**：0°–180°，步长 5°（37 点）— 与 H2hRT 默认 `aps_theta_bins=36` 对齐
3. **φ 范围**：0°–360°，步长 5°（73 点）— 与 H2hRT 默认 `aps_phi_bins=72` 对齐
4. **坐标系**：天线局部坐标系（Z 轴 = boresight / 偶极子轴）

**⚠️ 重要警告（HFSS 文档明确声明）**：HFSS 报告的峰值方向性和最大辐射强度**取决于用户指定的 θ/φ 网格是否包含实际峰值方向**。HFSS 不会在用户指定的角点之间做插值或子网格峰值搜索。因此：
- 对定向天线（喇叭），确保 boresight 方向（通常 θ=0°）在网格中
- 对全向天线（偶极子/双锥），确保水平面（θ=90°）在网格中
- **建议先用 1° 步长的精细网格确认峰值位置，再用 5° 步长导出**

### 2.5.2 Ludwig-3 极化坐标系

HFSS 实现的是 Arthur C. Ludwig 1973 年定义的第三类极化坐标系（IEEE TAP, vol. AP-21, pp. 116-119, 1973）：

$$
\begin{aligned}
E_x &= E_\theta \cos\phi - E_\phi \sin\phi \\
E_y &= E_\theta \sin\phi + E_\phi \cos\phi
\end{aligned}
$$

在 H2hRT 的表述中：
- Co-pol = 主极化分量（天线设计的极化方向）
- Cross-pol = 交叉极化分量（Ludwig-3 定义）

HFSS 的 Ludwig-3 默认 co-pol 参考方向为 X 轴。若天线极化沿 Y 轴（如垂直偶极子），需在 HFSS 中设置 Co-Polarization Reference 为 Y。

### 2.5.3 CSV 导出格式

**HFSS 导出步骤**：
1. Results → Create Far Field Report → Data Table
2. 选择：θ (deg), φ (deg), rEGain (dB), rETheta (Real, Imag), rEPhi (Real, Imag)
3. Export → CSV

**目标 CSV 格式**（H2hRT 兼容）：

```csv
theta_deg,phi_deg,gain_dBi,Etheta_re,Etheta_im,Ephi_re,Ephi_im
0,0,2.15,0.0,0.0,1.0,0.0
0,5,2.10,0.0,0.0,0.996,0.087
...
180,360,-40.0,0.0,0.0,0.001,0.0
```

**域值对照**：

| HFSS 导出量 | H2hRT CSV 列 | 备注 |
|---|---|---|
| `rEGain` (dB) | `gain_dBi` | HFSS 的 realized gain（含阻抗失配）→ 注意区分 IEEE gain 和 realized gain |
| `rETheta` (Real) | `Etheta_re` | 远场 E_θ 分量实部 |
| `rETheta` (Imag) | `Etheta_im` | 远场 E_θ 分量虚部 |
| `rEPhi` (Real) | `Ephi_re` | 远场 E_φ 分量实部 |
| `rEPhi` (Imag) | `Ephi_im` | 远场 E_φ 分量虚部 |

**⚠️ 增益类型选择**：
- `rEGain` (Realized Gain)：含阻抗失配损耗 = `Gain × (1 - |Γ|²)`
- `Gain` (IEEE Gain)：不含阻抗失配
- **推荐使用 Gain**（IEEE），因为 H2hRT 在 EM 计算中做了共轭匹配

### 2.5.4 FFD 格式替代方案

HFSS 支持 `.ffd` (Far Field Data) 格式导出，包含复 E 场分量，**不含几何模型信息**。可导入到其他 HFSS 项目或通过 PyAEDT Python API 转换为 CSV。

---

## 2.6 网格收敛性验证

### 2.6.1 收敛判据

| 判据 | 阈值 |
|---|---|
| |ΔS|max（S 参数最大变化） | < 0.005 |
| 增益变化（boresight） | < 0.1 dB |
| 输入阻抗变化 | < 2 Ω |
| 辐射效率变化 | < 0.5% |

### 2.6.2 操作步骤

1. 初始求解（自适应网格，Passes=15, ΔS=0.02）
2. 记录 boresight 增益 G₁ 和 S₁₁
3. 手动加密网格（Length-based refinement: 馈电区域 λ/20, 口径边缘 λ/10）
4. 重新求解 → 记录 G₂ 和 S₁₁
5. 若 |G₁ − G₂| > 0.1 dB → 继续加密 → 直到满足判据

---

## 2.7 仿真与理论解的交叉验证

### 2.7.1 偶极子验证

HFSS 仿真结果应与理论解对比：
- **2.4 GHz boresight 增益**：理论 2.15 dBi，仿真应取 2.15 ± 0.1 dBi
- **S₁₁**：理论谐振时 ≈ −20 dB（73Ω 匹配到 50Ω 的反射）
- **E 面方向图零点**：θ = 0° 和 180° 处 E_θ = 0

### 2.7.2 喇叭验证

HFSS 仿真结果应与孔径场法对比（主瓣 ±20° 内）：
- **12.5 GHz boresight 增益**：孔径场法估算 ~15 dBi，仿真应取 14.5–15.5 dBi
- **E 面 HPBW**：孔径场法 ~20°，仿真应在 ±2° 内一致
- 差异 > 1 dB → 检查网格收敛性、PML 设置、材料损耗参数

### 2.7.3 双锥验证

无限长解析解仅是定性参考。仿真的正确性检查应包括：
- |S₁₁| < −10 dB 在 2–18 GHz（若 |S₁₁| 持续 > −6 dB → 馈电模型或连接器建模有误）
- 水平面方向图全向性：φ 扫描时增益变化 < 0.5 dB
- 辐射效率 > 90%（PEC 模型）

---

## 2.8 数据管理

### 2.8.1 文件命名规范

```
{antenna_type}_{frequency}GHz_{source}.csv

例：
dipole_2.4GHz_hfss.csv
biconical_6GHz_hfss.csv
horn_12.5GHz_hfss.csv
```

### 2.8.2 元数据伴随文件

每个仿真导出的 CSV 应伴随一个 `_metadata.json`：

```json
{
  "antenna": "half-wave dipole",
  "frequency_Hz": 2400000000.0,
  "source": "HFSS 2024 R2 FEM",
  "mesh_passes": 12,
  "delta_S_max": 0.003,
  "boundary": "Radiation (λ/4 sphere)",
  "port_impedance_ohm": 73.0,
  "gain_type": "IEEE_Gain",
  "polarization_coord": "Ludwig-3 (co-pol = Y)",
  "theta_grid_deg": "0:5:180",
  "phi_grid_deg": "0:5:360",
  "date": "2026-07-01",
  "operator": "name"
}
```
